#pragma once


// CRButton

class CRButton : public CBitmapButton
{
	DECLARE_DYNAMIC(CRButton)

public:
	CRButton();
	virtual ~CRButton();

protected:
	//{{AFX_MSG(CRButton)
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


