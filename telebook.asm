;��	
int21	macro functn			;DOSϵͳ���ܵ���
		mov   ah,functn
		int   21h
		endm
crlf	macro				;�س�����
		push ax
		push dx
		mov dl,0ah
		int21 02h
		mov dl,0dh
		int21 02h
		pop dx
		pop ax
		endm
printString macro addstring
	lea dx,addstring
	int21 09h
	endm
savereg macro
	push ax
	push bx
	push cx
	push dx
	push si
	push di
	push ds
	endm
recoverreg macro
	pop ds
	pop di
	pop si
	pop dx
	pop cx
	pop bx
	pop ax
	endm
clearBuffer macro
	local cls
	mov byte ptr [inname+1],0
	mov bx,2
	mov cx,8
cls:
	mov byte ptr [inname+bx],' '
	inc bx
	loop cls
	endm
getchar macro
	;crlf
	int21 01h
	;int21 01h
	endm
;�����	

data segment
	inname db 8,0,8 dup(0)	;���������ַ��������Ķ���
	TelTable db 10 dup(8,0,8 dup(0),11,0,11 dup(0))
		db '00'	;�������0��֤���Ƿ��Ѿ�������		
	;�˵�
	 menu db 9,9,9,'|______menu______|',10,13
	      db 9,9,9,'|__0.AddTelTable_|',10,13
	      db 9,9,9,'|__1.AddNumber___|',10,13
	      db 9,9,9,'|__2.Search______|',10,13
	      db 9,9,9,'|__3.PrintAll____|',10,13
	      db 9,9,9,'|__4.Modify______|',10,13
	      db 9,9,9,'|__5.Delete______|',10,13
	      db 9,9,9,'|__6.Exit________|',10,13,'$'
	tips  db 10,13,'Choose one from the menu:','$'
	tips1 db 'Input name:',10,13,'$'
	tips2 db 'Each character of this name must be a letter!',10,13
		  db 'Enter name again:',10,13,'$'
	tips3 db 'TelTable is empty!',10,13,'$'
	tips4 db 'There is no such person in the telephone table.',10,13,'$'
	tips5 db 'The contact already exists.',10,13
	      db 'You can choose to modify or delete the contact.',10,13,'$'
	tips6 db 'Input his(or her) telephone number:',10,13,'$'
	tips7 db 'Each character of this Telephone number must be a digit!',10,13
	      db 'Input it again:',10,13,'$'
	tablehead db 'Contacts : Telephone number',10,13,'$'
	space db ' : ','$'
	modifytips db 'Please input new telephone number:',10,13,'$'
	choosemenu db 'Please choose number from the menu!',10,13,'$'
	continueAdd db 'Do you want to continue adding contacts?(y:yes,other:quit)',10,13,'$'
data ends

stack segment
	db 100 dup(?)
stack ends

code segment
assume cs:code,ds:data,ss:stack	
start:
;������------------------------------------------------------------
main proc far
	mov ax,data
	mov ds,ax
	mov ax,stack
	mov ss,ax
prompt: 
	;call ClearScreen	;���������ӳ���
	mov ax,3h	;�ٶ��ѵ�����������������
	int 10h
	printString menu
	printString tips	;��ʾѡ��˵���ѡ��
	int21 01h	;ѡ��˵��е�ѡ��
	crlf
	sub al,30h
	cmp al,0
	jz zero
	cmp al,1	;�ж�ѡ���ѡ��
	jz one
	cmp al,2
	jz two
	cmp al,3
	jz three
	cmp al,4
	jz four
	cmp al,5
	jz five
	cmp al,6
	jz quit
	printString choosemenu
	getchar
	jmp prompt
	;���ݲ˵�������Ӧ���ܵĺ���
zero:
	call AddTelTable
	getchar
	jmp prompt
one:
	call Addnumber
	getchar	
	jmp prompt
two:
	call Search
	getchar
	jmp prompt
three:
	call PrintAll
	getchar
	jmp prompt
four:
	call Modify
	getchar
	jmp prompt
five:
	call Delete
	getchar
	jmp prompt
quit:
	int21 4ch
main endp

;�����ӳ���(�������ƣ�����Ϥ�����ÿ��ƣ�-------------------------------------------------------------
ClearScreen proc near
	savereg
	mov ax,0b800h
	mov ds,ax	;���Դ������ȫ��д�ɿո�ﵽ����Ч��	
	mov cx,32h
	mov bx,0
spaceNum:
	add bx,0a0h
	loop spaceNum
	mov cx,bx
	mov bx,0
cls:	
	mov byte ptr ds:[bx],20h
	add bx,2
	cmp bl,0
	loop cls
	recoverreg
	ret

ClearScreen endp	

;�����ϵ���ӳ���------------------------------------------------------------
Addnumber proc near
	printString tips1	;��ʾ��������
	call I_name
	crlf
	call InsertToTable
	ret
Addnumber endp
	
;���������ӳ���------------------------------------------------------------+
I_name proc near
reInputName:
	clearBuffer
	mov bx,2
inputALetter:
	int21 01h
	cmp al,0dh
	jz return
isLetter:		;�ж�����������Ƿ�����ĸ
	cmp al,'A'
	jb reinput
	cmp al,'Z'
	jle next
	cmp al,'a'
	jb reinput
	cmp al,'z'
	jle next
	jg reinput
next:			;��ǰ�жϵ��ַ�����ĸ���ж���һ��
	mov byte ptr [inname+bx],al
	inc byte ptr [inname+1]
	inc bx
	jmp inputALetter
return:
	ret
reinput:
	printString tips2
	jmp reInputName
I_name endp

;��λ�ӳ���(��գ�CX=1��û�ҵ���CX=2���ҵ���CX=3��------------------------------------------------------------+
Locate proc near
	crlf
	mov bx,0
	cmp byte ptr TelTable[bx+1],0	
	jz TabNull			
cmpOneName:
	mov si,2
	mov dh,0
	mov dl,byte ptr inname[si]
	cmp byte ptr TelTable[bx+si],dl	;�ж�����ĸ�Ƿ���ͬ
	jnz nextinfo
	mov ch,0
	mov cl,byte ptr TelTable[bx+1]	;ȡ�ı������ֳ���Ϊѭ��������ѭ��
	dec cx
	cmpEveryLetter:
		inc si
		mov dl,byte ptr inname[si]
		cmp byte ptr TelTable[bx+si],dl
		jnz nextinfo
		loop cmpEveryLetter
	mov dl,byte ptr TelTable[bx+1]	;�жϻ������������Ƿ���������������Ѿ��ж���ɣ��ж�
	cmp byte ptr [inname+1],dl
	jnz nextinfo
	mov cx,3	;�ҵ�������
	ret
nextinfo:
	add bx,17h
	cmp byte ptr TelTable[bx+1],0
	jz tipNot			;������;���жϱ����һ����¼Ϊ0����˵�����Ѿ������꣬û�ҵ�
	jmp cmpOneName
TabNull:			;TelTableΪ��
	crlf
	mov cx,1
	ret
tipNot:				;TelTable��û�������
	mov cx,2
	ret
Locate endp

;�������������ϵ���ӳ���
InsertToTable proc near
	mov bx,0
	cmp byte ptr TelTable[bx+1],0
	jz CopyName	;��Ϊ��
cmpName:
	mov si,2
	mov dh,0
	mov dl,byte ptr inname[si]
	cmp byte ptr TelTable[bx+si],dl	;�ж�����ĸ�Ƿ���ͬ
	jg callMove	;��ǰ������ĸ����ĸ�ϴ󣬻���������Ӧ�ò��뵽��ǰλ��	
	jb nextName	
	mov ch,0
	mov cl,byte ptr TelTable[bx+1]	;ȡ�ı������ֳ�����ѭ��
	dec cx
	cmpEachLetter:
		inc si
		mov dl,byte ptr inname[si]
		cmp byte ptr TelTable[bx+si],dl
		jg callMove
		jb nextName
		loop cmpEachLetter
		mov dl,byte ptr [inname+1];���е�����ÿ���ַ����Ƚ����ˣ�˵�������������ָ���
		cmp dl,byte ptr TelTable[bx+1]
		jg nextName
		printString tips5
		ret
nextName:
	add bx,17h
	cmp byte ptr TelTable[bx+1],0
	jz CopyName			;������;���жϱ����һ����¼Ϊ0����˵�����Ѿ������꣬���������ֵ�����ĸ���Ӧ�������
	jmp cmpName
	
callMove:
	call Move 
CopyName:
	mov di,1
	mov ch,0
	mov cl,9
	mov byte ptr TelTable[bx],8	;��Ϊmove��������һ��8Ҳ���0��
	copy:
		mov dl,byte ptr [inname+di]
		mov byte ptr [inname+di],0	;Ϊ������һ�������������ĵ�һ������8������di��1��ʼ
		mov byte ptr TelTable[bx+di],dl
		inc di
		loop copy
	printString tips6
	call I_TelNo
	ret
InsertToTable endp

;�ƶ��绰������ϵ����Ϣ�ӳ���(����֮ǰ��bx��ֵ����di�Ĵ�����)------------------------------------------------------------
Move proc near
	mov si,bx
	mov di,si
	add di,17h
movepoint:
	cmp byte ptr TelTable[di+1],0
	jz preparemove
	add si,17h
	add di,17h
	jmp movepoint
preparemove:
	mov bp,0
	mov cx,17h
moveinfo:
	mov dl,byte ptr TelTable[si+bp]	
	mov byte ptr TelTable[si+bp],0	;�����е����ж�����Ϊ0
	mov byte ptr TelTable[di+bp],dl
	inc bp
	loop moveinfo
	cmp si,bx
	jz returnInsert
	sub si,17h
	sub di,17h
	jmp preparemove
returnInsert:
	ret
Move endp

;����绰�����ӳ���-----------------------------------------------------
I_TelNo proc near
	mov byte ptr TelTable[bx+0ah],0bh
inputNumber:
	mov si,0bh
inputNo:
	inc si
	int21 01h
	cmp al,0dh
	jz finAndret
	cmp al,'0'
	jb reInputNum
	cmp al,'9'
	jg reInputNum
	mov byte ptr TelTable[bx+si],al
	inc byte ptr TelTable[bx+0bh]
	jmp inputNo
finAndret:
	crlf
	ret
reInputNum:
	printString tips7
	jmp inputNumber
I_TelNo endp
	
;�����ӳ��򣨸���������ܣ�------------------------------------------------------------+
Search proc near
	printString tips1	;��ʾ��������
	call I_name
	call Locate
	cmp cx,1		;���ݶ�λ�ӳ���洢��cx�еķ���ֵ�������Ӧ�Ĳ��ҽ��
	jz printTabNull
	cmp cx,2
	jz printNot
	cmp cx,3
	jz callprint
printTabNull:		;���TebTableΪ��
	printString tips3
	ret
printNot:			;���û�ҵ�����ϵ��
	printString tips4
	ret
callprint:
	printString tablehead
	push cx
	call printinfo	;printinfo�ӳ�����޸�cx
	pop cx
	crlf
	ret
Search endp

;����绰���ӳ���------------------------------------------------------------?
PrintAll proc near
	crlf
	mov bx,0
	cmp byte ptr TelTable[bx+1],0	;�ж�TelTable�Ƿ�Ϊ��
	jz  printtips
	printString tablehead
printNextInfo:
	call printinfo
	crlf
	add bx,17h	;ָ����һ����Ϣ
	cmp byte ptr TelTable[bx+1],0	;���ﲻ����[bx]��8�ȣ�û������ݵ�һ��Ҳ��8.����������������
	jz  FinishPrint
	jmp printNextInfo
printtips:			;��ʾTelTableΪ��
	printString tips3
FinishPrint:			;�������
	ret
PrintAll endp

;�޸���ϵ���ӳ���----------------------------------------------------------
Modify proc near 
	call Search
	cmp cx,3
	jnz cantModify
	printString modifytips
	mov byte ptr TelTable[bx+0bh],0	;���볤�ȼ��������㣬�Ա��޸ĳɲ�ͬ���ȵĺ���
	call I_TelNo
cantModify:
	ret
Modify endp

;ɾ����ϵ���ӳ���--------------------------------------------------------------
Delete proc near
	call Search
	cmp cx,3
	jnz cantDelete
	mov di,bx
	mov si,di
	add si,17h
preparemoveup:
	mov bp,0
	mov cx,17h
moveup:
	mov dl,byte ptr TelTable[si+bp]	
	mov byte ptr TelTable[si+bp],0	;����һ�е����ж�����Ϊ0
	mov byte ptr TelTable[di+bp],dl
	inc bp
	loop moveup
	add si,17h
	add di,17h
	cmp byte ptr TelTable[si+1],0
	jnz preparemoveup
cantDelete:
	ret	;�������֮�����һ�л���û�����ݣ���һ�б�����û������
Delete endp
;���һ����ϵ����������绰����---------------------------------------------+
printinfo proc near
	mov ch,0							;�������
	mov cl,byte ptr TelTable[bx+1]	
	mov si,2
	printname:
		mov dl,byte ptr TelTable[bx+si]
		int21 02h
		inc si
		loop printname
	printString space
	mov ch,0							;����绰����
	mov cl,ds:byte ptr TelTable[bx+0bh]	
	mov si,0ch
	printTel:
		mov dl,byte ptr TelTable[bx+si]
		int21 02h
		inc si
		loop printTel
	ret
printinfo endp

;ͨѶ¼�ӳ���-------------------------------------------------------
AddTelTable proc near
addAnother:
	call Addnumber
	printString continueAdd
	int21 01h
	cmp al,'y'
	jz yes
	ret
yes:
	crlf
	jmp addAnother
AddTelTable endp
code ends
end start
