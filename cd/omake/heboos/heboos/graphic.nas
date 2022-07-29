; OS�� ���ø� "GRAPHIC.NAS"
; TAB = 4
; copyright(C) 2003 ī���� ������, KL-01

; prompt>nask graphic.nas graphic.hoa graphic.lst
; �� �����(assemble)�� �� �ֽ��ϴ�. nask�� tolset05 ���Ŀ� ���ԵǾ� �ֽ��ϴ�.
; tolset05�� http://www.imasy.orr/~kawai/osask/developers.html �� �ֽ��ϴ�.

[FORMAT "BIN"]
[INSTRSET "i386"]
[OPTIMIZE 1]
[OPTION 1]
[BITS 16]
			ORG		0x0100

;	�ȷ�Ʈ�� ����
;	0-15�� �ý��� �ȷ�Ʈ�� 16���Ĵ� �����Ӱ� ����� �� �ְ� �߽��ϴ�. (����)

			MOV		BX, 64
PALLOP:
			MOV		AX, 0x1010
			MOV		DH, BL
			AND		DH, 0x3f
			MOV		CH, DH
			MOV		CL, 0
			PUSH	BX
			INT		0x10
			POP		BX
			INC		BX
			CMP		BX, 0x7f
			JBE		PALLOP

;	VRAM�� write

			PUSH	DS
			MOV		AX, 0xa000
			MOV		DS, AX
			XOR		BX, BX

;	���� 8������ Ŭ����

			MOV		AX, 0x4040
CLR8LOP:
			MOV		[BX], AX
			ADD		BX, 2
			CMP		BX,320*8
			JB		CLR8LOP

;	�׶��̼�(gradation) ����

GRALOP0:
			MOV		CX,320*3/2
GRALOP1:
			MOV		[BX], AX
			ADD		BX, 2
			DEC		CX
			JNZ		GRALOP1
			ADD		AX, 0x0101
			CMP		AL, 0x7f
			JBE		GRALOP0

			POP		DS

;	�����ΰ� Ű�� ���� �ش�

			MOV		AH, 0x00
			INT		0x16

;	ȭ���� Ŭ�����ϰ� OS�� ���ƿ´�

			MOV		AX, 2
			INT		0x80
			XOR		AX, AX
			XOR		CX, CX
			INT		0x80
