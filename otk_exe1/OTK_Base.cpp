#include "stdafx.h"
#include "OTK_Base.h"
#include "tools.h"

const char *LOG_FILE = "ExLog.txt";

xstring OTKApp::AppName = L"My OTK App";
xstring OTKApp::Author = L"Anonymous";
xint OTKApp::Version = 1;
xint OTKApp::Release = 0;
xstring OTKApp::VersionTitle = L"1.0";
xstring OTKApp::Description = L"My App--OTK C++ Application";

xstring OTKApp::AppID = L"OTK.";
xstring OTKApp::DefMsgFile = L"otk_msg.txt";
xstring OTKApp::MenuFile = L"otk_menu.txt";

xbool OTKCommandListener::GetIsCommand(xstring cmdName)
{
	return cmdName == mCmdName;
}

xbool OTKAccessListener::GetIsCommand(xstring cmdName)
{
	return cmdName == mCmdName;
}

OTKPopupMenuListener::OTKPopupMenuListener() {
	mMenus = xstringsequence::create();
	mCmds = xstringsequence::create();
	mGroups = xstringsequence::create();
}

void OTKPopupMenuListener::AddToPopupmenu(xstring menuName, xstring cmdStr, xstring group) {
	mMenus->append(menuName);
	mCmds->append(cmdStr);
	mGroups->append(group);

}

void OTKPopupMenuListener::OnPopupmenuCreate(pfcPopupmenu_ptr Menu) {
	pfcSession_ptr sess = pfcGetProESession();
	wfcWSession_ptr wSess = wfcWSession::cast(sess);

	xstring appid = OTKApp::AppID;
	xstring menuFile = OTKApp::MenuFile;
	for (xint i = 0, len = mMenus->getarraysize(); i < len; i++) {
		try {

			xstring menuName = mMenus->get(i);
			if (menuName != Menu->GetName()) continue;

			xstring cmdStr = mCmds->get(i);
			xstring group = mGroups->get(i);
			pfcPopupmenu_ptr groupMenu;
			if (!group.IsNull()) {
				pfcPopupmenuOptions_ptr menuOpts = pfcPopupmenuOptions::Create(appid + group);
				menuOpts->SetLabel(GetMessage(wSess, group + ".TITLE", NULL, menuFile));
				menuOpts->SetHelptext(GetMessage(wSess, group + ".HELP", NULL, menuFile));
				groupMenu = Menu->AddMenu(menuOpts);
			}
			else {
				groupMenu = Menu;
			}

			vector<string> cv;
			SplitString((cStringT)cmdStr, cv, ",");
			for (size_t j = 0, size = cv.size(); j < size; j++) {
				xstring cmdName = xstring(cv[j].data());
				pfcUICommand_ptr cmd = sess->UIGetCommand(appid + cmdName);
				pfcPopupmenuOptions_ptr btnOpts = pfcPopupmenuOptions::Create(appid + cmdName + ".BTN");
				btnOpts->SetLabel(GetMessage(wSess, cmdName + ".TITLE", NULL, menuFile));
				btnOpts->SetHelptext(GetMessage(wSess, cmdName + ".HELP", NULL, menuFile));
				groupMenu->AddButton(cmd, btnOpts);
			}

			break;

		}

		OTK_EXCEPTION_HANDLE

	}

}

void DisplayMessage(wfcWSession_ptr sess, xstring format, xstringsequence_ptr texts, xstring msgFile) {

	try {
		sess->UIDisplayLocalizedMessage(msgFile, OTKApp::AppID + format, texts);
	}
	OTK_EXCEPTION_HANDLE

}

void DisplayTip(wfcWSession_ptr sess, xstring msg) {
	try {
		xstringsequence_ptr texts = xstringsequence::create();
		texts->append(msg);
		sess->UIDisplayLocalizedMessage(OTKApp::DefMsgFile, OTKApp::AppID + "TIP", texts);
	}
	OTK_EXCEPTION_HANDLE
}

xstring GetMessage(wfcWSession_ptr sess, xstring format, xstringsequence_ptr texts, xstring msgFile) {

	try {
		return sess->GetLocalizedMessageContents(msgFile, OTKApp::AppID + format, texts);
	}
	OTK_EXCEPTION_HANDLE
		return xstringnil;
}

inline void LogMsg(xstring msg, xbool pop) {

	ofstream logFile;
	logFile.open(LOG_FILE, ios::out);
	logFile << msg;
	logFile.close();
	if (pop)
	{
		try
		{
			wfcWSession_ptr wSess = wfcWSession::cast(pfcGetProESession());
			wSess->DisplayInformationWindow(LOG_FILE, 0.2, 0.2, 10, 30);
		}
		catch (xthrowable *x)
		{
		}
	}
}

void LogException(xthrowable_ptr x) {
	xstring errMsg = "Exception trail :\n";
	if (pfcXToolkitError::isObjKindOf(x))
	{
		pfcXToolkitError_ptr tx = pfcXToolkitError::cast(x);
		char ts[256];
		sprintf_s(ts, "Toolkit Func %s,Error Code: %d\n\n", const_cast<char *>((cStringT)tx->GetToolkitFunctionName()), tx->GetErrorCode());
		errMsg += ts;
		errMsg += "Method Name: " + tx->GetMethodName() + "\n";
	}
	errMsg += x->getCipTypeName(); LogMsg(errMsg, true);
}

