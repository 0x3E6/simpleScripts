;宏	
int21	macro functn			;DOS系统功能调用
		mov   ah,functn
		int   21h
		endm
crlf	macro				;回车换行
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
;宏结束	

data segment
	inname db 8,0,8 dup(0)	;姓名输入字符缓冲区的定义
	TelTable db 10 dup(8,0,8 dup(0),11,0,11 dup(0))
		db '00'	;最后两个0验证表是否已经遍历完		
	;菜单
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
;主程序------------------------------------------------------------
main proc far
	mov ax,data
	mov ds,ax
	mov ax,stack
	mov ss,ax
prompt: 
	;call ClearScreen	;调用清屏子程序
	mov ax,3h	;百度搜的清屏代码清屏代码
	int 10h
	printString menu
	printString tips	;提示选择菜单中选项
	int21 01h	;选择菜单中的选项
	crlf
	sub al,30h
	cmp al,0
	jz zero
	cmp al,1	;判断选择的选项
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
	;根据菜单调用相应功能的函数
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

;清屏子程序(并不完善，不熟悉，不好控制）-------------------------------------------------------------
ClearScreen proc near
	savereg
	mov ax,0b800h
	mov ds,ax	;将显存的数据全部写成空格达到清屏效果	
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

;添加联系人子程序------------------------------------------------------------
Addnumber proc near
	printString tips1	;提示输入姓名
	call I_name
	crlf
	call InsertToTable
	ret
Addnumber endp
	
;输入名字子程序------------------------------------------------------------+
I_name proc near
reInputName:
	clearBuffer
	mov bx,2
inputALetter:
	int21 01h
	cmp al,0dh
	jz return
isLetter:		;判断输入的名字是否都是字母
	cmp al,'A'
	jb reinput
	cmp al,'Z'
	jle next
	cmp al,'a'
	jb reinput
	cmp al,'z'
	jle next
	jg reinput
next:			;当前判断得字符是字母，判断下一个
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

;定位子程序(表空，CX=1；没找到，CX=2；找到，CX=3）------------------------------------------------------------+
Locate proc near
	crlf
	mov bx,0
	cmp byte ptr TelTable[bx+1],0	
	jz TabNull			
cmpOneName:
	mov si,2
	mov dh,0
	mov dl,byte ptr inname[si]
	cmp byte ptr TelTable[bx+si],dl	;判断首字母是否相同
	jnz nextinfo
	mov ch,0
	mov cl,byte ptr TelTable[bx+1]	;取的表中名字长度为循环次数做循环
	dec cx
	cmpEveryLetter:
		inc si
		mov dl,byte ptr inname[si]
		cmp byte ptr TelTable[bx+si],dl
		jnz nextinfo
		loop cmpEveryLetter
	mov dl,byte ptr TelTable[bx+1]	;判断缓冲区的名字是否更长，表中名字已经判断完成，判断
	cmp byte ptr [inname+1],dl
	jnz nextinfo
	mov cx,3	;找到该姓名
	ret
nextinfo:
	add bx,17h
	cmp byte ptr TelTable[bx+1],0
	jz tipNot			;查找中途，判断表的下一条记录为0，则说明表已经遍历完，没找到
	jmp cmpOneName
TabNull:			;TelTable为空
	crlf
	mov cx,1
	ret
tipNot:				;TelTable中没有这个人
	mov cx,2
	ret
Locate endp

;插入排序添加联系人子程序
InsertToTable proc near
	mov bx,0
	cmp byte ptr TelTable[bx+1],0
	jz CopyName	;表为空
cmpName:
	mov si,2
	mov dh,0
	mov dl,byte ptr inname[si]
	cmp byte ptr TelTable[bx+si],dl	;判断首字母是否相同
	jg callMove	;当前表中字母首字母较大，缓冲区名字应该插入到当前位置	
	jb nextName	
	mov ch,0
	mov cl,byte ptr TelTable[bx+1]	;取的表中名字长度做循环
	dec cx
	cmpEachLetter:
		inc si
		mov dl,byte ptr inname[si]
		cmp byte ptr TelTable[bx+si],dl
		jg callMove
		jb nextName
		loop cmpEachLetter
		mov dl,byte ptr [inname+1];表中的名字每个字符都比较完了，说明缓冲区的名字更长
		cmp dl,byte ptr TelTable[bx+1]
		jg nextName
		printString tips5
		ret
nextName:
	add bx,17h
	cmp byte ptr TelTable[bx+1],0
	jz CopyName			;查找中途，判断表的下一条记录为0，则说明表已经遍历完，缓冲区名字的首字母最大，应放在最后
	jmp cmpName
	
callMove:
	call Move 
CopyName:
	mov di,1
	mov ch,0
	mov cl,9
	mov byte ptr TelTable[bx],8	;因为move函数将第一个8也设成0了
	copy:
		mov dl,byte ptr [inname+di]
		mov byte ptr [inname+di],0	;为了在这一步保留缓冲区的第一个数字8，所以di从1开始
		mov byte ptr TelTable[bx+di],dl
		inc di
		loop copy
	printString tips6
	call I_TelNo
	ret
InsertToTable endp

;移动电话本中联系人信息子程序(调用之前，bx的值存在di寄存器中)------------------------------------------------------------
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
	mov byte ptr TelTable[si+bp],0	;将本行的所有都设置为0
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

;输入电话号码子程序-----------------------------------------------------
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
	
;查找子程序（附带输出功能）------------------------------------------------------------+
Search proc near
	printString tips1	;提示输入姓名
	call I_name
	call Locate
	cmp cx,1		;根据定位子程序存储在cx中的返回值，输出相应的查找结果
	jz printTabNull
	cmp cx,2
	jz printNot
	cmp cx,3
	jz callprint
printTabNull:		;输出TebTable为空
	printString tips3
	ret
printNot:			;输出没找到该联系人
	printString tips4
	ret
callprint:
	printString tablehead
	push cx
	call printinfo	;printinfo子程序会修改cx
	pop cx
	crlf
	ret
Search endp

;输出电话本子程序------------------------------------------------------------?
PrintAll proc near
	crlf
	mov bx,0
	cmp byte ptr TelTable[bx+1],0	;判断TelTable是否为空
	jz  printtips
	printString tablehead
printNextInfo:
	call printinfo
	crlf
	add bx,17h	;指向下一条信息
	cmp byte ptr TelTable[bx+1],0	;这里不能用[bx]与8比，没存的数据第一个也是8.。。。。。。。。
	jz  FinishPrint
	jmp printNextInfo
printtips:			;提示TelTable为空
	printString tips3
FinishPrint:			;结束输出
	ret
PrintAll endp

;修改联系人子程序----------------------------------------------------------
Modify proc near 
	call Search
	cmp cx,3
	jnz cantModify
	printString modifytips
	mov byte ptr TelTable[bx+0bh],0	;号码长度计数器清零，以便修改成不同长度的号码
	call I_TelNo
cantModify:
	ret
Modify endp

;删除联系人子程序--------------------------------------------------------------
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
	mov byte ptr TelTable[si+bp],0	;将下一行的所有都设置为0
	mov byte ptr TelTable[di+bp],dl
	inc bp
	loop moveup
	add si,17h
	add di,17h
	cmp byte ptr TelTable[si+1],0
	jnz preparemoveup
cantDelete:
	ret	;如果加了之后的下一行还是没有数据，那一行本来就没有数据
Delete endp
;输出一个联系人姓名及其电话号码---------------------------------------------+
printinfo proc near
	mov ch,0							;输出姓名
	mov cl,byte ptr TelTable[bx+1]	
	mov si,2
	printname:
		mov dl,byte ptr TelTable[bx+si]
		int21 02h
		inc si
		loop printname
	printString space
	mov ch,0							;输出电话号码
	mov cl,ds:byte ptr TelTable[bx+0bh]	
	mov si,0ch
	printTel:
		mov dl,byte ptr TelTable[bx+si]
		int21 02h
		inc si
		loop printTel
	ret
printinfo endp

;通讯录子程序-------------------------------------------------------
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
