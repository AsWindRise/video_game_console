; ===================================================================
; 实验二, 任务 2: 循环结构程序设计 (打印结果修改版)
; -------------------------------------------------------------------
; 任务：从键盘输入不多于10个字符，统计字符‘a’出现的次数，
;       并使用子程序在屏幕上输出统计结果。
; ===================================================================

DATAS  SEGMENT
    ; -----------------------------------------------------------------
    ; 定义变量
    ; -----------------------------------------------------------------
    
    ; 用于提示用户输入
    PROMPT     DB  'Please input up to 10 chars, then press ENTER: $'
    
    ; -----------------------------------------------------------------
    ; 定义 DOS 输入缓冲区
    ; -----------------------------------------------------------------
    BUFFER     DB  12      ; 1. 最大长度 (10个字符 + 回车 + 1)
               DB  ?       ; 2. 实际长度 (DOS 填充)
               DB  12 DUP(?) ; 3. 存放字符的地方

    ; 存放统计结果
    COUNT      DB  0       ; 'a' 的计数器, 初始为 0
    
    ; 输出结果用的字符串
    RESULT_MSG DB  0DH, 0AH, 'Count of ''a'': $' ; 0DH, 0AH 是回车换行
    
    ; 【新增】: 打印子程序需要一个 10
    TEN_CONST  DB  10

    ; 【不再需要】: RESULT_ASCII DB '0', '$' ; 我们用子程序代替
    
DATAS  ENDS

CODES  SEGMENT
    ASSUME CS:CODES, DS:DATAS

START:
    ; --- 准备工作：设置 DS (数据段寄存器) ---
    MOV  AX, DATAS
    MOV  DS, AX

    ; --- 步骤 1: 提示用户输入 ---
    LEA  DX, PROMPT
    MOV  AH, 9        ; 9号功能：显示字符串
    INT  21H

    ; --- 步骤 2: 读取键盘输入 (使用 0AH 功能) ---
    LEA  DX, BUFFER
    MOV  AH, 0AH      ; 0AH (或 10) 功能：带缓冲的键盘输入
    INT  21H

    ; --- 步骤 3: 循环统计 'a' 的个数 ---
    
    ; LEA: 将指向输入字符的第一个字节 (BUFFER+2) 的地址放入 SI
    LEA  SI, BUFFER+2
    
    ; 将实际读取的字符个数 (BUFFER+1) 放入 CL
    MOV  CL, [BUFFER+1]
    MOV  CH, 0        ; CH 清零, 使得 CX = 实际字符数
    
    ; 检查 CX 是否为 0 (用户可能直接按了回车)
    JCXZ DISPLAY_RESULT ; 如果 CX=0, 直接跳转去显示结果
    
    ; 将我们的计数器初始化为 0。我们用 BL 寄存器作为临时计数器
    MOV  BL, 0

    ; --- 循环开始 (LOOP_START 是一个标号) ---
LOOP_START:
    ; 比较 SI 指向的内存单元 (即当前字符) 是否为 'a'
    CMP  BYTE PTR [SI], 'a'
    
    ; JNE (Jump if Not Equal)
    ; 如果不等于 'a', 就跳转到 NOT_A 标号
    JNE  NOT_A

    ; --- 如果等于 'a' ---
    INC  BL           ; 'a' 计数器 (BL) 加 1

    ; --- 如果不等于 'a' ---
NOT_A:
    INC  SI           ; SI 指向下一个字符
    
    ; LOOP 指令 (CX 减 1, 如果 CX 不为 0 则跳转到 LOOP_START)
    LOOP LOOP_START

    ; --- 循环结束 ---
    
    ; 将 BL (最终的计数值) 存入我们的变量 COUNT
    MOV  COUNT, BL

    ; --- 步骤 4: 显示统计结果 ---
DISPLAY_RESULT:
    ; (a) 显示 "Count of 'a': "
    LEA  DX, RESULT_MSG
    MOV  AH, 9
    INT  21H
    
    ; (b) 【已修改】: 使用子程序打印 COUNT 的值
    ;     不再使用旧的 ADD AL, '0' 方法
    MOV  AL, COUNT
    CALL PRINT_AL_DECIMAL
    
    ; --- 步骤 5: 结束程序 ---
    MOV  AH, 4CH      ; 4CH 功能：正常退出
    INT  21H

; ===================================================================
; PRINT_AL_DECIMAL (子程序)
; -------------------------------------------------------------------
; 功能：将 AL 寄存器中的8位二进制数 (0-255)
;       作为两位十进制数打印到屏幕上。
; ===================================================================
PRINT_AL_DECIMAL PROC NEAR
    ; --- 保存要用的寄存器 ---
    PUSH AX
    PUSH BX
    PUSH DX

    ; --- 核心转换逻辑 (除以10取余) ---
    MOV  AH, 0         ; 清空 AH, 使得 AX = AL中的值
    MOV  BL, TEN_CONST ; BL = 10
    
    ; DIV BL: 将 AX 除以 BL (10)
    ; 商 (十位数) 存入 AL
    ; 余数 (个位数) 存入 AH
    DIV  BL            
    
    ; --- 转换为 ASCII ---
    ADD  AL, '0'       ; 将数值 (十位数) 转换为 ASCII
    ADD  AH, '0'       ; 将数值 (个位数) 转换为 ASCII
    
    ; --- 打印 ---
    PUSH AX            ; 先把两个ASCII码保存到堆栈
    
    ; 1. 打印十位数
    MOV  DL, AL        ; DL = 十位数的 ASCII
    MOV  AH, 2         ; 2号功能：打印单个字符
    INT  21H
    
    ; 2. 打印个位数
    POP  AX            ; 恢复 AX (AH=个位数, AL=十位数)
    MOV  DL, AH        ; DL = 个位数的 ASCII
    MOV  AH, 2
    INT  21H
    
    ; --- 恢复寄存器 ---
    POP  DX
    POP  BX
    POP  AX
    
    RET                ; (Return) 子程序返回
PRINT_AL_DECIMAL ENDP
; (ENDP = End Procedure)


CODES  ENDS
    END   START
