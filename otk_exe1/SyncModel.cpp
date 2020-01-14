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
 Function: 设置同步模型中的实体模型，如果模型已经存在同名的绘图，则自动载入会话为下一步动作做好准备。
	@model: 实体模型（可以是零件或组件）
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
Function: 打开同步模型的同名绘图并返回绘图对象
Return: 同步模型中的绘图对象。
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
Function: 创建同步模型中实体模型的同名绘图
@display: 创建好的绘图是否打开绘图模型窗口显示出来，单独创建设为xtrue,批量模式下设为xfalse.
Return: 返回创建的绘图对象
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
Function: 批量创建组件下所有元件的同名绘图
@asmModel:要批量创建的组件对象。
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
Function: 同步修改同步模型中实体模型和同名绘图的名称为新名称
@toName: 新名称
Return: 修改成功则返回xtrue否则xfalse.
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
Function: 同步修改同步模型的名称，需要用户输入新名称
Return: 修改成功则返回xtrue,否则返回xfalse
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
Funciton: 同步模型的批量添加名字前缀
@prefix:要添加的文件前缀
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

	//获取组件下所有元件的文件名集合（无重复）
	set<string> mfs;

	ExtractModelFiles(pfcAssembly::cast(mSolid), mfs);

	//遍历所有元件文件名并逐一进行改名
	SyncModel  *sm = new SyncModel();
	for (set<string>::iterator it = mfs.begin(); it != mfs.end(); ++it) {
		xstring modelFile = (*it).data();
		sm->SetModel(mSess->GetModelFromFileName(modelFile));
		xstring toName = prefix + "-" + sm->GetSolid()->GetInstanceName();
		sm->RenameTo(toName);
	}
	delete sm;

	//最后记得保存总装配避免下次打开失败
	mSolid->Save();
	
}
/*
Function: 备份同步模型（模型及同名绘图）到指定目录
@toPath: 备份的目标目录路径
Return: xtrue则备份成功，xfalse则失败
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
Function: 同步模型的同步备份功能
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
Function: 同步组件的元件遍历客户端应用动作函数。
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
Function: 同步模型的动作监听器响应代码，根据命令名称调用对应的操作函数。
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
Function: 同步功能的命令访问状态监听器
*/
pfcCommandAccess SyncAccessListener::OnCommandAccess(xbool AllowErrorMessages)
{
	try {

		pfcSession_ptr sess = pfcGetProESession();
		pfcModel_ptr model = sess->GetCurrentModel();
		if (NULL == model) return pfcACCESS_UNAVAILABLE;

		pfcModelType modelType = model->GetType();
		if (pfcMDL_PART != modelType && pfcMDL_ASSEMBLY != modelType) return pfcACCESS_UNAVAILABLE;

		//零件状态下批量功能不可访问
		if (pfcMDL_PART == modelType && (GetIsCommand("BatchRename") || GetIsCommand("BatchDrawing"))) return pfcACCESS_UNAVAILABLE;

		return pfcACCESS_AVAILABLE;
	}

	OTK_EXCEPTION_HANDLE

	return pfcACCESS_UNAVAILABLE;
}

/*
Function: 实现模型改名动作监听器的OnBeforeModelRename方法，在模型改名前先载入同名绘图
@Container: 包含改名前后的两个模型描述器的容器。
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
Function: 实现模型改名动作监听器的OnAfterModelRename方法，在模型改名后同步修改同名绘图名称并保存。
@FromMdl: 改名前的模型描述器
@ToMdl: 改名后的模型描述器
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
