; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; ������Ʈ ������ ����� ���	
[BITS 32]					; 32 ��Ʈ ������ ��� �����


; ������Ʈ ������ ���� ����

[FILE "naskfunc.nas"]				; ���� ���ϸ� ����

		GLOBAL	_io_hlt			; �� ���α׷��� ���ԵǴ� �Լ���


; ���ϴ� ������ �Լ�

[SECTION .text]				; ������Ʈ ���Ͽ����� �̰��� ���� ���� ���α׷��� ����

_io_hlt:	; void io_hlt(void);
		HLT
		RET
