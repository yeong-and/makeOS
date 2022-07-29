/*
	���ֺм� Ŭ������~scanner.h + scanner.cpp~
*/
#ifndef	__SCANNER_H
#define	__SCANNER_H

#pragma warning(disable:4786)

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <string>
#include <list>
#include <map>
#include <stack>

using namespace std;

#include "macro.h"
#include "module.h"
#include "tokendef.h"

class ScannerSub{
	int		nLine;				// ���� ó���ϰ� �ִ� �� ��ȣ
	Module	Mdl;					// ���� ó���ϰ� �ִ� ����
	FILE*	lpLogFP;				// ���� �޼��� ��¿� ���� ������
	char	linebuf[1024];				// ���� ó���ϰ� �ִ� ���� ����
	LPSTR	lpPos;					// ���� ó���ϰ� �ִ� ����� ��ġ
	Token	token;					// ��ū�� ����
	char	labelbuf[256];				// token�� TK_LABEL���� ���� ����
	LONG	numbuf;					// token�� TK_NUM�� ���� ����
	bool	bPeeked;				// ���� PeekToken()�� ��ū�� ����ϰ� ������
	int		nErrorCount;			// �� �ҽ��� �Ͼ ������ ��

	void	Init(void);				// ������ �ʱ�ȭ
	HRESULT	ReadLine(void);				// 1�� �о���δ�
	bool	IsToken(LPSTR &lp, LPSTR lp2);		// ��ġ�ϰ� ������ lp1�� ��ū�� ���������� �����Ѵ�
	void	CopyLabel(LPSTR& lpPos);		// lpPos�κ��� ������ ��ū���� ī��
	HRESULT	NumCheck(LPSTR& lpPos);			// �����ؼ� ��ġ��� numbuf��. �׷��� ������ ��ȯ����0�� �ƴϴ�
	void	GetQuotedLabel(LPSTR &lpPos);		// �ο� ���� ��´�
	Token	PeekToken2(void);			// PeekToken()�� ��ü
	void	Error(LPSTR str);			// ���� �޼����� ǥ��

  public:
	ScannerSub(){ lpLogFP=stderr; nErrorCount=0; Init(); }
	~ScannerSub(){}

#ifdef WINVC
	HRESULT	ReadFile(string& filename);// ������ �о���δ�
#else
	HRESULT	ReadFile(string filename);// ������ �о���δ�
#endif
	
	Token	GetToken(void);			// ������ token�� ��´�(read ������ �����Ѵ�)
	Token	PeekToken(void){ token=PeekToken2(); return token; }
							// ������ token�� ��´�(read ������ �������� �ʴ´�)
	LPSTR	GetLabel(void){ return labelbuf; }	// token�� TK_LABEL���� ��, �� ���� ���� �� �ִ�
	LONG	GetNum(void){ return numbuf; }		// token�� TK_NUM�� ��, �� ��ġ�� ���� �� �ִ�

	int		GetScanline(void){ return nLine; }
	string	GetFileName(void){ return Mdl.GetFileName(); }
	void	SetLogFile(FILE* fp){ lpLogFP = fp; }
	int		GetErrorCount(void){ return nErrorCount; }
};

typedef stack<ScannerSub*> StacklpScannerSub;

class Scanner{
	StacklpScannerSub files;
	int		nIncludeNest;			// ��Ŭ����� �׽�Ʈ ��. �������� 0
	FILE*	lpLogFP;				// ���� �޼��� ��¿� ���� ������
	int		nErrorCount;			// ���ֺм��ο��� �Ͼ ������ ��

  public:
	Scanner(){ lpLogFP=stderr; nIncludeNest=0; nErrorCount=0; }
	~Scanner(){ while(! files.empty()){ DELETE_SAFE(files.top()); files.pop(); } }

#ifdef WINVC
	HRESULT	ReadFile(string& filename);// ������ �о���δ�
#else
	HRESULT	ReadFile(string filename);// ������ �о���δ�
#endif

	Token	GetToken(void);		// ������ token�� ��´�(read ������ �����Ѵ�)
	Token	PeekToken(void);	// ������ token�� ��´�(read ������ �������� �ʴ´�)
	LPSTR	GetLabel(void){ return files.top()->GetLabel(); }
								// token�� TK_LABEL���� ��, �� ���� ���� �� �ִ�
	LONG	GetNum(void){ return files.top()->GetNum(); }
								// token�� TK_NUM�� ��, �� ��ġ�� ���� �� �ִ�
	int		GetScanline(void){ return files.top()->GetScanline(); }
	string	GetFileName(void){ return files.top()->GetFileName(); }
	void	SetLogFile(FILE* fp){ lpLogFP = fp; }
	int		GetErrorCount(void){ return nErrorCount; }
};

#endif
