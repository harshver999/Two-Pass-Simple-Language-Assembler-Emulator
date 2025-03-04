#include <iostream>
#include <fstream>
#include <string>
#include <deque>
#include <map>

class EmulatorCore {
private:
    struct CPUState {
        int programCounter = 0;
        int stackPointer = 0;
        int registerA = 0;
        int registerB = 0;
        std::deque<long long> memory;
        
        CPUState() : memory(1000, 0) {}
    } state;
    
    std::deque<std::string> instructions;
    
    std::string removeWhitespace(const std::string& input) {
        std::string result;
        for(char c : input) {
            if (!std::isspace(c)) result += c;
        }
        return result;
    }
    
    long long convertToDecimal(std::string hexVal) {
        hexVal = removeWhitespace(hexVal);
        
        if (hexVal.length() < 8 && hexVal[0] == 'f') {
            hexVal = "ff" + hexVal;
        }
        
        unsigned int temp = std::stoul(hexVal, nullptr, 16);
        return (temp >= 0x80000000) ? 
               static_cast<int>(temp - 0x100000000) : 
               static_cast<int>(temp);
    }

public:
    void loadProgramFromFile(const std::string& filename) {
        std::string currentLine;
        std::ifstream sourceFile(filename);
        int memIndex = 0;
        
        while (std::getline(sourceFile, currentLine)) {
            instructions.push_back(currentLine);
            
            bool isDataSegment = (currentLine.substr(6, 2) == "ff");
            if (isDataSegment) {
                state.memory[memIndex++] = convertToDecimal(currentLine.substr(0, 6));
            }
        }
    }
    
    void displayState() {
        std::cout << state.programCounter << "\t" 
                  << state.stackPointer << "\t" 
                  << state.registerA << "\t" 
                  << state.registerB << "\n";
                  
        for (const auto& val : state.memory) {
            if (val != 0) std::cout << val << "\t";
        }
        std::cout << "\n";
    }
    
    void executeProgram() {
        while (state.programCounter < instructions.size()) {
            std::string currentInstruction = instructions[state.programCounter];
            std::string operation = currentInstruction.substr(6, 2);
            std::string operand = currentInstruction.substr(0, 6);
            
            long long opCode = convertToDecimal(operation);
            long long value = convertToDecimal(operand);
            
            displayState();
            
            switch (opCode) {
                case 0: {
                    state.registerB = state.registerA;
                    state.registerA = value;
                    break;
                }
                case 1: {
                    state.registerA += value;
                    break;
                }
                case 2: {
                    state.registerB = state.registerA;
                    state.registerA = state.memory[state.stackPointer + value];
                    break;
                }
                case 3: {
                    state.memory[state.stackPointer + value] = state.registerA;
                    state.registerA = state.registerB;
                    break;
                }
                case 4: {
                    state.registerA = state.memory[state.stackPointer + value];
                    break;
                }
                case 5: {
                    state.memory[state.registerA + value] = state.registerB;
                    break;
                }
                case 6: {
                    state.registerA = state.registerB + state.registerA;
                    break;
                }
                case 7: {
                    state.registerA = state.registerB - state.registerA;
                    break;
                }
                case 8: {
                    state.registerA = state.registerB << state.registerA;
                    break;
                }
                case 9: {
                    state.registerA = state.registerB >> state.registerA;
                    break;
                }
                case 10: {
                    state.stackPointer += value;
                    break;
                }
                case 11: {
                    state.stackPointer = state.registerA;
                    state.registerA = state.registerB;
                    break;
                }
                case 12: {
                    state.registerB = state.registerA;
                    state.registerA = state.stackPointer;
                    break;
                }
                case 13: {
                    state.registerB = state.registerA;
                    state.registerA = state.programCounter;
                    state.programCounter += value;
                    continue;
                }
                case 14: {
                    state.programCounter = state.registerA;
                    state.registerA = state.registerB;
                    break;
                }
                case 15: {
                    if (state.registerA == 0) {
                        state.programCounter += value;
                        continue;
                    }
                    break;
                }
                case 16: {
                    if (state.registerA < 0) {
                        state.programCounter += value;
                        continue;
                    }
                    break;
                }
                case 17: {
                    state.programCounter += value;
                    continue;
                }
                case 18: {
                    return;
                }
            }
            state.programCounter++;
        }
    }
};

int main() {
    EmulatorCore emulator;
    emulator.loadProgramFromFile("arush.o");
    emulator.executeProgram();
    emulator.displayState();
    return 0;
}