/*
	���� �м� Ŭ������~parser.h + parser.cpp~
*/
#ifndef	__PARSER_H
#define	__PARSER_H

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
#include "scanner.h"
#include "generator.h"
#include "filepath.h"

class Parser{
  public:
	typedef	Generator::LineData	LineData;

  private:
	Scanner		scanner;			// ��ĵ Ŭ����
	Generator	generator;			// �ڵ� ���׷�����
	
	int			StructAlignCount;	// align�� ���� ���� �󺧸�
	int			LoopLabel[32];		// �׽�Ʈ �� loop������ break�ϱ� ����
	int			LoopLabelPoint;		// �׽�Ʈ��
	int			LocalLabelCounter;	// ���� �б⳪ ������ ���������� ����ϴ� ��
	int			StatementLevel;		// statement�� ����
	LPSTR		lpFunctionName;		// ���� ó���ϰ� �ִ� �Լ���
	Parameter	defaultlocal;		// ���� ������ local == SS:EBP;
	SegmentList*defaultsegment;		// �Լ��� �Ҵ��ϴ� default segment
	SegmentList*defaultdatasegment;		// ������ �Ҵ��ϴ� default segment
	int			SysVarLocalValue;	// �Լ����� ���� ���� ������ ����Ʈ��
	
	LPSTR		lpLogFileName;		// ���� �޼��� ��� ���ϸ�			
	FILE*		lpLogFP;			// ���� �޼��� ��¿� ���� ������
	int			nErrorCount;		// ���� �м��ο��� �Ͼ ������ ��

	//int			stackpoint;		// �Լ��� call�Ǿ��� �� stackpoint�κ��� �߰��ϰ� �ִ� ��

	// ���� HRESULT�� �ִ� ���� ���� ����ô� 0, ������ ��ƾ���� ������ ���� ���� 1,
	// �� �̿��� ���ڴ� �Լ� ���ο����� ������� �ϴ� �ǹ̸� ������. (��! =0�̶�� ����)
	HRESULT	Sizeof(Parameter& param);
	HRESULT	Address(Parameter& param);
	HRESULT	LocalAddress(Parameter& param);
	HRESULT	Selector(Parameter& param, LabelList* label);
	HRESULT	Selector2(Parameter& param);
	void	Member2Param(Parameter& param, MemberList* member);
	HRESULT	Param2Param(Parameter& to, Parameter& from);
	HRESULT	Array(Parameter& param);
	void	PointerCheck(RegisterList* reg);
	HRESULT	ArrayReg(Parameter& param, RegisterList* reg);
	HRESULT	Pointer(Parameter& param);
	HRESULT	GetParameter(Parameter& param);
	HRESULT	Cast(Parameter& param);
	HRESULT	Immedeate(Parameter& param);
	void	Expression(void);
	void	Statement(void);
	void	DefineFunction(void);
	void	DefineVariable(void);
	void	InitialArray(LabelList* label);
	void	DefineAlias(LabelList* label);
	void	DefineInitial(LabelList* label, LPSTR buf);
	void	DefineStruct(void);
	void	DefineMember(TagList* tag);
	void	DefineSegment(void);
	void	DefineDefault(void);
	void	StatementSequence(void);
	void	IfStatement(void);
	CompareType Compare(void);
	CompareType IsCmpOperator(bool bSigned);
	CompareType TransCompare(CompareType cmptype);
	CompareType FlagCompare(void);
	void	LoopStatement(void);
	void	BreakStatement(CompareType cmptype = C_JMP);
	void	AltStatement(void);
	void	AsmoutStatement(void);
	void	ForStatement(void);
	void	WhileStatement(void);
	void	DoStatement(void);
	void	ContinueStatement(CompareType cmptype = C_JMP);
	void	AssumeSegment(void);


//	void	alt_statement(void);
//	bool	case_block(DWORD dw); // �� ID
	void	StartParse(void);

	void	Error(LPSTR str);

  public:
	Parser(void);
	~Parser(){}
	
#ifdef WINVC
	HRESULT	Compile(string& filename);
#else
	HRESULT	Compile(string filename);
#endif

#ifdef WINVC
	HRESULT	Compile(string& filename, string& outfilename);
#else
	HRESULT	Compile(string filename, string outfilename);
#endif

	void	SetLogFile(LPSTR filename);
	int		GetErrorCount(void){ return nErrorCount; }
};

#endif
