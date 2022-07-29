; NASK�� ���� OS���� IPL(ATȣȯ������)
; TAB = 4
; copyright(C) 2003 ī���� ������, KL-01
; OS�� 512����Ʈ �̸��̶�� ����
; OS�� �Ǹ��� 0, ��� 0, ���� 2�� �� �ִٰ� ����

;	prompt>nask ipl.nas ipl.bin ipl.lst
; �� �����(assemble) �� �� �ֽ��ϴ�. nask�� tolset05 ���Ŀ� ���ԵǾ� �ֽ��ϴ�.
; tolset05�� http://www.imasy.orr/~kawai/osask/developers.html �� �ֽ��ϴ�.

; ��ũ �̹����� ������� ������ ���� �մϴ�.
;	prompt>copy /B ipl.bin+os.com fdimage.bin

; �̰��� ��ũ�� write�Ϸ��� ������ �����ϴ�.
; �켱 ������ ���� ��ũ�� A:�� �ִ´�.
;	prompt>imgtol w a: fdimage.bin
; imgtol�� tolset05���Ŀ� ���ԵǾ� �ֽ��ϴ�.


[FORMAT "BIN"]
[INSTRSET "i386"]
[OPTIMIZE 1]
[OPTION 1]
[BITS 16]
			ORG		0x7c00

			JMP		START
			NOP
			DB		'hogehoge'

; FAT�� �غ��ϰ� ���� �ʱ� ������ BPB�� ����� FAT12��� �� �д�
; �̰��� ���ָ�, ��ũ�� ���� ���� ������ imgtol�� read, write�ϴ� ���� �Ұ����ϹǷ�
; ��¿ �� ���� �־� �д�

			DW		512 	; ���� ����(����Ʈ ����)
			DB		1	; Ŭ������ ����(���� ����)
			DW		1	; boot sector ����(���� ����)
			DB		2	; FAT�� ��
			DW		0x00e0	; root directory entries.
			DW		2880	; �� ���ͼ�
			DB		0xf0	; media descriptor byte.
			DW		9	; FAT�� ����(���� ����)
			DW		18	; 1Ʈ���� ���Ե� ���ͼ�
			DW		2	; ����
			DD		0	; ���� ������ �ʴ� ���ͼ�
			DD		2880	; �� ���ͼ�
			DB		0, 0	; /* unknown */
			DB		0x29
			DD		0xffffffff
			DB		"hogehoge   "
			DB		"FAT12   "

; �� ��ũ�� DOS�� Win������ read, write�� �� ���� ������ �� ��Ź�帳�ϴ�

START:
			MOV		AX, 0x0201
			MOV		CX, 0x0002
			XOR		DX, DX
			MOV		BX, 0x0800
			MOV		ES, BX
			MOV		BX, 0x0100
			INT		0x13
			JC		ERR

; .COM ���Ϸ� DOSȣȯ�� �������� ���·� �� �ش�

			MOV		AX, 0x0800
			MOV		SS, AX
			MOV		SP, 0xfffe
			MOV		DS, AX
			MOV		ES, AX
			JMP		0x0800:0x0100
ERR:
			INT		0x18	; ROM-BASIC�� (����)

			RESB	0x7dfe-$	; 0x7dfe���� 0x00�� ä���

			DB		0x55, 0xaa
			