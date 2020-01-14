#pragma once

#include <vector>

//�쳣����ĺ�
#define OTK_EXCEPTION_HANDLE \
	xcatchbegin \
		xcatchcip(x) {\
			LogException(x); \
		}\
	xcatchend 

/*
Ӧ����Ϣ�࣬�����Ͷ���Ӧ�õ�ȫ����Ϣ�ͱ���
*/
class OTKApp {
public:
	static xstring AppName;     //Ӧ������ 
	static xstring Author;      //����
	static xint Version;        //�汾��
	static xint Release;        // ���к�
	static xstring VersionTitle; //�汾����
	static xstring Description;  //Ӧ������
	static xstring AppURL;       //Ӧ����ַ
	static xstring AppLogo;      //Ӧ��LOGO

	static xstring AppID;        //Ӧ�ñ�ʶ
	static xstring DefMsgFile;  //ȱʡ��Ϣ�ļ�
	static xstring MenuFile;     //�˵��ļ�
};

/*
Function: ȱʡ����������������캯�����ṩ�������ơ�ͨ����������ȷ����ǰ���õ�����
*/
class OTKCommandListener :
	public virtual pfcUICommandActionListener

{
public:
	virtual void OnCommand() {}
	OTKCommandListener(xstring cmdName) {
		mCmdName = cmdName;
	}
	xbool GetIsCommand(xstring cmdName);     //ͨ����������ȷ����ǰ���õ�����

private:
	xstring mCmdName;

};

/*
Function: ȱʡ�������״̬�����������캯�����ṩ�������ơ�ͨ����������ȷ����ǰ������
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
Function: �����˵�������������������ָ���ĵ����˵�
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
Function: ģ�⹤��ģʽ������������������
@TL: ����Ķ��������࣬�����OTKCommandListener����
@TC: ����ķ���״̬�����࣬�����OTKAccessListener����
*/
template<typename TL, typename TC>
class CommandFactory
{
public:
	/*
	Function: ����һ������
	@sess: �Ự����
	@cmdName: �������ƣ��������Ψһǰ׺�ͺ�׺
	@hasDetail: ��Ϣ�ļ����Ƿ��а����������ı�
	*/
	static pfcUICommand_ptr CreateCommand(wfcWSession_ptr sess, xstring cmdName, xbool hasDetail = xtrue)

	{
		pfcUICommand_ptr cmd;
		try {
			xstring cmdLabel = xstring(OTKApp::AppID);
			cmdLabel += cmdName;
			cmd = sess->UICreateCommand(cmdLabel, new TL(cmdName));

			//��������ͼ����뵥����׽�쳣������������ʧ�ܡ�
			try {
				cmd->SetIcon(cmdName + ".png");
			}
			catch (xthrowable *x) {
				delete x;
			}

			//�����Ϣ�ļ����а����ı��������ı�����ʹ����ϸģʽ�������
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
	Function: ������������
	@sess: �Ự����
	@cmdStr: �������������ɵ��ַ������������԰�Ƕ���,����
	@hasDetail: ��Ϣ�ļ����Ƿ��а����������ı�
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
