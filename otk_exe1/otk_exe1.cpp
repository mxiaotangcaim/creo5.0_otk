/*
ľ���ӣ�Wood��Box)---Creo/OTK C++���ο����̳�ר����ѵ����
*/

#include "stdafx.h"
#include "SyncModel.h"
#include "AboutMe.h"
#include <ProUI.h>

extern "C" int user_initialize(
	int argc,
	char *argv[],
	char *version,
	char *build,
	wchar_t errbuf[80])
{

	wfcWSession_ptr wSession;

	//��ʼ����Ʒ��Ϣ
	OTKApp::AppName = L"ľ����(Wood'Box)";   //��Ʒ����
	OTKApp::Author = L"IceFai";				 //������
	OTKApp::AppID = L"WDB.";				 //Ψһǰ׺
	OTKApp::DefMsgFile = "wdb_msg.txt";		 //ȱʡ�ı���Ϣ�ļ�
	OTKApp::MenuFile = "wdb_menu.txt";	     //�˵��ļ�

	OTKApp::Version = 1;					 //���汾��
	OTKApp::Release = 0;					 //���к�
	OTKApp::VersionTitle = L"1.0";			 //�汾����
	OTKApp::Description = L"ľ���ӣ�Wood'Box)Creo/OTK C++���ο����̳�ר����ѵ����";  //��Ʒ����

	try {

		pfcSession_ptr	session = pfcGetCurrentSessionWithCompatibility(pfcC4Compatible);
		wSession = wfcWSession::cast(session);

		ProUITranslationFilesEnable();

		//��ʾ��ӭ����Ȩ��Ϣ��
		DisplayMessage(wSession, "Welcome");
		DisplayMessage(wSession, "Copyright");

		//������˵������ļ�
		try {
			wSession->RibbonDefinitionfileLoad("WoodBox.rbn");
		}
		OTK_EXCEPTION_HANDLE


		//����ͬ���������SyncModelListener�Ƕ��������࣬SyncAccessListener�Ƿ���״̬������
		CommandFactory<SyncModelListener, SyncAccessListener>::CreateCommands(wSession, "OpenDrawing,CreateDrawing,BatchDrawing,SyncRename,SyncBackup,BatchRename");

		//����������Ϣ�����ť
		CommandFactory<AboutListener, OTKAccessListener>::CreateCommands(wSession, "WoodBox");

		//����IceFai��Ʒ���а�ť
		CommandFactory<OTKCommandListener, OTKAccessListener>::CreateCommands(wSession, "I_LETTER,C_LETTER,E_LETTER,F_LETTER,A_LETTER,IS_LETTER,Zuo,Pin", xfalse);


		//���ͬ��ģ�͹��ܰ�ť�������˵�.		
		OTKPopupMenuListener *popListener = new OTKPopupMenuListener();
		//Part_SelNoSel_Cmd_Group: ���ͼ�����հ״��Ҽ��˵�
		popListener->AddToPopupmenu("Part_SelNoSel_Cmd_Group", "OpenDrawing,CreateDrawing,SyncRename,SyncBackup", "SyncMenu");
		//Asm_SelNoSel_Cmd_Group:  ���ͼ�����հ״��Ҽ��˵�
		popListener->AddToPopupmenu("Asm_SelNoSel_Cmd_Group", "OpenDrawing,CreateDrawing,BatchDrawing,SyncRename,BatchRename,SyncBackup", "SyncMenu");
		//��ӵ����˵�������
		wSession->AddActionListener(popListener);


		//���ϵͳģ�͸��������ļ��������޸�ģ������ǰ������ͬ����ͼ
		wSession->AddActionListener(new SyncRenameSessionListener());
		//�޸�ģ�����ƺ����޸�ͬ����ͼ�����档
		wSession->AddActionListener(new SyncRenameModelListener());

	}

	OTK_EXCEPTION_HANDLE

	return 0;

}

extern "C" void user_terminate()
{



}



