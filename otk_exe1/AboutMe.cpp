#include "stdafx.h"
#include "AboutMe.h"

void AboutListener::OnCommand() {
	if (GetIsCommand("WoodBox")) {
		wfcWSession_ptr wSess = wfcWSession::cast(pfcGetProESession());
		wSess->UIShowMessageDialog(GetMessage(wSess, "Copyright"), NULL);
	}
}