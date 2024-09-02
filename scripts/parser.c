#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include<ctype.h>
#include "headers/macro.h"
#include "headers/macro_list.h"
#include "headers/shared.h"
#include "headers/global_vars.h"
#include "headers/utils.h"
#include "headers/symbols.h"
#include "headers/parser.h"
#include "headers/assembler.h"


static direct_arr_t directArray[4];
static opcode_arr_t opcodeArray[16];

static char *opcode_specs[16][3][1] = {
		{{"mov"},  {"-123"}, {"0123"}},
		{{"cmp"},  {"0123"}, {"0123"}},
		{{"add"},  {"-123"}, {"0123"}},
		{{"sub"},  {"-123"}, {"0123"}},
		{{"lea"},  {"-123"}, {"-1--"}},
		{{"clr"},  {"-123"}, {"----"}},
		{{"not"},  {"-123"}, {"----"}},
		{{"inc"},  {"-123"}, {"----"}},
		{{"dec"},  {"-123"}, {"----"}},
		{{"jmp"},  {"-12-"}, {"----"}},
		{{"bne"},  {"-12-"}, {"----"}},
		{{"red"},  {"-123"}, {"----"}},
		{{"prn"},  {"0123"}, {"----"}},
		{{"jsr"},  {"-12-"}, {"----"}},
		{{"rts"},  {"----"}, {"----"}},
		{{"stop"}, {"----"}, {"----"}}};


parser_t parser;
sep_commas_t seperator_c;


void parse(symbol_table_t *sym_tbl, word_table_t *wordTable, word_table_t *dataTable, char *filename) {
	char *buffer = calloc(SET_BUFFER_LENGTH, sizeof(char));/*sentence analyzed*/
	char *orig_buffer = buffer;/*meast to restet the buffer*/
	char *cmd = calloc(MAX_SYMBOL_NAME, sizeof(char));/*cmd parse*/
	char *cmd_extra = calloc(MAX_SYMBOL_NAME, sizeof(char)); /* 2nd cmd parse*/
	char *directive_str1 = NULL;/*auxiliary var*/
	char *directive_str2 = calloc(12, sizeof(char));/*auxiliary var*/
	char *sArr = calloc(MAX_SYMBOL_NAME, sizeof(char));/*.string array*/
	int *pos = calloc(1, sizeof(int));/*promotes the buffer*/
	symbol_table_t *externTable = NULL;/*auxiliary var*/
	int idx, numCount, scanned, result, buff_len, isSymbol, LEN; /*auxiliary vars*/
	idx = numCount = scanned = result = isSymbol = buff_len = LEN = 0;
	rewind(fptr_after);
	line_count = 0;

	externTable = init_symbol_table(externTable);
	initEnumArr();
	initDirectiveArray();/*todo check if needs more*/
/*number of sscanf arguments*/


	if (sym_tbl == NULL) {
		report_error(ERR_FAIL_CREATE_SYMBOL, line_count, PARS, CRIT, 0);
		return;
	}

	while (fgets(buffer, LINE_LENGTH, fptr_after) != NULL) {

		initParser();
		line_count++;
		buffer = strstrip(buffer);

		if (isError) {
			return;
		}

		if (isRestOfLineEmpty(buffer)) continue;

		if (buffer[0] == '\0') continue;

		/*option label is at start - will identify both label and opcode*/
		/*scanned how many succesfuly scanned*/
		if (sscanf(buffer, "%s", cmd) == 1) {
			switch (if_Symbol_if_Duplicate(sym_tbl, cmd, HEAD)) {
				/*returns 1 if symbol not in the table*/
				case 1:
					loadSymbolTable(sym_tbl, cmd, 0, _INSTRUCTION);
				case 2: /*doesn't add symbol */
					buffer = advance_buffer_if_possible(buffer, cmd);
					buffer = strstrip(buffer);
					sscanf(buffer, "%s%n", cmd_extra, pos);/*check 2nd argument*/
					/*scanned == 3 means is entry/extern*/
					(scanned = classify_line(cmd_extra));/*identifies opcode or directives*/
					if (scanned == 3) { /*extern and entry heading symbol should be ignored*/
						delete_symbol(sym_tbl, parser.symbol_name);
						parser.symbol_name[0] = '\0';
						report_error(WAR_EXTERN_ENTRY_SYMBOL, line_count, PARS, NON_CRIT, 0);
					}
					if (scanned > 0)
						buffer = advance_buffer_if_possible(buffer, cmd_extra);
					else
						report_error(ERR_DIRECTIVE_RECOGNITION, line_count, PARS, CRIT, 0);
					/*end of case 1 & 2*/
					break;
				case 0:
					if (classify_line(cmd) > 0)
						buffer = advance_buffer_if_possible(buffer, cmd);
					break;

			}
		} else report_error(ERR_LINE_UNRECOGNIZED, line_count, PARS, CRIT, 0);

		result = scanned = 0;
		buffer = strstrip(buffer);
		memset(cmd, '\0', MAX_SYMBOL_NAME * sizeof(char));
		memset(cmd_extra, '\0', MAX_SYMBOL_NAME * sizeof(char));
		switch (parser.line_type) {
			case OP_CODE:
				/*seperates the registers across an array of strings*/
				seperator_c = string_comma_seps(buffer);

				/*excludes the two op cmds wihtout registry from some of the tests */
				if (seperator_c.counter == 1 && (parser.op == rts || parser.op == stop)) {
					break;
				}
				/*isError - too many comma | register_count_by_op vs num of registers by opcode definition  */
				if (seperator_c.isError == 1 || seperator_c.counter != register_count_by_op(parser.op))
					report_error(ERR_OP_CODE_RECOGNITION, line_count, PARS, CRIT, 0);

				/*idx  1 if only destination reg exits , 0 if two regs exist*/
				idx = (seperator_c.counter == 2) ? 0 : 1;
				for (; idx < seperator_c.counter; idx++) {/*classifies registers*/

					strcpy(cmd_extra, seperator_c.cString[idx]);
					cmd_extra = strstrip(cmd_extra);
					parser.operands[idx].type_of_register = classifyRegisters(cmd_extra,
					                                                          idx);
				}
				/*if by the opcode registers a valid and if they are different*/
				if (checkRegisterCompliance() == 0)
					report_error(ERR_OP_CODE_REGISTRY_ILLEGAL, line_count, PARS, CRIT, 0);

				/*Creat symbol a register with direct : labels*/
				for (idx = 0; idx < seperator_c.counter; idx++) {
					if (parser.operands[idx].type_of_register == _DIRECT) {    /*_Direct means label*/
						/* if_Symbol_if_Duplicant:chks if symbol| 1 is new symbol|2 is duplicant|last arg decides if symbol is start/middle */
						if (if_Symbol_if_Duplicate(sym_tbl, parser.operands[idx].operand.symbol, MIDDLE) ==
						    1) {
							loadSymbolTable(sym_tbl, parser.operands[idx].operand.symbol, 0, _INSTRUCTION);
						}
					}
				}
				break;
			case DIRECTIVE:
				if ((result = parser.directive.cmd) == DATA) {
					/*separates the numbers across an array of strings*/
					numCount = countNumbersInString(buffer);
					seperator_c = string_comma_seps(buffer);
					/*check if buffer number count concurs with separator struct|isErrpr has
					 * error ocuured*/
					if (seperator_c.isError || seperator_c.counter != numCount)
						report_error(ERR_DATA_DIRECTIVE_NUMBER, line_count, PARS, CRIT, 0);
					/*amount of numbers*/
					parser.directive.operand.data_len = numCount;
					/*allocates memory for the numbers*/
					parser.directive.operand.data = (int *) calloc(parser.directive.operand.data_len, sizeof(int));
					/*assigns the numbers to the parser*/
					for (idx = 0; idx < seperator_c.counter; idx++) {
						directive_str1 = (seperator_c.cString[idx]);
						LEN = strlen(directive_str1);
						if (directive_str1[LEN - 1] == '\n') LEN--;
						sscanf(directive_str1, "%s", directive_str2);
						*pos = convertOrCheckStringToNum(directive_str2, 0);
						/*checking bounds of values*/
						if (*pos < MIN_15Bit_NUM || *pos > MAX_15Bit_NUM)
							report_error(ERR_DATA_DIRECTIVE_NUMBER, line_count, PARS, CRIT, 0);
						/*assigning  the number to the parser*/
						parser.directive.operand.data[idx] = *pos;
					}
					break;
				} else if (result == STRING) {
					buff_len = strlen(buffer) - 2;
					if (processString(buffer, sArr) == 0)
						report_error(ERR_STRING_QUOTATION_MISSING, line_count, PARS, CRIT, 0);
					else {
						parser.directive.operand.str = (char *) calloc(buff_len, sizeof(char));
						parser.directive.operand.data_len = buff_len;
					}
					for (idx = 0; idx < buff_len; idx++) {
						parser.directive.operand.str[idx] = sArr[idx];
					}
					break;
				} else if (result == ENTRY || result == EXTERN) {

					if (sscanf(buffer, "%s%n", cmd, pos) == 1) {
						strstrip(buffer);
						buffer += *pos;
						if (isRestOfLineEmpty(buffer) == 0)
							report_error(ERR_OP_CODE_FAILED_STRUCTURE, line_count, PARS, CRIT, 0);

						if (result == ENTRY) {
							if (isDuplicateSymbol(sym_tbl, cmd) == 0)
								loadSymbolTable(sym_tbl, cmd, 0, _ENTRY);

						} else if (result == EXTERN)
							loadSymbolTable(externTable, cmd, 0, _EXTERN);

						report_error(WAR_EXTERN_ENTRY_SYMBOL, line_count, PARS, NON_CRIT, 0);
						strcpy(parser.directive.operand.symbol, cmd);
						parser.directive.operand.data_len = 1;

					} else
						report_error(ERR_DIRECTIVE_RECOGNITION, line_count, PARS, CRIT, 0);
				}
				break;
			case ERR:
				report_error(ERR_LINE_UNRECOGNIZED, line_count, PARS, CRIT, 0);
				break;
			case TBD:
				report_error(ERR_DIRECTIVE_RECOGNITION, line_count, PARS, CRIT, 0);
				break;
			default:
				break;

		}/*end of switch*/
		if (isError) {
			break;
		}

		buffer = orig_buffer;
		first_pass(sym_tbl, externTable, wordTable, dataTable, filename);
	}
	free(cmd);
	free(directive_str2);
	free(sArr);
	free(pos);
} /*END OF PARSE*/



int classify_line(char *cmd) {
	int j = 0;
	int len = 0;

	if (*cmd == '\0') return -1;


	for (j = 0; j < 16; ++j) {
		if (strcmp(cmd, opcodeArray[j].opcode_name) == 0) {
			parser.line_type = OP_CODE;
			/*although only one opcode defines for two operands due to struct structure*/
			parser.op = opcodeArray[j].op_enum;
			return 1;
		}
	}
	for (j = 0; j < 4; ++j) {
		if (strcmp(cmd, directArray[j].direct_name) == 0) {
			parser.line_type = DIRECTIVE;
			parser.directive.cmd = directArray[j].cmd;
			/*extern and entry can't have a label before them*/
			if (parser.directive.cmd == EXTERN || parser.directive.cmd == ENTRY) {
				if (parser.symbol_name[0] != '\0')
					parser.directive.cmd = directArray[j].cmd;
				return 3;
			}
			return 1;
		}
	}
	parser.line_type = ERR;
	return 0;
}


/*checks type of registers and correctnees - sets then in parser*/
/*first or second operands relates to the  parser*/
type_of_register_t classifyRegisters(char *str, int first_or_second_operand) {

	int i;
	char *s_ptr = NULL;
	int len = strlen(str);
	int number = 0, flag = 0;
	/* Check if  '#' hence immediate */
	if (strncmp(str, "#", 1) == 0) {
		str++;
		/*minus is allowed only in the beginning */
		if (*str == '-') {
			str++;
			flag = 1;
		}
		s_ptr = str;
		while (isdigit(*str++));
		if (*str)
			report_error(ERR_OP_CODE_REGISTRY_ILLEGAL, line_count, PARS, CRIT, 0);
		else {

			number = convertOrCheckStringToNum(s_ptr, 0);
			number = (flag == 1) ? number * -1 : number;
			parser.operands[first_or_second_operand].operand.num = number;
			return _IMMEDIATE;
		}

		/* Check if '*'hence indirect */
		/* in the reg there's an address of the real value */
	} else if (strncmp(str, "*", 1) == 0) {
		if (str[1] == 'r' && isdigit(str[2]) && str[2] >= '0' && str[2] <= '7') {
			parser.operands[first_or_second_operand].operand.registry = str[2] - '0';
			return _INDIRECT; /*number 3 pointer type*/
		}
	}
		/* Check if the string starts with 'r' */
	else if (strncmp(str, "r", 1) == 0)
		switch (str[0] == 'r' && isdigit(str[1]) && (str[1]) >= '0' && (str[1]) <= '7') {
			case 0:
				report_error(ERR_REGISTRY_ILLEGAL, line_count, PARS, CRIT, 0);
				return _ERROR;
			case 1:
				parser.operands[first_or_second_operand].operand.registry = str[1] - '0';
				return _REGULAR; /*Number 4*/
		}
	else if (/* Default case for other characters */
			checkLegalName(str, ALPHANUM_COMBINED)) {

		str = removeColon(str);
		strcpy(parser
				       .operands[first_or_second_operand].operand.symbol, str);
		return _DIRECT; /*label*/   /*todo chk if label duplicate and add accordingly*/
	}

	return _ERROR; /*if nothing was found*/
}

/*return number of registers allowed, -1 means op_code number of registers is wrong*/
int register_count_by_op(op_code_t op) {
	if (op < mov || op > stop)
		return -1;
	if (op >= mov && op <= lea)
		return 2;
	else if (op >= clr && op <= jsr)
		return 1;
	else
		return 0;
}

/*checks type of register assigned to register is correct */
int checkRegisterCompliance() {
	type_of_register_t type1, type2;
	op_code_t opCode = parser.op;
	type1 = type2 = 0;

	switch (register_count_by_op(opCode)) {
		case 0:/*case of no operands always correct*/
			return 1;
		case 1:
			/*check destination operands only */
			type2 = parser.operands[0].type_of_register;


			if (type2 == opcode_specs[opCode][2][0][type1] - '0')
				return 1;
			return 0;
			break;
		case 2:
			/*check 2 operands*/
			type1 = parser.operands[0].type_of_register;
			type2 = parser.operands[1].type_of_register;
			/*comparison based opcpde_specs which defines all possible modes for each reg*/
			/*[OpCode][ 2 = source| 1 = destination][0][regsitry types define index] */
			if ((type1 == opcode_specs[opCode][2][0][type1] - '0') &&
			    (type2 == opcode_specs[opCode][1][0][type2] - '0'))
				return 1;
			return 0;
			break;
		default:
			break;
	}
}


char *advance_buffer_if_possible(char *buffer, char *cmd) {
	int cmd_len = strlen(cmd);
	if (strlen(buffer) > cmd_len) {
		buffer += cmd_len;
		return buffer;
	}
	return buffer;
}


/* process .string  string  0 : non printable asciis  1: success   */
int processString(char *str, char *sArr) {
	int len = strlen(str);
	int idx, i;
	i = idx = 0;

	if (len < 2) return 0;

	if (str[0] == '\"' && str[len - 1] == '\"') {
		str++;
		len -= 2;
	} else return 0;

	/* check each character using isprint that check for printable chars */
	for (i = 0; i < len; i++) {
		if (isprint(str[i])) {
			sArr[idx++] = str[i];
		} else
			return 0;
	}
	sArr[idx] = '\0';
	return 1;
}


sep_commas_t string_comma_seps(char *str) {
	sep_commas_t seperator = {0};
	char *s;
	int seps_count = 0;
	if (*str == '\0') {
		return seperator;
	}
	do {
		seperator.cString[seps_count] = str;
		seps_count++;
		s = strchr(str, ',');
		if (s) {
			*s = '\0';
			s++;
			if (*s == ',') {
				seperator.isError = 1;
				return seperator;
			}
		}
		str = s;
	} while (s);
	seperator.counter = seps_count;
	return seperator;
}

/* Function to initialize and populate an array of enum_arr*/
void initEnumArr() {
	int i = 0;
	op_code_t arr[16] = {mov, cmp, add, sub, lea, clr, not, inc, dec, jmp,
	                     bne, red, prn, jsr, rts, stop};

	char *opcode_names[16] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp",
	                          "bne", "red", "prn", "jsr", "rts", "stop"};
	/*Populate the array*/
	for (i = 0; i < 16; ++i) {
		opcodeArray[i].opcode_name = opcode_names[i];
		opcodeArray[i].op_enum = arr[i];
	}
}

/*Function to initialize and populate the directArray*/
void initDirectiveArray() {
	int i;
	directive_cmd_t arr[4] = {DATA, STRING, ENTRY, EXTERN};
	char *directives[4] = {".data", ".string", ".entry", ".extern"};
	for (i = 0; i < 4; ++i) {
		directArray[i].direct_name = directives[i];
		directArray[i].cmd = arr[i];
	}
}


/** Function to print the values of parser variables */
/** meant for development only */
void printParser() {
	/** Declare necessary variables */
	int i;

	/** Print line type */
	printf("Line Type: %d\n", parser.line_type);

	/** Print symbol name */
	printf("Symbol Name: %s\n", parser.symbol_name);

	/** Print directive fields */
	printf("Directive Enum: %d\n", parser.directive.cmd);
	printf("Directive Symbol: %s\n", parser.directive.operand.symbol);
	printf("Directive Data Length: %d\n", parser.directive.operand.data_len);
	if (parser.directive.operand.data != NULL) {
		printf("Directive Data: ");
		for (i = 0; i < parser.directive.operand.data_len; i++) {
			printf("%d ", parser.directive.operand.data[i]);
		}
		printf("\n");
	}
	printf("Directive String: %s\n", parser.directive.operand.str);

	printf("Operand %d Op Code: %d\n", i, parser.op);
	/** Print operands fields */
	for (i = 0; i < 2; ++i) {
		printf("Operand %d Type of Register: %d\n", i, parser.operands[i].type_of_register);
		printf("Operand %d Number: %d\n", i, parser.operands[i].operand.num);
		printf("Operand %d Symbol: %s\n", i, parser.operands[i].operand.symbol);
		printf("Operand %d Registry: %d\n", i, parser.operands[i].operand.registry);
	}
}

void initParser() {
	int i;

	/* Initialize line type */
	parser.line_type = ERR;
	/* Initialize symbol name */
	memset(parser.symbol_name, '\0', sizeof(parser.symbol_name));
	/* Initialize directive fields */
	parser.directive.cmd = DATA; /* Assuming DATA is a default value */
	memset(parser.directive.operand.symbol, 0, sizeof(parser.directive.operand.symbol)); /*non dynamic alloc*/

	free(parser.directive.operand.data);

	parser.directive.operand.data = NULL;

	free(parser.directive.operand.str);
	parser.directive.operand.str = NULL;

	parser.directive.operand.data_len = 0;

	/* Initialize operands fields */
	parser.op = mov; /* Assuming mov is a default value */
	for (i = 0; i < 2; ++i) {

		parser.operands[i].type_of_register = _TBD; /*type of register*/
		memset(parser.operands[i].operand.symbol, 0, sizeof(parser.operands[i].operand.symbol));
		parser.operands[i].operand.num = 0;
		parser.operands[i].operand.registry = 0;
	}
}

