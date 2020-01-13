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

xbool SyncModel::BackupTo(xstring toPath)
{
	return xbool();
}

xbool SyncModel::Backup()
{
	return xbool();
}
