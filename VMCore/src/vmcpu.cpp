#include "../include/vmcpu.hpp"

#ifdef _WIN32_DEV_ENVIRONMENT
    #include <string>
#endif

// #include <bitset>

VMCPU::VMCPU()
{
    AS = (PADDRESS_SPACE) new ADDRESS_SPACE;
    REGS = (PREGISTERSS) new REGISTERSS;

    memset(AS->codeData, 0, sizeof(AS->codeData));

    REGS->PC = 0;
    REGS->SP = sizeof(AS->stack) / sizeof(VDWORD);

    areFramesNeeded = false;
    currentFrameNumber = 0;
    isError = false;

    #ifdef _WIN32_DEV_ENVIRONMENT
        sysBus = new WIN32();
    #else //_LINUX_DEV_ENVIRONMENT
        sysBus = new UNIX();
    #endif


    dicOpcodesFunction.insert(std::make_pair(0x56, &VMCPU::funcNop));
    dicOpcodesFunction.insert(std::make_pair(0x6d, &VMCPU::funcNop));
    dicOpcodesFunction.insert(std::make_pair(NOP, &VMCPU::funcNop));
    dicOpcodesFunction.insert(std::make_pair(EE, &VMCPU::funcEE));
    dicOpcodesFunction.insert(std::make_pair(MOV, &VMCPU::funcMov));
    dicOpcodesFunction.insert(std::make_pair(MOVMB, &VMCPU::funcMovmb));
    dicOpcodesFunction.insert(std::make_pair(MOVMW, &VMCPU::funcMovmw));
    dicOpcodesFunction.insert(std::make_pair(MOVB, &VMCPU::funcMovb));
    dicOpcodesFunction.insert(std::make_pair(MOVW, &VMCPU::funcMovw));
    dicOpcodesFunction.insert(std::make_pair(MOVBM, &VMCPU::funcMovbm));
    dicOpcodesFunction.insert(std::make_pair(MOVWM, &VMCPU::funcMovwm));
    dicOpcodesFunction.insert(std::make_pair(MOVMRB, &VMCPU::funcMovmrb));
    dicOpcodesFunction.insert(std::make_pair(MOVMRW, &VMCPU::funcMovmrw));
    dicOpcodesFunction.insert(std::make_pair(MOVMD, &VMCPU::funcMovmd));
    dicOpcodesFunction.insert(std::make_pair(MOVD, &VMCPU::funcMovd));
    dicOpcodesFunction.insert(std::make_pair(MOVDM, &VMCPU::funcMovdm));
    dicOpcodesFunction.insert(std::make_pair(MOVMRD, &VMCPU::funcMovmrd));
    dicOpcodesFunction.insert(std::make_pair(JMP, &VMCPU::funcJmp));
    dicOpcodesFunction.insert(std::make_pair(JZ, &VMCPU::funcJz));
    dicOpcodesFunction.insert(std::make_pair(JNZ, &VMCPU::funcJnz));
    dicOpcodesFunction.insert(std::make_pair(JAE, &VMCPU::funcJae));
    dicOpcodesFunction.insert(std::make_pair(JBE, &VMCPU::funcJbe));
    dicOpcodesFunction.insert(std::make_pair(JB, &VMCPU::funcJb));
    dicOpcodesFunction.insert(std::make_pair(JA, &VMCPU::funcJa));
    dicOpcodesFunction.insert(std::make_pair(ADVR, &VMCPU::funcAdvr));
    dicOpcodesFunction.insert(std::make_pair(ADRR, &VMCPU::funcAdrr));
    dicOpcodesFunction.insert(std::make_pair(ADRRL, &VMCPU::funcAdrrl));
    dicOpcodesFunction.insert(std::make_pair(SUBVR, &VMCPU::funcSubvr));
    dicOpcodesFunction.insert(std::make_pair(SUBRR, &VMCPU::funcSubrr));
    dicOpcodesFunction.insert(std::make_pair(SUBRRL, &VMCPU::funcSubrrl));
    dicOpcodesFunction.insert(std::make_pair(XOR, &VMCPU::funcXor));
    dicOpcodesFunction.insert(std::make_pair(XORL, &VMCPU::funcXorl));
    dicOpcodesFunction.insert(std::make_pair(NOT, &VMCPU::funcNot));
    dicOpcodesFunction.insert(std::make_pair(NOTB, &VMCPU::funcNotb));
    dicOpcodesFunction.insert(std::make_pair(ADVRD, &VMCPU::funcAdvrd));
    dicOpcodesFunction.insert(std::make_pair(SUBVRD, &VMCPU::funcSubvrd));
    dicOpcodesFunction.insert(std::make_pair(SHR, &VMCPU::funcShr));
    dicOpcodesFunction.insert(std::make_pair(SHL, &VMCPU::funcShl));
    dicOpcodesFunction.insert(std::make_pair(CMP, &VMCPU::funcCmp));
    dicOpcodesFunction.insert(std::make_pair(CMPL, &VMCPU::funcCmpl));
    dicOpcodesFunction.insert(std::make_pair(VMSYSBUS, &VMCPU::funcVmSysbus));
    dicOpcodesFunction.insert(std::make_pair(PUSH, &VMCPU::funcPush));
    dicOpcodesFunction.insert(std::make_pair(POP, &VMCPU::funcPop));
    dicOpcodesFunction.insert(std::make_pair(CLST, &VMCPU::funcClSt));
    dicOpcodesFunction.insert(std::make_pair(SETSP, &VMCPU::funcSetSp));
    dicOpcodesFunction.insert(std::make_pair(POC, &VMCPU::funcPoc));
    dicOpcodesFunction.insert(std::make_pair(POCN, &VMCPU::funcPocn));
    dicOpcodesFunction.insert(std::make_pair(TIB, &VMCPU::funcTib));
    dicOpcodesFunction.insert(std::make_pair(GIC, &VMCPU::funcGic));
    dicOpcodesFunction.insert(std::make_pair(PIC, &VMCPU::funcPic));
    dicOpcodesFunction.insert(std::make_pair(PICN, &VMCPU::funcPicn));
    dicOpcodesFunction.insert(std::make_pair(PXV, &VMCPU::funcPxv));
    dicOpcodesFunction.insert(std::make_pair(PXVN, &VMCPU::funcPxvn));
}

VMCPU::~VMCPU()
{
    if(areFramesNeeded)
    {
        std::string fileNameToRemove;
        for (const auto& [key, value] : frameMap) 
        {
            fileNameToRemove = "./.cached." + std::to_string(key) + ".frame";
            int retVal = sysBus->deleteFile(fileNameToRemove);
        }
    }
    delete AS;
    delete REGS;
    delete sysBus;
    dicOpcodesFunction.clear();
}

void VMCPU::getDataFromCodeData(std::string &arg1, int startFrom)
{
    int counter = startFrom;
    int frameNumberToRestore = currentFrameNumber;
    std::stringstream ss;
    VBYTE b;
    while(true)
    {
        if(areFramesNeeded && (counter >= CODE_DATA_SIZE)) counter = loadFrame(counter);
        b = AS->codeData[counter++];
        if((b == 0x3) && (AS->codeData[counter] == 0xD)) break;
        ss << std::hex << b;
    }
    arg1 = ss.str();
    currentFrameNumber = frameNumberToRestore;
    restoreFrame();
    return;
} 

bool VMCPU::loadCode(VBYTE *mcode, int mcsize)
{
    memset(AS->codeData, 0, CODE_DATA_SIZE*sizeof(*(AS->codeData)));
    memset(AS->stack, 0, STACK_SIZE*sizeof(*(AS->stack)));
    memset(AS->dataBuffer, 0, INPUT_BUFFER_SIZE*sizeof(*(AS->dataBuffer)));
    if((unsigned) (mcsize) > (sizeof(AS->codeData) / sizeof(AS->codeData[0]))) 
    {
        std::cout << "[ERROR 101001] TOO BIG A CODE TO EXECUTE!\n";
        return false;
    }
    memcpy(AS->codeData, mcode, mcsize);
    for(int i = 0; i < 8; i++)
    {
        REGS->R[i] = (VDWORD) 0;
    }
    REGS->CF = 0;
    REGS->ZF = 0;
    return true;
}

void VMCPU::vmPrint(VBYTE s)
{
    std::cout << s;
}

void VMCPU::vmPrintHX(VDWORD hx)
{
    std::cout << std::hex << hx;
}

void VMCPU::vmPrintN(VBYTE s)
{
    std::cout << s << std::endl;
}

void VMCPU::vmPrintHXN(VDWORD hx)
{
    std::cout << std::hex << hx << std::endl;
}

void VMCPU::run()
{
    bool exit = false;
    VBYTE opcode;

    while(!exit)
    {
        if(areFramesNeeded && (REGS->PC >= frameMap[currentFrameNumber])) REGS->PC = loadFrame(REGS->PC);
        if(isError) return;
        opcode = AS->codeData[REGS->PC++];
        exit = executer(opcode);
    }
    return;
}

void VMCPU::writeByteIntoFrame(int bytePosition, int howManyBytes, std::vector<VBYTE> bytes)
{
    #ifdef V_DEBUG
        std::cout << "[DEBUG] Write bytes into frame" << std::endl;
    #endif
    auto sum = 0;
    auto frameNumber = -1;
    for (const auto& [key, value] : frameMap) {
        sum += value;
        if(sum >= (bytePosition + 1))
        {
            frameNumber = key;
            break;
        }
    }
    if(frameNumber == -1) goto error_getByteFromFrame;
    else
    {
        char byte;
        std::vector<VBYTE> readData;
        std::string frameToLoadName = ".cached." + std::to_string(frameNumber) + ".frame";
        std::ifstream fileBinToRead;
        std::ofstream fileBinToWrite;
        fileBinToRead.open(frameToLoadName, std::ios::binary);
        auto positionToGet = frameMap[frameNumber] - (sum - bytePosition);
        auto counter = 0;
        if(fileBinToRead.is_open())
        {
            while(fileBinToRead.get(byte))
            {
                ++counter;
                readData.push_back(byte);
            }
            fileBinToRead.close();
            
            for(auto i = 0; i < howManyBytes; i++) readData[positionToGet++] = bytes[i];
            
            fileBinToWrite.open(frameToLoadName.c_str(), std::fstream::out | std::ios::binary);
            VBYTE *dataToWrite = &readData[0];
            fileBinToWrite.write((char*)dataToWrite, counter-1);
            fileBinToWrite.close();
        }
        else goto error_getByteFromFrame;
    }
    goto ok_getByteFromFrame;

error_getByteFromFrame:
    #ifdef V_DEBUG
        std::cout << "[ERROR] Write bytes into frame" << std::endl;
    #endif
    isError = true;
    return;
ok_getByteFromFrame:
    isError = false;
    return;
}

std::vector<VBYTE> VMCPU::getByteFromFrame(int bytePosition, int howManyBytes)
{
    #ifdef V_DEBUG
        std::cout << "[DEBUG] Get a byte from frame" << std::endl;
    #endif
    std::vector<VBYTE> readBytes;
    char byte;
    auto sum = 0;
    auto frameNumber = -1;
    for (const auto& [key, value] : frameMap) {
        sum += value;
        if(sum >= (bytePosition + 1))
        {
            frameNumber = key;
            break;
        }
    }
    if(frameNumber == -1) goto error_getByteFromFrame;
    else
    {
        std::string frameToLoadName = ".cached." + std::to_string(frameNumber) + ".frame";
        std::ifstream fileBinToRead;
        fileBinToRead.open(frameToLoadName, std::ios::binary);
        auto positionToGet = frameMap[frameNumber] - (sum - bytePosition);
        if(fileBinToRead.is_open())
        {
            auto counter = 0;
            while(fileBinToRead.get(byte))
            {
                if(counter == positionToGet)
                { 
                    readBytes.push_back(byte);
                    --howManyBytes;
                    ++positionToGet;
                    if(howManyBytes == 0) break;
                }
                ++counter;
            }
            fileBinToRead.close();
        }
        else goto error_getByteFromFrame;
    }
    goto ok_getByteFromFrame;

error_getByteFromFrame:
    #ifdef V_DEBUG
        std::cout << "[ERROR] Failed get a byte from frame" << std::endl;
    #endif
    isError = true;
    readBytes.push_back(0x0);
    return readBytes;
ok_getByteFromFrame:
    isError = false;
    return readBytes;
}

int VMCPU::loadFrame(int pc)
{
    #ifdef V_DEBUG
        std::cout << "[DEBUG] Load a frame" << std::endl;
    #endif
    auto sum = 0;
    auto frameNumber = -1;
    for (const auto& [key, value] : frameMap) {
        sum += value;
        if(sum >= (pc+1))
        {
            frameNumber = key;
            break;
        }
    }

    if(frameMap[currentFrameNumber] == pc)
    {
        frameNumber = currentFrameNumber + 1;
        pc = 0;
        sum = frameMap[frameNumber];
    }

    #ifdef V_DEBUG
        std::cout << "[DEBUG] Frame number: " << frameNumber << std::endl;
    #endif
    if(frameNumber == -1) goto error_loadFrame;
    else
    {
        std::string frameToLoadName = ".cached." + std::to_string(frameNumber) + ".frame";
        std::ifstream fileBinToRead;
        fileBinToRead.open(frameToLoadName, std::ios::binary);
        if(fileBinToRead.is_open())
        {
            char vb;
            auto counter = 0;
            memset(AS->codeData, 0, CODE_DATA_SIZE*sizeof(*(AS->codeData)));
            while(fileBinToRead.get(vb)) AS->codeData[counter++] = vb;
            fileBinToRead.close();
        }
        else goto error_loadFrame;
    }
    goto ok_loadFrame;

error_loadFrame:
    #ifdef V_DEBUG
        std::cout << "[ERROR] Failed load a frame" << std::endl;
    #endif
    isError = true;
    return 0;

ok_loadFrame:
    currentFrameNumber = frameNumber;
    isError = false;
    return (frameMap[frameNumber] - (sum - pc));
}

void VMCPU::restoreFrame()
{
    #ifdef V_DEBUG
        std::cout << "[DEBUG] Restore a frame" << std::endl;
    #endif
    std::string frameToLoadName = ".cached." + std::to_string(currentFrameNumber) + ".frame";
    std::ifstream fileBinToRead;
    fileBinToRead.open(frameToLoadName, std::ios::binary);
    if(fileBinToRead.is_open())
    {
        char vb;
        auto counter = 0;
        memset(AS->codeData, 0, CODE_DATA_SIZE*sizeof(*(AS->codeData)));
        while(fileBinToRead.get(vb)) AS->codeData[counter++] = vb;
        fileBinToRead.close();
    }
    else goto error_restoreFrame;
    goto ok_restoreFrame;

error_restoreFrame:
    #ifdef V_DEBUG
        std::cout << "[ERROR] Failed restore a frame" << std::endl;
    #endif
    isError = true;
    return;
ok_restoreFrame:
    isError = false;
    return;
}