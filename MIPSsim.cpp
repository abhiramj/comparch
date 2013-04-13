/* On my honour, I have neither given nor received
 * any unauthorized aid in this assignment
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sstream>
#include<assert.h>
#include<math.h>
#include<iostream>
#include<algorithm>
#include<new>
#include<vector>
#include<fstream>
#include<string>
#include<map>
#include<iomanip>
#include<queue>
#include<deque>
// definitions
enum OpCode{
    // category -1
    J, JR, BEQ, BLTZ, BGTZ, BREAK, SW, LW, SLL, SRL, SRA, NOP,
    // category -2
    ADD, SUB, MUL, AND, OR, XOR, NOR, SLT, ADDI, ANDI, ORI, XORI
};
using namespace std;

// Classes and definitions----
// Instruction holds the instruction as a set
class Instruction{
    public:
        Instruction(){
            rd =0;
            rt =0;
            rs =0;
            immediate = 0;
            stringFormat = "";
            instructionAddress = 0;
        }
        OpCode opcode;
        int rd;
        int rt;
        int rs;   // works as sa for SLL, SRL
        int immediate; // works as offset for Jumps and Branches
        string stringFormat;
        int instructionAddress;
        int GetWriteOperands();
        bool Empty();
        vector<int > GetReadOperands();
};
bool Instruction::Empty(){
    if (instructionAddress == 0){
        return true;
    }
    return false;
}
vector<int > Instruction::GetReadOperands(){
    vector<int > operands;
    switch(opcode){
        case AND:
        case ADD:
        case SUB:
        case MUL:
        case OR:
        case XOR:
        case SLT:
        case SLL:
        case SRA:
        case SRL:
            operands.push_back(rs);
            operands.push_back(rt);
        case ADDI:
        case ANDI:
        case ORI:
        case XORI:
        case JR:
        case LW:
        case SW:
            operands.push_back(rs);
            break;
        default:
            cout<<"No operands to read for "<<stringFormat<<endl;
            break;
    }
    return operands;
}
int Instruction::GetWriteOperands(){
    switch(opcode){
        case AND:
        case ADD:
        case SUB:
        case MUL:
        case OR:
        case XOR:
        case SLT:
        case SLL:
        case SRA:
        case SRL:
            return rd;
            break;
        case ADDI:
        case ANDI:
        case ORI:
        case XORI:
        case LW:
            return rt;
        default:
            cout<<"No operands to write for "<<stringFormat<<endl;
            break;
    }
    return -1;

}
int GetWriteOperands(Instruction currentInstruction);
queue<int > GetReadOperands(Instruction currentInstruction);
bool isWARHazard(Instruction currentInstruction);
bool isWAWHazard(Instruction currentInstruction);
bool isRAWHazard(Instruction currentInstruction);
class Disassembler{
    public:
        int memstart;
        int memend;
        int PC;
        ofstream disassembly;
        ofstream simulation;

        vector <Instruction > instructionSet;
        Disassembler(int startMemory = 256):disassembly("disassembly.txt"), simulation("simulation.txt")
    {
        memstart = startMemory;
        PC = 256;
    };
        ~Disassembler(){
            if (disassembly.is_open()){
                disassembly.close();
            }
            if (simulation.is_open()){
                simulation.close();
            }
        };
        void ParseInstruction(string, bool &, int);
        void ParseRegRegOperands(string , int &, int &, int &);
        void ParseShiftOperands(string instruction, int &, int &, int &);
        void ParseRegMemOperands(string, int &, int &, int &);
        void ParseJumpOperands(string , int &);
        int GetValueFromStr(string, bool, bool);
        void AdvancePC();
        void DoADDInstruction(Instruction instruction);
        void DoSUBInstruction(Instruction instruction);
        void DoMULInstruction(Instruction instruction);
        void DoANDInstruction(Instruction instruction);
        void DoORInstruction(Instruction instruction);
        void DoXORInstruction(Instruction instruction);
        void DoNORInstruction(Instruction instruction);
        void DoSLTInstruction(Instruction instruction);
        void DoADDIInstruction(Instruction instruction);
        void DoANDIInstruction(Instruction instruction);
        void DoORIInstruction(Instruction instruction);
        void DoXORIInstruction(Instruction instruction);
        void DoJInstruction(Instruction instruction);
        void DoJRInstruction(Instruction instruction);
        void DoBEQInstruction(Instruction instruction);
        void DoBLTZInstruction(Instruction instruction);
        void DoBGTZInstruction(Instruction instruction);
        void DoNOPInstruction(Instruction instruction);
        void DoSWInstruction(Instruction instruction);
        void DoLWInstruction(Instruction instruction);
        void DoSLLInstruction(Instruction instruction);
        void DoSRLInstruction(Instruction instruction);
        void DoSRAInstruction(Instruction instruction);

} disassembler;


//globals
static  map<string, OpCode> opcodeMap; // map from opcode to operation
// registers
static int registers[32];
// memory
map<int,int > memory;

void InitializeMemory(int memloc, int value){
    memory[memloc] = value;

}

int Disassembler::GetValueFromStr(string input, bool isSigned = true, bool isShifted = false){
    int numBits = input.length();
    string shiftedString; // calculating shifted string for offsets
    const char* bitVal = NULL;
    if (isShifted){
        shiftedString = input;
        shiftedString.append("00");
        bitVal = shiftedString.c_str();
        numBits += 2;
    }
    else {
        bitVal = input.c_str();
    }
    int parseVal = 0;
    for (int i =0; i < numBits; i++){
        int currVal  = bitVal[i] -'0';
        currVal = currVal*pow(2,numBits-(i+1));
        if (i==0 && isSigned){
            currVal *= -1;
        }
        parseVal += currVal;
    }
    return parseVal;
}

void Disassembler::ParseInstruction(string instruction, bool &isEndInstruction, int instructionAddress){
    string opcodeField = instruction.substr(0,6);
    ostringstream ss;

    disassembly<<instruction<<"\t"<<disassembler.PC<<"\t";
    if (!isEndInstruction){
        Instruction currInstruction;
        switch(opcodeMap[opcodeField]){
            case J:
                ss<<"J";
                break; 
            case JR:
                ss<<"JR";
                break; 
            case BEQ:
                ss<<"BEQ";
                break; 
            case BLTZ:
                ss<<"BLTZ";
                break; 
            case BGTZ:
                ss<<"BGTZ";
                break; 
            case BREAK:
                ss<<"BREAK";
                disassembler.memstart = (disassembler.PC) +4;
                isEndInstruction = true;
                break; 
            case SW:
                ss<<"SW";
                break; 
            case LW:
                ss<<"LW";
                break; 
            case SLL:
                ss<<"SLL";
                break; 
            case SRL:
                ss<<"SRL";
                break; 
            case SRA:
                ss<<"SRA";
                break; 
            case NOP:
                ss<<"NOP";
                break; 
            case ADD:
                ss<<"ADD";
                break; 
            case SUB:
                ss<<"SUB";
                break;
            case MUL:
                ss<<"MUL";
                break; 
            case AND:
                ss<<"AND";
                break; 
            case OR:
                ss<<"OR";
                break; 
            case XOR:
                ss<<"XOR";
                break; 
            case NOR:
                ss<<"NOR";
                break; 
            case SLT:
                ss<<"SLT";
                break; 
            case ADDI:
                ss<<"ADDI";
                break; 
            case ANDI:
                ss<<"ANDI";
                break; 
            case ORI:
                ss<<"ORI";
                break; 
            case XORI:
                ss<<"XORI";
                break; 
            default:
                ss<<opcodeField;
                break;
        }
        currInstruction.opcode = opcodeMap[opcodeField];
        switch (currInstruction.opcode){
            // parsing reg-reg instructions
            case AND:
            case ADD:
            case SUB:
            case MUL:
            case OR:
            case XOR:
            case NOR:
            case SLT:
                ParseRegRegOperands(instruction, currInstruction.rd,
                        currInstruction.rs, currInstruction.rt);
                ss<<" R"<<currInstruction.rd<<", R"<<currInstruction.rs;
                ss<<", R"<<currInstruction.rt<<endl;
                break;
            case ADDI:
            case ANDI:
            case ORI:
            case XORI:
                ParseRegMemOperands(instruction, currInstruction.rt,
                        currInstruction.rs, currInstruction.immediate);
                ss<<" R"<<currInstruction.rt<<", R"<<currInstruction.rs;
                ss<<", #"<<currInstruction.immediate<<endl;
                break;
            case LW:
            case SW:
                ParseRegMemOperands(instruction, currInstruction.rt,
                        currInstruction.rs, currInstruction.immediate);
                ss<<" R"<<currInstruction.rt<<", ";
                ss<<currInstruction.immediate<<"(R"<<currInstruction.rs<<")"<<endl;
                break;
            case SLL:
            case SRA:
            case SRL:
                ParseShiftOperands(instruction, currInstruction.rd, currInstruction.rs, currInstruction.rt );
                ss<<" R"<<currInstruction.rd<<", R"<<currInstruction.rt;
                ss<<", #"<<currInstruction.rs<<endl;
                break;
                // we just need rs
            case JR:
                ParseRegRegOperands(instruction, currInstruction.rd,
                        currInstruction.rs, currInstruction.rt); // other values are purely junk
                ss<<" R"<<currInstruction.rs<<endl;
                break;
            case BEQ:
                ParseRegMemOperands(instruction, currInstruction.rt,
                        currInstruction.rs, currInstruction.immediate);
                currInstruction.immediate = currInstruction.immediate<<2;
                ss<<" R"<<currInstruction.rs<<", R"<<currInstruction.rt;
                ss<<", #"<<currInstruction.immediate<<endl;
                break;
            case BLTZ:
            case BGTZ:
                ParseRegMemOperands(instruction, currInstruction.rt,
                        currInstruction.rs, currInstruction.immediate);
                currInstruction.immediate = currInstruction.immediate<<2;
                ss<<" R"<<currInstruction.rs;
                ss<<", #"<<currInstruction.immediate<<endl;
                break;
            case J:
                ParseJumpOperands(instruction,currInstruction.immediate);
                ss<<" #"<<currInstruction.immediate<<endl;
                break;
            case BREAK:
                ss<<endl;
                break;

        }
        // store the instruction
        currInstruction.stringFormat = ss.str();
        currInstruction.instructionAddress = instructionAddress;
        disassembly<<ss.str();
        instructionSet.push_back(currInstruction);


    } else {
        disassembly<<GetValueFromStr(instruction)<<endl;
        InitializeMemory(PC, GetValueFromStr(instruction));
    }
    PC +=4;

}

void Disassembler::ParseRegRegOperands(string instruction, int &rd, int &rs, int &rt){
    rs = GetValueFromStr(instruction.substr(6,5), false);
    rt = GetValueFromStr(instruction.substr(11,5), false);
    rd = GetValueFromStr(instruction.substr(16,5), false);
}

void Disassembler::ParseShiftOperands(string instruction, int &rd, int &sa, int &rt){
    sa = GetValueFromStr(instruction.substr(21,5), false);
    rt = GetValueFromStr(instruction.substr(11,5), false);
    rd = GetValueFromStr(instruction.substr(16,5), false);
}
void Disassembler::ParseRegMemOperands(string instruction, int &rt, int &rs, int &immediate){
    rs = GetValueFromStr(instruction.substr(6,5), false);
    rt = GetValueFromStr(instruction.substr(11,5), false);
    immediate = GetValueFromStr(instruction.substr(16,16));
}

void Disassembler::ParseJumpOperands(string instruction, int &offset){
    offset = GetValueFromStr(instruction.substr(6,26));
    offset = offset<<2;

}

void Disassembler::DoADDInstruction(Instruction instruction){
    registers[instruction.rd] = registers[instruction.rs] + registers[instruction.rt];
    AdvancePC();
}

void Disassembler::DoADDIInstruction(Instruction instruction){
    registers[instruction.rt] = registers[instruction.rs] + instruction.immediate;
    AdvancePC();
}

void Disassembler::DoSUBInstruction(Instruction instruction){
    registers[instruction.rd] = registers[instruction.rs] - registers[instruction.rt];
    AdvancePC();
}

void Disassembler::DoORInstruction(Instruction instruction){
    registers[instruction.rd] = registers[instruction.rs] | registers[instruction.rt];
    AdvancePC();
}

void Disassembler::DoORIInstruction(Instruction instruction){
    registers[instruction.rt] = registers[instruction.rs] | instruction.immediate;
    AdvancePC();
}


void Disassembler::DoXORIInstruction(Instruction instruction){
    registers[instruction.rt] = registers[instruction.rs] ^ instruction.immediate;
    AdvancePC();
}

void Disassembler::DoANDIInstruction(Instruction instruction){
    registers[instruction.rt] = registers[instruction.rs] & instruction.immediate;
    AdvancePC();
}

void Disassembler::DoANDInstruction(Instruction instruction){
    registers[instruction.rd] = registers[instruction.rs] & registers[instruction.rt];
    AdvancePC();
}

void Disassembler::DoNORInstruction(Instruction instruction){
    registers[instruction.rd] = ~(registers[instruction.rs] | registers[instruction.rt]);
    AdvancePC();
}

void Disassembler::DoSLTInstruction(Instruction instruction){
    if (registers[instruction.rs] < registers[instruction.rt])
        registers[instruction.rd] = 1;
    else registers[instruction.rd] = 0;
    AdvancePC();
}

void Disassembler::DoMULInstruction(Instruction instruction){
    AdvancePC();
    int64_t result = registers[instruction.rs] * registers[instruction.rt];
    registers[instruction.rd] = result |( ((int64_t) -1)<<32);
}

void Disassembler::DoNOPInstruction(Instruction instruction){
    AdvancePC();
}

void Disassembler::DoJInstruction(Instruction instruction){
    AdvancePC();
    PC = (PC & (-1 << 28 ) ) | instruction.immediate;
}

void Disassembler::DoJRInstruction(Instruction instruction){
    AdvancePC();
    PC = registers[instruction.rs];
}

void Disassembler::DoBEQInstruction(Instruction instruction){
    AdvancePC();
    if (registers[instruction.rs] == registers[instruction.rt])
    {
        PC = PC + instruction.immediate;
    }
}

void Disassembler::DoBLTZInstruction(Instruction instruction){
    AdvancePC();
    if (registers[instruction.rs] < 0)
    {
        PC = PC + instruction.immediate;
    }
}

void Disassembler::DoBGTZInstruction(Instruction instruction){
    AdvancePC();
    if (registers[instruction.rs] > 0)
    {
        PC = PC + instruction.immediate;
    }
}


void Disassembler::DoSWInstruction(Instruction instruction){
    memory[registers[instruction.rs] + instruction.immediate]=registers[instruction.rt];
    AdvancePC();
}


void Disassembler::DoLWInstruction(Instruction instruction){
    registers[instruction.rt] = memory[registers[instruction.rs] + instruction.immediate];
    AdvancePC();
}

void Disassembler::DoSLLInstruction(Instruction instruction){
    registers[instruction.rd] = registers[instruction.rt]<<instruction.rs;
    AdvancePC();
}

void Disassembler::DoSRLInstruction(Instruction instruction){
    registers[instruction.rd] = ((unsigned int) registers[instruction.rt])>> (unsigned int)instruction.rs;
    AdvancePC();

}

void Disassembler::DoSRAInstruction(Instruction instruction){
    registers[instruction.rd] = (registers[instruction.rt])>> (unsigned int)instruction.rs;
    AdvancePC();
}

void Disassembler::DoXORInstruction(Instruction instruction){
    registers[instruction.rd] =  registers[instruction.rt] ^ registers[instruction.rs];
    AdvancePC();
}

void Disassembler::AdvancePC(){
    PC += 4;
}

static void InitializeOpcodeMap();



// Functions
static void InitializeOpcodeMap(){
    opcodeMap["010000"] = J;
    opcodeMap["010001"] = JR;
    opcodeMap["010010"] = BEQ;
    opcodeMap["010011"] = BLTZ;
    opcodeMap["010100"] = BGTZ;
    opcodeMap["010101"] = BREAK;
    opcodeMap["010110"] = SW;
    opcodeMap["010111"] = LW;
    opcodeMap["011000"] = SLL;
    opcodeMap["011001"] = SRL;
    opcodeMap["011010"] = SRA;
    opcodeMap["011011"] = NOP;
    opcodeMap["110000"] = ADD;
    opcodeMap["110001"] = SUB;
    opcodeMap["110010"] = MUL;
    opcodeMap["110011"] = AND;
    opcodeMap["110100"] = OR;
    opcodeMap["110101"] = XOR;
    opcodeMap["110110"] = NOR;
    opcodeMap["110111"] = SLT;
    opcodeMap["111000"] = ADDI;
    opcodeMap["111001"] = ANDI;
    opcodeMap["111010"] = ORI;
    opcodeMap["111011"] = XORI;

}
static void PrintRegisters(){
    disassembler.simulation<<"Registers";
    for (int i =0 ; i < 32 ; i++){
        if (i % 8 ==0){
            disassembler.simulation<<""<<endl;
            if (i==0){
                disassembler.simulation<<"R"<<i<<i<<":";
            }
            else if (i==8)
                disassembler.simulation<<"R0"<<i<<":";
            else disassembler.simulation<<"R"<<i<<":";
        }
        disassembler.simulation<<"\t"<<registers[i];
    }
    disassembler.simulation<<endl;
}

static void PrintData(){
    int currMem = disassembler.memstart;
    disassembler.simulation<<"Data"<<endl;
    // assume memory is aligned to start
    for (int i=0; i < (disassembler.memend-disassembler.memstart)/32 ; i++){
        disassembler.simulation<<currMem<<":";
        for (int j = 0 ; j< 8 ; j++){
            if (memory.find(currMem +j*4) == memory.end()){
                disassembler.simulation<<"\t"<<0;
            }
            else disassembler.simulation<<"\t"<<memory[currMem + j*4];
        }
        disassembler.simulation<<endl;
        currMem += 32;
    }
}





/***** Assignment 2 starts here *****/
vector<int > regReadMap(32,0);
vector<int > regWriteMap(32,0);
class IssueUnit{
    public:
        IssueUnit(){
            indexIssued =0;
            loadStoreIssued =0;
            nonLoadStoreIssued =0;
            firstStore = NULL;
        }
        deque<Instruction> preIssueBuffer;
        int indexIssued;
        int loadStoreIssued;
        int nonLoadStoreIssued;
        Instruction* firstStore;
        vector<Instruction> preALU1;
        vector<Instruction> preALU2;
        bool IsFullQ(vector<Instruction> );
        bool IsEmptyQ(vector<Instruction> );
        bool IsFullBuffer();
        bool IsEmptyBuffer();
        void DoIssue();
        void CheckAndIssue(bool , deque<Instruction>::iterator &);
        void DoIssueInstruction(Instruction);

};

class IFDecodeUnit{
    public:
        IFDecodeUnit(IssueUnit &issueUnit):issueUnit(issueUnit), instIterator(disassembler.instructionSet.begin()){
            isStallLastCycle = false;
            isEndExecution = false;
            indexFetched = 0;
        }
        vector<Instruction>::iterator instIterator;
        bool isStallLastCycle;
        bool isEndExecution;
        deque<Instruction> iFDecodeQ;
        int indexFetched;
        IssueUnit &issueUnit;
        bool IFDecodeQFull();
        bool IFDecodeQEmpty();
        void PutInIssueUnit(Instruction );
        void DoIFDecode();
        void DoIFDecodeInstruction();
        void PrintStatus();


};

bool IFDecodeUnit::IFDecodeQFull(){
    assert(iFDecodeQ.size() <= 2);
    if (iFDecodeQ.size() == 2){
        return true;
    }
    return false;
}
bool IFDecodeUnit::IFDecodeQEmpty(){
    assert(iFDecodeQ.size() <= 2);
    if (iFDecodeQ.size() == 0){
        return true;
    }
    return false;
}

void IFDecodeUnit::PutInIssueUnit(Instruction currInstruction){
    issueUnit.preIssueBuffer.push_back(currInstruction);
    cout<<"pushed "<<currInstruction.stringFormat<<endl;
    assert(issueUnit.preIssueBuffer.size() <= 4);
}
void IFDecodeUnit::DoIFDecode(){
    do{
        if (!isStallLastCycle){
            DoIFDecodeInstruction(); // stall gets updated here
        }
        else isStallLastCycle = false;
        indexFetched++;
    }while (indexFetched < 2);
}
void IFDecodeUnit::DoIFDecodeInstruction(){
    // Not stalled, inst can exec
    if (!issueUnit.IsFullBuffer() ){
        bool isNOP = false;
        bool isReadableBranchInst = true;
        bool isBranch = false;
        Instruction currentInstruction = *(instIterator);
        cout<<currentInstruction.stringFormat<<endl;
        ++instIterator;
        switch (currentInstruction.opcode){
            // check write cases for all the branch instuctions
            case BEQ:
                isBranch = true;
                if (regWriteMap[currentInstruction.rs] != 0 || regWriteMap[currentInstruction.rt] != 0){
                    isReadableBranchInst = false;
                }
                break;
            case BLTZ:
            case BGTZ:
            case JR:
                isBranch = true;
                if (regWriteMap[currentInstruction.rs] != 0 ){
                    isReadableBranchInst = false;
                }
                break;
            case NOP:
                isNOP = true;
                break;
            case BREAK:
                isEndExecution = true;
                break;
        }
        if (!isReadableBranchInst){
            isStallLastCycle = true;
            iFDecodeQ.push_back(currentInstruction);
            indexFetched =2; // endind execution here and not fetching next inst
        }
        else if (!isNOP && !isEndExecution && !isBranch){
            PutInIssueUnit(currentInstruction);
        }

    }

}
void IFDecodeUnit::PrintStatus(){

}

bool IssueUnit::IsFullQ(vector<Instruction> q){
    assert(q.size()  <= 2);
    if (q.size() == 2){
        return true;
    }
    return false;
}
bool IssueUnit::IsEmptyQ(vector<Instruction> q){
    assert(q.size()  <= 2);
    if (q.size() == 0){
        return true;
    }
    return false;

}
bool IssueUnit::IsFullBuffer(){
    assert(preIssueBuffer.size()  <= 4);
    if (preIssueBuffer.size() == 4){
        return true;
    }
    return false;
}
bool IssueUnit::IsEmptyBuffer(){
    assert(preIssueBuffer.size()  <= 4);
    if (preIssueBuffer.size() == 0){
        return true;
    }
    return false;
}

void IssueUnit::DoIssue(){
    firstStore = NULL;
    indexIssued = 0;
    loadStoreIssued = 0;
    nonLoadStoreIssued =0;
    deque<Instruction>::iterator preIssueIterator= preIssueBuffer.begin();
    
    while (indexIssued < 2 && preIssueIterator != preIssueBuffer.end()){
        Instruction currentInstruction = *(preIssueIterator);
        // check hazards here
        switch (currentInstruction.opcode){
            case SW:
                // store the first store instruction
                if (firstStore == NULL){
                    firstStore = &currentInstruction;
                }
            case LW:
                CheckAndIssue(true, preIssueIterator);
                break;
            default:
                CheckAndIssue(false, preIssueIterator);
        }
    }


}
void IssueUnit::DoIssueInstruction(Instruction currentInstruction){
    cout<<"Issued "<<currentInstruction.stringFormat<<endl;
    getchar();
    vector<int > readOperands = currentInstruction.GetReadOperands();
    int writeOperands = currentInstruction.GetWriteOperands();
    if (writeOperands != -1){
        int currentAddress = currentInstruction.instructionAddress;
        if (currentAddress > regWriteMap[writeOperands]){
            regWriteMap[writeOperands] = currentAddress;
        }
    }
    for (int i=0 ; i< readOperands.size(); i++){
        if (readOperands[i] != -1){
            int currentAddress = currentInstruction.instructionAddress;
            if (currentAddress > regReadMap[readOperands[i]]){
                regReadMap[readOperands[i]] = currentAddress;
            }
        }
    }
    readOperands.clear();

    switch (currentInstruction.opcode){
        case LW:
        case SW:
            preALU1.push_back(currentInstruction);
            break;
        default:
            preALU2.push_back(currentInstruction);
    }
    indexIssued++;
}


// This is the ScoreBoard Executor
void ScoreBoardExecution(){
    IssueUnit issueUnit;
    IFDecodeUnit iFDecodeUnit(issueUnit);
    int cycle =1;
    while (!iFDecodeUnit.isEndExecution){
        issueUnit.DoIssue();
        iFDecodeUnit.DoIFDecode();
        PrintHeader();
        iFDecodeUnit.PrintStatus();
        issueUnit.PrintStatus();
        getchar();
        cycle++;
    }
}
void IssueUnit::CheckAndIssue(bool isLoadStore, deque<Instruction>::iterator &preIssueIterator){
    Instruction currentInstruction = *(preIssueIterator);
    cout<<"Got inst   "<<currentInstruction.stringFormat<<endl;
    if (!IsFullQ(isLoadStore?preALU1:preALU2) && !isRAWHazard(currentInstruction) 
            && !isWARHazard(currentInstruction) && !isWAWHazard(currentInstruction)){
        // Check with non-issued instructions
        bool confilctsWithNonIssue = false;
        deque<Instruction>::iterator earlierInstructionIterator = preIssueBuffer.begin();
        vector<int > readOperands1 = currentInstruction.GetReadOperands();
        int writeOperands1 = currentInstruction.GetWriteOperands();

        while (!confilctsWithNonIssue && earlierInstructionIterator != preIssueIterator){
            int writeOperands2 = (*earlierInstructionIterator).GetWriteOperands();
            // WAW hazard check
            if ((writeOperands2 == writeOperands1) && writeOperands1 != -1){
                confilctsWithNonIssue = true;
                cout<<"Has WAW hazard"<<endl;
            }
            // RAW hazard check
            for(int i =0; i< readOperands1.size(); i++){
                if ((readOperands1[i] == writeOperands2) && writeOperands2 != -1){
                    confilctsWithNonIssue = true;
                    cout<<"Has RAW hazard"<<endl;
                }
            }
            // WAR hazard check
            vector<int >readOperands2 = (*earlierInstructionIterator).GetReadOperands();
            for (int i =0; i < readOperands2.size(); i++){
                if ((readOperands2[i]== writeOperands1) && writeOperands1 != -1){
                    confilctsWithNonIssue = true;
                    cout<<"Has WAR hazard"<<endl;
                }
            }
            readOperands2.clear();

            ++earlierInstructionIterator;
        }
        readOperands1.clear();
        // Instruction issued, no non-issue conficts
        if (!confilctsWithNonIssue){
            if (isLoadStore){
                if (loadStoreIssued == 0){
                    switch (currentInstruction.opcode){
                        case SW:
                            // Check stores issued in order
                            if (currentInstruction.stringFormat.compare(firstStore->stringFormat) == 0){
                                DoIssueInstruction(currentInstruction);
                                preIssueIterator = preIssueBuffer.erase(preIssueIterator);
                                loadStoreIssued++;
                            }
                            break;
                        case LW:
                            if (firstStore== NULL){
                                DoIssueInstruction(currentInstruction);
                                preIssueIterator = preIssueBuffer.erase(preIssueIterator);
                                loadStoreIssued++;
                            }
                    }
                }
            }
            else{ 
                if (nonLoadStoreIssued == 0){
                    DoIssueInstruction(currentInstruction);
                    preIssueIterator = preIssueBuffer.erase(preIssueIterator);
                    nonLoadStoreIssued += 1;
                }
            }
        }
        else ++preIssueIterator; // InstructionNotIssued

    }
    else ++preIssueIterator;
}


int main(int argc, char** argv){
    // Setup our translator
    InitializeOpcodeMap();
    // Read input
    //cout<<"Number of input arguments"<<argc<<endl; 
    assert(argc == 2);
    //cout<<"Now reading input file"<<argv[1]<<endl;
    ifstream inputFile(argv[1]);
    string currentLine;
    // Open and read file by each line
    int instructionAddress = 256;
    if (inputFile.is_open()){
        bool isEndInstruction = false;
        do{
            getline(inputFile,currentLine);
            if (currentLine.length() < 32){
                break;
            }
            currentLine = currentLine.substr(0,32);
            disassembler.ParseInstruction(currentLine, isEndInstruction, instructionAddress);
            instructionAddress +=4;
        } while (true);
    }
    inputFile.close();
    disassembler.disassembly.close();
    ScoreBoardExecution();
    return 0;
}


void LinearExecution(){

    int temp = disassembler.PC;
    disassembler.PC = 256;
    disassembler.memend = temp;
    //cout<<endl<<"File parsed"<<endl;
    //cout<<"Executing instructions:::::"<<endl;
    int instructionOffset = 0;
    bool isEndInstruction = false;
    int i=1;
    while (true && !isEndInstruction){
        // calculate offset and fetch next instruction
        cout<<disassembler.PC<<endl;
        Instruction current = disassembler.instructionSet[(disassembler.PC - 256)/4];
        disassembler.simulation<<"--------------------"<<endl;
        disassembler.simulation<<"Cycle:"<<i++<<"\t"<<disassembler.PC<<"\t"<<current.stringFormat<<endl;
        cout<<"Cycle:"<<i<<"\t"<<disassembler.PC<<"\t"<<current.stringFormat<<endl;
        getchar();
        switch(current.opcode){
            case J:
                disassembler.DoJInstruction(current);
                break; 
            case JR:
                disassembler.DoJRInstruction(current);
                break; 
            case BEQ:
                disassembler.DoBEQInstruction(current);
                break; 
            case BLTZ:
                disassembler.DoBLTZInstruction(current);
                break; 
            case BGTZ:
                disassembler.DoBGTZInstruction(current);
                break; 
            case BREAK:
                isEndInstruction = true;
                break; 
            case SW:
                disassembler.DoSWInstruction(current);
                break; 
            case LW:
                disassembler.DoLWInstruction(current);
                break; 
            case SLL:
                disassembler.DoSLLInstruction(current);
                break; 
            case SRL:
                disassembler.DoSRLInstruction(current);
                break; 
            case SRA:
                disassembler.DoSRAInstruction(current);
                break; 
            case NOP:
                disassembler.DoNOPInstruction(current);
                break; 
            case ADD:
                disassembler.DoADDInstruction(current);
                break; 
            case SUB:
                disassembler.DoSUBInstruction(current);
                break;
            case MUL:
                disassembler.DoMULInstruction(current);
                break; 
            case AND:
                disassembler.DoANDInstruction(current);
                break; 
            case OR:
                disassembler.DoORInstruction(current);
                break; 
            case XOR:
                disassembler.DoXORInstruction(current);
                break; 
            case NOR:
                disassembler.DoNORInstruction(current);
                break; 
            case SLT:
                disassembler.DoSLTInstruction(current);
                break; 
            case ADDI:
                disassembler.DoADDIInstruction(current);
                break; 
            case ANDI:
                disassembler.DoANDIInstruction(current);
                break; 
            case ORI:
                disassembler.DoORIInstruction(current);
                break; 
            case XORI:
                disassembler.DoXORIInstruction(current);
                break; 
            default:
                cout<<"Could not find opcode "<<current.opcode<<endl;
                break;
        }
        PrintRegisters();
        disassembler.simulation<<endl;
        PrintData();
        //disassembler.simulation<<endl;
    }
}

bool isRAWHazard(Instruction currentInstruction){
    switch(currentInstruction.opcode){
        case AND:
        case ADD:
        case SUB:
        case MUL:
        case OR:
        case XOR:
        case SLT:
        case SLL:
        case SRA:
        case SRL:
            if ( regWriteMap[currentInstruction.rs] == 0 || regWriteMap[currentInstruction.rt] == 0 ){
                return false;
            }
            return true;
            break;
        case ADDI:
        case ANDI:
        case ORI:
        case XORI:
        case JR:
        case LW:
        case SW:
            if ( regWriteMap[currentInstruction.rs] == 0){
                return false;
            }
            return true; 
            break;
        default:
            cout<<"Incorrect case for RAW hazard check!"<<endl;
            break;
    }
}
bool isWAWHazard(Instruction currentInstruction){
    switch(currentInstruction.opcode){
        case AND:
        case ADD:
        case SUB:
        case MUL:
        case OR:
        case XOR:
        case SLT:
        case SLL:
        case SRA:
        case SRL:
            if ( regWriteMap[currentInstruction.rd] == 0){
                return false;
            }
            return true;
            break;
        case ADDI:
        case ANDI:
        case ORI:
        case XORI:
        case LW:
            if ( regWriteMap[currentInstruction.rt] == 0){
                return false;
            }
            return true; 
            break;
        default:
            cout<<"Incorrect case for WAW hazard check!"<<endl;
            break;
    }

}
bool isWARHazard(Instruction currentInstruction){
    switch(currentInstruction.opcode){
        case AND:
        case ADD:
        case SUB:
        case MUL:
        case OR:
        case XOR:
        case SLT:
        case SLL:
        case SRA:
        case SRL:
            if ( regReadMap[currentInstruction.rd] == 0){
                return false;
            }
            return true;
            break;
        case ADDI:
        case ANDI:
        case ORI:
        case XORI:
        case LW:
            if ( regReadMap[currentInstruction.rt] == 0){
                return false;
            }
            return true; 
            break;
        default:
            cout<<"Incorrect case for WAR hazard check!"<<endl;
            break;
    }

}

