#include "windows.h"

/* ����Ʈ�� ���� �ƴ����� ������ */

void free(void *p)
{
	HeapFree(GetProcessHeap(), 0, p);
	return;
}
