#pragma once

class AboutListener :public virtual OTKCommandListener {

public:
	AboutListener(xstring cmdName) :OTKCommandListener(cmdName) {};
	void OnCommand();

};