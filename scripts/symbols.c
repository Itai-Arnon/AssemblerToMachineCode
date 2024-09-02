#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "headers/symbols.h"
#include "headers/global_vars.h"
#include "headers/macro_list.h"
#include "headers/shared.h"
#include "headers/utils.h"
#include "headers/first_pass.h"
#include "headers/assembler.h"
#include "headers/parser.h"


/* todo 0 failure 1:success*/
int loadSymbolTable(symbol_table_t *sym_tbl, char *symbol_name, int address, memory_t type) {
	int res = 0;
	symbol_t *end = sym_tbl->symbol_List;
	symbol_t *node = create_symbol(sym_tbl, symbol_name, address, type);/*create symbols takes care of error*/

	if (node == NULL) {
		return 0;
	}

	sym_tbl->size++;
	if (end != NULL) {
		while (end->next_sym != NULL) {
			end = end->next_sym;
		}
		end->next_sym = node;
	} else
		sym_tbl->symbol_List = node;

	return 1;
}


symbol_table_t *init_symbol_table(symbol_table_t *sym_tbl) {

	if (!(sym_tbl = calloc(1, sizeof(symbol_table_t)))) {
		report_error(ERR_FAIL_CREATE_SYMBOL_TBL, line_count, SYM, CRIT, 0);
		return NULL;
	}
	sym_tbl->symbol_List = NULL;
	sym_tbl->size = 0;

	return sym_tbl;

}

symbol_t *create_symbol(symbol_table_t *sym_tbl, char *symbol_name, int address, memory_t type) {
	symbol_t *node = NULL;

	int LEN = strlen(symbol_name);
	if (symbol_name[LEN - 1] == ':')
		LEN -= 1;
	/*check similarity with opcodes and directives*/

	if (is_symbol_name_duplicate(sym_tbl, symbol_name) == 1 ) {
		report_error(ERR_DUPLICATE_SYMBOL_NAME, line_count, SYM, CRIT, 0);
		return NULL;
	}

	if ((node = malloc(sizeof(symbol_t)))) {
		sym_tbl->size++;
		memset(node->symbol_name, '\0', sizeof(node->symbol_name));
		strncpy(node->symbol_name, symbol_name, LEN);
		node->address = address;
		node->type = type;
		node->next_sym = NULL;
		node->isUpdate = 0;
		node->index = sym_tbl->size;
		return node;
	}
	report_error(ERR_FAIL_CREATE_SYMBOL, line_count, SYM, CRIT, 0);
	return NULL;
}

symbol_t *findSymbol(symbol_table_t *sym_tbl, char *symbol_name) {
	symbol_t *head = NULL;
	int LEN = strlen(symbol_name);

	if (sym_tbl == NULL) {
		return NULL;
	}
	/*in case the colon is attached LEN will be shortened and strncmp is used*/
	LEN = (symbol_name[LEN - 1] == ':') ? LEN - 1 : LEN;

	head = sym_tbl->symbol_List;

	while (head != NULL) {
		if (strncmp(head->symbol_name, symbol_name, LEN) == 0)
			return head;
		head = head->next_sym;
	}
	return NULL; /*didn't find symbol*/
}

/*general func |-1 symbol list is empty, 0-no duplicants , 1 it is a duplicate*/
int isDuplicateSymbol(symbol_table_t *sym_tbl, char symbol_name[]) {
	symbol_t *head = sym_tbl->symbol_List;
	int LEN = strlen(symbol_name);

	if (symbol_name[LEN - 1] == ':')
		LEN -= 1;

	while (head != NULL) {
		if (strncmp(symbol_name, head->symbol_name, (LEN)) == 0) {
			/*if symbol already exist returns 1*/
			return 1;
		}
		head = head->next_sym;
	}
	return 0; /*no duplicate*/
}
/*if YES it willl update override , NO just checck if addresss*/
/*identifies it's a symbols and reports if it's duplicate or not 1 -dup 2-not dup
 * isHeadOrMid selects between beginning label or those that are inside */
int if_Symbol_if_Duplicate(symbol_table_t *sym_tbl, char *cmd, symbol_loci_t isHEadOrMid) {
	int len = 0;
	len = strlen(cmd);

/*if isHeadOrMid == 1 will not allow symbol w/o ':"*/
/*HEAD of the line */
	switch (isHEadOrMid) {
		case HEAD:
			if (cmd[len - 1] != ':') {
				parser.line_type = ERR;
				return 0;
			}
			strncpy(parser.symbol_name, cmd, len - 1);
			return isDuplicateSymbol(sym_tbl, cmd) == 0 ? 1 : 2;
			break;
			/* MIDDLE in one the registries */
		case MIDDLE:
			return isDuplicateSymbol(sym_tbl, cmd) == 0 ? 1 : 2;
			break;
	}
}

/*meant to detect if name is a duplicate directive or opcode or prior labels are */
int is_symbol_name_duplicate(symbol_table_t *sym_tbl, char *symbol_name) {

	char *opcode_names[16] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp",
	                          "bne", "red", "prn", "jsr", "rts", "stop"};
	char *directives[4] = {".data", ".string", ".entry", ".extern"};

	symbol_t *head = NULL;
	int LEN = strlen(symbol_name);
	int j = 0;
	int first_index = 0;

	if (symbol_name[0] == '\0' || sym_tbl == NULL)
		return 0;




	for (j = 0; j < 16; ++j) {
		if (strcmp(symbol_name, opcode_names[j]) == 0)
			return 1;
	}
	for (j = 0; j < 4; ++j) {
		if (strcmp(symbol_name, directives[j]) == 0)
			return 1;
	}

		head = sym_tbl->symbol_List;
		if(head != NULL){
			first_index = head->index;
		}

	LEN = (symbol_name[LEN - 1] == ':') ? LEN - 1 : LEN;
	while (head != NULL) {
		if (strncmp(head->symbol_name, symbol_name, LEN) == 0 && first_index != head->index)
			return 1;
		else
			head = head->next_sym;
	}

	return 0;
}

int delete_symbol(symbol_table_t *sym_tbl, char *symbol_name) {
	symbol_t *head = NULL;
	symbol_t *prev = NULL;
	int LEN = strlen(symbol_name);

	if (symbol_name[LEN - 1] == ':')
		LEN -= 1;
	if (sym_tbl == NULL || sym_tbl->symbol_List == NULL)
		return 0;



	/*if head is the requested one, we unlink it the free head*/
	head= sym_tbl->symbol_List;

	if (head->next_sym == NULL && strncmp(symbol_name, head->symbol_name, (LEN)) == 0) {
		free(head);
		head = NULL;
		return 1;
		/* search for requested node*/
	}else {
		while(head != NULL && ((strncmp(symbol_name, head->symbol_name, LEN)) != 0)) {
			prev = head;
			head = head->next_sym;
		}

			/*case null the requested symbol was not found*/
			if (head == NULL)
				return 0; /*we reached LL End node not found*/

			if (head->next_sym != NULL)
				prev->next_sym = head->next_sym;
			else
				prev->next_sym = NULL;

			free(head);
			head = NULL;
			sym_tbl->size--;

			return 1;
		}
}
