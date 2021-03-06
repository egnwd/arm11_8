#include "parse.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void parseProgram(SymbolTable *map, Program *program) {
  Token *tokenPtr = program->tokens->tokens;
  while (tokenPtr->type != ENDFILE) {
    parseLine(tokenPtr, program);
    do {
      tokenPtr++;
    } while(tokenPtr->type != NEWLINE);
    tokenPtr++;
    if (tokenPtr->type == OTHER) {
      program->addr += WORD_SIZE;
    }
  }
  //loaded variables
  while (!isEmpty(program->loadExpr)) {
    instruction instr = dequeue(program->loadExpr);
    program->addr += WORD_SIZE;
    outputData(instr, program);
  }
}

void parseLine(Token *token, Program *program) {
  if (token->type == OTHER) {
    parseInstruction(token, program);
  }
}

void parseInstruction(Token *token, Program *program) {
  switch(index_of(token->value, mnemonicStrings)) {
    case ADD: case SUB: case RSB: case AND: case EOR:
    case ORR: parseTurnaryDataProcessing(token, program); break;

    case MOV: case TST: case TEQ:
    case CMP: parseBinaryDataProcessing(token, program); break;

    case MUL:
    case MLA: parseMul(token, program); break;

    case LDR:
    case STR: parseSingleDataTransfer(token, program); break;

    case BEQ: case BNE: case BGE: case BLT: case BGT: case BLE:
    case B: parseBranch(token, program); break;

    case LSL: parseLsl(token, program); break;

    case ANDEQ: generateHaltInstruction(program); break;
  }
}

//Parse Instructions
void parseTurnaryDataProcessing(Token *token, Program *program) {
  //Get args in Token form
  Token *rd_token = token + 1;
  Token *rn_token = token + 2;
  Token *operand_token = token + 3;

  //Get numbers associated to args
  int rd,rn,operand,i;
  rd = map_get(&registerTable, rd_token->value);
  rn = map_get(&registerTable, rn_token->value);

  //Find if operand is an immediate value (i = 1)
  if(operand_token->type == LITERAL) {
    i = SET;
    char *ptr;
    operand = (int) strtol(operand_token->value, &ptr, 0);
  } else {
    //Register
    i = NOT_SET;
    int rm = map_get(&registerTable, operand_token->value);
    operand = rm & 0xF;
    generateShift((operand_token + 1), &operand);
  }

  generateDataProcessingInstruction(map_get(&mnemonicTable, token->value), rd, rn, operand, NOT_NEEDED, i, program);
}

void parseBinaryDataProcessing(Token *token, Program *program) {
  //Get args in Token form
  Token *rdOrRn_token = token + 1;
  Token *operand_token = token + 2;

  //Get numbers associated to args
  int rdOrRn,operand,i;
  rdOrRn = map_get(&registerTable, rdOrRn_token->value);
  //Find if operand is an immediate value (i = 1)
  if(operand_token->type == LITERAL) {
    char *ptr;
    i = SET;
    operand = (int) strtol(operand_token->value, &ptr, 0);
  } else {
    //Register
    i = NOT_SET;
    int rm = map_get(&registerTable, operand_token->value);
    operand = rm & 0xF;
    generateShift((operand_token + 1), &operand);
  }

  if (strcmp(token->value,"mov") == 0) {
    generateDataProcessingInstruction( map_get(&mnemonicTable, token->value), rdOrRn,
                                  NOT_NEEDED, operand, NOT_SET, i, program);
  } else {
    generateDataProcessingInstruction( map_get(&mnemonicTable, token->value), NOT_NEEDED,
                                  rdOrRn, operand, SET, i, program);
  }
}

void parseMul(Token *token, Program *program) {
  //Get args in Token form
  Token *rd_token = token + 1;
  Token *rm_token = token + 2;
  Token *rs_token = token + 3;
  Token *rn_token = token + 4;

  //Get numbers associated to args
  int rd, rm, rs, rn, a;
  rd = map_get(&registerTable, rd_token->value);
  rm = map_get(&registerTable, rm_token->value);
  rs = map_get(&registerTable, rs_token->value);

  if (rn_token->type != NEWLINE) {
    rn = map_get(&registerTable, rn_token->value);
    a = SET;
  } else {
    rn = NOT_SET;
    a = NOT_SET;
  }
  generateMultiplyInstruction(map_get(&mnemonicTable, token->value), rd, rm, rs,
                              rn, a, program);
}

void parseSingleDataTransfer(Token *token, Program *program) {
  // Define fields
  uint32_t i, p, u, l, rd, rn;
  int32_t offset = NOT_SET;

  Token *rdToken = token + 1;
  Token *addrToken = token + 2;

  l = !strcmp(token->value, "ldr");
  rd = map_get(&registerTable, rdToken->value);

  if (addrToken->type == EXPRESSION) { /* <ldr/str> Rd, =expr */
    //Get Expression
    char *ptr;
    uint32_t ex = (uint32_t) strtol(addrToken->value, &ptr, 0);

    bool isMov = (ex <= 0xFF);
    program->length += isMov ? 0 : WORD_SIZE;
    offset = isMov ? ex : program->length - program->addr - ARM_OFFSET;

    p = SET;
    rn = map_get(&registerTable, k_PC);
    u = !isMov;

    if (offset < 0) {
      offset *= -1;
    }

    if (isMov) {
      i = SET;
      generateDataProcessingInstruction( map_get(&mnemonicTable, "mov"), rd, NOT_NEEDED,
                                    offset, NOT_SET, i, program);
      return;
    } else {
      i = NOT_SET;
      enqueue(program->loadExpr, ex);
    }
  }
  else {
    char *addrValue = malloc(100);
    strcpy(addrValue, addrToken->value);
    Token *offsetToken = (addrToken + 1);
    if (offsetToken->type == NEWLINE) { /* <ldr/str> Rd, [Rn] */
      p = SET;
      u = SET;
      i = NOT_SET;
      offset = NOT_SET;
      rn = map_get(&registerTable, stripBrackets(addrValue));
    }
    else {
      /* pre/post cases */
      p = isPreIndex(addrValue);
      rn = map_get(&registerTable, stripBrackets(addrValue));

      int rm = map_get(&registerTable,stripLastBracket(offsetToken->value));
      if (rm == NOT_FOUND) { /* <ldr/str> Rd, Rn, #offset */
        i = NOT_SET;
        char *ptr;
        offset = strtol(offsetToken->value, &ptr, 0);
        if (offset < 0) {
          offset *= -1;
          u = 0;
        } else {
          u = 1;
        }
      } else { /* <ldr/str> Rd, Rn, Rm {, <shift>} */
        offset = rm & 0xF;

        Token *shiftTypeToken = (offsetToken + 1);
        generateShift(shiftTypeToken, &offset);

        i = SET;
        u = SET;
      }
    }
  }
  generateSingleDataTransferInstruction(i, p, u, l, rd, rn, offset, program);
}

void parseBranch(Token *token, Program *program) {
  //Get args in Token form
  Token *lblToken = token + 1;

  //Get numbers associated to args
  uint8_t cond; int offset;
  cond = (uint8_t) map_get(&mnemonicTable, token->value);
  offset = map_get(lblToAddr, lblToken->value) - program->addr - ARM_OFFSET;

  generateBranchInstruction(cond, offset >> 2, program);
}

void parseLsl(Token *token, Program *program) {
  Token *rn_token = token + 1;
  Token *operand_token = token + 2;

  int rn = map_get(&registerTable, rn_token->value);
  char *ptr;
  int operand =  (strtol(operand_token->value, &ptr, 0) << 7) & 0xF80;
  operand |= rn & 0xF;

  generateDataProcessingInstruction( map_get(&mnemonicTable, "mov"), rn, NOT_NEEDED,
                                operand, NOT_SET, NOT_SET, program);
}

//Helper Functions

void generateShift(Token *shiftTypeToken, int32_t *currOffset) {
  if (shiftTypeToken->type != NEWLINE) { // Shift Needed
    int shift;
    Token *shiftToken = (shiftTypeToken + 1);
    if (shiftToken->type == LITERAL || shiftToken->type == EXPRESSION) {
      char *ptr;
      shift = (int) strtol(shiftToken->value, &ptr, 0);
      *currOffset |= (shift & 0x1F) << 7;
    } else { // Shift is a register
      shift = map_get(&registerTable, shiftToken->value);
      *currOffset |= SET << 4;
      *currOffset |= (shift & 0xF) << 8;
    }
    int shift_type = map_get(&shiftTable, shiftTypeToken->value);
    *currOffset |= (shift_type & 3) << 5;
  }
}

int index_of(char *value, char **arr) {
  for (int i = 0; i < 23; i++) {
    if (strcmp(arr[i], value) == 0) {
       return i;
    }
  }
  return NOT_FOUND;
}

char* stripBrackets(char *str) {
  stripLastBracket(str);
  return ++str;
}

char* stripLastBracket(char *str) {
  int lastChar = strlen(str)-1;
  if (strcmp((str + lastChar), "]") == 0) {
    str[lastChar] = '\0';
  }
  return str;
}

bool isPreIndex(char *str) {
  return !(strchr(str, ']') == &str[strlen(str)-1]);
}
