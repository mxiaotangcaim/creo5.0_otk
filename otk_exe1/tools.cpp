#include "stdafx.h"
const char*LOG_FILE = "Exlog.txt";

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
//ceshi