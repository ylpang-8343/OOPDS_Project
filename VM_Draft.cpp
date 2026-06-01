#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <string>
#include <fstream>

using namespace std;

int normalizeByte(int value);
string upperText(string text);
string trimText(string text);
bool isRegisterName(string text);
bool isNumberText(string text);
bool isMemoryText(string text);
bool isMemoryRegister(string text);
int toNumber(string text);
int registerNumber(string text);
int memoryNumber(string text);
void stopProgram(string message);
void tokenize(string line, string parts[], int& count);
void addToken(string& word, string parts[], int& count);
void decimalToBits(int value, int bits[]);
int bitsToDecimal(int bits[]);
string memoryInside(string text);

// Developer 1: Custom vector used to store program lines and instructions.
template <class T>
class MyVector {
private:
    T* items;
    int length;
    int capacity;

    void resize() {
        int newCapacity = capacity * 2;
        T* newItems = new T[newCapacity];
        for (int i = 0; i < length; i++) {
            newItems[i] = items[i];
        }
        delete[] items;
        items = newItems;
        capacity = newCapacity;
    }

public:
    MyVector() {
        capacity = 10;
        length = 0;
        items = new T[capacity];
    }

    ~MyVector() {
        delete[] items;
    }

    void pushBack(const T& value) {
        if (length == capacity) {
            resize();
        }
        items[length] = value;
        length++;
    }

    T& get(int index) {
        if (index < 0 || index >= length) {
            cerr << "Vector error: invalid index" << endl;
            exit(1);
        }
        return items[index];
    }

    int size() const {
        return length;
    }

    bool isEmpty() const {
        return length == 0;
    }

    T popBack() {
        if (length == 0) {
            cerr << "Vector error: empty vector" << endl;
            exit(1);
        }
        length--;
        return items[length];
    }
};

// Developer 1: Custom stack used by PUSH and POP instructions.
template <class T>
class MyStack {
private:
    MyVector<T> values;

public:
    void push(const T& value) {
        values.pushBack(value);
    }

    T pop() {
        if (values.isEmpty()) {
            cerr << "System crash: stack underflow" << endl;
            exit(1);
        }
        return values.popBack();
    }

    bool isEmpty() const {
        return values.isEmpty();
    }

    int size() const {
        return values.size();
    }
};

// Developer 1: Simple custom queue used when loading assembly lines.
template <class T>
class MyQueue {
private:
    T* items;
    int count;
    int capacity;

    void resize() {
        int newCapacity = capacity * 2;
        T* newItems = new T[newCapacity];
        for (int i = 0; i < count; i++) {
            newItems[i] = items[i];
        }
        delete[] items;
        items = newItems;
        capacity = newCapacity;
    }

public:
    MyQueue() {
        capacity = 10;
        count = 0;
        items = new T[capacity];
    }

    ~MyQueue() {
        delete[] items;
    }

    void enqueue(const T& value) {
        if (count == capacity) {
            resize();
        }
        items[count] = value;
        count++;
    }

    T dequeue() {
        if (isEmpty()) {
            cerr << "Queue error: empty queue" << endl;
            exit(1);
        }
        T result = items[0];
        for (int i = 1; i < count; i++) {
            items[i - 1] = items[i];
        }
        count--;
        return result;
    }

    bool isEmpty() const {
        return count == 0;
    }

    int size() const {
        return count;
    }
};

// Developer 2: Register stores one signed byte value.
class Register {
private:
    signed char value;

public:
    Register() {
        value = 0;
    }

    void setValue(int newValue) {
        value = static_cast<signed char>(normalizeByte(newValue));
    }

    int getValue() const {
        return static_cast<int>(value);
    }
};

// Developer 2: GeneralRegister represents R0 to R7.
class GeneralRegister : public Register {
public:
    GeneralRegister() {
    }
};

// Developer 2: FlagRegister stores OF, UF, CF, and ZF.
class FlagRegister {
private:
    bool overflowFlag;
    bool underflowFlag;
    bool carryFlag;
    bool zeroFlag;

public:
    FlagRegister() {
        resetAll();
    }

    void updateFromResult(int result) {
        overflowFlag = result > 127;
        underflowFlag = result < -128;
        carryFlag = result > 255 || result < -256;
        zeroFlag = result == 0;
    }

    void resetFlag(string name) {
        if (name == "OF") overflowFlag = false;
        if (name == "UF") underflowFlag = false;
        if (name == "CF") carryFlag = false;
        if (name == "ZF") zeroFlag = false;
    }

    void resetAll() {
        overflowFlag = false;
        underflowFlag = false;
        carryFlag = false;
        zeroFlag = false;
    }

    bool getOF() const {
        return overflowFlag;
    }

    bool getUF() const {
        return underflowFlag;
    }

    bool getCF() const {
        return carryFlag;
    }

    bool getZF() const {
        return zeroFlag;
    }
};

// Developer 2: Memory stores 64 signed byte cells.
class Memory {
private:
    signed char cells[64];

    void validateAddress(int address) const {
        if (address < 0 || address > 63) {
            cerr << "Memory error: invalid address" << endl;
            exit(1);
        }
    }

public:
    Memory() {
        clear();
    }

    void clear() {
        for (int i = 0; i < 64; i++) {
            cells[i] = 0;
        }
    }

    void write(int address, int value) {
        validateAddress(address);
        cells[address] = static_cast<signed char>(normalizeByte(value));
    }

    int read(int address) const {
        validateAddress(address);
        return static_cast<int>(cells[address]);
    }

    int size() const {
        return 64;
    }
};

// Developer 3: CPU contains all virtual machine parts.
class CPU {
private:
    GeneralRegister registers[8];
    signed char programCounter;
    signed char stackIndex;
    FlagRegister flags;
    Memory memory;
    MyStack<int> systemStack;

    void printPadded(ostream& out, int value) const {
        if (value < 0) out << "-" << setw(3) << setfill('0') << -value;
        else out << setw(4) << setfill('0') << value;
        out << setfill(' ');
    }

public:
    CPU() {
        reset();
    }

    void reset() {
        programCounter = 0;
        stackIndex = 0;
        flags.resetAll();
        memory.clear();
        for (int i = 0; i < 8; i++) {
            registers[i].setValue(0);
        }
    }

    int getRegister(int index) const {
        validateRegister(index);
        return registers[index].getValue();
    }

    void setRegister(int index, int value) {
        validateRegister(index);
        registers[index].setValue(value);
    }

    void setRegisterWithFlags(int index, int value) {
        flags.updateFromResult(value);
        setRegister(index, value);
    }

    void validateRegister(int index) const {
        if (index < 0 || index > 7) {
            cerr << "Register error: invalid register" << endl;
            exit(1);
        }
    }

    int getPC() const {
        return static_cast<int>(programCounter);
    }

    void incrementPC() {
        programCounter = static_cast<signed char>(normalizeByte(programCounter + 1));
    }

    int getSI() const {
        return static_cast<int>(stackIndex);
    }

    FlagRegister& getFlags() {
        return flags;
    }

    Memory& getMemory() {
        return memory;
    }

    void pushRegister(int index) {
        systemStack.push(getRegister(index));
        stackIndex = static_cast<signed char>(normalizeByte(stackIndex + 1));
    }

    void popToRegister(int index) {
        setRegisterWithFlags(index, systemStack.pop());
        stackIndex = static_cast<signed char>(normalizeByte(stackIndex - 1));
    }

    void dump(ostream& out) const {
        out << "#Begin#" << endl;
        dumpRegisters(out);
        dumpFlags(out);
        dumpMemory(out);
        out << "#End#" << endl;
    }

    void dumpRegisters(ostream& out) const {
        out << "#Registers#";
        for (int i = 0; i < 8; i++) {
            printPadded(out, registers[i].getValue());
            out << "#";
        }
        out << endl;
    }

    void dumpFlags(ostream& out) const {
        out << "#Flags#" << flags.getOF() << "#";
        out << flags.getUF() << "#" << flags.getCF();
        out << "#" << flags.getZF() << "#" << endl;
        out << "#PC#";
        printPadded(out, getPC());
        out << "#" << endl;
    }

    void dumpMemory(ostream& out) const {
        out << "#Memory#" << endl;
        for (int row = 0; row < 8; row++) {
            dumpMemoryRow(out, row);
        }
    }

    void dumpMemoryRow(ostream& out, int row) const {
        out << "#";
        for (int col = 0; col < 8; col++) {
            printPadded(out, memory.read(row * 8 + col));
            out << "#";
        }
        out << endl;
    }
};

// Developer 3: Base instruction class for polymorphism.
class Instruction {
public:
    virtual ~Instruction() {
    }

    virtual void execute(CPU& cpu) = 0;
};

// Developer 3: Middle instruction groups required by the assignment.
class ArithmeticInstruction : public Instruction {
public:
    virtual void execute(CPU& cpu) = 0;
};

class IOInstruction : public Instruction {
public:
    virtual void execute(CPU& cpu) = 0;
};

class ShiftInstruction : public Instruction {
public:
    virtual void execute(CPU& cpu) = 0;
};

class MemoryInstruction : public Instruction {
public:
    virtual void execute(CPU& cpu) = 0;
};

class DataInstruction : public Instruction {
public:
    virtual void execute(CPU& cpu) = 0;
};

class SystemInstruction : public Instruction {
public:
    virtual void execute(CPU& cpu) = 0;
};

class MovInstruction : public DataInstruction {
private:
    int destination;
    int sourceMode;
    int sourceValue;

public:
    MovInstruction(int dest, int mode, int value) {
        destination = dest;
        sourceMode = mode;
        sourceValue = value;
    }

    void execute(CPU& cpu) {
        int value = sourceValue;
        if (sourceMode == 1) value = cpu.getRegister(sourceValue);
        if (sourceMode == 2) value = cpu.getMemory().read(cpu.getRegister(sourceValue));
        cpu.setRegisterWithFlags(destination, value);
    }
};

class BinaryMathInstruction : public ArithmeticInstruction {
private:
    int destination;
    int sourceMode;
    int sourceValue;
    char operation;

public:
    BinaryMathInstruction(int dest, int mode, int value, char op) {
        destination = dest;
        sourceMode = mode;
        sourceValue = value;
        operation = op;
    }

    void execute(CPU& cpu) {
        int result = calculate(cpu);
        cpu.setRegisterWithFlags(destination, result);
    }

    int calculate(CPU& cpu) {
        int source = sourceValue;
        if (sourceMode == 1) source = cpu.getRegister(sourceValue);
        if (operation == '+') return cpu.getRegister(destination) + source;
        if (operation == '-') return cpu.getRegister(destination) - source;
        if (operation == '*') return cpu.getRegister(destination) * source;
        if (source == 0) stopProgram("Math error: division by zero");
        return cpu.getRegister(destination) / source;
    }
};

class AddInstruction : public BinaryMathInstruction {
public:
    AddInstruction(int dest, int mode, int value)
        : BinaryMathInstruction(dest, mode, value, '+') {
    }
};

class SubInstruction : public BinaryMathInstruction {
public:
    SubInstruction(int dest, int mode, int value)
        : BinaryMathInstruction(dest, mode, value, '-') {
    }
};

class MulInstruction : public BinaryMathInstruction {
public:
    MulInstruction(int dest, int mode, int value)
        : BinaryMathInstruction(dest, mode, value, '*') {
    }
};

class DivInstruction : public BinaryMathInstruction {
public:
    DivInstruction(int dest, int mode, int value)
        : BinaryMathInstruction(dest, mode, value, '/') {
    }
};

class OneRegisterMathInstruction : public ArithmeticInstruction {
private:
    int destination;
    int change;

public:
    OneRegisterMathInstruction(int dest, int amount) {
        destination = dest;
        change = amount;
    }

    void execute(CPU& cpu) {
        cpu.setRegisterWithFlags(destination, cpu.getRegister(destination) + change);
    }
};

class IncInstruction : public OneRegisterMathInstruction {
public:
    IncInstruction(int dest) : OneRegisterMathInstruction(dest, 1) {
    }
};

class DecInstruction : public OneRegisterMathInstruction {
public:
    DecInstruction(int dest) : OneRegisterMathInstruction(dest, -1) {
    }
};

class InputInstruction : public IOInstruction {
private:
    int destination;

public:
    InputInstruction(int dest) {
        destination = dest;
    }

    void execute(CPU& cpu) {
        string inputText;
        cout << "? ";
        cin >> inputText;
        if (!isNumberText(inputText)) stopProgram("Input error: number expected");
        cpu.setRegisterWithFlags(destination, toNumber(inputText));
    }
};

class DisplayInstruction : public IOInstruction {
private:
    int source;

public:
    DisplayInstruction(int src) {
        source = src;
    }

    void execute(CPU& cpu) {
        cout << cpu.getRegister(source) << endl;
    }
};

class LoadInstruction : public MemoryInstruction {
private:
    int destination;
    int addressMode;
    int addressValue;

public:
    LoadInstruction(int dest, int mode, int value) {
        destination = dest;
        addressMode = mode;
        addressValue = value;
    }

    void execute(CPU& cpu) {
        int address = addressValue;
        if (addressMode == 1) address = cpu.getRegister(addressValue);
        cpu.setRegisterWithFlags(destination, cpu.getMemory().read(address));
    }
};

class StoreInstruction : public MemoryInstruction {
private:
    int source;
    int addressMode;
    int addressValue;

public:
    StoreInstruction(int src, int mode, int value) {
        source = src;
        addressMode = mode;
        addressValue = value;
    }

    void execute(CPU& cpu) {
        int address = addressValue;
        if (addressMode == 1) address = cpu.getRegister(addressValue);
        cpu.getMemory().write(address, cpu.getRegister(source));
    }
};

class PushInstruction : public SystemInstruction {
private:
    int source;

public:
    PushInstruction(int src) {
        source = src;
    }

    void execute(CPU& cpu) {
        cpu.pushRegister(source);
    }
};

class PopInstruction : public SystemInstruction {
private:
    int destination;

public:
    PopInstruction(int dest) {
        destination = dest;
    }

    void execute(CPU& cpu) {
        cpu.popToRegister(destination);
    }
};

class ResetInstruction : public SystemInstruction {
private:
    string flagName;

public:
    ResetInstruction(string name) {
        flagName = name;
    }

    void execute(CPU& cpu) {
        cpu.getFlags().resetFlag(flagName);
    }
};

class BitMoveInstruction : public ShiftInstruction {
private:
    int destination;
    int count;
    char operation;

public:
    BitMoveInstruction(int dest, int moveCount, char op) {
        destination = dest;
        count = moveCount;
        operation = op;
    }

    void execute(CPU& cpu) {
        int bits[8];
        decimalToBits(cpu.getRegister(destination), bits);
        for (int i = 0; i < count; i++) moveOnce(bits);
        cpu.setRegisterWithFlags(destination, bitsToDecimal(bits));
    }

    void moveOnce(int bits[]) {
        if (operation == 'L') shiftLeft(bits);
        if (operation == 'R') shiftRight(bits);
        if (operation == 'O') rotateLeft(bits);
        if (operation == 'P') rotateRight(bits);
    }

    void shiftRight(int bits[]) {
        for (int i = 7; i > 0; i--) bits[i] = bits[i - 1];
        bits[0] = 0;
    }

    void shiftLeft(int bits[]) {
        for (int i = 0; i < 7; i++) bits[i] = bits[i + 1];
        bits[7] = 0;
    }

    void rotateLeft(int bits[]) {
        int first = bits[0];
        for (int i = 0; i < 7; i++) bits[i] = bits[i + 1];
        bits[7] = first;
    }

    void rotateRight(int bits[]) {
        int last = bits[7];
        for (int i = 7; i > 0; i--) bits[i] = bits[i - 1];
        bits[0] = last;
    }
};

class ShlInstruction : public BitMoveInstruction {
public:
    ShlInstruction(int dest, int count) : BitMoveInstruction(dest, count, 'L') {
    }
};

class ShrInstruction : public BitMoveInstruction {
public:
    ShrInstruction(int dest, int count) : BitMoveInstruction(dest, count, 'R') {
    }
};

class RolInstruction : public BitMoveInstruction {
public:
    RolInstruction(int dest, int count) : BitMoveInstruction(dest, count, 'O') {
    }
};

class RorInstruction : public BitMoveInstruction {
public:
    RorInstruction(int dest, int count) : BitMoveInstruction(dest, count, 'P') {
    }
};

class Runner {
private:
    CPU cpu;
    MyQueue<string> lineQueue;
    MyVector<string> lines;
    MyVector<Instruction*> program;

public:
    ~Runner() {
        for (int i = 0; i < program.size(); i++) delete program.get(i);
    }

    void loadFile(string fileName) {
        ifstream file(fileName.c_str());
        string line;
        if (!file) stopProgram("File error: cannot open asm file");
        while (getline(file, line)) {
            line = trimText(line);
            if (line != "") lineQueue.enqueue(line);
        }
        while (!lineQueue.isEmpty()) lines.pushBack(lineQueue.dequeue());
    }

    void parseProgram() {
        for (int i = 0; i < lines.size(); i++) {
            program.pushBack(createInstruction(lines.get(i)));
        }
    }

    void executeProgram() {
        for (int i = 0; i < program.size(); i++) {
            program.get(i)->execute(cpu);
            cpu.incrementPC();
        }
    }

    void dumpResult(string outputFile) {
        ofstream file(outputFile.c_str());
        if (!file) stopProgram("File error: cannot create output file");
        cpu.dump(cout);
        cpu.dump(file);
    }

    void run(string inputFile, string outputFile) {
        loadFile(inputFile);
        parseProgram();
        executeProgram();
        dumpResult(outputFile);
    }

private:
    Instruction* createInstruction(string line) {
        string parts[5];
        int count = 0;
        tokenize(line, parts, count);
        if (count == 0) stopProgram("Parse error: empty line");
        return chooseInstruction(parts, count);
    }

    Instruction* chooseInstruction(string parts[], int count) {
        string op = parts[0];
        if (op == "MOV") return parseMov(parts, count);
        if (op == "ADD" || op == "SUB") return parseMath(parts, count);
        if (op == "MUL" || op == "DIV") return parseMath(parts, count);
        if (op == "INC" || op == "DEC") return parseIncDec(parts, count);
        if (op == "INPUT" || op == "DISPLAY") return parseIO(parts, count);
        if (op == "LOAD" || op == "STORE") return parseMemory(parts, count);
        if (op == "PUSH" || op == "POP") return parseStack(parts, count);
        if (op == "RESET") return parseReset(parts, count);
        if (op == "ROL" || op == "ROR") return parseShift(parts, count);
        if (op == "SHL" || op == "SHR") return parseShift(parts, count);
        stopProgram("Parse error: unknown instruction");
        return 0;
    }

    Instruction* parseMov(string parts[], int count) {
        if (count != 3 || !isRegisterName(parts[1])) stopProgram("Bad MOV");
        int dest = registerNumber(parts[1]);
        if (isRegisterName(parts[2])) return new MovInstruction(dest, 1, registerNumber(parts[2]));
        if (isNumberText(parts[2])) return new MovInstruction(dest, 0, toNumber(parts[2]));
        if (isMemoryRegister(parts[2])) return new MovInstruction(dest, 2, memoryNumber(parts[2]));
        stopProgram("Bad MOV source");
        return 0;
    }

    Instruction* parseMath(string parts[], int count) {
        if (count != 3 || !isRegisterName(parts[1])) stopProgram("Bad math");
        int mode = 0;
        int value = sourceValue(parts[2], mode);
        int dest = registerNumber(parts[1]);
        if (parts[0] == "ADD") return new AddInstruction(dest, mode, value);
        if (parts[0] == "SUB") return new SubInstruction(dest, mode, value);
        if (parts[0] == "MUL") return new MulInstruction(dest, mode, value);
        return new DivInstruction(dest, mode, value);
    }

    int sourceValue(string text, int& mode) {
        if (isRegisterName(text)) {
            mode = 1;
            return registerNumber(text);
        }
        if (isNumberText(text)) {
            mode = 0;
            return toNumber(text);
        }
        stopProgram("Parse error: bad source");
        return 0;
    }

    Instruction* parseIncDec(string parts[], int count) {
        if (count != 2 || !isRegisterName(parts[1])) stopProgram("Bad INC DEC");
        int dest = registerNumber(parts[1]);
        if (parts[0] == "INC") return new IncInstruction(dest);
        return new DecInstruction(dest);
    }

    Instruction* parseIO(string parts[], int count) {
        if (count != 2 || !isRegisterName(parts[1])) stopProgram("Bad IO");
        int reg = registerNumber(parts[1]);
        if (parts[0] == "INPUT") return new InputInstruction(reg);
        return new DisplayInstruction(reg);
    }

    Instruction* parseStack(string parts[], int count) {
        if (count != 2 || !isRegisterName(parts[1])) stopProgram("Bad stack");
        int reg = registerNumber(parts[1]);
        if (parts[0] == "PUSH") return new PushInstruction(reg);
        return new PopInstruction(reg);
    }

    Instruction* parseReset(string parts[], int count) {
        if (count != 2) stopProgram("Bad RESET");
        bool valid = parts[1] == "OF" || parts[1] == "UF";
        valid = valid || parts[1] == "CF" || parts[1] == "ZF";
        if (!valid) stopProgram("Bad flag");
        return new ResetInstruction(parts[1]);
    }

    Instruction* parseShift(string parts[], int count) {
        if (count != 3 || !isRegisterName(parts[1])) stopProgram("Bad shift");
        if (!isNumberText(parts[2])) stopProgram("Bad shift count");
        int reg = registerNumber(parts[1]);
        int amount = toNumber(parts[2]);
        if (parts[0] == "SHL") return new ShlInstruction(reg, amount);
        if (parts[0] == "SHR") return new ShrInstruction(reg, amount);
        if (parts[0] == "ROL") return new RolInstruction(reg, amount);
        return new RorInstruction(reg, amount);
    }

    Instruction* parseMemory(string parts[], int count) {
        if (count != 3) stopProgram("Bad memory instruction");
        if (parts[0] == "LOAD") return parseLoad(parts);
        return parseStore(parts);
    }

    Instruction* parseLoad(string parts[]) {
        if (!isRegisterName(parts[1])) stopProgram("Bad LOAD");
        int dest = registerNumber(parts[1]);
        if (!isMemoryText(parts[2])) stopProgram("Bad LOAD address");
        if (isMemoryRegister(parts[2])) {
            return new LoadInstruction(dest, 1, memoryNumber(parts[2]));
        }
        return new LoadInstruction(dest, 0, memoryNumber(parts[2]));
    }

    Instruction* parseStore(string parts[]) {
        if (isRegisterName(parts[1]) && isNumberText(parts[2])) {
            return new StoreInstruction(registerNumber(parts[1]), 0, toNumber(parts[2]));
        }
        if (isNumberText(parts[1]) && isRegisterName(parts[2])) {
            return new StoreInstruction(registerNumber(parts[2]), 0, toNumber(parts[1]));
        }
        if (isMemoryRegister(parts[1]) && isRegisterName(parts[2])) {
            return new StoreInstruction(registerNumber(parts[2]), 1, memoryNumber(parts[1]));
        }
        stopProgram("Bad STORE");
        return 0;
    }
};

int normalizeByte(int value) {
    while (value > 127) {
        value -= 256;
    }
    while (value < -128) {
        value += 256;
    }
    return value;
}

void stopProgram(string message) {
    cerr << message << endl;
    exit(1);
}

string upperText(string text) {
    for (int i = 0; i < static_cast<int>(text.length()); i++) {
        if (text[i] >= 'a' && text[i] <= 'z') text[i] = text[i] - 32;
    }
    return text;
}

string trimText(string text) {
    int start = 0;
    int end = static_cast<int>(text.length()) - 1;
    while (start <= end && (text[start] == ' ' || text[start] == '\t')) start++;
    while (end >= start && (text[end] == ' ' || text[end] == '\t')) end--;
    if (start > end) return "";
    return text.substr(start, end - start + 1);
}

void tokenize(string line, string parts[], int& count) {
    string word = "";
    count = 0;
    line = upperText(line);
    for (int i = 0; i <= static_cast<int>(line.length()); i++) {
        char ch = i < static_cast<int>(line.length()) ? line[i] : ' ';
        if (ch == ',' || ch == ' ' || ch == '\t') addToken(word, parts, count);
        else word += ch;
    }
}

void addToken(string& word, string parts[], int& count) {
    word = trimText(word);
    if (word == "") return;
    if (count >= 5) stopProgram("Parse error: too many tokens");
    parts[count] = word;
    count++;
    word = "";
}

bool isRegisterName(string text) {
    if (text.length() != 2) return false;
    if (text[0] != 'R') return false;
    return text[1] >= '0' && text[1] <= '7';
}

int registerNumber(string text) {
    if (!isRegisterName(text)) stopProgram("Parse error: bad register");
    return text[1] - '0';
}

bool isNumberText(string text) {
    int start = 0;
    if (text == "") return false;
    if (text[0] == '-') start = 1;
    if (start == static_cast<int>(text.length())) return false;
    for (int i = start; i < static_cast<int>(text.length()); i++) {
        if (text[i] < '0' || text[i] > '9') return false;
    }
    return true;
}

int toNumber(string text) {
    int sign = 1;
    int value = 0;
    int start = 0;
    if (!isNumberText(text)) stopProgram("Parse error: bad number");
    if (text[0] == '-') {
        sign = -1;
        start = 1;
    }
    for (int i = start; i < static_cast<int>(text.length()); i++) {
        value = value * 10 + (text[i] - '0');
    }
    return value * sign;
}

bool isMemoryText(string text) {
    int last = static_cast<int>(text.length()) - 1;
    if (text.length() < 3) return false;
    return text[0] == '[' && text[last] == ']';
}

string memoryInside(string text) {
    if (!isMemoryText(text)) stopProgram("Parse error: bad memory");
    return text.substr(1, text.length() - 2);
}

bool isMemoryRegister(string text) {
    if (!isMemoryText(text)) return false;
    return isRegisterName(memoryInside(text));
}

int memoryNumber(string text) {
    string inside = memoryInside(text);
    if (isRegisterName(inside)) return registerNumber(inside);
    return toNumber(inside);
}

void decimalToBits(int value, int bits[]) {
    int number = value;
    if (number < 0) number += 256;
    for (int i = 7; i >= 0; i--) {
        bits[i] = number % 2;
        number = number / 2;
    }
}

int bitsToDecimal(int bits[]) {
    int value = 0;
    for (int i = 0; i < 8; i++) {
        value = value * 2 + bits[i];
    }
    if (value > 127) value -= 256;
    return value;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "Usage: VM_Draft input.asm output.txt" << endl;
        return 1;
    }
    Runner runner;
    runner.run(argv[1], argv[2]);
    return 0;
}
