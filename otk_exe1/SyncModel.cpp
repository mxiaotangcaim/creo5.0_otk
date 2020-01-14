#include "stdafx.h"
#include "SyncModel.h"

SyncModel::SyncModel()
{
	mSess = wfcWSession::cast(pfcGetProESession());
}

SyncModel::SyncModel(pfcModel_ptr model)
{
	mSess = wfcWSession::cast(pfcGetProESession());
	SetModel(model);
}
/*
 Function: ����ͬ��ģ���е�ʵ��ģ�ͣ����ģ���Ѿ�����ͬ���Ļ�ͼ�����Զ�����ỰΪ��һ����������׼����
	@model: ʵ��ģ�ͣ�����������������
*/
void SyncModel::SetModel(pfcModel_ptr model)
{
	mSolid = NULL;
	mDrawing = NULL;

	try
	{
		if (NULL == model) model = mSess->GetActiveModel();
		if (NULL == model) return;

		pfcModelType mtype = model->GetType();
		if (mtype != pfcMDL_PART && mtype != pfcMDL_ASSEMBLY) return;
		mSolid = model;

		pfcModelDescriptor_ptr mdesc = mSolid->GetDescr();

		xstring fullPath = mdesc->GetDevice() + ":" + mdesc->GetPath() + GetFileNameWithVersion(model);

		mNameData = mSess->ParseFileName((cStringT)fullPath);

		xstring drwName = mNameData->GetName();
		pfcModel_ptr drwModel = mSess->GetModel(drwName, pfcMDL_DRAWING);

		if (drwModel==NULL)
		{
			pfcModelDescriptor_ptr drwDesc = pfcModelDescriptor::Create(pfcMDL_DRAWING, drwName, NULL);
			drwDesc->SetPath(mdesc->GetPath());
			drwDesc->SetDevice(mdesc->GetDevice());
			pfcRetrieveModelOptions_ptr opts = pfcRetrieveModelOptions::Create();
			opts->SetAskUserAboutReps(false);

			drwModel = mSess->RetrieveModelWithOpts(drwDesc, opts);

		}

		if (drwModel != NULL)  mDrawing = drwModel;

		
	}

	OTK_EXCEPTION_HANDLE

}
/*
Function: ��ͬ��ģ�͵�ͬ����ͼ�����ػ�ͼ����
Return: ͬ��ģ���еĻ�ͼ����
*/
pfcModel_ptr SyncModel::OpenDrawing()
{
	try
	{
		if (NULL == mDrawing)
		{
			mSess->UIShowMessageDialog(GetMessage(mSess, "DrawingNotExist"), NULL);
			return NULL;
		}

		pfcWindow_ptr win = mSess->GetModelWindow(pfcModel::cast(mDrawing));

		if (NULL == win)
		{
			win = mSess->CreateModelWindow(mDrawing);
			mDrawing->Display();
		}
		win->Activate();
		win->Repaint();
	}

	OTK_EXCEPTION_HANDLE

	return mDrawing;
}
/*
Function: ����ͬ��ģ����ʵ��ģ�͵�ͬ����ͼ
@display: �����õĻ�ͼ�Ƿ�򿪻�ͼģ�ʹ�����ʾ����������������Ϊxtrue,����ģʽ����Ϊxfalse.
Return: ���ش����Ļ�ͼ����
*/
pfcModel_ptr SyncModel::CreateDrawing(xbool display)
{
	try
	{
	   if (mDrawing!=NULL)
	   {
		   if (display) OpenDrawing();
		   return mDrawing;
	   }

	   xstring dreTplFile = GetFirstOption(mSess, "template_drawing");

	   wfcParsedFileNameData_ptr fdata = mSess->ParseFileName(dreTplFile);

	   xstring orgStartDir = GetFirstOption(mSess, "start_model_dir");
	   mSess->SetConfigOption("start_model_dir", fdata->GetDirectoryPath());

	   pfcDrawingCreateOptions_ptr createOpts = pfcDrawingCreateOptions::create();
	   if (display)createOpts->append(pfcDRAWINGCREATE_DISPLAY_DRAWING);
	
	   mDrawing = mSess->CreateDrawingFromTemplate(mNameData->GetName(), fdata->GetName() + ".drw", mSolid->GetDescr(), createOpts);

	   mDrawing->Save();

	   if (!display) mDrawing->Erase();
       
	   mSess->SetConfigOption("start_model_dir", orgStartDir);

	}

    OTK_EXCEPTION_HANDLE
	
	
	return mDrawing;
}
/*
Function: �����������������Ԫ����ͬ����ͼ
@asmModel:Ҫ�����������������
*/
void SyncModel::BatchDrawing(pfcModel_ptr asmModel)
{
	try
	{
		if (asmModel == NULL) asmModel = mSolid;

		if (asmModel->GetType() != pfcMDL_ASSEMBLY)return;

		if (!ConfirmAction(mSess, GetMessage(mSess, "ConfigBatchDrw"))) return;

		set<string> mfs;		
		ExtractModelFiles(pfcAssembly::cast(asmModel), mfs);

		SyncModel *sm = new SyncModel();
		for (set<string>::iterator it = mfs.begin(); it != mfs.end();it++)
		{
			sm->SetModel(mSess->GetModelFromFileName((*it).data()));
			sm->CreateDrawing(xfalse);
		}
		
		delete sm;


	}

	
	OTK_EXCEPTION_HANDLE
}
/*
Function: ͬ���޸�ͬ��ģ����ʵ��ģ�ͺ�ͬ����ͼ������Ϊ������
@toName: ������
Return: �޸ĳɹ��򷵻�xtrue����xfalse.
*/
xbool SyncModel::RenameTo(xstring toName)
{
	if (toName.IsNull())return xfalse;
	try
	{
		mSolid->Rename(toName);
		if (mDrawing!=NULL)
		{
			mDrawing->Rename(toName);
			mDrawing->Save();
		}
		mSolid->Save();
		return xtrue;
	}
   
	OTK_EXCEPTION_HANDLE
	
	return xfalse;
}
/*
Function: ͬ���޸�ͬ��ģ�͵����ƣ���Ҫ�û�����������
Return: �޸ĳɹ��򷵻�xtrue,���򷵻�xfalse
*/
xbool SyncModel::Rename()
{   
	xstring toName = xstringnil;
	try
	{
		if (mSolid == NULL) SetModel(NULL);
		DisplayMessage(mSess, "ToName");
		toName = mSess->UIReadStringMessage(false);
		RenameTo(toName);
		return xtrue;
	}

    OTK_EXCEPTION_HANDLE

	return xfalse;
}
/*
Funciton: ͬ��ģ�͵������������ǰ׺
@prefix:Ҫ��ӵ��ļ�ǰ׺
*/
void SyncModel::BatchRename(xstring prefix)
{

	if (mSolid->GetType() != pfcMDL_ASSEMBLY) return;

	if (!ConfirmAction(mSess, GetMessage(mSess, "ConfirmBatchRename"))) return;

	if (prefix.IsNull()) {
		try {
			DisplayMessage(mSess, "Prefix");
			prefix = mSess->UIReadStringMessage(false);
		}
		catch (xthrowable *x) {
			delete x;
			return;
		}
	}

	//��ȡ���������Ԫ�����ļ������ϣ����ظ���
	set<string> mfs;

	ExtractModelFiles(pfcAssembly::cast(mSolid), mfs);

	//��������Ԫ���ļ�������һ���и���
	SyncModel  *sm = new SyncModel();
	for (set<string>::iterator it = mfs.begin(); it != mfs.end(); ++it) {
		xstring modelFile = (*it).data();
		sm->SetModel(mSess->GetModelFromFileName(modelFile));
		xstring toName = prefix + "-" + sm->GetSolid()->GetInstanceName();
		sm->RenameTo(toName);
	}
	delete sm;

	//���ǵñ�����װ������´δ�ʧ��
	mSolid->Save();
	
}
/*
Function: ����ͬ��ģ�ͣ�ģ�ͼ�ͬ����ͼ����ָ��Ŀ¼
@toPath: ���ݵ�Ŀ��Ŀ¼·��
Return: xtrue�򱸷ݳɹ���xfalse��ʧ��
*/
xbool SyncModel::BackupTo(xstring toPath)
{
	if (toPath.IsNull()) return xfalse;
	try
	{
		xstring solidFile = GetFileNameWithVersion(mSolid);
		CopyFile(mNameData->GetDirectoryPath() + solidFile, toPath + solidFile, true);
		if (mDrawing!=NULL)
		{
			xstring drwFile = GetFileNameWithVersion(mDrawing);
			CopyFile(mNameData->GetDirectoryPath() + drwFile, toPath + drwFile, true);
		}
		return xtrue;
	}
   
	OTK_EXCEPTION_HANDLE

	return xbool();
}/*
Function: ͬ��ģ�͵�ͬ�����ݹ���
*/
xbool SyncModel::Backup()
{
	xstring toPath = xstringnil;
	try
	{
		DisplayMessage(mSess, "SelectFolder");
		pfcDirectorySelectionOptions_ptr dirOpts = pfcDirectorySelectionOptions::Create();
		dirOpts->SetDefaultPath(mSess->GetCurrentDirectoryW());
		dirOpts->SetDialogLabel("Select a folder");
		toPath = mSess->UISelectDirectory(dirOpts);
		if (mSolid->GetType()==pfcMDL_ASSEMBLY)
		{
			set<string> mfs;
			ExtractModelFiles(pfcAssembly::cast(mSolid), mfs);
			SyncModel *sm = new SyncModel();
			for (set<string>::iterator it = mfs.begin(); it != mfs.end(); ++it) {
				sm->SetModel(mSess->GetModelFromFileName((*it).data()));
				sm->BackupTo(toPath);
			}
			delete sm;
		}
		else
		{
			BackupTo(toPath);
		}

		DisplayMessage(mSess, "DoneBackup");

		return xtrue;
	}

	OTK_EXCEPTION_HANDLE

		return xfalse;

	return xbool();
}

/*
Function: ͬ�������Ԫ�������ͻ���Ӧ�ö���������
*/
wfcStatus SyncModelVisitor::ApplyAction(pfcObject_ptr pfc_object, wfcStatus filter_status)
{
	if (wfcTK_NO_ERROR == filter_status)
	{
		try
		{
			wfcWComponentFeat_ptr comFeat = wfcWComponentFeat::cast(pfc_object);
			pfcModelDescriptor_ptr descr = comFeat->GetModelDescr();
			pfcModel_ptr model = mSess->GetModelFromDescr(descr);
			SyncModel *sm = new SyncModel(model);
			sm->CreateDrawing(xfalse);
			delete sm;
		}
		OTK_EXCEPTION_HANDLE

			return	wfcTK_NO_ERROR;
	}

	else
	{
		return filter_status;
	}

}

/*
Function: ͬ��ģ�͵Ķ�����������Ӧ���룬�����������Ƶ��ö�Ӧ�Ĳ���������
*/
void SyncModelListener::OnCommand()
{
	SyncModel *syncModel = NULL;

	try {

		pfcModel_ptr curModel = NULL;
		pfcSession_ptr sess = pfcGetProESession();
		pfcSelectionBuffer_ptr selBuff = sess->GetCurrentSelectionBuffer();
		pfcSelections_ptr sels = selBuff->GetContents();
		if (sels != NULL && sels->getarraysize() == 1) {
			pfcSelection_ptr sel = sels->get(0);
			curModel = sel->GetSelModel();
			if (curModel != NULL && (curModel->GetType() != pfcMDL_ASSEMBLY && curModel->GetType() != pfcMDL_PART)) {
				curModel = NULL;
			}
		}

		syncModel = new SyncModel(curModel);

		if (GetIsCommand("OpenDrawing")) {
			syncModel->OpenDrawing();
		}
		else if (GetIsCommand("CreateDrawing")) {
			syncModel->CreateDrawing();
		}
		else if (GetIsCommand("BatchDrawing")) {
			syncModel->BatchDrawing();
		}
		else if (GetIsCommand("SyncRename")) {
			syncModel->Rename();
		}
		else if (GetIsCommand("SyncBackup")) {
			syncModel->Backup();
		}
		else if (GetIsCommand("BatchRename")) {
			syncModel->BatchRename(xstringnil);
		}
	}
	OTK_EXCEPTION_HANDLE

	if (NULL != syncModel) delete syncModel;

}
/*
Function: ͬ�����ܵ��������״̬������
*/
pfcCommandAccess SyncAccessListener::OnCommandAccess(xbool AllowErrorMessages)
{
	try {

		pfcSession_ptr sess = pfcGetProESession();
		pfcModel_ptr model = sess->GetCurrentModel();
		if (NULL == model) return pfcACCESS_UNAVAILABLE;

		pfcModelType modelType = model->GetType();
		if (pfcMDL_PART != modelType && pfcMDL_ASSEMBLY != modelType) return pfcACCESS_UNAVAILABLE;

		//���״̬���������ܲ��ɷ���
		if (pfcMDL_PART == modelType && (GetIsCommand("BatchRename") || GetIsCommand("BatchDrawing"))) return pfcACCESS_UNAVAILABLE;

		return pfcACCESS_AVAILABLE;
	}

	OTK_EXCEPTION_HANDLE

	return pfcACCESS_UNAVAILABLE;
}

/*
Function: ʵ��ģ�͸���������������OnBeforeModelRename��������ģ�͸���ǰ������ͬ����ͼ
@Container: ��������ǰ�������ģ����������������
*/
void SyncRenameSessionListener::OnBeforeModelRename(pfcDescriptorContainer2_ptr Container)
{
	try {

		pfcSession_ptr session = pfcGetCurrentSession();
		pfcModel_ptr curModel = session->GetActiveModel();
		if (curModel->GetType() != pfcMDL_PART && curModel->GetType() != pfcMDL_ASSEMBLY) return;
		pfcModelDescriptor_ptr drwDesc = pfcModelDescriptor::Create(pfcMDL_DRAWING, curModel->GetInstanceName(), NULL);
		pfcModel_ptr drwModel = session->RetrieveModel(drwDesc);

	}
	OTK_EXCEPTION_HANDLE

}
/*
Function: ʵ��ģ�͸���������������OnAfterModelRename��������ģ�͸�����ͬ���޸�ͬ����ͼ���Ʋ����档
@FromMdl: ����ǰ��ģ��������
@ToMdl: �������ģ��������
*/
void SyncRenameModelListener::OnAfterModelRename(pfcModelDescriptor_ptr FromMdl, pfcModelDescriptor_ptr ToMdl)
{
	try {

		pfcSession_ptr session = pfcGetCurrentSession();
		if (FromMdl->GetType() != pfcMDL_PART && FromMdl->GetType() != pfcMDL_ASSEMBLY) return;
		pfcModelDescriptor_ptr drwDesc = pfcModelDescriptor::Create(pfcMDL_DRAWING, FromMdl->GetInstanceName(), NULL);
		pfcModel_ptr drw = session->GetModelFromDescr(drwDesc);
		if (drw != NULL) {
			drw->Rename(ToMdl->GetInstanceName(), xtrue);
			drw->Save();
		}

	}
	OTK_EXCEPTION_HANDLE
}
