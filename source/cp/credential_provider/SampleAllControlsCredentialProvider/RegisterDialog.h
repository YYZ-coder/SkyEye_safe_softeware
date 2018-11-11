#pragma once
#include "resource.h"
#include "stdafx.h"

// RegisterDialog 对话框
// CDialogEx和CDialog的区别:
//		CDialogEx 包含对对话框设置背景图片或者背景颜色

class RegisterDialog : public CDialogEx
{
	DECLARE_DYNAMIC(RegisterDialog)

public:
	RegisterDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~RegisterDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnEnChangeEdit2();
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnEnChangeEdit3();
	afx_msg void OnEnChangeEdit4();
	afx_msg void OnEnChangeRichedit21();
};
