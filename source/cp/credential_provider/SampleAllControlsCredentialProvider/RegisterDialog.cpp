// RegisterDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "RegisterDialog.h"
#include "afxdialogex.h"


// RegisterDialog 对话框

IMPLEMENT_DYNAMIC(RegisterDialog, CDialogEx)

RegisterDialog::RegisterDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(RegisterDialog::IDD, pParent)
{

}

RegisterDialog::~RegisterDialog()
{
}

void RegisterDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(RegisterDialog, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO1, &RegisterDialog::OnCbnSelchangeCombo1)
	ON_EN_CHANGE(IDC_EDIT2, &RegisterDialog::OnEnChangeEdit2)
	ON_BN_CLICKED(IDC_RADIO1, &RegisterDialog::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &RegisterDialog::OnBnClickedRadio2)
	ON_EN_CHANGE(IDC_EDIT3, &RegisterDialog::OnEnChangeEdit3)
	ON_EN_CHANGE(IDC_EDIT4, &RegisterDialog::OnEnChangeEdit4)
	ON_EN_CHANGE(IDC_RICHEDIT21, &RegisterDialog::OnEnChangeRichedit21)
END_MESSAGE_MAP()


// RegisterDialog 消息处理程序


void RegisterDialog::OnCbnSelchangeCombo1()
{
	// TODO:  在此添加控件通知处理程序代码
}


void RegisterDialog::OnEnChangeEdit2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void RegisterDialog::OnBnClickedRadio1()
{
	// TODO:  在此添加控件通知处理程序代码
}


void RegisterDialog::OnBnClickedRadio2()
{
	// TODO:  在此添加控件通知处理程序代码
}


void RegisterDialog::OnEnChangeEdit3()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void RegisterDialog::OnEnChangeEdit4()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void RegisterDialog::OnEnChangeRichedit21()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
