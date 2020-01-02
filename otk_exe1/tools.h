#pragma once
inline void LogMsg(xstring msg, xbool pop=true);
void LogException(xthrowable_ptr x);
void SplitString(const string&s, vector<string>&v, const string c);
xstring GetFileNameWithVersion(pfcModel_ptr model);
xstring GetFirstOption(wfcWSession_ptr sess, xstring cfgName);
xbool ConfirmAction(wfcWSession_ptr sess, xstring msg);
void ExtractModelFiles(pfcAssembly_ptr assembly, set<string>&modelFiles);
template<typename T>
xstring AnyToString(const T &data) {
	stringstream ss;
	ss << data;
	return xstring(ss);
}
