/* Ÿ�̸� ���� */

#include "bootpack.h"

#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040

struct TIMERCTL timerctl;

#define TIMER_FLAGS_ALLOC		1	/* Ȯ���� ���� */
#define TIMER_FLAGS_USING		2	/* Ÿ�̸� �۵��� */

void init_pit(void)
{
	int i;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	timerctl.next = 0xffffffff; /* ó������ �۵����� Ÿ�̸Ӱ� ���� ������ */
	timerctl.using = 0;
	for (i = 0; i < MAX_TIMER; i++) {
		timerctl.timers0[i].flags = 0; /* �̻�� */
	}
	return;
}

struct TIMER *timer_alloc(void)
{
	int i;
	for (i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timers0[i].flags == 0) {
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timers0[i];
		}
	}
	return 0; /* �߰ߵ��� �ʾҴ� */
}

void timer_free(struct TIMER *timer)
{
	timer->flags = 0; /* �̻�� */
	return;
}

void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	int e;
	struct TIMER *t, *s;
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	timerctl.using++;
	if (timerctl.using == 1) {
		/* ������ Ÿ�̸Ӱ� 1���� ��� */
		timerctl.t0 = timer;
		timer->next = 0; /* ������ ���� */
		timerctl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}
	t = timerctl.t0;
	if (timer->timeout <= t->timeout) {
		/* ���ο� �� �� �ִ� ��� */
		timerctl.t0 = timer;
		timer->next = t; /* ������ t */
		timerctl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}
	/* ��� ���� ���� ã�´� */
	for (;;) {
		s = t;
		t = t->next;
		if (t == 0) {
			break; /* ���� �ڰ� �Ǿ��� */
		}
		if (timer->timeout <= t->timeout) {
			/* s�� t�� ���̿� �� �� �ִ� ��� */
			s->next = timer; /* s�� ������ timer */
			timer->next = t; /* timer�� ������ t */
			io_store_eflags(e);
			return;
		}
	}
	/* ���� �ڿ� �� �� �ִ� ��� */
	s->next = timer;
	timer->next = 0;
	io_store_eflags(e);
	return;
}

void inthandler20(int *esp)
{
	int i;
	struct TIMER *timer;
	io_out8(PIC0_OCW2, 0x60);	/* IRQ-00 ���� �ϷḦ PIC�� ���� */
	timerctl.count++;
	if (timerctl.next > timerctl.count) {
		return;
	}
	timer = timerctl.t0; /* �켱 ������ ������ timer�� ���� */
	for (i = 0; i < timerctl.using; i++) {
		/* timers�� Ÿ�̸Ӵ� ��� �������� ���̹Ƿ�, flags�� Ȯ������ �ʴ´� */
		if (timer->timeout > timerctl.count) {
			break;
		}
		/* Ÿ�� �ƿ� */
		timer->flags = TIMER_FLAGS_ALLOC;
		fifo32_put(timer->fifo, timer->data);
		timer = timer->next; /* ���� Ÿ�̸��� ������ timer�� ���� */
	}
	timerctl.using -= i;

	/* ���Ӱ� �Ű� ���� */
	timerctl.t0 = timer;

	/* timerctl.next�� ���� */
	if (timerctl.using > 0) {
		timerctl.next = timerctl.t0->timeout;
	} else {
		timerctl.next = 0xffffffff;
	}
	return;
}
