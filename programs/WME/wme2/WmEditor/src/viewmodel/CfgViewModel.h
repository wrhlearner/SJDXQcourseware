﻿/*
** Xin YUAN, 2019, BSD (2)
*/

////////////////////////////////////////////////////////////////////////////////
#ifndef __CFG_VIEW_MODEL_H__
#define __CFG_VIEW_MODEL_H__
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
namespace CSL {
////////////////////////////////////////////////////////////////////////////////

// CfgViewModel

class CfgViewModel : public PropertyTrigger
{
public:
	CfgViewModel() noexcept;
	CfgViewModel(const CfgViewModel&) = delete;
	CfgViewModel& operator=(const CfgViewModel&) = delete;
	~CfgViewModel() noexcept;

//properties
	RefPtr<Fl_Color> get_BackColor() noexcept;

//commands
	std::function<bool(Fl_Color)> get_SetBackColorCommand();

//methods
	void attach_Model(const RefPtr<CfgModel>& refModel) noexcept;
	RefPtr<CfgModel> detach_Model() noexcept;

public:
	PropertyNotification get_Notification();

private:
	RefPtr<CfgModel>  m_refModel;
	Fl_Color  m_clrBack;
};

////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////////////////////////
