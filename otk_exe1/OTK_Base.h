#pragma once

#include <vector>

//异常处理的宏
#define OTK_EXCEPTION_HANDLE \
	xcatchbegin \
		xcatchcip(x) {\
			LogException(x); \
		}\
	xcatchend 

/*
应用信息类，描述和定义应用的全局信息和变量
*/
class OTKApp {
public:
	static xstring AppName;     //应用名称 
	static xstring Author;      //作者
	static xint Version;        //版本号
	static xint Release;        // 发行号
	static xstring VersionTitle; //版本标题
	static xstring Description;  //应用描述
	static xstring AppURL;       //应用网址
	static xstring AppLogo;      //应用LOGO

	static xstring AppID;        //应用标识
	static xstring DefMsgFile;  //缺省信息文件
	static xstring MenuFile;     //菜单文件
};

/*
Function: 缺省的命令监听器，构造函数需提供命令名称。通过命令名称确定当前调用的命令
*/
class OTKCommandListener :
	public virtual pfcUICommandActionListener

{
public:
	virtual void OnCommand() {}
	OTKCommandListener(xstring cmdName) {
		mCmdName = cmdName;
	}
	xbool GetIsCommand(xstring cmdName);     //通过命令名称确定当前调用的命令

private:
	xstring mCmdName;

};

/*
Function: 缺省命令访问状态监听器，构造函数需提供命令名称。通过命令名称确定当前的命令
*/
class OTKAccessListener :
	public virtual pfcUICommandAccessListener

{
public:
	OTKAccessListener(xstring cmdName) { mCmdName = cmdName; }

	virtual pfcCommandAccess OnCommandAccess(xbool AllowErrorMessages) { return pfcACCESS_AVAILABLE; }
	xbool GetIsCommand(xstring cmdName);

private:
	xstring mCmdName;

};

/*
Function: 弹出菜单监听器。用于添加命令到指定的弹出菜单
*/
class OTKPopupMenuListener :public virtual pfcPopupmenuListener {
public:
	OTKPopupMenuListener();
	void OnPopupmenuCreate(pfcPopupmenu_ptr Menu);
	void AddToPopupmenu(xstring menuName, xstring cmdStr, xstring group = xstringnil);
private:
	xstringsequence_ptr mMenus;
	xstringsequence_ptr mCmds;
	xstringsequence_ptr mGroups;
};

/*
Function: 模拟工厂模式，用于批量创建命令
@TL: 命令的动作监听类，必须从OTKCommandListener派生
@TC: 命令的访问状态监听类，必须从OTKAccessListener派生
*/
template<typename TL, typename TC>
class CommandFactory
{
public:
	/*
	Function: 创建一个命令
	@sess: 会话对象
	@cmdName: 命令名称，无需带上唯一前缀和后缀
	@hasDetail: 信息文件中是否有帮助和描述文本
	*/
	static pfcUICommand_ptr CreateCommand(wfcWSession_ptr sess, xstring cmdName, xbool hasDetail = xtrue)

	{
		pfcUICommand_ptr cmd;
		try {
			xstring cmdLabel = xstring(OTKApp::AppID);
			cmdLabel += cmdName;
			cmd = sess->UICreateCommand(cmdLabel, new TL(cmdName));

			//设置命令图标必须单独捕捉异常以免造成命令创建失败。
			try {
				cmd->SetIcon(cmdName + ".png");
			}
			catch (xthrowable *x) {
				delete x;
			}

			//如果信息文件中有帮助文本和描述文本，则使用详细模式创建命令。
			if (hasDetail) {
				cmd->Designate(OTKApp::MenuFile, cmdLabel + ".TITLE", cmdLabel + ".HELP", cmdLabel + ".DESC");
			}
			else {
				cmd->Designate(OTKApp::MenuFile, cmdLabel + ".TITLE", xstringnil, xstringnil);
			}

			cmd->AddActionListener(new TC(cmdName));
		}
		OTK_EXCEPTION_HANDLE

			return cmd;
	}

	/*
	Function: 批量创建命令
	@sess: 会话对象
	@cmdStr: 多个命令名称组成的字符串，命令名以半角逗号,隔开
	@hasDetail: 信息文件中是否有帮助和描述文本
	*/
	static void CreateCommands(wfcWSession_ptr sess, string cmdStr, xbool hasDetail = true)

	{
		vector<string> cs;
		SplitString(cmdStr, cs, ",");
		try {
			for (size_t i = 0, size = cs.size(); i<size; ++i) {
				xstring cmdName = xstring(cs[i].data());
				CreateCommand(sess, cmdName, hasDetail);
			}
		}
		OTK_EXCEPTION_HANDLE
	}


};

inline void LogMsg(xstring msg, xbool pop = true);
void LogException(xthrowable_ptr x);
void DisplayMessage(wfcWSession_ptr sess, xstring format, xstringsequence_ptr texts = NULL, xstring msgFile = OTKApp::DefMsgFile);
void DisplayTip(wfcWSession_ptr sess, xstring msg);
xstring GetMessage(wfcWSession_ptr sess, xstring format, xstringsequence_ptr texts = NULL, xstring msgFile = OTKApp::DefMsgFile);
