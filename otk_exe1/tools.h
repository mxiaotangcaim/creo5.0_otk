#pragma once
inline void LogMsg(xstring msg, xbool pop=true);
void LogException(xthrowable_ptr x);
xstring GetFileNameWithVersion(pfcModel_ptr model);
