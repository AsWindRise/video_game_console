; ===================================================================
; 实验二, 任务 1: 分支结构程序设计 (打印结果版)
; -------------------------------------------------------------------
; 任务：设有3个字节型无符号数存放在变量x, y, z中，
;       编写程序找出三个数的中间值（既不是最大也不是最小的），
;       将其存入变量media中 (并打印到屏幕)。
; ===================================================================

DATAS  SEGMENT
    ; -----------------------------------------------------------------
    ; 定义变量
    ; -----------------------------------------------------------------
    X     DB  5     ; 第一个数
    Y     DB  1   ; 第二个数
    Z     DB  2     ; 第三个数

    MEDIA DB  ?     ; 用于存放中间值结果

    ; 新增：用于打印结果的提示符
    MSG_RESULT DB  0DH, 0AH, 'The middle value is: $' ; 0DH, 0AH 是回车换行
    
    ; 用于除法的新增常量
    TEN_CONST  DB  10

DATAS  ENDS

CODES  SEGMENT
    ASSUME CS:CODES, DS:DATAS

START:
    ; --- 准备工作：设置 DS (数据段寄存器) ---
    MOV  AX, DATAS
    MOV  DS, AX

    ; --- 核心逻辑：找出中间值 ---
    ; 我们用 AL 存放 X, BL 存放 Y, CL 存放 Z
    MOV  AL, X
    MOV  BL, Y
    MOV  CL, Z

    ; 比较 AL 和 BL (即 X 和 Y)
    CMP  AL, BL
    ; JAE (Jump if Above or Equal) -> 如果 X >= Y, 跳转
    JAE  X_GTE_Y

    ; --- 分支 1: X < Y ---
    CMP  BL, CL     ; 比较 Y 和 Z
    JB   Y_LT_Z     ; (Jump if Below)

    ; (X < Y) 并且 (Y >= Z)
    CMP  AL, CL     ; 比较 X 和 Z
    JAE  X_IS_MID   ; (Z <= X < Y) -> X 是中间值
    JMP  Z_IS_MID   ; (X < Z <= Y) -> Z 是中间值

Y_LT_Z:
    ; (X < Y) 并且 (Y < Z)
    JMP  Y_IS_MID   ; (X < Y < Z) -> Y 是中间值

    ; --- 分支 2: X >= Y ---
X_GTE_Y:
    ; (X >= Y 的情况)
    CMP  BL, CL     ; 比较 Y 和 Z
    JA   Y_GT_Z     ; (Jump if Above)

    ; (X >= Y) 并且 (Y <= Z)
    CMP  AL, CL     ; 比较 X 和 Z
    JBE  X_IS_MID   ; (Y <= X <= Z) -> X 是中间值
    JMP  Z_IS_MID   ; (Y <= Z < X) -> Z 是中间值

Y_GT_Z:
    ; (X >= Y) 并且 (Y > Z)
    JMP  Y_IS_MID   ; (X >= Y > Z) -> Y 是中间值

    ; --- 结果存储与打印 ---
X_IS_MID:
    MOV  MEDIA, AL  ; 仍然保存结果到内存
    JMP  PRINT_RESULT ; 跳转到打印部分

Y_IS_MID:
    MOV  MEDIA, BL  ; 仍然保存结果到内存
    MOV  AL, BL     ; 把结果(BL)放到 AL 中以便打印
    JMP  PRINT_RESULT ; 跳转到打印部分

Z_IS_MID:
    MOV  MEDIA, CL  ; 仍然保存结果到内存
    MOV  AL, CL     ; 把结果(CL)放到 AL 中以便打印
    JMP  PRINT_RESULT ; 跳转到打印部分

PRINT_RESULT:
    ; 1. 打印提示信息 "The middle value is: "
    LEA  DX, MSG_RESULT
    MOV  AH, 9
    INT  21H
    
    ; 2. 调用子程序打印 AL 中的值
    ; (AL 中必须是 X, Y, 或 Z 的值)
    CALL PRINT_AL_DECIMAL
    JMP  FINISH

FINISH:
    ; --- 步骤 7: 结束程序 ---
    MOV  AH, 4CH    ; AH=4CH, 正常退出
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
