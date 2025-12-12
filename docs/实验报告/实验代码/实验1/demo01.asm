; ===================================================================
; 实验一 (修改版：直接打印结果)
; ===================================================================
DATAS  SEGMENT
    
    ; MSG: 提示用户输入的字符串
    MSG_INPUT  DB  'Please input a number (0-9): $'

    ; BUF: 存放从键盘输入的原始数字 X
    BUF        DB  ?  
    
    ; 新增：用于打印结果的提示符
    MSG_X2     DB  0DH, 0AH, 'X * 2 = $' ; 0DH, 0AH 是回车换行
    MSG_X4     DB  0DH, 0AH, 'X * 4 = $'
    MSG_X10    DB  0DH, 0AH, 'X * 10 = $'
    
    ; 用于除法的新增常量
    TEN_CONST  DB  10

; DATAS ENDS: "数据段" 在这里定义结束。
DATAS  ENDS


; ===================================================================
; 代码段 (CODES SEGMENT)
; ===================================================================
CODES  SEGMENT

    ; ASSUME: 告诉汇编器 CS 指向 CODES 段, DS 指向 DATAS 段
    ASSUME CS:CODES, DS:DATAS

; ===================================================================
; START: 程序的入口点标号
; ===================================================================
START:
    ; --- 准备工作：设置 DS (数据段寄存器) ---
    MOV  AX, DATAS
    MOV  DS, AX


    ; --- 步骤 1: 在屏幕上显示提示信息 ---
    LEA  DX, MSG_INPUT
    MOV  AH, 9
    INT  21H


    ; --- 步骤 2: 从键盘读取一个字符 ---
    MOV  AH, 1
    INT  21H


    ; --- 步骤 3: 将 "ASCII 码" 转换为 "数值" ---
    SUB  AL, '0'  
    MOV  BUF, AL  


    ; --- 步骤 4: 计算并打印 X * 2 ---
    LEA  DX, MSG_X2    ; 打印 "X * 2 = "
    MOV  AH, 9
    INT  21H
    
    MOV  AL, BUF       ; AL = X
    SHL  AL, 1         ; AL = X * 2
    CALL PRINT_AL_DECIMAL ; 调用子程序打印 AL 的值


    ; --- 步骤 5: 计算并打印 X * 4 ---
    LEA  DX, MSG_X4    ; 打印 "X * 4 = "
    MOV  AH, 9
    INT  21H

    MOV  AL, BUF       ; AL = X
    SHL  AL, 1         ; AL = X * 2
    SHL  AL, 1         ; AL = X * 4
    CALL PRINT_AL_DECIMAL ; 调用子程序打印 AL 的值
    

    ; --- 步骤 6: 计算并打印 X * 10 ---
    LEA  DX, MSG_X10   ; 打印 "X * 10 = "
    MOV  AH, 9
    INT  21H

    MOV  AL, BUF       ; AL = X
    MOV  BL, AL        ; BL = X
    
    SHL  AL, 1         ; AL = X * 2
    SHL  AL, 1         ; AL = X * 4
    SHL  AL, 1         ; AL = X * 8
    
    SHL  BL, 1         ; BL = X * 2
    
    ADD  AL, BL        ; AL = (X*8) + (X*2) = X*10
    CALL PRINT_AL_DECIMAL ; 调用子程序打印 AL 的值

    ; --- 步骤 7: 结束程序并返回 DOS ---
    MOV  AH, 4CH
    INT  21H

; ===================================================================
; PRINT_AL_DECIMAL (子程序)
; -------------------------------------------------------------------
; 功能：将 AL 寄存器中的8位二进制数 (0-255)
;       作为两位十进制数打印到屏幕上。
;       例如：AL = 14H (20) -> 打印 "20"
; ===================================================================
PRINT_AL_DECIMAL PROC NEAR
    ; --- 保存要用的寄存器 ---
    PUSH AX
    PUSH BX
    PUSH DX

    ; --- 核心转换逻辑 (除以10取余) ---
    MOV  AH, 0         ; 清空 AH, 使得 AX = AL中的值
    MOV  BL, TEN_CONST ; BL = 10
    
    ; DIV BL: 将 AX (0-255) 除以 BL (10)
    ; 商 (十位数) 存入 AL
    ; 余数 (个位数) 存入 AH
    DIV  BL            
    
    ; --- 转换为 ASCII ---
    ; AL 中是十位数, AH 中是个位数
    ; (例如: 20 / 10 -> AL=2, AH=0)
    ADD  AL, '0'       ; 将数值 2 转换为 ASCII '2'
    ADD  AH, '0'       ; 将数值 0 转换为 ASCII '0'
    
    ; --- 打印 ---
    PUSH AX            ; 先把两个ASCII码保存到堆栈
    
    ; 1. 打印十位数
    MOV  DL, AL        ; DL = '2'
    MOV  AH, 2         ; 2号功能：打印单个字符
    INT  21H
    
    ; 2. 打印个位数
    POP  AX            ; 恢复 AX (AH='0', AL='2')
    MOV  DL, AH        ; DL = '0'
    MOV  AH, 2
    INT  21H
    
    ; --- 恢复寄存器 ---
    POP  DX
    POP  BX
    POP  AX
    
    RET                ; (Return) 子程序返回
PRINT_AL_DECIMAL ENDP
; (ENDP = End Procedure)

; CODES ENDS: "代码段" 在这里定义结束。
CODES  ENDS

; END START: 告诉汇编器程序结束，并指定入口点为 START
    END   START
