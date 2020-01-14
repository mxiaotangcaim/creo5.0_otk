#pragma once
#include<pfcSection.h>
#include <wfcSession.h>
#include <set>
#include <nopkdefs_pfc.h>

/*
1.OPenDrawing:打开同名绘图
2.CreateDrawing：创建同名绘图
3.BatchDrawing：批量创建组件同名绘图
4.Rename：改名
5.Backup：备份
6.BatchRename：批量改名
...
*/
class SyncModel
{
public:
	SyncModel();
	SyncModel(pfcModel_ptr model);//指定实体模型的构造函数

	void SetModel(pfcModel_ptr model);
	pfcModel_ptr GetSolid() { return mSolid; }
	pfcModel_ptr GetDrawing() { return mDrawing; }
	pfcModel_ptr OpenDrawing();
	pfcModel_ptr CreateDrawing(xbool display = xtrue);
	void BatchDrawing(pfcModel_ptr asmModel = NULL);

	xbool RenameTo(xstring toName);
	xbool Rename();
	void BatchRename(xstring prefix);

	xbool BackupTo(xstring toPath);
	xbool Backup();


private:
	
	wfcWSession_ptr mSess = NULL;
	pfcModel_ptr mSolid = NULL;
	pfcModel_ptr mDrawing = NULL;
	wfcParsedFileNameData_ptr mNameData = NULL;


};

/*
同步模型功能的动作监听器
@cmdName: 命令名称
*/
class SyncModelListener:public virtual OTKCommandListener
{
public:
	SyncModelListener(xstring cmdName) :OTKCommandListener(cmdName) {}
	void OnCommand();
   
};

/*
同步模型功能的命令访问状态监听器
@cmdName: 命令名称
*/

class SyncAccessListener:public virtual OTKAccessListener
{
public:
	SyncAccessListener(xstring cmdName):OTKAccessListener(cmdName){}
	pfcCommandAccess OnCommandAccess(xbool AllowErrorMessages);

};

/*
同步模型组件的元件遍历客户端
*/

class SyncModelVisitor:public wfcDefaultVisitingClient
{
public:
	SyncModelVisitor() {

		mSess = wfcWSession::cast(pfcGetProESession());
	}

	wfcStatus ApplyAction(pfcObject_ptr pfc_object, wfcStatus filter_status);

private:
	wfcWSession_ptr mSess;
};


/*
Function: 监听系统模型改名动作，在模型改名前载入同名绘图（如果有的话）
*/
class SyncRenameSessionListener : public virtual pfcDefaultSessionActionListener
{
public:
	void OnBeforeModelRename(pfcDescriptorContainer2_ptr Container);
};

/*
Function: 监听系统模型改名后动作，在模型改名后修改同名绘图名称并保存
*/
class SyncRenameModelListener : public virtual pfcDefaultModelEventActionListener
{
public:
	void OnAfterModelRename(pfcModelDescriptor_ptr FromMdl, pfcModelDescriptor_ptr ToMdl);
};


