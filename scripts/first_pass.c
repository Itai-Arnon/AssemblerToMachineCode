#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "headers/macro.h"
#include "headers/shared.h"
#include "headers/global_vars.h"
#include "headers/utils.h"
#include "headers/symbols.h"
#include "headers/parser.h"
#include "headers/first_pass.h"
#include "headers/assembler.h"
#include "headers/second_pass.h"


static int IC = 100; /*first address of the instruction table is preset in tabel init*/
static int DC = 0;

/**  out.txt ***/
void first_pass(symbol_table_t *sym_tbl, symbol_table_t *externTable, word_table_t *wordTable, word_table_t *dataTable,
                char *filename) {
	int result = 0, n = 0;

	if (isError) {
		return;
	}
	switch (parser.line_type) {
		case OP_CODE:
			setOPCODE_INSTRUCTION(sym_tbl, wordTable);

			break;
		case DIRECTIVE:
			if ((result = parser.directive.cmd) == DATA) {
				set_DATA_WORDS(sym_tbl, dataTable);
			} else if (result == STRING) {
				set_STRING_WORDS(sym_tbl, dataTable);

			} else if (result == ENTRY || result == EXTERN) {

				set_EXTnEntry(sym_tbl, externTable, dataTable);
			}
			break;
		case ERR:
			report_error(ERR_GENERAL_FIRST_PASS_ERROR, line_count, FIRST, CRIT, 0);
			break;
		case TBD:
			report_error(ERR_LINE_UNRECOGNIZED, line_count, FIRST, CRIT, 0);
			break;
		default:
			break;
	}


	second_pass(sym_tbl, externTable, wordTable, dataTable, filename,0);
}

/*sets the instruction word and the send to othre func to set other words*/
void setOPCODE_INSTRUCTION(symbol_table_t *sym_tbl, word_table_t *table) {
	symbol_t *symbol = findSymbol(sym_tbl, parser.symbol_name);

	line_t *line1 = add_line(table, _offset, symbol, A);


	if (symbol != NULL && symbol->address < 100) {
		symbol->address = line1->line_num;
		symbol->type = (symbol->type == _ENTRY) ? _ENTRY : _INSTRUCTION;
		line1->symbol = symbol;
	}
	IC++;
	switch (register_count_by_op(parser.op)) {
		case 0:

			set_opcode_into_word(&(line1->word), parser.op);
			set_ARE_into_word(&(line1->word), A);


			break;
		case 1:

			set_opcode_into_word(&(line1->word), parser.op);
			set_ARE_into_word(&(line1->word), A);
			set_operand_type_into_word(&(line1->word), DESTINATION, parser.operands[1].type_of_register);
			setOPCODE_WORDS(sym_tbl, table, 1, 0);


			break;
		case 2:

			set_opcode_into_word(&(line1->word), parser.op);
			set_ARE_into_word(&(line1->word), A);
			set_operand_type_into_word(&(line1->word), DESTINATION, parser.operands[0].type_of_register);
			set_operand_type_into_word(&(line1->word), SOURCE, parser.operands[1].type_of_register);



			/*INDIRECT and DIRECT Register are set only once to setOPCODE_WORDS register selection decides*/


			if (registerSelection() == 2) {
				/*all combinations except INDIRECT and Regular registry(4th type) type is zero*/
				setOPCODE_WORDS(sym_tbl, table, 0, 0);
				setOPCODE_WORDS(sym_tbl, table, 1, 0);
			} else
				/*INDIRECT and Regular registry, type is one*/
				setOPCODE_WORDS(sym_tbl, table, 1, 1);
	}


}


/*if type = 1  2 regists indirect and/or direct |type=0 only one */
void setOPCODE_WORDS(symbol_table_t *sym_tbl, word_table_t *table, int idx, int type) {
	symbol_t *symbol2 = NULL;
	line_t *line2 = NULL;
	unsigned short num;

	IC++;
	switch (parser.operands[idx].type_of_register) {
		case _IMMEDIATE:
			line2 = add_line(table, _offset, NULL, A);
			num = parser.operands[idx].operand.num;
			/*conversion to two complement*/
			num = convertToTwoComp(num);
			set_immediate_into_empty_word(&(line2->word), num);
			set_ARE_into_word(&(line2->word), A);
			break;

		case _DIRECT:
			line2 = add_line(table, _offset, NULL, R);
			symbol2 = findSymbol(sym_tbl, parser.operands[idx].operand.symbol);
			(symbol2 != NULL) ?: report_error(ERR_SYMBOL_NOT_FOUND, line_count, FIRST, CRIT, IC);
			line2->symbol = symbol2;
			set_label_into_empty_word(&(line2->word), symbol2->address);
			set_ARE_into_word(&(line2->word), R);
			break;

		case _INDIRECT:/*here a check is made if there are two regiters or not*/
		case _REGULAR:

			line2 = add_line(table, _offset, NULL, A);

			if (type == 0) {
				set_register_into_empty_word(&(line2->word), parser.operands[idx].operand.registry, 0);
				set_ARE_into_word(&(line2->word), A);
			}
			if (type == 1) {
				set_register_into_empty_word(&(line2->word), parser.operands[0].operand.registry, 1);
				set_register_into_empty_word(&(line2->word), parser.operands[1].operand.registry, 0);
				set_ARE_into_word(&(line2->word), A);

			}
			break;
		case _ERROR:
			report_error(ERR_REGISTRY_ILLEGAL, line_count, FIRST, CRIT, 0);

		case _TBD:
			/*default value is TBD*/
			break;
	}

}


void set_DATA_WORDS(symbol_table_t *sym_tbl, word_table_t *table) {

	symbol_t *symbol = findSymbol(sym_tbl, parser.symbol_name);
	line_t *line = NULL;
	int i = 0;
	unsigned short num = 0;


	if (symbol != NULL && !(symbol->isUpdate)) {
		symbol->address = DC;
		symbol->type = _DATA;
		symbol->isUpdate = 1;
	}

	for (i = 0; i < parser.directive.operand.data_len; i++) {
		/*only the first line has a potential symbol*/
		line = add_line(table, 0, (i == 0) ? symbol : NULL, A);
		num = parser.directive.operand.data[i];
		num = convertToTwoComp(num);
		set_number_data_word(&(line->word), num);
		DC++;
	}
}

void set_STRING_WORDS(symbol_table_t *sym_tbl, word_table_t *table) {
	symbol_t *symbol = findSymbol(sym_tbl, parser.symbol_name);
	line_t *line = NULL;
	int i = 0;
	if (symbol != NULL && !(symbol->isUpdate)) {
		symbol->address = DC;
		symbol->type = _DATA;
		symbol->isUpdate = 1;
	}

	for (i = 0; i < parser.directive.operand.data_len; i++) {
		/*only the first line has a potential symbol*/
		line = add_line(table, 0, (i == 0) ? symbol : NULL, A);
		set_char_string_word(&(line->word), parser.directive.operand.str[i]);
		DC++;
	}
	/*adding '\0' */
	line = add_line(table, 0, NULL, A);
	set_number_data_word(&(line->word), '\0');
	DC++;


}

/*sets the entry or extern word*/
void set_EXTnEntry(symbol_table_t *sym_tbl, symbol_table_t *externTable, word_table_t *table) {
	symbol_t *symbol = NULL;
	line_t *line = NULL;

	switch (parser.directive.cmd) {

		case EXTERN:
			/*in case EXTERN we are done, label address is zero*/
			symbol = findSymbol(externTable, parser.directive.operand.symbol);
			if (symbol != NULL) {
				line = add_line(table, 0, symbol, E);
				symbol->address = line->line_num;
				/*symbol address should be updated*/
				set_ARE_into_word(&line->word, E);
				DC++;
			}

			break;
		case ENTRY:
			symbol = findSymbol(sym_tbl, parser.directive.operand.symbol);
			if (symbol != NULL) {
				symbol->type = _ENTRY;
				symbol->isUpdate = 1;
				symbol->address = 0; /*TBD on second pass*/

			} else report_error(ERR_ENTRY_SYMBOL_PROB, line_count, FIRST, CRIT, 0);
			break;
		default:
			break;

	}

}


int registerSelection() {
	type_of_register_t type0 = parser.operands[0].type_of_register;
	type_of_register_t type1 = parser.operands[1].type_of_register;
	switch (type0) {
		case _DIRECT:
		case _IMMEDIATE:
			return 2;
		case _REGULAR:
		case _INDIRECT:
			if (type1 == _DIRECT || type1 == _IMMEDIATE)
				return 2;
			else
				return 1;

		default:
			return 2;
			break;
	}
}

unsigned short convertToTwoComp(unsigned short num) {
	if (num >= 0) return num;
	return (~num) + 1;
}

void set_number_data_word(word_t *word, char value) {
	int intVal = convertToTwoComp(value);
	set_value_to_word(word, intVal);
}

void set_char_string_word(word_t *word, char value) {
	char ascii_value = (int) value;
	set_value_to_word(word, ascii_value);

}

/*set the opcode into word*/
void set_opcode_into_word(word_t *word, op_code_t op_code) {
	*word |= op_code << opcode_shift;
}

/*insert operand type into instruction - otype:direct/indirect |type_of_register immdediate/direct/.. */
void set_operand_type_into_word(word_t *word, operand_t otype, type_of_register_t type) {
	set_value_to_word(word, (1 << type) << (operand_shift + operand_size * otype));
}

/* Set ARE in first 3 bits */
void set_ARE_into_word(word_t *word, ARE_T ARE) {
	set_value_to_word(word, 1 << ARE);
}

/*insert operand into new empty word*/
void set_immediate_into_empty_word(word_t *word, unsigned short value) {
	set_value_to_word(word, value << immediate_shift);
}


void set_label_into_empty_word(word_t *word, int value) {
	set_value_to_word(word, value << label_shift);
}

void set_register_into_empty_word(word_t *word, int value, int isSource) {
	set_value_to_word(word, value << (register_shift + isSource * register_size));
}

/*elementry handling of the shift*/
void set_value_to_word(word_t *word, int value) {
	/*0x7FFF against times that 16th bit lights up*/
	*word |= value & 0x7FFF;
}

word_table_t *initTable(word_table_t *table, int memInit) {

	if (!(table = calloc(1, sizeof(word_table_t)))) {
		report_error(ERR_WORD_TABLE_ALLOCATION, line_count, FIRST, CRIT, 0);
		return NULL;
	}

	if (!(table->lines = calloc(1, sizeof(line_t)))) {
		report_error(ERR_WORD_TABLE_ALLOCATION, line_count, FIRST, CRIT, 0);
		return NULL;
	}
	table->size = 0;
	table->lines->line_num = table->size + memInit;
	table->lines->symbol = NULL;
	table->lines->_ARE = A;

	return table;
}


/*adds a line to wordTable/dataTable  the first line has an address 100*/
line_t *add_line(word_table_t *table, int offset, symbol_t *symbol, ARE_T ARE) {
	line_t *new_ptr = NULL;

	if (table == NULL)
		return NULL;

	table->size++;

	if (!(new_ptr = (line_t *) realloc(table->lines, (table->size) * sizeof(line_t)))) {
		report_error(ERR_WORD_TABLE_ALLOCATION, line_count, FIRST, CRIT, 0);
		return NULL;
	}
	if (new_ptr != table->lines)
		table->lines = new_ptr;

	/*data assignmet to new member */
	table->lines[table->size - 1].line_num = table->size + offset - 1; /*address*/
	table->lines[table->size - 1].symbol = symbol;/*pointer to symbol*/
	table->lines[table->size - 1].word = 0;
	table->lines[table->size - 1]._ARE = ARE;


	return &(table->lines[(table->size - 1)]);
}


/*adds a number to all the words in the table*/
void addNumberToWordTable(word_table_t *table, int number) {
	int i;
	if (table == NULL || table->lines == NULL) return;

	for (i = 0; i < table->size; i++) {
		table->lines[i].line_num += number;
	}
}










