#include "../include/vmcpu.hpp"

int VMCPU::executer(VBYTE opcode)
{
    int valToReturn = 0;
    dicOpcodesFunction[opcode]();
    return isError;
}

void VMCPU::funcException(std::string e)
{
    #ifdef _VM_CPU_TEST_
        if(vcpuFlag == VCpuFlag::OK) vcpuFlag = VCpuFlag::ERROR;
    #endif
    #ifndef _VM_CPU_TEST_
        std::cout << "[ERROR] vCPU CRASH!" << std::endl;
        std::cout << "[ERROR] INFO: " << e << std::endl;
    #endif
    isError = true;
}

/* NOP */
void VMCPU::funcNop()
{
    #ifdef V_DEBUG
        std::cout << "[DEBUG] NOP" << std::endl;
    #endif
}

/* EE - end of code */
void VMCPU::funcEE()
{
    #ifdef V_DEBUG
        std::cout << "[DEBUG] EE" << std::endl;
    #endif
    valToReturn = 1;
}

/*
    MOV - move from register to register
    01 02 05 => MOV R2, R5
    01 00 00 => MOV R0, R0
*/
void VMCPU::funcMov()
{
    VBYTE bTmp_0;
    VBYTE bTmp_1;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] MOV" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    bTmp_1 = AS->codeData[REGS->PC++];
    #ifdef V_DEBUG
        std::cout << std::hex << REGS->R[bTmp_0] << std::endl;
        std::cout << std::hex << REGS->R[bTmp_1] << std::endl;
        // std::cout << std::hex << bTmp_0 << "\n";
        // std::cout << std::hex << bTmp_1 << "\n";
        // std::cout << std::bitset<8>(bTmp_0) << std::endl;
        // std::cout << std::bitset<8>(bTmp_1) << std::endl;
    #endif
    if((bTmp_0 >= 0 && bTmp_0 <= 7) && (bTmp_1 >= 0 && bTmp_1 <= 7))
    {
        REGS->R[bTmp_0] = REGS->R[bTmp_1];
    }
    else funcException("register index out of range!");
}

/* 
    MOVMB - move and extend byte from memory to register 
    02 03 04 01 => MOVMB R3, 0104
*/
void VMCPU::funcMovmb()
{
    VBYTE bTmp_0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] MOVMB" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    wTmp_0 = *(VWORD*) &AS->codeData[REGS->PC];
    //if(wTmp_0 >= sizeof(AS->codeData)) funcException("memory index out of range!");
    REGS->PC += 2;
    REGS->R[bTmp_0] = 0;
    if(areFramesNeeded && (wTmp_0 >= frameMap[currentFrameNumber]))
    {
        std::vector<VBYTE> bytes = getByteFromFrame(wTmp_0, 1);
        if(isError) funcException("get byte from frame!");
        *(VBYTE*) &REGS->R[bTmp_0] = bytes[0];
    }
    else *(VBYTE*) &REGS->R[bTmp_0] = AS->codeData[wTmp_0];
}

/* 
    MOVMW - move and extend word from memory to register 
    03 03 04 01 => MOVMW R3, 0104
*/
void VMCPU::funcMovmw()
{
    VBYTE bTmp_0;
    VWORD wTmp_0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] MOVMW" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    wTmp_0 = *(VWORD*) &AS->codeData[REGS->PC];
    //if(wTmp_0 >= sizeof(AS->codeData)) funcException("memory index out of range!");
    REGS->PC += 2;
    if(areFramesNeeded && (wTmp_0 >= frameMap[currentFrameNumber]))
    {
        std::vector<VBYTE> bytes = getByteFromFrame(wTmp_0, 2);
        if(isError) funcException("get byte from frame!");
        VBYTE hb = bytes[0];
        VBYTE lb = bytes[1];
        VWORD w = ((VWORD) hb << 8) | lb;
        *(VWORD*) &REGS->R[bTmp_0] = w;
    }
    else *(VWORD*) &REGS->R[bTmp_0] = *(VWORD*) &AS->codeData[wTmp_0];
}

/* 
    MOVB - move and extend byte to register 
    04 02 43 => MOVB R2, 43
*/
void VMCPU::funcMovb()
{
    VBYTE bTmp_0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] MOVB" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    REGS->R[bTmp_0] = 0;
    *(VBYTE *) &REGS->R[bTmp_0] = AS->codeData[REGS->PC++];
}

/* 
    MOVW - move and extend word to register 
    05 01 15 28 => MOVW R1, 2815
*/
void VMCPU::funcMovw()
{
    VBYTE bTmp_0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] MOVW" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    *(VWORD*) &REGS->R[bTmp_0] = *(VWORD *) &AS->codeData[REGS->PC];
    REGS->PC += 2;
}

/* 
    MOVBM - move byte from register to memory location 
    07 43 13 04 => MOVBM 1343, R4
*/
void VMCPU::funcMovbm()
{
    VBYTE bTmp_0;
    VWORD wTmp_0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] MOVBM" << std::endl;
    #endif
    wTmp_0 = *(VWORD*) &AS->codeData[REGS->PC];
    //if(wTmp_0 >= sizeof(AS->codeData)) funcException("memory index out of range!");
    REGS->PC += 2;
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    if(areFramesNeeded && (wTmp_0 >= frameMap[currentFrameNumber]))
    {
        std::vector<VBYTE> byteToWrite;
        byteToWrite.push_back(*(VBYTE*) &REGS->R[bTmp_0]);
        writeByteIntoFrame(wTmp_0, 1, byteToWrite);
        if(isError) funcException("write byte to frame!");
    }
    else AS->codeData[wTmp_0] = *(VBYTE*) &REGS->R[bTmp_0];
}

/* 
    MOVWM - move word from register to memory location 
    07 43 13 04 => MOVWM 1343,R4
*/
void VMCPU::funcMovwm()
{
    VBYTE bTmp_0;
    VWORD wTmp_0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] MOVWM" << std::endl;
    #endif
    wTmp_0 = *(VWORD*) &AS->codeData[REGS->PC];
    //if(wTmp_0 >= sizeof(AS->codeData)) funcException("memory index out of range!");
    REGS->PC += 2;
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    if(areFramesNeeded && (wTmp_0 >= frameMap[currentFrameNumber]))
    {
        std::vector<VBYTE> byteToWrite;
        VWORD w = *(VWORD*) &REGS->R[bTmp_0];
        byteToWrite.push_back(w >> 8);
        byteToWrite.push_back(w & 0xff);
        writeByteIntoFrame(wTmp_0, 2, byteToWrite);
        if(isError) funcException("write byte to frame!");
    }
    else *(VWORD*) &AS->codeData[wTmp_0] = REGS->R[bTmp_0];
}

/* 
    MOVMRB - move and extend byte from memory to register
            get addr from register
    08 02 01 => MOVMRB R2, R1
*/
void VMCPU::funcMovmrb()
{
    VBYTE bTmp_0;
    VBYTE bTmp_1;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] MOVMRB" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    bTmp_1 = AS->codeData[REGS->PC++];
    if(bTmp_1 > 8) funcException("register index out of range!");
    //if(REGS->R[bTmp_1] >= sizeof(AS->codeData)) funcException("memory index out of range!");
    REGS->R[bTmp_0] = 0;
    if(areFramesNeeded && (REGS->R[bTmp_1] >= frameMap[currentFrameNumber]))
    {
        std::vector<VBYTE> bytes = getByteFromFrame(REGS->R[bTmp_1], 1);
        if(isError) funcException("get byte from frame!");
        *(VBYTE*) &REGS->R[bTmp_0] = bytes[0];
    }
    else *(VBYTE*) &REGS->R[bTmp_0] = AS->codeData[REGS->R[bTmp_1]];
}

/* 
    MOVMRW - move and extend word from memory to register
            get addr from register
    09 02 01 => MOVMRW R2, R1
*/
void VMCPU::funcMovmrw()
{
    VBYTE bTmp_0;
    VBYTE bTmp_1;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] MOVMRW" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    bTmp_1 = AS->codeData[REGS->PC++];
    if(bTmp_1 > 8) funcException("register index out of range!");
    //if(REGS->R[bTmp_1] >= sizeof(AS->codeData))  funcException("memory index out of range!");
    if(areFramesNeeded && (REGS->R[bTmp_1] >= frameMap[currentFrameNumber]))
    {
        std::vector<VBYTE> bytes = getByteFromFrame(REGS->R[bTmp_1], 2);
        if(isError) funcException("get byte from frame!");
        VBYTE hb = bytes[0];
        VBYTE lb = bytes[1];
        VWORD w = ((VWORD) hb << 8) | lb;
        *(VWORD*) &REGS->R[bTmp_0] = w;
    }
    else *(VWORD*) &REGS->R[bTmp_0] = *(VWORD*) &AS->codeData[REGS->R[bTmp_1]];
}

/* 
    MOVMD - move double word from memory to register 
    0A 03 04 01 => MOVMW R3, 0104
*/
void VMCPU::funcMovmd()
{
    VBYTE bTmp_0;
    VBYTE bTmp_1;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] MOVMD" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    dTmp_0 = *(VDWORD*) &AS->codeData[REGS->PC];
    //if(dTmp_0 >= sizeof(AS->codeData)) funcException("memory index out of range!");
    REGS->PC += 2;
    if(areFramesNeeded && (dTmp_0 >= frameMap[currentFrameNumber]))
    {
        std::vector<VBYTE> bytes = getByteFromFrame(dTmp_0, 4);
        if(isError) funcException("get byte from frame!");
        VBYTE hb3 = bytes[0];
        VBYTE hb2 = bytes[1];
        VBYTE hb1 = bytes[2];
        VBYTE lb = bytes[3];
        VDWORD dw = ((VDWORD) hb3 << 24) | ((VDWORD) hb2 << 16) | ((VDWORD) hb1 << 8) | lb;
        REGS->R[bTmp_0] = dw;
    }
    else REGS->R[bTmp_0] = *(VDWORD*) &AS->codeData[dTmp_0];
}

/* 
    MOVD - move value to register 
    0B 01 00 00 15 28 => MOVW R1, 2815
*/
void VMCPU::funcMovd()
{
    VBYTE bTmp_0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] MOVD" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) gfuncException("register index out of range!");
    REGS->R[bTmp_0] = *(VDWORD *) &AS->codeData[REGS->PC];
    REGS->PC += 4;
}

/* 
    MOVDM - move double word from register to memory location 
    0C 43 13 04 => MOVWM 1343, R4
*/
void VMCPU::funcMovdm()
{
    VBYTE bTmp_0;
    VWORD wTmp_0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] MOVDM" << std::endl;
    #endif
    wTmp_0 = *(VWORD*) &AS->codeData[REGS->PC];
    //if(wTmp_0 >= sizeof(AS->codeData)) funcException("memory index out of range!");
    REGS->PC += 2;
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    if(areFramesNeeded && (wTmp_0 >= frameMap[currentFrameNumber]))
    {
        std::vector<VBYTE> byteToWrite;
        VDWORD dw = REGS->R[bTmp_0];
        byteToWrite.push_back(dw >> 24);
        byteToWrite.push_back(dw >> 16);
        byteToWrite.push_back(dw >> 8);
        byteToWrite.push_back(dw);
        writeByteIntoFrame(wTmp_0, 2, byteToWrite);
        if(isError) funcException("write byte to frame!");
    }
    else *(VDWORD*) &AS->codeData[wTmp_0] = REGS->R[bTmp_0];
}

/* 
    MOVMRD - move double word from memory to register
            get addr from register
    0D 02 01 => MOVMRW R2, R1
*/
void VMCPU::funcMovmrd()
{
    VBYTE bTmp_0;
    VBYTE bTmp_1;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] MOVMRD" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    bTmp_1 = AS->codeData[REGS->PC++];
    if(bTmp_1 > 8) funcException("register index out of range!");
    //if(REGS->R[bTmp_1] >= sizeof(AS->codeData)) funcException("memory index out of range!");
    if(areFramesNeeded && (REGS->R[bTmp_1] >= frameMap[currentFrameNumber]))
    {
        std::vector<VBYTE> bytes = getByteFromFrame(REGS->R[bTmp_1], 4);
        if(isError) funcException("get byte from frame!");
        VBYTE hb3 = bytes[0];
        VBYTE hb2 = bytes[1];
        VBYTE hb1 = bytes[2];
        VBYTE lb = bytes[3];
        VDWORD dw = ((VDWORD) hb3 << 24) | ((VDWORD) hb2 << 16) | ((VDWORD) hb1 << 8) | lb;
        REGS->R[bTmp_0] = dw;
    }
    else REGS->R[bTmp_0] = *(VDWORD*) &AS->codeData[REGS->R[bTmp_1]];
}

/*
    JMP - unconditional jump
    20 15 00 => JMP 0015
*/
void VMCPU::funcJmp()
{
    VWORD wTmp_0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] JMP" << std::endl;
    #endif
    wTmp_0 = *(VWORD*) &AS->codeData[REGS->PC];
    REGS->PC += 2;
    // if(wTmp_0 > sizeof(AS->codeData)) funcException("memory index out of range!");
    REGS->PC = wTmp_0;
}

/*
    JZ - jump if equal
    21 15 00 => JZ 0015
*/
void VMCPU::funcJz()
{
    VWORD wTmp_0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] JZ" << std::endl;
    #endif
    wTmp_0 = *(VWORD*) &AS->codeData[REGS->PC];
    REGS->PC += 2;
    // if(wTmp_0 > sizeof(AS->codeData)) funcException("memory index out of range!");
    if(REGS->ZF) REGS->PC = wTmp_0;
}

/*
    JNZ - jump if not equal
    22 15 00 => JNZ 0015
*/
void VMCPU::funcJnz()
{
    VWORD wTmp_0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] JNZ" << std::endl;
    #endif
    wTmp_0 = *(VWORD*) &AS->codeData[REGS->PC];
    REGS->PC += 2;
    // if(wTmp_0 > sizeof(AS->codeData)) funcException("memory index out of range!");
    if(!REGS->ZF) REGS->PC = wTmp_0;
}

/*
    JAE - jump if above or equal
    23 15 00 => JAE 0015
*/
void VMCPU::funcJae()
{
    VWORD wTmp_0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] JAE" << std::endl;
    #endif
    wTmp_0 = *(VWORD*) &AS->codeData[REGS->PC];
    REGS->PC += 2;
    // if(wTmp_0 > sizeof(AS->codeData)) funcException("memory index out of range!");
    if(REGS->ZF || !REGS->CF) REGS->PC = wTmp_0;
}

/*
    JBE - jump if below or equal
    24 15 00 => JBE 0015
*/
void VMCPU::funcJbe()
{
    VWORD wTmp_0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] JBE" << std::endl;
    #endif
    wTmp_0 = *(VWORD*) &AS->codeData[REGS->PC];
    REGS->PC += 2;
    // if(wTmp_0 > sizeof(AS->codeData)) funcException("memory index out of range!");
    if(REGS->ZF || REGS->CF) REGS->PC = wTmp_0;
}

/*
    JB - jump if below
    25 15 00 => JB 0015
*/
void VMCPU::funcJb()
{
    VWORD wTmp_0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] JB" << std::endl;
    #endif
    wTmp_0 = *(VWORD*) &AS->codeData[REGS->PC];
    REGS->PC += 2;
    // if(wTmp_0 > sizeof(AS->codeData)) funcException("memory index out of range!");
    if(!REGS->ZF && REGS->CF) REGS->PC = wTmp_0;
}

/*
    JA - jump if above
    26 15 00 => JA 0015
*/
void VMCPU::funcJa()
{
    VWORD wTmp_0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] JA" << std::endl;
    #endif
    wTmp_0 = *(VWORD*) &AS->codeData[REGS->PC];
    REGS->PC += 2;
    // if(wTmp_0 > sizeof(AS->codeData)) funcException("memory index out of range!");
    if(!REGS->ZF && !REGS->CF) REGS->PC = wTmp_0;
}

/*
    ADVR - Add word value to register
    30 02 10 00 => ADVR R2, 10
*/
void VMCPU::funcAdvr()
{
    VBYTE bTmp_0;
    VWORD wTmp_0;
    VWORD wTmp_1;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] ADVR" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    wTmp_0 = *(VWORD*) &AS->codeData[REGS->PC];
    REGS->PC += 2;
    wTmp_1 = REGS->R[bTmp_0] + wTmp_0;          
    if(wTmp_1 == 0) REGS->ZF = 1;
    else REGS->ZF = 0;
    if(wTmp_1 < REGS->R[bTmp_0]) REGS->CF = 1;
    else REGS->CF = 0;
    *(VWORD *) &REGS->R[bTmp_0] = wTmp_1;
}

/*
    ADRR - Add two registers
            and save result in first
    31 02 01 => ADRR R2, R1
*/
void VMCPU::funcAdrr()
{
    VBYTE bTmp_0;
    VBYTE bTmp_1;
    VDWORD dTmp_0;
    VDWORD dTmp_1;
    VDWORD dTmp_2;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] ADRR" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    bTmp_1 = AS->codeData[REGS->PC++];
    if(bTmp_1 > 8) funcException("register index out of range!");
    dTmp_0 = REGS->R[bTmp_0];
    dTmp_1 = REGS->R[bTmp_1];
    dTmp_2 = dTmp_0 + dTmp_1;
    REGS->R[bTmp_0] = dTmp_2;
    if(dTmp_2 == 0) REGS->ZF = 1;
    else REGS->ZF = 0;
    if(dTmp_2 < dTmp_0) REGS->CF = 1;
    else REGS->CF = 0;
}

/*
    ADRRL - Add two registers (low byte)
            and save result in first
    32 02 01 => ADRR R2, R1
*/
void VMCPU::funcAdrrl()
{
    VBYTE bTmp_0;
    VBYTE bTmp_1;
    VBYTE bTmp_2;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] ADRRL" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    bTmp_1 = AS->codeData[REGS->PC++];
    if(bTmp_1 > 8) funcException("register index out of range!");
    bTmp_2 = *(VBYTE *) &REGS->R[bTmp_0];
    bTmp_1 = *(VBYTE *) &REGS->R[bTmp_1];
    bTmp_1 += bTmp_2;
    *(VBYTE *) &REGS->R[bTmp_0] = bTmp_1;
    if(bTmp_1 == 0) REGS->ZF = 1;
    else REGS->ZF = 0;
    if(bTmp_1 < bTmp_2) REGS->CF = 1;
    else REGS->CF = 0;
}

/*
    SUBVR - Substract word value from register
    33 02 10 00 => SUBVR R2, 10
*/
void VMCPU::funcSubvr()
{
    VBYTE bTmp_0;
    VWORD wTmp_0;
    VWORD wTmp_1;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] SUBVR" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    wTmp_0 = *(VWORD*) &AS->codeData[REGS->PC];
    REGS->PC += 2;
    wTmp_1 = REGS->R[bTmp_0] - wTmp_0;          
    if(wTmp_1 == 0) REGS->ZF = 1;
    else REGS->ZF = 0;
    if(wTmp_1 > REGS->R[bTmp_0]) REGS->CF = 1;
    else REGS->CF = 0;
    *(VWORD *) &REGS->R[bTmp_0] = wTmp_1;
}

/*
    SUBRR - Substract two registers
            and save result in first
    34 02 01 => SUBRR R2, R1
*/
void VMCPU::funcSubrr()
{
    VBYTE bTmp_0;
    VBYTE bTmp_1;
    VDWORD dTmp_0;
    VDWORD dTmp_1;
    VDWORD dTmp_2;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] SUBRR" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    bTmp_1 = AS->codeData[REGS->PC++];
    if(bTmp_1 > 8) funcException("register index out of range!");
    dTmp_0 = REGS->R[bTmp_0];
    dTmp_1 = REGS->R[bTmp_1];
    dTmp_2 = dTmp_0 - dTmp_1;
    REGS->R[bTmp_0] = dTmp_2;
    if(dTmp_2 == 0) REGS->ZF = 1;
    else REGS->ZF = 0;
    if(dTmp_2 > dTmp_0) REGS->CF = 1;
    else REGS->CF = 0;
}

/*
    SUBRRL - Substract two registers (low byte)
            and save result in first
    35 02 01 => ADRR R2, R1
*/
void VMCPU::funcSubrrl()
{
    VBYTE bTmp_0;
    VBYTE bTmp_1;
    VBYTE bTmp_2;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] SUBRRL" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    bTmp_1 = AS->codeData[REGS->PC++];
    if(bTmp_1 > 8) funcException("register index out of range!");
    bTmp_2 = *(VBYTE *) &REGS->R[bTmp_0];
    bTmp_1 = *(VBYTE *) &REGS->R[bTmp_1];
    bTmp_1 -= bTmp_2;
    *(VBYTE *) &REGS->R[bTmp_0] = bTmp_1;
    if(bTmp_1 == 0) REGS->ZF = 1;
    else REGS->ZF = 0;
    if(bTmp_1 > bTmp_2) REGS->CF = 1;
    else REGS->CF = 0;
}

/*
    XOR - Xor two registers
        and save result in first
    36 02 01 => XOR R2, R1
*/
void VMCPU::funcXor()
{
    VBYTE bTmp_0;
    VBYTE bTmp_1;
    VDWORD dTmp_0;
    VDWORD dTmp_1;
    VDWORD dTmp_2;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] XOR" << std::endl;
    #endif 
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    bTmp_1 = AS->codeData[REGS->PC++];
    if(bTmp_1 > 8) funcException("register index out of range!");
    dTmp_0 = REGS->R[bTmp_0];
    dTmp_1 = REGS->R[bTmp_1];
    dTmp_2 = dTmp_0 ^ dTmp_1;
    if(dTmp_2 == 0) REGS->ZF = 1;
    else REGS->ZF = 0;
    REGS->R[bTmp_0] = dTmp_2;
    REGS->CF = 0;
}

/*
    XORL - Xor two registers (lower bytes)
        and save result in first
    37 02 01 => XOR R2, R1
*/
void VMCPU::funcXorl()
{
    VBYTE bTmp_0;
    VBYTE bTmp_1;
    VBYTE bTmp_2;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] XORL" << std::endl;
    #endif 
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    bTmp_1 = AS->codeData[REGS->PC++];
    if(bTmp_1 > 8) funcException("register index out of range!");
    bTmp_2 = REGS->R[bTmp_0];
    bTmp_1 = REGS->R[bTmp_1];
    bTmp_2 ^= bTmp_1;
    if(bTmp_2 == 0) REGS->ZF = 1;
    else REGS->ZF = 0;
    *(VBYTE *) &REGS->R[bTmp_0] = bTmp_2;
    REGS->CF = 0;
}

/*
    NOT - Bitwise not on value in a register
        and save result in this register
    38 02 => NOT R2
*/
void VMCPU::funcNot()
{
    VBYTE bTmp_0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] NOT" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    REGS->R[bTmp_0] = ~ REGS->R[bTmp_0];
}

/*
    NOTB - Bitwise not on value in a register (lower bytes)
        and save result in this register
    39 02 => NOT R2
*/
void VMCPU::funcNotb()
{
    VBYTE bTmp_0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] NOTB" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    *(VBYTE *) &REGS->R[bTmp_0] = ~ (*(VBYTE *) &REGS->R[bTmp_0]);
}

/*
    ADVRD - Add double word value to register
    3A 02 10 00 00 00 => ADVR R2, 10
*/
void VMCPU::funcAdvrd()
{
    VBYTE bTmp_0;
    VDWORD dTmp_0;
    VDWORD dTmp_1;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] ADVRD" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    dTmp_0 = *(VDWORD*) &AS->codeData[REGS->PC];
    REGS->PC += 4;
    dTmp_1 = REGS->R[bTmp_0] + dTmp_0;          
    if(dTmp_1 == 0) REGS->ZF = 1;
    else REGS->ZF = 0;
    if(dTmp_1 < REGS->R[bTmp_0]) REGS->CF = 1;
    else REGS->CF = 0;
    REGS->R[bTmp_0] = dTmp_1;
}

/*
    SUBVRD - Substract double word value from register
    3B 02 10 00 00 00 => SUBVR R2, 10
*/
void VMCPU::funcSubvrd()
{
    VBYTE bTmp_0;
    VDWORD dTmp_0;
    VDWORD dTmp_1;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] SUBVRD" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    dTmp_0 = *(VDWORD*) &AS->codeData[REGS->PC];
    REGS->PC += 4;
    dTmp_1 = REGS->R[bTmp_0] - dTmp_0;          
    if(dTmp_1 == 0) REGS->ZF = 1;
    else REGS->ZF = 0;
    if(dTmp_1 > REGS->R[bTmp_0]) REGS->CF = 1;
    else REGS->CF = 0;
    *(VDWORD *) &REGS->R[bTmp_0] = dTmp_1;
}

/*
    SHR - Shift the bits of the operand destination to the right,
            by the number of bits specified in the count operand
    3C 02 05 => SHR R2, 5
*/
void VMCPU::funcShr()
{
    VBYTE bTmp_0;
    VBYTE bTmp_1;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] SHR" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    bTmp_1 = AS->codeData[REGS->PC++];
    REGS->R[bTmp_0] = REGS->R[bTmp_0] >> bTmp_1;
}

/*
    SHL -Shift the bits of the operand destination to the left,
            by the number of bits specified in the count operand
    3D 02 05 => SHL R2, 5
*/
void VMCPU::funcShl()
{
    VBYTE bTmp_0;
    VBYTE bTmp_1;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] SHL" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    bTmp_1 = AS->codeData[REGS->PC++];
    REGS->R[bTmp_0] = REGS->R[bTmp_0] << bTmp_1;
}

/*
    CMP - compare two registers
    50 02 01 => CMP R2, R1
*/
void VMCPU::funcCmp()
{
    VBYTE bTmp_0;
    VBYTE bTmp_1;
    VDWORD dTmp_0;
    VDWORD dTmp_1;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] CMP" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    bTmp_1 = AS->codeData[REGS->PC++];
    if(bTmp_1 > 8) funcException("register index out of range!");
    dTmp_0 = REGS->R[bTmp_0];
    dTmp_1 = REGS->R[bTmp_1];
    #ifdef V_DEBUG
        std::cout << dTmp_0 << std::endl;
        std::cout << dTmp_1 << std::endl;
    #endif
    if(dTmp_1 == dTmp_0) REGS->ZF = 1;
    else REGS->ZF = 0;
    if(dTmp_1 > dTmp_0) REGS->CF = 1;
    else REGS->CF = 0;
}

/*
    CMPL - compare two registers (lower byte)
    51 02 01 => CMP R2, R1
*/
void VMCPU::funcCmpl()
{
    VBYTE bTmp_0;
    VBYTE bTmp_1;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] CMPL" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    bTmp_1 = AS->codeData[REGS->PC++];
    if(bTmp_1 > 8) funcException("register index out of range!");
    bTmp_0 = *(VBYTE*) &REGS->R[bTmp_0];
    bTmp_1 = *(VBYTE*) &REGS->R[bTmp_1];
    if(bTmp_0 == bTmp_1) REGS->ZF = 1;
    else REGS->ZF = 0;
    if(bTmp_0 < bTmp_1) REGS->CF = 1;
    else REGS->CF = 0;
}

/*
    VMSYSBUS FUNC_CODE
    60 01 00 => SYSBUS createDirectory
*/
void VMCPU::funcVmSysbus()
{
    VWORD wTmp_0 = *(VWORD*) &AS->codeData[REGS->PC];
    VWORD wTmp_1;
    REGS->PC += 2;
    REGS->R[6] = 0;
    REGS->R[7] = 0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] VMSYSBUS" << std::endl;
        std::cout << "[DEBUG] CALL " << wTmp_0 << std::endl;
    #endif
    switch(wTmp_0)
    {
        case SYSBUS_CREATE_DIR:
            {
                wTmp_1 = AS->stack[REGS->SP]; // second arg
                REGS->SP += 1;
                wTmp_0 = AS->stack[REGS->SP]; // first arg
                REGS->SP += 1;
                std::string arg1 = "";
                getDataFromCodeData(arg1, wTmp_0);
                REGS->R[6] = sysBus->createDirectory(arg1, wTmp_1);
            }
            break;
        case SYSBUS_DELETE_DIR:
            {
                wTmp_0 = AS->stack[REGS->SP];
                REGS->SP += 1;
                std::string arg1 = "";
                getDataFromCodeData(arg1, wTmp_0);
                REGS->R[6] = sysBus->deleteDirectory(arg1);
            }
            break;
        case SYSBUS_MOVE_DIR:
            {
                wTmp_1 = AS->stack[REGS->SP]; // second arg
                REGS->SP += 1;
                wTmp_0 = AS->stack[REGS->SP]; // first arg
                REGS->SP += 1;
                std::string arg1 = "";
                std::string arg2 = "";
                getDataFromCodeData(arg2, wTmp_1);
                getDataFromCodeData(arg1, wTmp_0);
                REGS->R[6] = sysBus->moveDirectory(arg1, arg2);
            }
            break;
        case SYSBUS_COPY_DIR:
            {
                wTmp_1 = AS->stack[REGS->SP]; // second arg
                REGS->SP += 1;
                wTmp_0 = AS->stack[REGS->SP]; // first arg
                REGS->SP += 1;
                std::string arg1 = "";
                std::string arg2 = "";
                getDataFromCodeData(arg2, wTmp_1);
                getDataFromCodeData(arg1, wTmp_0);
                REGS->R[6] = sysBus->copyDirectory(arg1, arg2);
            }
            break;
        case SYSBUS_CREATE_FILE:
            {
                wTmp_1 = AS->stack[REGS->SP];
                REGS->SP += 1;
                wTmp_0 = AS->stack[REGS->SP];
                REGS->SP += 1;
                std::string arg2 = "";
                int counter = wTmp_1;
                int dataLength = 0;
                VBYTE b;
                std::vector<VBYTE> dataVbyte;
                int frameNumberToRestore = currentFrameNumber;
                while(true)
                {
                    if(areFramesNeeded && (counter >= frameMap[currentFrameNumber])) counter = loadFrame(counter);
                    if(isError) goto EXCEPTION;
                    b = AS->codeData[counter++];
                    if((b == 0x3) && (AS->codeData[counter] == 0xD)) break;
                    ++dataLength;
                    dataVbyte.push_back(b);
                    std::cout << std::hex << b << std::endl;
                }
                currentFrameNumber = frameNumberToRestore;
                restoreFrame();
                if(isError) goto EXCEPTION;
                VBYTE *dataToWrite = &dataVbyte[0];
                std::string arg1 = "";
                getDataFromCodeData(arg1, wTmp_0);
                REGS->R[6] = sysBus->createFile(arg1, dataToWrite, dataLength);
            }
            break;
        case SYSBUS_DELETE_FILE:
            {
                wTmp_0 = AS->stack[REGS->SP];
                REGS->SP += 1;
                std::string arg1 = "";
                getDataFromCodeData(arg1, wTmp_0);
                REGS->R[6] = sysBus->deleteFile(arg1);
            }
            break;
        case SYSBUS_MOVE_FILE:
            {
                wTmp_1 = AS->stack[REGS->SP]; // second arg
                REGS->SP += 1;
                wTmp_0 = AS->stack[REGS->SP]; // first arg
                REGS->SP += 1;
                std::string arg1 = "";
                std::string arg2 = "";
                getDataFromCodeData(arg2, wTmp_1);
                getDataFromCodeData(arg1, wTmp_0);
                REGS->R[6] = sysBus->moveFile(arg1, arg2);
            }
            break;
        case SYSBUS_COPY_FILE:
            {
                wTmp_1 = AS->stack[REGS->SP]; // second arg
                REGS->SP += 1;
                wTmp_0 = AS->stack[REGS->SP]; // first arg
                REGS->SP += 1;
                std::string arg1 = "";
                std::string arg2 = "";
                getDataFromCodeData(arg2, wTmp_1);
                getDataFromCodeData(arg1, wTmp_0);
                REGS->R[6] = sysBus->copyFile(arg1, arg2);
            }
            break;
        default:
            REGS->R[7] = 1;
    }
}

/*
    PUSH REGISTER
    90 03 => PUSH R3
*/
void VMCPU::funcPush()
{
    VBYTE bTmp_0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] PUSH" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    REGS->SP -= 1;
    if(REGS->SP == 0xFFFFFFFF) {
        #ifdef _VM_CPU_TEST_
            vcpuFlag = VCpuFlag::OVERFLOW;
        #endif
        #ifndef _VM_CPU_TEST_
            std::cout << "[ERROR] STACK OVERFLOW!" << std::endl;
        #endif
        funcException("stack overflow!");
    }
    AS->stack[REGS->SP] = REGS->R[bTmp_0];
    #ifdef V_DEBUG
        std::cout <<"val: " << std::hex << AS->stack[REGS->SP] << std::endl;
    #endif
}

/*
    POP TO A REGISTER
    91 03 => POP R3
*/
void VMCPU::funcPop()
{
    VBYTE bTmp_0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] POP" << std::endl;
    #endif
    bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 > 8) funcException("register index out of range!");
    if((&AS->stack[REGS->SP]) == (&AS->stack[sizeof(AS->stack)/sizeof(VDWORD)])){
        #ifdef _VM_CPU_TEST_
            vcpuFlag = VCpuFlag::UNDERFLOW;
        #endif
        #ifndef _VM_CPU_TEST_
            std::cout << "[ERROR] STACK UNDERFLOW!" << std::endl;
        #endif
        funcException("stack underflow!");
    }
    REGS->R[bTmp_0] = AS->stack[REGS->SP];
    REGS->SP += 1;
}

/*
    Clear the stack
    92 => CLST
*/
void VMCPU::funcClSt()
{
    #ifdef V_DEBUG
        std::cout << "[DEBUG] CLST" << std::endl;
    #endif
    memset(AS->stack, 0, STACK_SIZE*sizeof(*(AS->stack)));
    REGS->SP = 0;
}

/*
    Set the stack pointer
    93 01 00 00 00 => SETSP 1
*/
void VMCPU::funcSetSp()
{
    #ifdef V_DEBUG
        std::cout << "[DEBUG] SETSP" << std::endl;
    #endif
    REGS->SP = 0;
    REGS->SP = *(VDWORD *) &AS->codeData[REGS->PC];
    if(REGS->SP > STACK_SIZE) funcException("stack pointer to high!");
    REGS->PC += 4;
}

/*
    POC - Print char without new line
        the value must be at the top of
        the stack
    A0 => POC
*/
void VMCPU::funcPoc()
{
    VBYTE bTmp_0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] POC" << std::endl;
    #endif
    if(&AS->stack[REGS->SP] == &AS->stack[sizeof(AS->stack)/sizeof(VDWORD)]){
        #ifdef _VM_CPU_TEST_
            vcpuFlag = VCpuFlag::UNDERFLOW;
        #endif
        #ifndef _VM_CPU_TEST_
            std::cout << "[ERROR] STACK UNDERFLOW!" << std::endl;
        #endif
        funcException("stack underflow!");
    }
    if(areFramesNeeded && (AS->stack[REGS->SP] >= frameMap[currentFrameNumber]))
    {
        std::vector<VBYTE> bytes = getByteFromFrame(AS->stack[REGS->SP++], 1);
        bTmp_0 = bytes[0];
    }
    else bTmp_0 = *(VBYTE*) &AS->codeData[AS->stack[REGS->SP++]];
    if(isError) funcException("get byte from frame!");
    vmPrint(bTmp_0);
}

/*
    POCN - Print char with new line
        the value must be at the top of
        the stack
    A1 => POCN
*/
void VMCPU::funcPocn()
{
    VBYTE bTmp_0;
    #ifdef V_DEBUG
        std::cout << "[DEBUG] POCN" << std::endl;
    #endif
    if(&AS->stack[REGS->SP] == &AS->stack[sizeof(AS->stack)/sizeof(VDWORD)]){
        #ifdef _VM_CPU_TEST_
            vcpuFlag = VCpuFlag::UNDERFLOW;
        #endif
        #ifndef _VM_CPU_TEST_
            std::cout << "[ERROR] STACK UNDERFLOW!" << std::endl;
        #endif
        funcException("stack underflow!");
    }
    if(areFramesNeeded && (AS->stack[REGS->SP] >= frameMap[currentFrameNumber]))
    {
        std::vector<VBYTE> bytes = getByteFromFrame(AS->stack[REGS->SP++], 1);
        bTmp_0 = bytes[0];
    }
    else bTmp_0 = *(VBYTE*) &AS->codeData[AS->stack[REGS->SP++]];
    if(isError) funcException("get byte from frame!");
    vmPrintN(bTmp_0);
}

/*
    TIB - Take input and move to the data buffer,
            the length of the string is stored in R[7]
    A2 => TIB
*/
void VMCPU::funcTib()
{
    #ifdef V_DEBUG
        std::cout << "[DEBUG] TIB" << std::endl;
    #endif
    {
        memset(AS->dataBuffer, 0, INPUT_BUFFER_SIZE*sizeof(*(AS->dataBuffer)));
        std::string inData = "";
        std::getline(std::cin, inData);
        VBYTE endOfText = 0x3;
        if(inData.length() > (INPUT_BUFFER_SIZE - 1)) 
        {
            inData = inData.substr(0, (INPUT_BUFFER_SIZE - 1));
        }
        int counter = 0;
        for(char const &c: inData)
        {
            AS->dataBuffer[counter++] = c;
        }
        AS->dataBuffer[counter++] = endOfText;
        REGS->R[7] = 0;
        *(VWORD *) &REGS->R[7] = counter;
    }
}

/*
    GIC - Get a specific char from input, that is stored in the data buffer,
            the value will be stored in R[7],
            pass the position of char via a some register
    A3 02 => GIC R2
*/
void VMCPU::funcGic()
{
    #ifdef V_DEBUG
        std::cout << "[DEBUG] GIC" << std::endl;
    #endif
    VBYTE bTmp_0 = AS->codeData[REGS->PC++];
    if(bTmp_0 >= 0 && bTmp_0 <= 7)
    {
        if(REGS->R[bTmp_0] >= INPUT_BUFFER_SIZE) funcException("input buffer too big!");
        REGS->R[7] = 0;
        *(VBYTE *) &REGS->R[7] = AS->dataBuffer[REGS->R[bTmp_0]];
    }
    else funcException("register index out of range!");
}

/*
    PIC - Print char from input without new line
        the value must be at the top of
        the stack
    A4 => PIC
*/
void VMCPU::funcPic()
{
    #ifdef V_DEBUG
        std::cout << "[DEBUG] PIC" << std::endl;
    #endif
    if(&AS->stack[REGS->SP] == &AS->stack[sizeof(AS->stack)/sizeof(VDWORD)]){
        #ifdef _VM_CPU_TEST_
            vcpuFlag = VCpuFlag::UNDERFLOW;
        #endif
        #ifndef _VM_CPU_TEST_
            std::cout << "[ERROR] STACK UNDERFLOW!" << std::endl;
        #endif
        funcException("stack underflow!");
    }
    VBYTE bTmp_0 = AS->stack[REGS->SP++];
    vmPrint(bTmp_0);
}

/*
    PICN - Print char from input with new line
        the value must be at the top of
        the stack
    A5 => POCN
*/
void VMCPU::funcPicn()
{
    #ifdef V_DEBUG
        std::cout << "[DEBUG] PICN" << std::endl;
    #endif
    if(&AS->stack[REGS->SP] == &AS->stack[sizeof(AS->stack)/sizeof(VDWORD)]){
        #ifdef _VM_CPU_TEST_
            vcpuFlag = VCpuFlag::UNDERFLOW;
        #endif
        #ifndef _VM_CPU_TEST_
            std::cout << "[ERROR] STACK UNDERFLOW!" << std::endl;
        #endif
        funcException("stack underflow!");
    }
    VBYTE bTmp_0 = AS->stack[REGS->SP++];
    vmPrintN(bTmp_0);
}

/*
    PXV - Print a value in hex;
        the value must be at the top of the stack
    A6 => PXV
*/
void VMCPU::funcPxv()
{
    #ifdef V_DEBUG
        std::cout << "[DEBUG] PXV" << std::endl;
    #endif
    if(&AS->stack[REGS->SP] == &AS->stack[sizeof(AS->stack)/sizeof(VDWORD)]){
        #ifdef _VM_CPU_TEST_
            vcpuFlag = VCpuFlag::UNDERFLOW;
        #endif
        #ifndef _VM_CPU_TEST_
            std::cout << "[ERROR] STACK UNDERFLOW!" << std::endl;
        #endif
        funcException("stack underflow!");
    }
    VDWORD dTmp_0 = AS->stack[REGS->SP++];
    vmPrintHX(dTmp_0);
}

/*
    PXVN - Print a value in hex with a new line;
        the value must be at the top of the stack
    A7 => PXV
*/
void VMCPU::funcPxvn()
{
    #ifdef V_DEBUG
        std::cout << "[DEBUG] PXVN" << std::endl;
        // std::cout << std::bitset<32>(AS->stack[REGS->SP]) << std::endl;
    #endif
    if(&AS->stack[REGS->SP] == &AS->stack[sizeof(AS->stack)/sizeof(VDWORD)]){
        #ifdef _VM_CPU_TEST_
            vcpuFlag = VCpuFlag::UNDERFLOW;
        #endif
        #ifndef _VM_CPU_TEST_
            std::cout << "[ERROR] STACK UNDERFLOW!" << std::endl;
        #endif
        funcException("stack underflow!");
    }
    VDWORD dTmp_0 = AS->stack[REGS->SP++];
    vmPrintHXN(dTmp_0);
}