/*
	������ ���̺� Ŭ������~table.h + table.cpp~
*/
#ifndef	__TABLE_H
#define	__TABLE_H

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

enum RegisterType{
	R_GENERAL,		// ���� ��������
	R_SEGREG,		// ���׸�Ʈ(segment) ��������
	R_CTRL,			// ��Ʈ�� ��������
	R_DEBUG,		// ����� ��������
	R_TEST,			// �׽�Ʈ ��������
	R_FLOAT,		// �ε� �Ҽ��� ��������
	R_MMX,			// MMX ��������
};

class RegisterList{
  public:
	LPSTR			name;		// �������͸�
	RegisterType	type;			// �������� Ÿ��
	int				size;	// �������� ������
	bool			bBase;	// ���̽� ���������ϱ�
	bool			bIndex;	// �ε��� ���������ϱ�

	RegisterList(LPSTR n, RegisterType t, int s, bool b, bool i)
			{ name=n; type=t; size=s; bBase=b; bIndex=i; }
};

typedef map<string, RegisterList*> MaplpRegisterList;

class Register{
	MaplpRegisterList	mapregister;
	
  public:
	Register(void);
	~Register();
	
	void			Add(LPSTR key, RegisterList* r){ string k=key; mapregister[k]=r; }
	RegisterList*	Find(LPSTR key){
		string k = key;
		MaplpRegisterList::iterator it = mapregister.find(k);
		if(it == mapregister.end()) return NULL; else return it->second;
	}
};

class LabelList;

class StaticDataList{
  public:
	LPSTR			name;
	LabelList*		label;
	LPSTR			lpInit;
};

typedef list<StaticDataList> ListStaticDataList;

class SegmentList{
  public:
	ListStaticDataList liststatic;

	LPSTR			name;			// ���׸�Ʈ(segment) ����
	Token			align;			// align �Ӽ�
	Token			combine;		// �޹��� �Ӽ�
	Token			use;			// USE �Ӽ�
	Token			access;			// �׼��� �Ӽ�
	LPSTR			segmentclass;		// ���׸�Ʈ(segment) Ŭ����

	RegisterList*	assume;				// segment CODE == ES;�� ES�� �����ϱ� ���ؼ� ����Ѵ�

	SegmentList(void)
		{ name=NULL; align=TK_BYTE; combine=TK_PRIVATE; use=TK_USE32;
			access=TK_ER; segmentclass=NULL; assume=NULL; }
	SegmentList(LPSTR n, Token al, Token co, Token us, Token ac, LPSTR cl)
		{ name=n; align=al; combine=co; use=us; access=ac; segmentclass=cl; assume=NULL; }

	void	AddStaticData(LPSTR varname, LabelList* label, LPSTR initialize);
};

typedef map<string, SegmentList*> MaplpSegmentList;

class Segment{
  public:
	MaplpSegmentList	mapsegment;
	
	Segment(void);
	~Segment();
	
	void			Add(LPSTR key, SegmentList* s){ string k=key; mapsegment[k]=s; }
	SegmentList*	Find(LPSTR key){
		string k = key;
		MaplpSegmentList::iterator it = mapsegment.find(k);
		if(it == mapsegment.end()) return NULL; else return it->second;
	}
};

class TagList;

class MemberList{
  public:
	bool			bSigned;	// ��ȣ�� �ִ� �����ΰ�
	TagList*		type;		// ������ ����
	Token			ptype;		// near:TK_NEAR, far:TK_FAR
	int				pdepth;	// ������ ����. �̰��� 0�̶�� �����Ͱ� �ƴϴ�
	bool			bArray;		// �迭�ϱ�
	int				size;	// ���� ������(�迭, ������, ����ü�� ����)
	int				offset;	// ����ü������ ����� ������(offset)

	MemberList(void)
		{ type=NULL; bSigned=true; ptype=TK_FAR; pdepth=0; bArray=false; size=0; offset=0; }
	MemberList(bool bs, TagList* t, Token pt, int pd, bool ba, int s, int o)
		{bSigned=bs; type=t; ptype=pt; pdepth=pd; bArray=ba; size=s; offset=o; }
	MemberList(MemberList* m)
		{bSigned=m->bSigned; type=m->type; ptype=m->ptype; pdepth=m->pdepth;
			bArray=m->bArray; size=m->size; offset=m->offset; }
	MemberList&	operator=(MemberList& label);
};

typedef map<string, MemberList*> MaplpMemberList;

// typedef�� ó������ �ִ� ���´� "type"��� �ϴ� ����� ����. typedef��� member->type! =NULL
class TagList{
  public:
	int				size;		// �� ������ ������
	bool			bStruct;		// ����ü�ϱ�
	MaplpMemberList	mapmember;			// ����ü�̸� ����� ����Ʈ
	
	TagList(bool bs){ bStruct=bs; size=0;}
	~TagList();

	void			AddMemberList(LPSTR n, bool bs, TagList* t, Token pt, int pd, bool ba, int s);
	MemberList*		FindMemberList(LPSTR key){
		string k = key;
		MaplpMemberList::iterator it = mapmember.find(k);
		if(it == mapmember.end()) return NULL; else return it->second;
	}
};

typedef map<string, TagList*> MaplpTagList;

class Tag{
	MaplpTagList	maptag;
	
  public:
	Tag(void);
	~Tag();
	
	void			Add(LPSTR key, TagList* t){ string k=key; maptag[k]=t; }
	TagList*		Find(LPSTR key){
		string k = key;
		MaplpTagList::iterator it = maptag.find(k);
		if(it == maptag.end()) return NULL; else return it->second;
	}
};


enum ParameterType{
	P_ERR,						// ���ʱ�ȭ Ȥ�� ����
	P_REG,						// �Ķ���� Ÿ���� ��������. base�� ����
	P_MEM,						// �Ķ���� Ÿ���� �޸�. base�� ����
	P_IMM,						// �Ķ���� Ÿ���� ��������. base�� ����
};

class Parameter{
  public:
	ParameterType	paramtype;	// P_ERR, P_REG, P_MEM, P_IMM
	RegisterList*	seg;
	RegisterList*	base;
	RegisterList*	index;
	int				scale;		// 1, 2, 4, 8
	string			disp;
	int				ndisp;		// ����� ��� �˻��� ������(offset) ������ �ִ�
	bool			bLabel;			// P_MEM������ Label�ΰ�
	bool			bSigned;		// ��ȣ�� �ִ� �����ΰ�
	TagList*		type;			// ������ ����
	Token			ptype;		// near:TK_NEAR, far:TK_FAR, offset:TK_OFFSET, segment:TK_SEGMENT
	int				pdepth;		// ������ ����. �̰��� 0�̶�� �����Ͱ� �ƴϴ�
	int				size;		// ���� ������(�迭, ������, ����ü�� ����)
	bool			bArray;			// �迭�ϱ�

	void	Initialize()
		{ paramtype=P_ERR; seg=NULL; base=NULL; index=NULL; scale=1; disp=""; ndisp=0; size=0;
			bLabel=false; bSigned=true; type=NULL; ptype=TK_FAR; pdepth=0; bArray=false; }
	
	Parameter(void){ Initialize(); }
	Parameter&	operator=(Parameter& param);
};

//typedef list<TagList*> ListlpTagList;

//label�� type:dword, ptype:normal, bAddress:false, size:4�� �Ѵ�
class LabelList{
  public:
	bool			bStatic;
	bool			bSigned;	// ��ȣ�� �ִ� �����ΰ�
	TagList*		type;		// ���� ����
	Token			ptype;		// near:TK_NEAR, far:TK_FAR
	int				pdepth;		// ������ ����. �̰��� 0�̶�� �����Ͱ� �ƴϴ�
	SegmentList*	segment;			// �����͸� ��� �Ҵ��ұ�
	int				size;		// ���� ������(�迭, ������, ����ü�� ����)
	bool			bArray;		// �迭�ϱ�
	bool			bAlias;		// �ٸ��(alias)�ϱ�
	Parameter		alias;		// �ٸ��(alias)(������ ���⿡ ���ڸ� �ִ´�)
	int				nLocalAddress;	// ���� ���̶�� ���� �ּҸ� �ִ´�
	bool			bFunction;	// �Լ��ϱ�
//	ListlpTagList	listtag;		// �Լ���� �Լ��� �μ��� ���¸� �ִ´�

	LabelList(void)
		{bSigned=true; type=NULL; ptype=TK_FAR; pdepth=0; segment=NULL; size=0; bArray=false;
			bAlias=false; nLocalAddress=0; bFunction=false; bStatic=false; }
	LabelList(bool bs, TagList* t, Token pt, int pd, SegmentList* s
			, int sz, bool bary, bool balias, Parameter& a, int nla, bool bf)
		{ bSigned=bs; type=t; ptype=pt; pdepth=pd; bArray=bary; bFunction=bf; nLocalAddress=nla;
			size=sz; bAlias=balias; alias=a; segment=s; bStatic=false; }
	LabelList(LabelList* l)
		{ bSigned=l->bSigned; type=l->type; ptype=l->ptype; pdepth=l->pdepth; bArray=l->bArray;
			bFunction=l->bFunction; nLocalAddress=l->nLocalAddress; size=l->size; bAlias=l->bAlias;
			alias=l->alias; segment=l->segment; bStatic=l->bStatic; }
	LabelList&	operator=(LabelList& label);
};

typedef map<string, LabelList*> MaplpLabelList;

class Label{
	MaplpLabelList	maplabel;

  public:
	Label(void){}
	~Label(){ Clear(); }
	
	void			Clear(void);
	void			Add(LPSTR key, LabelList* l){ string k=key; maplabel[k]=l; }
	LabelList*		Find(LPSTR key){
		string k = key;
		MaplpLabelList::iterator it = maplabel.find(k);
		if(it == maplabel.end()) return NULL; else return it->second;
	}
};

enum CompareType{
	C_NOTHING,	// ������
	C_JA,		// (unsigned)>
	C_JAE,		// (unsigned)>=
	C_JB,		// (unsigned)<
	C_JBE,		// (unsigned)<=
	C_JG,		//   (signed)>
	C_JGE,		//   (signed)>=
	C_JL,		//   (signed)<
	C_JLE,		//   (signed)<=
	C_JE,		// ==
	C_JNE,		// ! =
	C_JC,		// CF == 1
	C_JNC,		// CF == 0
	C_JMP,		// 
};

#endif
