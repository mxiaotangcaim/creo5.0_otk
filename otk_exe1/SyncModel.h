#pragma once
#include<pfcSection.h>
#include <wfcSession.h>
#include <set>
#include <nopkdefs_pfc.h>

/*
1.OPenDrawing:��ͬ����ͼ
2.CreateDrawing������ͬ����ͼ
3.BatchDrawing�������������ͬ����ͼ
4.Rename������
5.Backup������
6.BatchRename����������
...
*/
class SyncModel
{
public:
	SyncModel();
	SyncModel(pfcModel_ptr model);//ָ��ʵ��ģ�͵Ĺ��캯��

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
ͬ��ģ�͹��ܵĶ���������
@cmdName: ��������
*/
class SyncModelListener:public virtual OTKCommandListener
{
public:
	SyncModelListener(xstring cmdName) :OTKCommandListener(cmdName) {}
	void OnCommand();
   
};

/*
ͬ��ģ�͹��ܵ��������״̬������
@cmdName: ��������
*/

class SyncAccessListener:public virtual OTKAccessListener
{
public:
	SyncAccessListener(xstring cmdName):OTKAccessListener(cmdName){}
	pfcCommandAccess OnCommandAccess(xbool AllowErrorMessages);

};

/*
ͬ��ģ�������Ԫ�������ͻ���
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
Function: ����ϵͳģ�͸�����������ģ�͸���ǰ����ͬ����ͼ������еĻ���
*/
class SyncRenameSessionListener : public virtual pfcDefaultSessionActionListener
{
public:
	void OnBeforeModelRename(pfcDescriptorContainer2_ptr Container);
};

/*
Function: ����ϵͳģ�͸�����������ģ�͸������޸�ͬ����ͼ���Ʋ�����
*/
class SyncRenameModelListener : public virtual pfcDefaultModelEventActionListener
{
public:
	void OnAfterModelRename(pfcModelDescriptor_ptr FromMdl, pfcModelDescriptor_ptr ToMdl);
};


