#include "windows.h"

/* ����Ʈ�� ���� �ƴ�����, ������ */

void *malloc(unsigned int bytes)
{
	return HeapAlloc(GetProcessHeap(), 0, bytes);
}
