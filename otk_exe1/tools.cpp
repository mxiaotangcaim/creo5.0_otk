#include "stdafx.h"

xstring GetFileNameWithVersion(pfcModel_ptr model) {

	if (NULL == model)return xstringnil;
	pfcModelDescriptor_ptr desc = model->GetDescr();
	xstring fn = desc->GetFileName();
	xint ver = desc->GetFileVersion();
	if (ver>0)
	{
		fn += xstring::Printf(".%d", ver);
		//fn += "." + AnyToString(ver);
	}
	return fn;

}

xstring GetFirstOption(wfcWSession_ptr sess, xstring cfgName) {

	try
	{
		xstringsequence_ptr vals = sess->GetConfigOptionValues(cfgName);
		//返回第一个值
		if (vals != NULL && vals->getarraysize() > 0) return vals->get(0);
	}
	catch (xthrowable *x)
	{
		delete x;
	}
	return xstringnil;
}

void ExtractModelFiles(pfcAssembly_ptr assembly, set<string>&modelFiles) {

	modelFiles.insert((cStringT)assembly->GetFileName());

	pfcSession_ptr sess = pfcSession::cast(assembly->GetDBParent());
	//获取元件特征
	pfcFeatures_ptr cfs = assembly->ListFeaturesByType(true, pfcFEATTYPE_COMPONENT);
	for (xint i = 0, len = cfs->getarraysize(); i < len;i++)
	{
		pfcFeature_ptr cf = cfs->get(i);
		pfcComponentFeat_ptr comFeat = pfcComponentFeat::cast(cf);

		pfcModelDescriptor_ptr comDesc = comFeat->GetModelDescr();
		pfcModel_ptr comp = sess->GetModelFromDescr(comDesc);

		if (comp->GetType()==pfcMDL_ASSEMBLY)
		{
			ExtractModelFiles(pfcAssembly::cast(comp), modelFiles);
		}
		else
		{
			modelFiles.insert((cStringT)comp->GetFileName());
		}
	}

}

xbool ConfirmAction(wfcWSession_ptr sess, xstring msg) {

	pfcMessageDialogOptions_ptr dlgOpts = pfcMessageDialogOptions::Create();
	//设置按钮
	pfcMessageButtons_ptr btns = pfcMessageButtons::create();
	btns->append(pfcMESSAGE_BUTTON_CONFIRM);
	btns->append(pfcMESSAGE_BUTTON_CANCEL);
	dlgOpts->SetButtons(btns);
	
	dlgOpts->SetDefaultButton(pfcMESSAGE_BUTTON_CANCEL);
	dlgOpts->SetDialogLabel("Confirm dialog");
	dlgOpts->SetMessageDialogType(pfcMESSAGE_WARNING);

	pfcMessageButton btn = sess->UIShowMessageDialog(msg, dlgOpts);

	return(btn == pfcMESSAGE_BUTTON_CONFIRM);

}

void SplitString(const string& s, vector<string>& v, const string& c)
{
	string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
}

/*
Function:获取指定参数的值并一字符串形式返回。
@sess: 会话对象
@param: 指定的参数对象
Return: 代表参数值的字符串
*/
xstring GetParamStringValue(wfcWSession_ptr sess, pfcParameter_ptr param) {

	pfcParamValue_ptr pv = param->GetValue();
	switch (pv->Getdiscr()) {

	case pfcPARAM_BOOLEAN:
		return pv->GetBoolValue() ? "xtrue" : "xfalse";

	case pfcPARAM_DOUBLE:
		return xstring::Printf("%.3f", pv->GetDoubleValue());

	case pfcPARAM_INTEGER:
		return xstring::Printf("%d", pv->GetIntValue());

	case pfcPARAM_NOTE:
		return xstring::Printf("%d", pv->GetNoteId());

	case pfcPARAM_STRING:
		return pv->GetStringValue();

	default:
		return xstringnil;
	}

}