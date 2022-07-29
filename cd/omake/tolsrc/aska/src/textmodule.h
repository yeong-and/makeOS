/*
	�ؽ�Ʈ ���� ���ȭ Ŭ������~textmodule.h + textmodule.cpp~
															Ver. [2000/02/17]
*/
#ifndef	__TEXTMODULE_H
#define	__TEXTMODULE_H

#pragma warning(disable:4786)

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <list>
#include <map>
#include <stack>

using namespace std;


class	TextModule{
  public:
// ���� 2���� ����, String�� ��ü�� string�ΰ� wstring�ΰ����� �������� �ʰ� �Ѵ�
	typedef string					String;
	typedef	unsigned char			Letter;
	typedef	long					SizeType;
	typedef	list<String>			ListString;
	typedef	ListString::iterator	LineData;

  protected:
	ListString	TextData;			// �ؽ�Ʈ �����͸� �����ϴ� ����
	
	String		FileName;			// ���� ���� ���� �� �־��� ���ϸ�
	LineData	LineItr;			// ������ ���� ���׷���Ÿ
	int			LinePos;		// ������ �� ��ȣ
	bool		BufferEOF;			// EOF��� true�� �ȴ�

  public:
	TextModule(){ LineItr=TextData.begin(); LinePos=1; BufferEOF=false; }
	~TextModule(){}
	
	void		New();					// �ű� �ۼ�
	void		Open(String& filename);	// open
	void		Close(){ New(); }		// close
	void		Save();					// ����� ����
	void		Save(String& filename){ FileName=filename; Save(); }	// �̸��� �ٿ� ����
	
	String		GetFileName(){ return FileName; }	// ���ϸ� ���
	SizeType	GetLinePos(){ return LinePos; }		// ó���� ���
//	SizeType	GetFileSize();					// ���� ������ ���
	SizeType	GetMaxLinePos(){ return TextData.size(); }	// �ִ���=�� ������ ��
	bool		IsEOF(){ return BufferEOF; }		// EOF ����
	
	void		NextLine(SizeType line=1);		// ���� ������ ����
	void		PrevLine(SizeType line=1);		// ���� ������ ���ƿ´�
	void		SeekLine(SizeType linepos);		// �� ��ȣ�� ����
	void		SeekLine(LineData);				// �࿡ ����
	SizeType	Seek(LineData);					// �� �����κ��� �� ��ȣ ���

	String		PeekLine();						// �� read. ������ ������� �ʴ´�
	String		GetLine();						// �� read. ������ ����ȴ�

#ifdef WINVC
	void		PutLine(String& str){ InsertLine(LineItr, str); }
#else
	void		PutLine(String str){ InsertLine(LineItr, str); }	// �� write. ������ ����ȴ�
#endif
	LineData	ReserveLine(){ return InsertLine(LineItr, String()); }	// �������� ����. �ϸ�ũ�� ����Ѵ�. ����ϸ� EraseLine()�� �Ұ�
#ifdef WINVC
	LineData	InsertLine(LineData, String&);
#else
	LineData	InsertLine(LineData, String);	// �� ����
#endif
	void		EraseLine(LineData);		// �� �Ұ�
};

#endif
