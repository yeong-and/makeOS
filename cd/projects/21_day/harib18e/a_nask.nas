[FORMAT "WCOFF"]				; ������Ʈ ������ ����� ���	
[INSTRSET "i486p"]				; 486��ɱ��� ����ϰ� �ʹٰ� �ϴ� ���
[BITS 32]					; 32��Ʈ ������ ��� ����� �Ѵ�
[FILE "a_nask.nas"]				; ���� ���ϸ� ����

		GLOBAL	_api_putchar

[SECTION .text]

_api_putchar:	; void api_putchar(int c);
		MOV		EDX,1
		MOV		AL,[ESP+4]	; c
		INT		0x40
		RET
