#pragma once
#include<vector>
#include<sstream>
#include<set>

void SplitString(const string&s, vector<string>&v, const string& c);
xstring GetFileNameWithVersion(pfcModel_ptr model);
xstring GetFirstOption(wfcWSession_ptr sess, xstring cfgName);
xbool ConfirmAction(wfcWSession_ptr sess, xstring msg);
void ExtractModelFiles(pfcAssembly_ptr assembly, set<string>&modelFiles);
xstring GetParamStringValue(wfcWSession_ptr sess, pfcParameter_ptr param);
template<typename T>
xstring AnyToString(const T &data) {
	stringstream ss;
	ss << data;
	return xstring(ss);
}
