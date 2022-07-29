/* "LL.c":Lilte-Linker */
/* Copyright(C) 2002 H.Kawai */

/* ��ħ���ʾ �������ϱ� �켱 ������ ��!  */
/* �˰����� �����ϴ� ���� �����̰� �ǰ� ���� */

#define	DEBUG			0

#include "../include/stdlib.h"

#if (DEBUG)
	#include "../include/stdio.h"
#endif

#define	INVALID_DELTA		0x40000000

extern int nask_maxlabels;

#define	MAXSIGMAS				  4

#define	UCHAR			unsigned char

struct STR_SIGMA {
	int scale;
	unsigned int subsect, terms;
};

struct STR_VALUE {
	int min;
	unsigned int delta, flags;
	int scale[2];
	unsigned int label[2];
	struct STR_SIGMA sigma[MAXSIGMAS];
};

#define	VFLG_ERROR			0x01	/* ���� */
#define	VFLG_SLFREF			0x02	/* �ڱ� ���� ���� */
#define	VFLG_UNDEF			0x04	/* ������ ���� */
#define	VFLG_EXTERN			0x10	/* �ܺ� �� */
#define	VFLG_CALC			0x20	/* ����� */
#define	VFLG_ENABLE			0x40	/* STR_LABEL�� ��ȿ�� ���� ��Ÿ���� */

struct STR_LABEL {
	struct STR_VALUE value;
	UCHAR *define; /* �̰��� NULL��� extlabel */
};

struct STR_SUBSECTION {
	unsigned int min, delta, unsolved; /* unsolved == 0 �̶�� ����ȭ�� �ʿ� ���� */
	UCHAR *sect0, *sect1;
};

static struct STR_LABEL *label0;
static struct STR_SUBSECTION *subsect0;

unsigned int solve_subsection(struct STR_SUBSECTION *subsect, char force);
UCHAR *skip_mc30(UCHAR *s, UCHAR *bytes, char flag);
void calcsigma(struct STR_VALUE *value);
void addsigma(struct STR_VALUE *value, struct STR_SIGMA sigma);
UCHAR *LL_skip_expr(UCHAR *p);
UCHAR *LL_skip_mc30(UCHAR *s, UCHAR *bytes, char flag);
UCHAR *LL_skipcode(UCHAR *p);

void init_value(struct STR_VALUE *value)
{
	int i;
	value->min = value->delta = value->scale[0] = value->scale[1] = 0;
	value->flags = 0;
	value->label[0] = value->label[1] = -1;
	for (i = 0; i < MAXSIGMAS; i++) {
		value->sigma[i]. scale = 0;
		value->sigma[i]. subsect = 0;
		value->sigma[i]. terms = 0;
	}
	return;
}

unsigned int get_id(int len, UCHAR **ps, int i)
/* len = 0~3 */
{
	union UINT_UCHAR {
		unsigned int i;
		UCHAR c[4];
	} u;
	UCHAR *s = *ps;
	u.i = i;
	i = 0;
	do {
		u.c[i++] = *s++;
	} while (--len >= 0);
	*ps = s;
	return u.i;
}

void calc_value0(struct STR_VALUE *value, UCHAR **pexpr);

void calc_value(struct STR_VALUE *value, UCHAR **pexpr)
{
	calc_value0(value, pexpr);
	if (value->sigma[0]. scale)
		calcsigma(value);
	return;
}

void enable_label(struct STR_LABEL *label)
{
	struct STR_VALUE value;
	UCHAR *t;

//	if (label->value.flags & VFLG_CALC) {
//		label->value.flags |= VFLG_SLFREF;
//		return;
//	}
//	init_value(&value);
//	value.label[0] = label - label0; /* for EXTERN */
//	value.scale[0] = 1;
//	value.flags |= label->value.flags & VFLG_EXTERN;
	if ((t = label->define) ! = NULL) {
		label->value.flags |= VFLG_CALC;
		calc_value0(&value, &t);
		if (value.flags & VFLG_SLFREF) {
			init_value(&value);
			value.flags |= VFLG_SLFREF;
		}
	} else /* if ((label->value.flags & VFLG_EXTERN) == 0) */ {
		/* EXTERN���� ��ٷ� ENABLE�� �ǹǷ�, ���⿡ �� �� ���� */
		init_value(&value);
		value.flags |= VFLG_UNDEF;
	}
	label->value = value;
	label->value.flags |= VFLG_ENABLE;
	return;
}

void calc_value0(struct STR_VALUE *value, UCHAR **pexpr)
{
	UCHAR *expr = *pexpr, c, *t;
	int i, j, k;
	struct STR_VALUE tmp, tmp2;
	struct STR_SUBSECTION *subsect;
	c = *expr++;
	init_value(value);
	if (c <= 6) {
		/* ���� */
		i = 0;
		if (c & 0x01)
			i--;
		value->min = get_id(c >> 1, &expr, i);
		goto fin;
	}
	if (c == 0x07) {
		/* ���� 1 */
		value->min = 1;
		goto fin;
	}
	if (c <= 0x0b) {
		/* �� �ο� */
		struct STR_LABEL *label;
		label = &label0[i = get_id(c - 8, &expr, 0)];
		if ((label->value.flags & VFLG_ENABLE) == 0) {
			if (label->value.flags & VFLG_CALC) {
				value->flags |= VFLG_SLFREF;
				goto fin;
			}
			enable_label(label);
		}
		*value = label->value;
		goto fin;
	}
	#if (DEBUG)
		if (c < 0x10)
			goto dberr;
	#endif
	if (c < 0x20) {
		/* ������ */
		calc_value0(value, &expr);
		if (c == 0x10)
			goto fin; /* ���� ������ + */
		if (c == 0x11) {
			/* ���� ������ - */
			value->min = - (int) (value->min + value->delta);
	minus2:
			value->scale[0] *= -1;
 			value->scale[1] *= -1;
			for (i = 0; i < MAXSIGMAS; i++)
				value->sigma[i]. scale *= -1;
			goto fin;
		}
		if (c == 0x12) {
			/* ���� ������ ~ */
			/* ~a == - a - 1 */
			value->min = - (int) (value->min + value->delta) - 1;
			goto minus2;
		}
		calc_value0(&tmp, &expr);
		value->flags |= tmp.flags;
		if (c == 0x13) {
			/* 2�׿����� + */
add2:
			for (i = 0; i < 2; i++) {
				if (value->scale[i] == 0)
					break;
			}
			for (j = 0; j < 2; j++) {
				if (tmp.scale[j] == 0)
					break;
				for (k = 0; k < i; k++) {
					if (value->label[k] == tmp.label[j]) {
						/* ������ ���� */
						if ((value->scale[k] += tmp.scale[j]) == 0) {
							/* ���� �Ҹ� */
							i--;
							value->label[k] = 0xffffffff;
							if (k)
								goto next_product;
							value->scale[0] = value->scale[1];
							value->label[0] = value->label[1];
							value->scale[1] = 0;
							value->label[1] = 0xffffffff;
						}
						goto next_product;
					}
				}
				/* �������� ������ ������ ���� �߰� */
				if (i >= 2) {
					value->flags |= VFLG_ERROR;
					goto fin;
				}
				value->scale[i] = tmp.scale[j];
				value->label[i] = tmp.label[j];
				i++;
next_product:
				;
			}
			for (j = 0; j < MAXSIGMAS; j++) {
				if (tmp.sigma[j]. scale == 0)
					break;
				addsigma(value, tmp.sigma[j]);
			}
			value->min += tmp.min;
			value->delta += tmp.delta;
			goto fin;
		}
		if (c == 0x14) {
			/* 2�׿����� - */
			tmp.min = - (int) (tmp.min + tmp.delta);
			tmp.scale[0] *= -1;
 			tmp.scale[1] *= -1;
			for (i = 0; i < MAXSIGMAS; i++)
				tmp.sigma[i]. scale *= -1;
			goto add2;
		}
		if (c == 0x15) { /* "*" */
			if (value->scale[0])
				goto no_exchange;
			if (tmp.scale[0])
				goto exchange;
		}
		if (c == 0x15 || 0x1d <= c && c <= 0x1f) { /* "*", "&", "|", "^" */
			if (value->delta)
				goto no_exchange;
			if (tmp.delta)
				goto exchange;
			if (value->sigma[0]. scale)
				goto no_exchange;
			if (tmp.sigma[0]. scale) {
		exchange:
				/* ���� ��ȯ */
				/* ����ġ�� 1�׸� �������� ���� ��ȯ */
				tmp.flags = value->flags;
				tmp2 = *value;
				*value = tmp;
				tmp = tmp2;
			}
		}
	no_exchange:
		calcsigma(&tmp);
		if (tmp.delta) {
			/* ����ġ�� ����ϱ� ���� ���� ���� ������ �� ���� */
			value->flags |= VFLG_ERROR;
			goto fin;
		}
		if (c == 0x15) {
			/* 2�׿����� * (signed) */
			if (tmp.scale[0]) {
	error:
				/* ���� ��� label�� �����ϰ� ������ ���� */
				value->flags |= VFLG_ERROR;
				goto fin;
			}
multiply2:
			if (tmp.min == 0) {
				init_value(value);
				goto fin;
			}
			value->scale[0] *= tmp.min;
			value->scale[1] *= tmp.min;
			for (i = 0; i < MAXSIGMAS; i++)
				value->sigma[i]. scale *= tmp.min;
			i = (signed int) value->min * (signed int) tmp.min;
			j = (signed int) (value->min + value->delta) * (signed int) tmp.min;
minmax:
			value->min = i;
			value->delta = j - i;
			if (i > j) {
				value->min = j;
				value->delta = i - j;
			}
			goto fin;
		}
		/* �� ���Ĵ� tmp���� label�� ĵ���ȴ� */
		if (c == 0x16) {
			/* 2�׿����� << */
			tmp.min = 1 << tmp.min;
			goto multiply2;
		}
		if (c == 0x17) {
			/* 2�׿����� / (unsigned) */
divu2:
			if (tmp.min == 0)
				goto error;
divu_ij:
			i = (unsigned int) value->min / (unsigned int) tmp.min;
			j = (unsigned int) (value->min + value->delta) / (unsigned int) tmp.min;
			for (k = 0; k < MAXSIGMAS; k++) {
				if (value->sigma[k]. scale % tmp.min) {
					calcsigma(value);
					goto divu_ij;
				}
			}
divall:
			if (value->scale[0] % tmp.min)
				goto error;
			if (value->scale[1] % tmp.min)
				goto error;
			value->scale[0] /= tmp.min;
			value->scale[1] /= tmp.min;
			for (k = 0; k < MAXSIGMAS; k++)
				value->sigma[k]. scale /= tmp.min;
			goto minmax;
		}
		if (c == 0x19) {
			/* 2�׿����� / (signed) */
divs2:
			if (tmp.min == 0)
				goto error;
divs_ij:
			i = (signed int) value->min / (signed int) tmp.min;
			j = (signed int) (value->min + value->delta) / (signed int) tmp.min;
			for (k = 0; k < MAXSIGMAS; k++) {
				if (value->sigma[k]. scale % tmp.min) {
					calcsigma(value);
					goto divs_ij;
				}
			}
			goto divall;
		}
		if (c == 0x1b) {
			/* 2�׿����� >> (unsigned) */
			tmp.min = 1 << tmp.min;
			goto divu2;
		}
		if (c == 0x1c) {
			/* 2�׿����� >> (signed) */
			tmp.min = 1 << tmp.min;
			goto divs2;
		}
		value->scale[0] = value->scale[1] = 0; /* �ڵ� ĵ�� */
		calcsigma(value);
		if (c == 0x18) {
			/* 2�׿����� % (unsigned) */
			if (tmp.min == 0)
				goto error;
			value->min = (unsigned int) value->min % (unsigned int) tmp.min;
			if (value->delta) {
				value->min = 0;
				value->delta = tmp.min - 1;
			}
			goto fin;
		}
		if (c == 0x1a) {
			/* 2�׿����� % (signed) */
			if (tmp.min == 0)
				goto error;
			value->min = (signed int) value->min % (signed int) tmp.min;
			if (value->delta) {
				value->delta = tmp.min - 1;
				if (value->min >= 0) {
					value->min = 0;
				} else if ((signed int) (value->min + value->delta) <= 0) {
					value->min = 1 - tmp.min;
				} else {
					value->min = 1 - tmp.min;
					value->delta *= 2;
				}
			}
			goto fin;
		}
		if (c == 0x1d) {
			/* 2�׿����� & (unsigned) */
			if (tmp.min == -1)
				goto fin; /* �׳� ����ħ */
			if (value->delta == 0) {
				value->min &= tmp.min;
				goto fin;
			}
			if (tmp.min & 0x80000000)
				goto error; /* ���� �� �� ���� */
			/* ����� �� �� �����ϰ� �� �� ������, �����Ƽ� �����ʰ� �ִ� */
			value->min = 0;
			value->delta = tmp.min;
			goto fin;
		}
		if (c == 0x1e) {
			/* 2�׿����� | (unsigned) */
			if (tmp.min == 0)
				goto fin; /* �׳� ����ħ */
			if (value->delta == 0) {
				value->min |= tmp.min;
				goto fin;
			}
			if (tmp.min & 0x80000000)
				goto error; /* ���� �� �� ���� */
			value->delta += tmp.min;
			if ((value->delta & 0x80000000) == 0)
				goto fin;
			value->delta = 0;
			goto error;
		}
	//	if (c == 0x1f) {
			/* 2�׿����� ^ (unsigned) */
			if (tmp.min == 0)
				goto fin; /* �׳� ����ħ */
			if (value->delta == 0) {
				value->min ^= tmp.min;
				goto fin;
			}
			if (tmp.min & 0x80000000)
				goto error; /* ���� �� �� ���� */
			if ((signed int) value->min > (signed int) 0 && value->min >= tmp.min) {
				value->min -= tmp.min;
				value->delta += tmp.min;
				goto fin;
			}
			goto error; /* ���� �� �� ���� */
	//	}
	}
	#if (DEBUG)
		if (c < 0x80)
			goto dberr;
	#endif
#if 0
	if (c == 0x80) {
		/* subsect size sum */
		/* sum��, delta�� 2 GB��� ������ �� ������(�����Ƽ�) */
		calc_value(&tmp, &expr);
		i = tmp.min;
		calc_value(&tmp, &expr);
		subsect = subsect0 + tmp.min;
		do {
			value->min += subsect->min;
			value->delta += subsect->delta;
			if (subsect->delta >= INVALID_DELTA)
				value->flags |= VFLG_ERROR;
		} while (--i);
		goto fin;
	}
#endif
	#if (DEBUG)
dberr:
		fprintf(stderr, "calc_value:%02x\n", c);
		GOL_sysabort(GO_TERM_BUGTRAP);
	#endif

fin:
	*pexpr = expr;
	return;
}

static UCHAR *skip_expr(UCHAR *expr)
{
	return LL_skip_expr(expr);
}





/* ���� ���� ���:
	 �Ϲݽ�
	80 variable-sum, 0000bbaa(aa:�׼�-1, bb:������ ��ȣ), 
	84~87 sum, i - 1, expr, expr, ...

  ��80~8 f:LL�� ���� ó�������� ����Ѵ�
	80~83:variable ����(1~4����Ʈ)
	88~8f:sum(variable), (1~4, 1~4) : ó���� �׼�-1, ������ ������ ��ȣ
		{ "|>", 12, 18 }, { "&>", 12, 17 },
		{ "<<", 12, 16 }, { ">>", 12, 17 },
		{ "//", 14,  9 }, { "%%", 14, 10 },
		{ "+",  13,  4 }, { "-",  13,  5 },
		{ "*",  14,  6 }, { "/",  14,  7 },
		{ "%",  14,  8 }, { "^",   7, 14 },
		{ "&",   8, 12 }, { "|",   6, 13 },
		{ "",    0,  0 }

	s+
	s-
	s~

	+, -, *, <<, /u, %u, /s, %s
	>>u, >>s, &, |, ^

*/






#if 0

	< ���� >

ó���� ���.
����� ������(DW) = 12
������ �ڵ�(DW)
���� �Ѽ�(DW)


  ��38:���� ���� DB�� ��ġ
  ��39:���� ���� DW�� ��ġ
  ��3a:���� ���� 3����Ʈ�� ��ġ
  ��3b:���� ���� DD�� ��ġ
  ����, ��3f���� �ִ�.
  ��40~47:���� Ŀ�ǵ�. if������ �ļӹ��� ����ȭ�Ѵ�(2~9).
  ��48:����Ʈ ����(���� ���̰� ����Ʈ�� �ļ�).
  ��49:���� ����.
  ��4a:����Ʈ ����.
  ��4b:���� ���� ����.
  ��4c:��Ÿ�� if����.
  ��4d:������ if����.
  ��4e:������ �ٿ���� if����. ���� ������ ����, �ٿ���� ���� ��ӵȴ�.
  ��4f:endif.
  ��Ÿ�� if��, endif�� �� ������ �󸶵��� �þ���� �� �ִ�. endif�� �� ������,
  ��� else-if�� �ٷ������. �������� else�� ����� �ʹٸ� ������ ���� 1�̵�.
  ���͹̳����ʹ� �� ���Ƿ� 0xffffffff.

  ��58:ORG

  ��60:�ƶ���. ����Ʈ�� ���� ����� ���������� �����Ѵ�. �������̰��� ��Ÿ�� if������ ����� �� �ִ�.

  ��70~77:������ ����Ʈ ����(�����󿡼��� 40~4b�� �ļ� �ϴ� ���� ���������, ����Ʈ�ϰ� ���� �ʴ�. ����Ǵ� ���� 30~3f)
  ��78~7f:������ ����Ʈ ����

  ��80~8f:LL�� ���� ó�������� ����Ѵ�
	80~83:variable ����(1~4����Ʈ)
	88~8f:sum(variable), (1~4, 1~4) : ó���� �׼�-1, ������ ������ ��ȣ


  if���߿����� ������ ����Ʈ ���� �ۿ� �� �� ����.

	ORG�� ����. ���� �μ��� ���̾ ������, �� ���������� �������� �����ϰ� �ִ�.




#endif

/* ibuf, obuf�� ���� ���� */
/* �ñ׳�ó 8����Ʈ, ���� 4����Ʈ, ������ 4����Ʈ */
/* ���� ������ ���� 4B, ���� ������ ��ŸƮ 4B */

/* ���̷� ��ٷο� ���� ���� �ʴ´� */
/* ��ŵ Ŀ�ǵ带 ����� ��ó�Ѵ� */

static UCHAR table98typlen[] = { 0x38, 0x38, 0x39, 0x39, 0x3b, 0x3b, 0x38 };
static UCHAR table98range [] = { 0x00, 0x02, 0x00, 0x03, 0x00, 0x03, 0x03 };

struct STR_LL_VB {
	UCHAR *expr, typlen, range;
};

UCHAR *LL_define_VB(struct STR_LL_VB *virtualbyte, UCHAR *s)
{
	UCHAR c;
	virtualbyte->typlen = c = *s++;
	#if (DEBUG)
		if (c < 0x30)
			goto dberr0;
	#endif
	if (c < 0x38) {
		virtualbyte->expr = s;
		s += c & 0x07;
		goto fin;
	}
	if (c < 0x3c)
		virtualbyte->range = *s++;
	else {
		#if (DEBUG)
			if (c < 0x98)
				goto dberr0;
			if (c >= 0x9f)
				goto dberr0;
		#endif
		virtualbyte->typlen = table98typlen[c & 0x07];
		virtualbyte->range  = table98range [c & 0x07];
	}
	virtualbyte->expr = s;
	s = skip_expr(s);
fin:
	return s;	

	#if (DEBUG)
dberr0:
		fprintf(stderr, "LL_define_VB:%02x\n", c);
		GOL_sysabort(GO_TERM_BUGTRAP);
		return NULL;
	#endif
}

UCHAR *lccbug_LL_mc90_func(UCHAR *src0, struct STR_LL_VB *vbb, struct STR_LL_VB *vbc, char len, char c)
{
	do {
		if (c ! = 0) {
			src0 = skip_mc30(src0, NULL, 1);
			if (vbc)
				src0 = skip_mc30(src0, NULL, 1);
		} else {
			src0 = LL_define_VB(vbb, src0);
			if (vbc)
				src0 = LL_define_VB(vbc, src0);
		}
		c--;
	} while (--len);
	return src0;
}

UCHAR *LL(UCHAR *src0, UCHAR *src1, UCHAR *dest0, UCHAR *dest1)
/* ���� src�� write�� �ϹǷ� ������ */
/* ���ο� dest1�� �����ش� */
{
	struct STR_SUBSECTION *subsect1, *subsect;
	unsigned int l0, min, max, unsolved, unsolved0;
	struct STR_LABEL *label;
	struct STR_VALUE *value, *labelvalue;
	struct STR_LL_VB virtualbyte[8], *vba, *vbb, *vbc;
	signed int times_count;
	UCHAR *s, *times_src0, *times_dest0, c, len, range;

	label0 = malloc(nask_maxlabels * sizeof (struct STR_LABEL));
	subsect0 = malloc(nask_maxlabels /* == MAXSUBSETCS */ * sizeof (struct STR_SUBSECTION));
	value = malloc(sizeof (struct STR_VALUE));
	labelvalue = malloc(sizeof (struct STR_VALUE));

	/* �󺧽��� ����Ѵ� */
//	l0 = 0; /* ������ �ּҴ� l0���� lc�� */
//	lc = 0;
//	min = max = 0; /* ������ �����ϴ� ����, variable0[l0 + lc] */

	label = label0;
	for (unsolved = nask_maxlabels; unsolved > 0; label++, unsolved--) {
		label->value.flags = 0; /* enable�� ����� (����)������ */
		label->define = NULL;
	}

	/* �и��Ѵ� */
	/* ���� �����Ѵ� */
	subsect = subsect0;
	subsect->sect0 = src0;
	init_value(labelvalue); /* �� ���� ���� */
	while (src0 < src1) {
		c = *src0;
		if (c == 0x2d) {
			/* EQU */
			src0++;
			calc_value(value, &src0);
			label = &label0[value->min];
			label->define = src0;
			src0 = skip_expr(src0);
			continue;
		}
		if (c == 0x0e) {
			/* �� ���� */
			struct STR_SIGMA sigma;
			sigma.scale = 1;
			sigma.subsect = subsect - subsect0;
			sigma.terms = 1;
			subsect->sect1 = src0;
			subsect++;
			subsect->sect0 = src0;
			src0++;
			calc_value(value, &src0);
			label = &label0[value->min];
			label->define = src0;
			addsigma(labelvalue, sigma);
			label->value = *labelvalue;
			label->value.flags |= VFLG_ENABLE;
			continue;
		}
		if (c == 0x2c) {
			/* extern �� ���� */
			src0++;
			calc_value(value, &src0);
			label = &label0[value->min];
			init_value(&label->value);
		//	label->define = NULL;
			label->value.flags |= VFLG_EXTERN | VFLG_ENABLE;
			label->value.label[0] = value->min;
			label->value.scale[0] = 1;
			continue;
		}
		if (c == 0x58) {
			/* ORG */
			subsect->sect1 = src0;
			subsect++;
			subsect->sect0 = src0;
			src0++;
			calc_value(labelvalue, &src0);
			continue;
		}
		src0 = LL_skipcode(src0);
	}
	subsect->sect1 = src1;
	subsect1 = subsect + 1;

	for (subsect = subsect0; subsect < subsect1; subsect++) {
		subsect->min = 0;
		subsect->delta = INVALID_DELTA;
		subsect->unsolved = 1;
	}
	len = 0;
	c = 0;
	unsolved0 = 0x7fffffff;
	do {
		for (subsect = subsect0; subsect < subsect1; subsect++) {
			if (subsect->unsolved)
				solve_subsection(subsect, len);
		}
		unsolved = 0;
		for (subsect = subsect1 - 1; subsect >= subsect0; subsect--) {
			if (subsect->unsolved)
				unsolved += solve_subsection(subsect, len);
		}
		c++;
		if (unsolved0 > unsolved) {
			c = 0;
			unsolved0 = unsolved;
		}
		if (c >= 3) {
			len++;
			c = 0;
		}
	} while (unsolved);

	for (subsect = subsect0; subsect < subsect1; subsect++) {
		times_count = 0;
		for (src0 = subsect->sect0; src0 < subsect->sect1; ) {
			c = *src0++;
			if (dest0 + 64 >= dest1) {
	error:
				return NULL; /* buffer overrun */
			}
			if (c == 0x2d) {
				/* EQU */
				calc_value(value, &src0);
				label = &label0[value->min];
				if ((label->value.flags & VFLG_ENABLE) == 0)
					enable_label(label);
				*value = label->value;
				calcsigma(value);
				c = 0x0c;
				if (value->scale[0])
					c++;
				*dest0++ = c;
				len = 4;
				src0 = skip_expr(src0);
				goto putvalue0;
			}
			if (c == 0x0e) {
				src0 = skip_expr(src0);
				continue;
			}
			if (c == 0x0f) {
				dest0[0] = 0xf6; /* REM_8B */
				dest0[1] = *src0++;
				c = *src0++;
				label = &label0[get_id(c - 8, &src0, 0)];
				if ((label->value.flags & VFLG_ENABLE) == 0)
					enable_label(label);
				*value = label->value;
				calcsigma(value);
				dest0[2] = 0; /* base extern label (none) */
				dest0[3] = 0;
				dest0[4] = value->min & 0xff;
				dest0[5] = (value->min >>  8) & 0xff;
				dest0[6] = (value->min >> 16) & 0xff;
				dest0[7] = (value->min >> 24) & 0xff;
				dest0 += 8;
				if (value->scale[0]) {
					dest0[2 - 8] = value->label[0] & 0xff;
					dest0[3 - 8] = (value->label[0] >> 8) & 0xff;
				//	if (value->scale[0] ! = 1 || value->scale[1] ! = 0) {
				//		/* ������ ��� */
				//	}
				}
				continue;
			}
			if (c == 0x2c) {
				dest0[0] = 0x2c;
				dest0[1] = src0[0];
				dest0[2] = src0[1];
				dest0[3] = src0[2];
				src0 += 3;
				dest0 += 4;
				continue;
			}
			#if (DEBUG)
				if (c < 0x30)
					goto dberr0;
			#endif
			if (c < 0x37) {
				/* 30~37:��Ʈ DB(����, 0~7����Ʈ) */
				len = c & 0x07;
	copy:
				*dest0++ = c;
				if (len) {
					times_count -= len;
					do {
						*dest0++ = *src0++;
					} while (--len);
	check_times_count:
					if (times_count == 0) {
	times_expand:
						s = skip_expr(times_src0 + 4); /* len */
						calc_value(value, &s); /* �ݺ� ȸ�� */
						c = 0;
						value->min = times_src0[0] | times_src0[1] << 8
							| times_src0[2] << 16 | times_src0[3] << 24;
						/* ��force������ ��߳��� ���� ������ �ʵ��� �ϱ� ���� */
						if (value->delta) {
							/* ����ġ ���� */
							c = 1;
							*dest0++ = 0xe9;
						}
						if (value->flags & VFLG_UNDEF) {
							/* ������ ���� */
							c = 1;
							*dest0++ = 0xec;
						}
						if (value->flags & VFLG_SLFREF) {
							/* �ڱ� ���� ���� */
							c = 1;
							*dest0++ = 0xea;
						}
						if (value->flags & VFLG_ERROR) {
							/* ��ġ �Ϲ� ���� */
							c = 1;
							*dest0++ = 0xeb;
						}
						if (c ! = 0 || (unsigned int) value->min >= 0xfffffff0) {
							*dest0++ = 0xe6; /* TIMES error */
							dest0 = times_dest0; /* �ݺ� ȸ�� 0���� �����Ѵ� */
							continue;
						}
						value->delta = dest0 - times_dest0;
						if (dest0 + value->delta * value->min + 64 >= dest1)
							goto error; /* 1ȸ�� ���ʿ��ϰ� ���ϰ� �ִ� */
						for (dest0 = times_src0 = times_dest0; value->min > 0; value->min--) {
							s = times_src0;
							for (times_count = value->delta; times_count > 0; times_count--)
								*dest0++ = *s++;
						}
					}
	times_count0:
					times_count = 0;
				}
				continue;
			}
			if (c < 0x3c) {
				/* ���� ���� �д� */
				len = c - 0x37;
				range = *src0++;
				calc_value(value, &src0);
	putvalue:
				if (value->scale[0] && len == 4 && value->delta == 0
					&& (value->flags & (VFLG_UNDEF | VFLG_SLFREF | VFLG_ERROR)) == 0) {
					/* ���� ����(1�ΰ���1) */
					if (value->delta == 0 && value->flags)
					if (value->scale[0] == -1) {
						value->label[0] = value->label[1];
						value->scale[1] = -1;
					}
					/* ���⼭ �����ؾ� �� ���̽� */
					/* scale[0] == 1, scale[1] ==  0 : ���� ���� */
					/* scale[0] == 1, scale[1] == -1 : ��� ���� */
					/* 0x2e : ���� ���� �䱸, 0x2f : ��� ���� �䱸 */
					dest0[0] = 0x2e - value->scale[1];
					dest0[1] = 0x09;
					dest0[2] = value->label[0] & 0xff;
					dest0[3] = (value->label[0] >> 8) & 0xff;
					dest0 += 4;
				}
				*dest0++ = 0x30 | len; /* 31~34 */
				/* ������ üũ���� �Ѵ� */
				/* scale üũ�� ��, �ʿ��ϸ� ��ũ�� �ʿ��� ������ ���δ� */
	putvalue0:
				/* len ����Ʈ�� �д� */
				dest0[0] = value->min         & 0xff;
				dest0[1] = (value->min >>  8) & 0xff;
				dest0[2] = (value->min >> 16) & 0xff;
				dest0[3] = (value->min >> 24) & 0xff;
				dest0 += len;
				if (value->delta) {
					/* ����ġ ���� */
					*dest0++ = 0xe9;
				}
				if (value->flags & VFLG_UNDEF) {
					/* ������ ���� */
					*dest0++ = 0xec;
				}
				if (value->flags & VFLG_SLFREF) {
					/* �ڱ� ���� ���� */
					*dest0++ = 0xea;
				}
				if (value->flags & VFLG_ERROR) {
					/* ��ġ �Ϲ� ���� */
					*dest0++ = 0xeb;
				}
				if ((times_count -= len) == 0)
					goto times_expand;
				goto times_count0;
			//	times_count = 0;
			//	continue;
			}
			if (c == 0x58) {
				/* ORG */
				calc_value(value, &src0);
				*dest0++ = 0x5a;
				len = 4;
				goto putvalue0;
			}
			if (c == 0x59) {
				/* TIMES */
				times_dest0 = dest0;
				times_src0 = src0;
				src0 += 4;
				calc_value(value, &src0); /* len */
				/* len�� �κп� ������ ���ٰ� �ϴ� ���� �ǰ� �ִ�(�ν�) */
				times_count = value->min;
				src0 = skip_expr(src0);
				continue;
			}
			if (c == 0x68) {
				/* endian */
				src0++;
				continue;
			}
			#if (DEBUG)
				if (c < 0x70)
					goto dberr0;
			#endif
			if (c < 0x78) {
				/* 0 x70~0x77:virtual byte ���� */
				src0 = LL_define_VB(&virtualbyte[c & 0x07], src0);
				continue;
			}
			if (c < 0x80) {
				/* 0 x78~0x7f:virtual byte ���� */
				vba = &virtualbyte[c & 0x07];
				s = vba->expr;
				if (vba->typlen < 0x38) {
					len = vba->typlen & 0x07;
					*dest0++ = vba->typlen;
					if (len) {
						times_count -= len;
						do {
							*dest0++ = *s++;
						} while (--len);
						goto check_times_count;
					}
					continue;
				}
				len = vba->typlen - 0x37;
				range = vba->range;
				calc_value(value, &s);
				goto putvalue;
			}
			#if (DEBUG)
				if (c < 0x90)
					goto dberr0;
			#endif
			if (c <= 0x91) {
				/* microcode90 */
				/* microcode91 */
				len = src0[2]; /* z-bbb-d-aaa */
				vba = &virtualbyte[len & 0x07];
				vbb = &virtualbyte[(len >> 4) & 0x07];
				vbc = vbb + 1;
				if (c == 0x90)
					vbc = NULL;
				vba->typlen = 0x37 + (src0[1] & 0x0f);
				vba->range = 0x03; /* non-overflow */
				if (vba->typlen == 1)
					vba->range = 0x02; /* normal-signed */
				c = src0[0] & 0x0f;
				len = ((len /* z-bbb-d-aaa */ >> 7) & 0x01) + 2;
				vba->expr = src0 + 3;
				src0 = skip_expr(vba->expr);

				src0 = lccbug_LL_mc90_func(src0, vbb, vbc, len, c);
#if 0
				do {
					if (c ! = 0) {
						src0 = skip_mc30(src0, NULL, 1);
						if (vbc)
							src0 = skip_mc30(src0, NULL, 1);
					} else {
						src0 = LL_define_VB(vbb, src0);
						if (vbc)
							src0 = LL_define_VB(vbc, src0);
					}
					c--;
				} while (--len);
#endif
				continue;
			}
			#if (DEBUG)
				if (c < 0x94)
					goto dberr0;
			#endif
			if (c <= 0x95) {
				/* microcode94 */
				/* microcode95 */
				len = src0[2]; /* z-bbb-d-aaa */
				vba = &virtualbyte[len & 0x07];
				vbb = &virtualbyte[(len >> 4) & 0x07];
				vbc = vbb + 1;
				if (c == 0x94)
					vbc = NULL;
				vba->expr = src0 + 3;
				c = src0[0] & 0x0f;
				src0 = skip_expr(vba->expr);
				len = *src0++;
				goto outmc94_skip;
				do {
					c--;
					src0 = skip_expr(src0);
			outmc94_skip:
					if (c == 0) {
						range = *src0++;
						if (range < 0x3c) {
							vba->typlen = range;
							vba->range = *src0++;
						} else {
							vba->typlen = table98typlen[range & 0x07];
							vba->range = table98range[range & 0x07];
						}
						src0 = LL_define_VB(vbb, src0);
					} else {
						src0 = skip_mc30(src0, NULL, 0);
						src0 = skip_mc30(src0, NULL, 1);
					}
					if (vbc) {
						if (c == 0)
							src0 = LL_define_VB(vbc, src0);
						else
							src0 = skip_mc30(src0, NULL, 1);
					}
				} while (--len);
				continue;
			}
			#if (DEBUG)
				if (c < 0x98)
					goto dberr0;
			#endif
			if (c < 0x9f) {
				len = table98typlen[c & 0x07] & 0x07;
				range = table98range[c & 0x07];
				calc_value(value, &src0);
				goto putvalue;
			}
			#if (DEBUG)
				if (c < 0xe0)
					goto dberr0;
			#endif
			if (c < 0xf0) {
				/* 1����Ʈ ����ũ */
				*dest0++ = c;
				continue;
			}
			if (c < 0xf8) {
				/* n����Ʈ ����ũ */
				len = c - 0xef;
				goto copy;
			}
			#if (DEBUG)
	dberr0:
				fprintf(stderr, "LL:%02x\n", c);
				GOL_sysabort(GO_TERM_BUGTRAP);
			#endif
		}
	}

	free(label0);
	free(subsect0);
	free(value);
	free(labelvalue);

	return dest0;
}


UCHAR *skip_mc30(UCHAR *s, UCHAR *bytes, char flag)
{
	return LL_skip_mc30(s, bytes, flag);
}

unsigned int solve_subsection(struct STR_SUBSECTION *subsect, char force)
/* unsolved���� �����ش� */
/* subsect->unsolved == 0 �̶�� call���� �� �� */
{
	struct {
		signed char min, max;
	} vb[8], *vba, *vbb, *vbc;
	unsigned int unsolved = 0, min = 0, delta = 0;
	int i;
	struct STR_VALUE value, tmp;
	UCHAR *s, *t, c, d, e, f, g, h;

	for (s = subsect->sect0; s < subsect->sect1; ) {
		c = *s++;
		if (c == 0x2d) {
			/* EQU �� ���� */
			s = skip_expr(s);
			s = skip_expr(s);
			continue;
		}
		if (c == 0x0e) {
			/* ��EQU ���� ���� */
			/* ������ �ν��̹Ƿ� ���� ������ ���ϰ� ���� */
			s = skip_expr(s);
			continue;
			/* ������ �̰��� ���� �ʴ´�(? ) */
		}
		if (c == 0x0f) {
			s += s[1] - (0x08 - 3); /* GLOBAL�� ��ȯ */
			continue;
		}
		if (c == 0x2c) {
			s = skip_expr(s);
			continue;
		}
		#if (DEBUG)
			if (c < 0x30)
				goto dberr0;
		#endif
		if (c < 0x3c) {
skipmc30:
			/* 30~37:��Ʈ DB(����, 0~7����Ʈ) */
			/* ���� DB~DD�� �д� */
			s = skip_mc30(s - 1, &c, 1);
			min += c;
			continue;
		}
		if (c == 0x58) {
			/* ORG */
			s = skip_expr(s);
			continue;
		}
		if (c == 0x59) {
			/* TIMES */
			t = s;
			i = s[0] | s[1] << 8 | s[2] << 16 | s[3] << 24;
			s += 4;
			calc_value(&value, &s); /* len */
			/* len�� ������ ���ٴ� ����(���α׷� ī���͸� �������� �ʴ� ������) */
			if ((unsigned int) i < 0xfffffff0) {
				min += (i - 1) * value.min; /* i�� �ݺ� ȸ�� */
				s = skip_expr(s);
				continue;
			}
			i = value.min;
			calc_value(&value, &s); /* �ݺ� ȸ�� */
			if (value.flags & (VFLG_SLFREF | VFLG_UNDEF)) /* �ڱ� ���� ���� */
				goto mc59_force;
			if (value.flags & VFLG_ERROR) {
				/* ��ġ �Ϲ� ���� */
				if (force >= 2)
					goto mc59_force;
				delta = INVALID_DELTA;
				unsolved++;
				continue;
			}
			min += i * (value.min - 1);
			if (value.delta == 0) {
				t[0] = value.min         & 0xff;
				t[1] = (value.min >>  8) & 0xff;
				t[2] = (value.min >> 16) & 0xff;
				t[3] = (value.min >> 24) & 0xff;
				continue;
			}
			if (force < 2) {
				delta += i * value.delta;
				unsolved++;
				continue;
			}
	mc59_force:
			t[0] = 0xfe; /* force */
		//	t[1] = 0xff;
		//	t[2] = 0xff;
		//	t[3] = 0xff;
			continue;
		}
		if (c == 0x68) {
			/* endian */
			s++;
			continue;
		}
		#if (DEBUG)
			if (c < 0x70)
				goto dberr0;
		#endif
		if (c < 0x78) {
			/* 0 x70~0x77:virtual byte ���� */
			vba = &vb[c & 0x07];
			s = skip_mc30(s, &vba->min, 1);
			vba->max = vba->min;
			continue;
		}
		if (c < 0x80) {
			/* 0 x78~0x7f:virtual byte ���� */
			vba = &vb[c & 0x07];
			min += vba->min;
			delta += vba->max - vba->min;
			continue;
		}
		#if (DEBUG)
			if (c < 0x90)
				goto dberr0;
		#endif
		if (c <= 0x91) {
			/* microcode90, microcode91 */
			/* [90 decision z-bbb-d-aaa expr] [3x (d, w)] [3x (b)] ([3x (z)]) */
			/* [91 decision z-bbb-d-aaa expr] [3x 3x (d, w)] [3x 3x (b)] ([3x 3x (z)]) */
			/* ���ǽ��� vba�� */
			/* decision�� 2����Ʈ. ���� ��ȣ�� bbbb-aaaa */
			d = c;
			c = s[2]; /* z-bbb-d-aaa */
			vba = &vb[c        & 0x07];
			vbb = &vb[(c >> 4) & 0x07];
			vbc = vbb + 1;
			if (d == 0x90)
				vbc = NULL;
			if (((e = s[0]) & 0x0f) ! = 0x0f) {
				/* �̹� �ذ� ���� */
				d = s[1];
				s = skip_expr(s + 3);
				vba->min = vba->max = d        & 0x0f;
				vbb->min = vbb->max = (d >> 4) & 0x0f;
			//	if (vbc)
			//		vbc->min = vbc->max = (e >> 4) & 0x0f;
				d = 2;
				if (c & 0x80)
					d = 3;
				if (vbc) {
					vbc->min = vbc->max = (e >> 4) & 0x0f;
					d *= 2;
				}
				do {
					s = skip_mc30(s, NULL, 1);
				} while (--d);
				continue;
			}
			t = s;
			s += 3;
			calc_value(&value, &s);
			d = 1 | 2 | 4; /* unknown */
			if ((value.flags & VFLG_ERROR) == 0) {
				i = value.min + value.delta; /* max */
				if (value.scale[0])
					d = 1; /* word(dword) */
				else if (value.min == 0 && i == 0)
					d = 4; /* zero */
				else if (-128 <= value.min && i <= 127) {
					d = 2; /* byte */
					if (value.min <= 0 && 0 <= i)
						d = 2 | 4; /* byte | zero */
				} else {
					d = 1; /* word(dword) */
					if (value.min <= 127 && -128 <= i) {
						d = 1 | 2; /* word | byte */
						if (value.min <= 0 && 0 <= i)
							d = 1 | 2 | 4; /* word | byte | zero */
					}
				}
			}
			if (force) {
				if (d & 1)
					d = 1;
				else
					d = 2;
			}
			if ((c & 0x80) == 0) { /* zero ���� */
				if ((d &= (1 | 2)) == 0)
					d = 2; /* byte */
			}
			vba->min = vbb->min =  127;
			vba->max = vbb->max = -128;
			if (vbc) {
				vbc->min =  127;
				vbc->max =  -128;
			}
			for (e = 1; e <= 4; e <<= 1) {
				if (e == 4 && (c & 0x80) == 0)
					break;
				s = skip_mc30(s, &f, 1);
				if (vbc)
					s = skip_mc30(s, &g, 1);
				if (d & e) {
					if (vbb->min > f)
						vbb->min = f;
					if (vbb->max < f)
						vbb->max = f;
					if (vbc) {
						if (vbc->min > g)
							vbc->min = g;
						if (vbc->max < g)
							vbc->max = g;
					}
					f = 2;
					if (c & 0x08)
						f = 4;
					if (e == 2)
						f = 1;
					if (e == 4)
						f = 0;
					if (vba->min > f)
						vba->min = f;
					if (vba->max < f)
						vba->max = f;
				}
			}
			if ((d & (d - 1)) == 0) {
				c = 0;
				if (d == 2)
					c = 1;
				if (d == 4)
					c = 2;
				t[0] = c;
				if (vbc)
					t[0] |= vbc->min << 4;
				t[1] = vba->min | vbb->min << 4;
			} else
				unsolved++;
			continue;
		}
		#if (DEBUG)
			if (c < 0x94)
				goto dberr0;
		#endif
		if (c <= 0x95) {
			/* microcode94, microcode95 */
			/* [94 decision 0-bbb-0-aaa expr] [len] [def-a def-b] ([expr def-a def-b] ...) */
			/* [95 decision 0-bbb-0-aaa expr] [len] [def-a def-b def-c] ([expr def-a def-b def-c] ...) */
			/* def-a �� 0 x98~0x9e (���� ���ǽ�) */
			/* len�� ���� 1(case ����), ó���� ������ ����, �ļ��� ��ġ�ϸ� ����, ��ġ�� ������ ������ ���İ� ��ȿ */
			d = c;
			c = s[2]; /* 0-bbb-0-aaa */
			vba = &vb[c        & 0x07];
			vbb = &vb[(c >> 4) & 0x07];
			vbc = vbb + 1;
			if (d == 0x94)
				vbc = NULL;
			if (((g = s[0]) & 0x0f) ! = 0x0f) {
				/* �̹� �ذ� ���� */
				d = s[1];
				s = skip_expr(s + 3);
				e = *s++; /* len */
				vba->min = vba->max = d        & 0x0f;
				vbb->min = vbb->max = (d >> 4) & 0x0f;
				if (vbc)
					vbc->min = vbc->max = (g >> 4) & 0x0f;
				s = skip_mc30(s, NULL, 0);
				s = skip_mc30(s, NULL, 1);
				if (vbc)
					s = skip_mc30(s, NULL, 1);
				while (--e) {
					s = skip_expr(s);
					s = skip_mc30(s, NULL, 0);
					s = skip_mc30(s, NULL, 1);
					if (vbc)
						s = skip_mc30(s, NULL, 1);
				}
				continue;
			}
			t = s;
			s += 3;
			calc_value(&value, &s);
			c = *s++; /* len */
			s = skip_mc30(s, &vba->min, 0);
			s = skip_mc30(s, &vbb->min, 1);
			vba->max = vba->min;
			vbb->max = vbb->min;
			if (vbc) {
				s = skip_mc30(s, &vbc->min, 1);
				vbc->max = vbc->min;
			}
			f = 0;
			if (force == 0) {
				if (value.delta ! = 0 || (value.flags & VFLG_ERROR) ! = 0)
					f = 0xff;
				for (g = 1; g < c; g++) {
					/* ��� ������ ���� ���Ѻ�����?  */
					/* �����Ƽ� ��ΰ� Ȯ���� ������ decide���� �ʴ´� */
					calc_value(&tmp, &s);
					if (tmp.delta ! = 0 || (tmp.flags & VFLG_ERROR) ! = 0)
						f = 0xff;
					s = skip_mc30(s, &d, 0);
					s = skip_mc30(s, &e, 1);
					if (vbc)
						s = skip_mc30(s, &h, 1);
					if (f ! = 0xff) {
						if ((value.min ^ tmp.min |
							value.scale[0] ^ tmp.scale[0] | value.label[0] ^ tmp.label[0] |
							value.scale[1] ^ tmp.scale[1] | value.label[1] ^ tmp.label[1]) == 0) {
							vba->min = vba->max = d;
							vbb->min = vbb->max = e;
							if (vbc)
								vbc->min = vbc->max = h;
							f = g;
						}
					} else {
						if (vba->min > d)
							vba->min = d;
						if (vba->max < d)
							vba->max = d;
						if (vbb->min > e)
							vbb->min = e;
						if (vbb->max < e)
							vbb->max = e;
						if (vbc) {
							if (vbb->min > h)
								vbb->min = h;
							if (vbb->max < h)
								vbb->max = h;
						}
					}
				}
			}
			if (f ! = 0xff) {
				t[0] = f;
				if (vbc)
					t[0] |= vbc->min << 4;
				t[1] = vba->min | vbb->min << 4;
			} else
				unsolved++;
			continue;
		}
		#if (DEBUG)
			if (c < 0x98)
				goto dberr0;
		#endif
		if (c < 0x9f)
			goto skipmc30;
		#if (DEBUG)
			if (c < 0xe0)
				goto dberr0;
		#endif
		if (c < 0xf0)
			continue; /* 1����Ʈ ����ũ */
		if (c < 0xf8) {
			/* n����Ʈ ����ũ */
			s += c - 0xef;
			continue;
		}
		#if (DEBUG)
dberr0:
			fprintf(stderr, "solve_subsect:%02X\n", c);
			GOL_sysabort(GO_TERM_BUGTRAP);
		#endif


/*
  [90(91�� ����)] [disp��] [ba] : a�� 0~7, b�� 0~7 : virtual byte ��ȣ : B=8�� �̻���� �ǹ��Ѵ�
  a�� bit3��, D��Ʈ. a�� ���ؼ���, ���� ����. ������ üũ�� 03 Ÿ��.
  [90] [disp��] [09] [31, 00-nnn-r/m] [31 01-nnn-r/m] [31 10-nnn-r/m]
  [bits 32]������ PUSH�� �̷���:[91] [imm��] [3a] [31 6a] [31 68]
  94�� ����� �������� ����(a�� �ڵ� ���Ǵ� ����).
  [94] [��] [ba] [case] [def0] [def1] [case] [def0] [def1] ...
  �� ���ϸ� ����
  [98] = [38 00 08 00] UCHAR
  [99] = [38 02 08 00] SCHAR
  [9a] = [39 00 08 00] USHORT
  [9b] = [39 03 08 00] non-overflow SHORT
  [9c] = [3b 00 08 00] UINT
  [9d] = [3b 03 08 00] non-overflow INT
  [9e] = [38 03 08 00] non-overflow CHAR




  ��70~77:������ ����Ʈ ����(�����󿡼��� 40~4b�� �ļ� �ϴ� ���� ���������, ����Ʈ�ϰ� ���� �ʴ�. ����Ǵ� ���� 30~3f)
  ��78~7f:������ ����Ʈ ����

  ��80~8f:LL�� ���� ó�������� ����Ѵ�
	80~83:variable ����(1~4����Ʈ)
	88~8f:sum(variable), (1~4, 1~4) : ó���� �׼�-1, ������ ������ ��ȣ

  ��90:0-B-else �ٿ���� if
  ��91:B-else �ٿ���� if
  ��92:0BWD �ٿ���� if
  ��93:else
  �������� ���� if
*/

	}
	if (delta > INVALID_DELTA)
		delta = INVALID_DELTA;
	subsect->min = min;
	subsect->delta = delta;
	subsect->unsolved = unsolved;

	return unsolved;
}

void calcsigma(struct STR_VALUE *value)
/* sigma�� ��� ������ ������ */
{
	int i, j, s, t, min;
	unsigned int delta;
	struct STR_SUBSECTION *subsect;
	for (i = 0; i < MAXSIGMAS; i++) {
		if ((s = value->sigma[i]. scale) == 0)
		//	continue;
			break;
		value->sigma[i]. scale = 0;
		if ((t = value->sigma[i]. terms) == 0)
			continue;
		value->sigma[i]. terms = 0;
		subsect = &subsect0[value->sigma[i]. subsect];
		value->sigma[i]. subsect = 0;
		min = delta = 0;
		do {
			min += subsect->min;
			delta += subsect->delta;
			if (delta > INVALID_DELTA)
				delta = INVALID_DELTA;
			subsect++;
		} while (--t);
		if (delta >= INVALID_DELTA) {
			value->delta = INVALID_DELTA;
		}
		if (value->delta >= INVALID_DELTA) {
			value->flags |= VFLG_ERROR;
			continue;
		}
		if (s > 0) {
			value->min += s * min;
			value->delta += s * delta;
		} else {
			value->min += s * (min + delta);
			value->delta -= s * delta;
		}
	}
	return;
}

void addsigma(struct STR_VALUE *value, struct STR_SIGMA sigma)
{
	int i, j;
retry:
	for (i = 0; i < MAXSIGMAS && value->sigma[i]. scale ! = 0; i++) {
		if (sigma.subsect + sigma.terms <= value->sigma[i]. subsect)
			break;
		if (value->sigma[i]. subsect + value->sigma[i]. terms <= sigma.subsect)
			continue;
	add:
		if (value->sigma[i]. subsect == sigma.subsect && value->sigma[i]. terms <= sigma.terms) {
	add2:
			sigma.terms -= value->sigma[i]. terms;
			sigma.subsect += value->sigma[i]. terms;
			if ((value->sigma[i]. scale += sigma.scale) == 0) {
	delete:
				for (j = i; j <= MAXSIGMAS - 2; j++)
					value->sigma[j] = value->sigma[j + 1];
				value->sigma[MAXSIGMAS - 1]. scale = 0;
				value->sigma[MAXSIGMAS - 1]. subsect = 0;
				value->sigma[MAXSIGMAS - 1]. terms = 0;
			}
	check:
			do {
				if (value->sigma[i]. scale ! = 0) {
					if (i > 0) {
						if (value->sigma[i - 1]. scale == value->sigma[i]. scale) {
							if (value->sigma[i - 1]. subsect + value->sigma[i - 1]. terms == value->sigma[i]. subsect) {
								value->sigma[i - 1]. terms += value->sigma[i]. terms;
								goto delete;
							}
						}
					}
				}
			} while (++i < MAXSIGMAS);
			if (sigma.terms > 0)
				goto retry;
			return;
		}
		if (value->sigma[MAXSIGMAS - 1]. scale ! = 0) {
			calcsigma(value);
			goto retry;
		}
		for (j = MAXSIGMAS - 2; i <= j; j--)
			value->sigma[j + 1] = value->sigma[j];
		if (value->sigma[i]. subsect == sigma.subsect) {
			value->sigma[i]. terms = sigma.terms;
			value->sigma[i + 1]. subsect += sigma.terms;
			value->sigma[i + 1]. terms -= sigma.terms;
			goto add2;
		}
#if 0
		if (sigma.subsect < value->sigma[i]. subsect) {
			value->sigma[i]. terms = j = value->sigma[i]. subsect - sigma.subsect;
			value->sigma[i]. subsect = sigma.subsect;
			value->sigma[i]. scale = sigma.scale;
			sigma.terms -= j;
			sigma.subsect += j;
			i++;
			value->sigma[i]. subsect = sigma.subsect;
			value->sigma[i]. terms -= j;
			goto add;
		}
	//	if (value->sigma[i]. subsect < sigma.subsect) {
			value->sigma[i]. terms = j = sigma.subsect - value->sigma[i]. subsect;
			i++;
			value->sigma[i]. subsect = sigma.subsect;
			value->sigma[i]. terms -= j;
			goto add;
	//	}
#endif
		if (sigma.subsect < value->sigma[i]. subsect) {
			value->sigma[i]. terms = j = value->sigma[i]. subsect - sigma.subsect;
			value->sigma[i]. subsect = sigma.subsect;
			value->sigma[i]. scale = sigma.scale;
			sigma.terms -= j;
			sigma.subsect += j;
		} else {
			value->sigma[i]. terms = j = sigma.subsect - value->sigma[i]. subsect;
		}
		i++;
		value->sigma[i]. subsect = sigma.subsect;
		value->sigma[i]. terms -= j;
		goto add;
	}
	if (value->sigma[MAXSIGMAS - 1]. scale ! = 0) {
		calcsigma(value);
		goto retry;
	}
	for (j = MAXSIGMAS - 2; i <= j; j--)
		value->sigma[j + 1] = value->sigma[j];
	value->sigma[i] = sigma;
	sigma.terms = 0;
	goto check;
}