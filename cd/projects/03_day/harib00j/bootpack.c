/* �ٸ� ���Ϸ� ���� �Լ��� ������ C�����Ϸ��� �˷��ش� */

void io_hlt(void);

/* �Լ� �����ε� {}�� ���� ���ڱ� ;�� ����
	�ٸ� ���Ͽ� �ִٴ� �ǹ��Դϴ�. */

void HariMain(void)
{

fin:
	io_hlt(); /* �̰����� naskfunc.nas�� _io_hlt�� ����˴ϴ� */
	goto fin;

}
