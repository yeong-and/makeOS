; OS�� ���ø� "HELLO.NAS"
; TAB = 4
; copyright(C) 2003 ī���� ������, KL-01

;	prompt>nask hello.nas hello.hoa hello.lst
; �� �����(assemble) �� �� �ֽ��ϴ�. nask�� tolset05���Ŀ� ���ԵǾ� �ֽ��ϴ�.
; tolset05�� http://www.imasy.orr/~kawai/osask/developers.html�� �ֽ��ϴ�.

[FORMAT "BIN"]
[INSTRSET "i386"]
[OPTIMIZE 1]
[OPTION 1]
[BITS 16]
			ORG		0x0100

			MOV		AX, 1
			MOV		SI, msg
			INT		0x80
			XOR		AX, AX
			XOR		CX, CX
			INT		0x80
msg:
			DB		"hello, world", 10,0
