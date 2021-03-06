#include "generate.h"

void generateDataProcessingInstruction(int32_t opcode,
                                  int32_t rd,
                                  int32_t rn,
                                  int32_t operand,
                                  int32_t s,
                                  int32_t i,
                                  Program *program) {
  instruction instr = NOT_SET;

  //Append all fields
  setField(&instr, 4, 0xE);
  setField(NULL, 2, NOT_SET);
  setField(NULL, 1, i);
  setField(NULL, 4, opcode);
  setField(NULL, 1, s);
  setField(NULL, 4, rn);
  setField(NULL, 4, rd);

  //If immediate must calculate rotation
  if (i == SET && operand > 0xff) {
    setField(NULL, 12, generateImmediate(operand));
  } else {
    setField(NULL, 12, operand);
  }

  outputData(instr, program);
}

void generateMultiplyInstruction(int32_t opcode,
                            int32_t rd,
                            int32_t rm,
                            int32_t rs,
                            int32_t rn,
                            int32_t a,
                            Program *program) {
  instruction instr = NOT_SET;

  //Append all fields
  setField(&instr, 4, 0xE);
  setField(NULL, 6, NOT_SET);
  setField(NULL, 1, a);
  setField(NULL, 1, NOT_SET);
  setField(NULL, 4, rd);
  setField(NULL, 4, rn);
  setField(NULL, 4, rs);
  setField(NULL, 4, 9);
  setField(NULL, 4, rm);

  outputData(instr, program);
}

void generateBranchInstruction(int32_t cond, int32_t offset, Program *program) {
  instruction instr = NOT_SET;

  //Append all fields
  setField(&instr, 4, cond);
  setField(NULL, 4, 0xA);
  setField(NULL, 24, offset);

  outputData(instr, program);
}

void generateSingleDataTransferInstruction(uint32_t i,
                                      uint32_t p,
                                      uint32_t u,
                                      uint32_t l,
                                      uint32_t rd,
                                      uint32_t rn,
                                      uint32_t offset,
                                      Program *program) {
  instruction instr = NOT_SET;

  //Append all fields
  setField(&instr, 4, 0xE);
  setField(NULL, 2, SET);
  setField(NULL, 1, i);
  setField(NULL, 1, p);
  setField(NULL, 1, u);
  setField(NULL, 2, NOT_SET);
  setField(NULL, 1, l);
  setField(NULL, 4, rn);
  setField(NULL, 4, rd);
  setField(NULL, 12, offset);

  outputData(instr, program);
}

void generateHaltInstruction(Program *program) {
  int32_t instr = 0;
  outputData(instr, program);
}

uint32_t generateImmediate(uint32_t value) {
  uint32_t output = 0;
  if (value != 0) {
    int rotation = 32;
    int32_t imm = value;
    while (imm % 4 == 0) {
      rotation--;
      imm = imm >> 2;
    }
    output |= (rotation & 0xf) << 8;
    output |= imm & 0xff;
  }
  return output;
}
