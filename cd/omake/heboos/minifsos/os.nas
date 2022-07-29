; NASK�� �������. COM ������OS (ATȣȯ���� ����)
;		mini-FirstStepOS
;
; TAB = 4
; copyright(C) 2003 ī���� ������, KL-01
;
; .COM ������ ���α׷��� 64KB�� ���� �ʴ� �� ���� ����
; ��κ��� ����� ���������� OS�� 64KB�� �ѱ� ������ .COM�� 
; �����ϰ� ���� ���� ���̴�. �׷��� ���ʺ��� .EXE�� ��� ���� �ʿ�� ����.
; 64KB�� ������ �� ���� .EXEȭ�ϸ� ���� ������.
; .COM��� ���׸�Ʈ(segment)�� �Ű澲�� �ʾƵ� �Ǳ� ������ ���� ���̴�.

; prompt>nask os.nas os.com os.lst
; �� �����(assemble)�� �� �ֽ��ϴ�. nask�� tolset05 ���Ŀ� ���ԵǾ� �ֽ��ϴ�.
; tolset05�� http://www.imasy.orr/~kawai/osask/developers.html�� �ֽ��ϴ�.

[FORMAT "BIN"]
[INSTRSET "i386"]
[OPTIMIZE 1]
[OPTION 1]
[BITS 16]
		ORG		0x0100 ; .COM�� �ݵ�� ORG 0x0100�� ���۵ȴ�

; �׷��� ������ �ұ�. �켱 320x200x256�����ε� �� �׶��̼�(gradation)�� 
; ǥ���ϸ� ��ƺ���? ("hello"������ �ʹ� ����ϴϱ�)

;	ȭ�� ���� �ȷ�Ʈ ����

			MOV 	AX, 0x0013
			INT		0x10
			XOR		BX, BX
PALLOP:
			MOV		AX, 0x1010
			MOV		DH, BL
			MOV		CH, BL
			MOV		CL, 0
			PUSH	BX
			INT		0x10
			POP		BX
			INC		BX
			CMP		BX, 0x3f
			JBE		PALLOP

			MOV		AX, 0x1010
			MOV		BL, 255
			MOV		DH, 0x3f
			MOV		CX, 0x3f3f
			INT		0x10

;	VRAM�� WRITE

			PUSH	DS
			MOV		AX, 0xa000
			MOV		DS, AX
			XOR		BX, BX

;	���� 8������ Ŭ����

			XOR		AX, AX
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
			CMP		AL, 0x3f
			JBE		GRALOP0

			POP		DS

			JMP		$	; ��(���� ��ư�� ��������)
