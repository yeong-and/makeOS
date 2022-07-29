/*
	���� ���ȭ Ŭ������~module.h + module.cpp~
*/
#ifndef	__MODULE_H
#define	__MODULE_H

#pragma warning(disable:4786)

#include <stdio.h>
#include <stdlib.h>
#ifndef LINUX
#include <io.h>
#endif
#include <iostream>

#include <string>
#include <list>
#include <map>
#include <stack>

using namespace std;

#include "macro.h"

class Module{
	LPVOID	lpMdlAdr;		// ��� �ּ�
	DWORD	dwMdlSize;		// ��� ������
	LPSTR	lpMdlPos;		// ReadLine�� ó�� ��ġ
	string	FileName;		// ���ȭ�ϰ� �ִ� ���ϸ�

	string	MakeFullPath(string& filename);		// ���丮�� �����Ѵ�

  public:
	Module(void){ lpMdlAdr=NULL; dwMdlSize=0; lpMdlPos=NULL; }
	~Module(){ Release(); }
#ifdef WINVC
	void	Release(void){ DELETEPTR_SAFE(lpMdlAdr); dwMdlSize=0; lpMdlPos=NULL; }
#else
	void	Release(void){ DELETEPTR_SAFE((unsigned char*) lpMdlAdr); dwMdlSize=0; lpMdlPos=NULL; }
#endif

	string	GetFileName(void){ return FileName; }
	LPVOID	GetModuleAddress(void){ return lpMdlAdr; }
	DWORD	GetModuleSize(void){ return dwMdlSize; }
	HRESULT ReadFile(string& filename);
		// ��ȯ�� 0:���� ����  1:fileopen ����  3:�޸� Ȯ�� ����  4:read ����  5:fileclose ����
	HRESULT ReadLine(LPSTR buf);
		// ��ȯ�� 0:���� ����  1:EOF  2:���� overflow  3:���� non read
};

//���ܰ迡���� ���丮�� �߰� ���� �ϰ� ���� �ʴ�. �� ��
//include ���丮�� �߰��ؼ� ������ �˻��� ������ ������� �ϰ� �ʹ�.

#endif
