[INSTRSET "i486p"]
[BITS 32]
		MOV		EAX,1*8			; OS���� ���׸�Ʈ(segment) ��ȣ
		MOV		DS, AX			; �̰��� DS�� �־������
		MOV		BYTE [0x102600],0
		RETF
