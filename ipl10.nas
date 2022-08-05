; haribote-ipl
; TAB=4

CYLS	EQU		10

		ORG		0x7c00
		
; standard FAT12 format floppy disk

		JMP		entry
		DB		0x90
		DB		"HARIBOTE"
		DW		512
		DB		1
		DW		1
		DB		2
		DW		224
		DW		2880
		DB		0xf0
		DW		9
		DW		18
		DW		2
		DD		0
		DD		2880
		DB		0,0,0x29
		DD		0xffffffff
		DB		"HARIBOTEOS	"
		DB		"FAT12	"
		RESB	18
		
entry:
		MOV		AX,0
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX

		MOV		AX,0x0820
		MOV		ES,AX
		MOV		CH,0			; cylinder 0
		MOV		DH,0			; head 0
		MOV		CL,2			; sector 2

readloop:
		MOV		SI, 0			; register that counts number of failure

retry:
		MOV		AH,0x02			; AH=0x02 disk read
		MOV		AL,1			; sector 1
		MOV		BX,0
		MOV		DL,0x00			; A drive
		INT		0x13			; call disk BIOS
		JNC		next			; (jump if not carry) goto next
		ADD		SI,1			; SI += 1
		CMP		SI,5			; cmpare SI to 5
		JAE		error			; (jump if above or equal) SI to 5 goto error
		MOV		AH,0x00
		MOV		DL,0x00			; A drive
		INT		0x13			; drive reset
		JMP		retry

next:
		MOV		AX,ES			; address += 0x200 (0x0020 * 16), ES = 16bit segment register
		ADD		AX,0X0020
		MOV		ES,AX			; cannot use command like 'ADD ES, 0x020', so split to 3step
		ADD		CL,1
		CMP		CL,18
		JBE		readloop		; (jump if below or equal) CL To 18 goto readloop
		MOV		CL,1
		ADD		DH,1
		CMP		DH,2
		JB		readloop		; if DH < 2 goto readloop
		MOV		DH,0
		ADD		CH,1
		CMP		CH,CYLS
		JB		readloop		; if CH < CYLS goto readloop

		MOV		[0x0ff0],CH
		JMP		0xc200

error:
		MOV		AX,0
		MOV		ES,AX
		MOV		SI,msg

putloop:
		MOV		AL,[SI]
		ADD		SI,1
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e
		MOV		BX,15
		INT		0x10
		JMP		putloop

fin:
		HLT
		JMP		fin	

msg:
		DB		0x0a,0x0a
		DB		"load error"
		DB		0x0a
		DB		0

		RESB	0x7dfe-$

		DB		0x55,0xaa
