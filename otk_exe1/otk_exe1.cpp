/*
木盒子（Wood’Box)---Creo/OTK C++二次开发教程专用培训案例
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

	//初始化产品信息
	OTKApp::AppName = L"木盒子(Wood'Box)";   //产品名称
	OTKApp::Author = L"IceFai";				 //作者名
	OTKApp::AppID = L"WDB.";				 //唯一前缀
	OTKApp::DefMsgFile = "wdb_msg.txt";		 //缺省文本信息文件
	OTKApp::MenuFile = "wdb_menu.txt";	     //菜单文件

	OTKApp::Version = 1;					 //主版本号
	OTKApp::Release = 0;					 //发行号
	OTKApp::VersionTitle = L"1.0";			 //版本标题
	OTKApp::Description = L"木盒子（Wood'Box)Creo/OTK C++二次开发教程专用培训案例";  //产品描述

	try {

		pfcSession_ptr	session = pfcGetCurrentSessionWithCompatibility(pfcC4Compatible);
		wSession = wfcWSession::cast(session);

		ProUITranslationFilesEnable();

		//显示欢迎及版权信息。
		DisplayMessage(wSession, "Welcome");
		DisplayMessage(wSession, "Copyright");

		//载入带菜单定义文件
		try {
			wSession->RibbonDefinitionfileLoad("WoodBox.rbn");
		}
		OTK_EXCEPTION_HANDLE


		//创建同步功能命令，SyncModelListener是动作监听类，SyncAccessListener是访问状态监听类
		CommandFactory<SyncModelListener, SyncAccessListener>::CreateCommands(wSession, "OpenDrawing,CreateDrawing,BatchDrawing,SyncRename,SyncBackup,BatchRename");

		//创建关于信息命令及按钮
		CommandFactory<AboutListener, OTKAccessListener>::CreateCommands(wSession, "WoodBox");

		//创建IceFai作品序列按钮
		CommandFactory<OTKCommandListener, OTKAccessListener>::CreateCommands(wSession, "I_LETTER,C_LETTER,E_LETTER,F_LETTER,A_LETTER,IS_LETTER,Zuo,Pin", xfalse);


		//添加同步模型功能按钮到弹出菜单.		
		OTKPopupMenuListener *popListener = new OTKPopupMenuListener();
		//Part_SelNoSel_Cmd_Group: 零件图形区空白处右键菜单
		popListener->AddToPopupmenu("Part_SelNoSel_Cmd_Group", "OpenDrawing,CreateDrawing,SyncRename,SyncBackup", "SyncMenu");
		//Asm_SelNoSel_Cmd_Group:  组件图形区空白处右键菜单
		popListener->AddToPopupmenu("Asm_SelNoSel_Cmd_Group", "OpenDrawing,CreateDrawing,BatchDrawing,SyncRename,BatchRename,SyncBackup", "SyncMenu");
		//添加弹出菜单监听器
		wSession->AddActionListener(popListener);


		//添加系统模型改名动作的监听器，修改模型名称前先载入同名绘图
		wSession->AddActionListener(new SyncRenameSessionListener());
		//修改模型名称后再修改同名绘图并保存。
		wSession->AddActionListener(new SyncRenameModelListener());

	}

	OTK_EXCEPTION_HANDLE

	return 0;

}

extern "C" void user_terminate()
{



}



