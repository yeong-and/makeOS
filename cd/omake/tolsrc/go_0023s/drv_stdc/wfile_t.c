/* for stdc */

int GOLD_write_t(const UCHAR *name, int len, const UCHAR *p0)
/* �ؽ�Ʈ ���� ���Ͽ� ���. ���� name�� NULL��� ǥ�� ��¿� ��� */
{
	int ll = 0;
	FILE *fp = stdout;
	if (name) {
		fp = fopen(name, "w");
		if (fp == NULL)
			goto err;
	}
	if (len)
		ll = fwrite(p0, 1, len, fp);
	if (name)
		fclose(fp);
	if (ll ! = len)
		goto err;
	return 0;
err:
	return 1;
}

