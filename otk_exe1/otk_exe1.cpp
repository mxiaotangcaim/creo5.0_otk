// otk_exe1.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

pfcDrawing_ptr OpenDrawingOfModel(pfcModel_ptr model);

class Ex1CommmandListener :public virtual pfcUICommandActionListener {
public:

	void OnCommand();

};

class EX1AcessListense :public virtual pfcUICommandAccessListener {

public:
	pfcCommandAccess OnCommandAccess(xbool AllowErrorMessages);

};

class EX1PopmenuListense:public virtual pfcPopupmenuListener
{
public:
    
	void OnPopupmenuCreate(pfcPopupmenu_ptr Menu);

};

extern "C" int user_initialize(int argc,char *argv[],char *version,wchar_t *errbuf[80])
{
	pfcSession_ptr Session = pfcGetCurrentSessionWithCompatibility(pfcC4Compatible);

	wfcWSession_ptr wSession = wfcWSession::cast(Session);

	try{
		pfcUICommand_ptr cmdDrw = wSession->UICreateCommand("Ex1.Named_Drawing", new Ex1CommmandListener());
		cmdDrw->Designate("message.txt", "EX1.Named_Drawing.TITLE", "EX1.Named_Drawing.HELP", "EX1.Named_Drawing.DESC");
		cmdDrw->AddActionListener(new EX1AcessListense());
		cmdDrw->SetIcon("Named_Drawing_large2");
		wSession->AddActionListener(new EX1PopmenuListense());
	}
	xcatchbegin
	    xcatchcip(x) {
		LogException(x);
	}
	xcatchend

	return 0;


}
//菜单终止时调用的函数，程序清理工作
extern  "C" void user_terminate()
{
	
}


pfcCommandAccess EX1AcessListense::OnCommandAccess(xbool AllowErrorMessages)
{
	pfcSession_ptr Sess = pfcGetProESession();

	pfcModel_ptr model = Sess->GetCurrentModel();

	if (NULL==model)
	{
		return pfcACCESS_INVISIBLE;
	}
	pfcModelType modeltype = model->GetType();

	if (pfcMDL_PART!=modeltype &&pfcMDL_ASSEMBLY!=modeltype)
	{
		return pfcACCESS_INVISIBLE;
	}

	return pfcACCESS_AVAILABLE;

}

void EX1PopmenuListense::OnPopupmenuCreate(pfcPopupmenu_ptr Menu)
{
	pfcSession_ptr Sess = pfcGetProESession();

	xstring menuName = Menu->GetName();

	if (menuName!="Part_SelNoSel_Cmd_Group" && menuName!="Asm_SelNoSel_Cmd_Group")
	{
		return;
	}

	pfcUICommand_ptr cmd = Sess->UIGetCommand("Ex1.Named_Drawing");

	pfcPopupmenuOptions_ptr menunOpts = pfcPopupmenuOptions::Create("Ex1.PopoupMenu.MENU");
	menunOpts->SetLabel(Sess->GetLocalizedMessageContents("message.txt", "EX1.PopupMenu.TITLE", NULL));
	menunOpts->SetHelptext(Sess->GetLocalizedMessageContents("message.txt", "EX1.PopupMenu.HELP", NULL));

	pfcPopupmenu_ptr popmenu=Menu->AddMenu(menunOpts);


	pfcPopupmenuOptions_ptr btnOpts = pfcPopupmenuOptions::Create("Ex1.Named_Drawing.BTN");

	btnOpts->SetLabel(Sess->GetLocalizedMessageContents("message.txt", "EX1.Named_Drawing.TITLE", NULL));

	btnOpts->SetHelptext(Sess->GetLocalizedMessageContents("message.txt", "EX1.Named_Drawing.HELP", NULL));

	popmenu->AddButton(cmd, btnOpts);

}


pfcDrawing_ptr OpenDrawingOfModel(pfcModel_ptr model) throw(xthrowable)
{
	pfcSession_ptr Sess = pfcSession::cast(model->GetDBParent());

	xstring fileName = model->GetFileName();
	xstring instName = fileName.Substring(0, fileName.GetLength() - 4);
	pfcModelDescriptor_ptr drwDesc = pfcModelDescriptor::CreateFromFileName(instName+".drw");
	pfcModel_ptr drwModel;
	try
	{
		drwModel = Sess->RetrieveModel(drwDesc);
	}
    xcatchbegin
		xcatch(pfcXToolkitError, x) {
		//drwModel=CreateDrawingOfModel(model);
		Sess->UIShowMessageDialog("Now Create Drawing", NULL);
		return NULL;
	}
	xcatchend
	drwModel = Sess->RetrieveModel(drwDesc);
	pfcWindow_ptr win = Sess->CreateModelWindow(drwModel);
	drwModel->Display();
	win->Activate();

	return pfcDrawing::cast(drwModel);


}

void Ex1CommmandListener::OnCommand()
{	
		pfcSession_ptr Sess = pfcGetProESession();
		wfcWSession_ptr wSess = wfcWSession::cast(Sess);

		//wSess->UIShowMessageDialog("Got it !", NULL);
		pfcModel_ptr model = wSess->GetCurrentModel();
		if (NULL == model) return;
		if (pfcMDL_PART != model->GetType() && pfcMDL_ASSEMBLY != model->GetType()) return;
		OpenDrawingOfModel(model);
	
	
}


