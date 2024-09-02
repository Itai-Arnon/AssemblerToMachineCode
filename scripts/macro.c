#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "headers/macro.h"
#include "headers/macro_list.h"
#include "headers/shared.h"
#include "headers/global_vars.h"
#include "headers/utils.h"
#include "headers/symbols.h"
#include "headers/assembler.h"
#include "headers/parser.h"

int line_count;


void read_preprocessor(macro_table_t *tbl, symbol_table_t *sym_tbl) {
	char *buffer = malloc(sizeof(char) * SET_BUFFER_LENGTH);
	char *buffer_orig = buffer;
	char *macro_name = (char *) calloc(MAX_MACRO_NAME, sizeof(char));
	int LEN = 0, count = 0;
	int idx = 0;
	int *pos = calloc(1, sizeof(int));



	line_count = 0;

	if (tbl == NULL || sym_tbl == NULL) {
		report_error(ERR_MACRO_TABLE_GENERAL_ERROR, line_count, MAC, CRIT, 0);
	}

	if (fgets(buffer, SET_BUFFER_LENGTH, fptr_before) == 0)
		report_error("Warning FGETS is empty", line_count, MAC, NON_CRIT, 0);

		rewind(fptr_before);
		line_count = 0;
	/*start of reading from file*/
	while (fgets(buffer, SET_BUFFER_LENGTH, fptr_before) != NULL) {

		line_count++;
		if (isError)
			return;

		if (strlen(buffer) > LINE_LENGTH) {
			report_error(ERR_LINE_LENGTH, line_count, MAC, CRIT, 0);
			return;
		}


		/*check for empty line*/
		if (isRestOfLineEmpty(buffer)) /*checks case of empty line*/
			continue;
       /*remove white space from front and back*/



		isHeadOfSentenceValid(tbl, buffer);




		if (*buffer == ';' || *buffer == '\0') { /*check for comment ;*/
			continue;
		}
		/*check if sentence is too long */
		/*find \n in mid sentence means lenght illegal*/
		if (findSeperator(buffer, '\n') == 1) {
			report_error(ERR_ILLEGAL_CHAR, line_count, MAC, CRIT, 0);
			return;
		}


		switch (typeofline(tbl, buffer, macro_name, sym_tbl)) {
			case MACRO_START:

				tbl->isMacroOpen = 1;
				break;
			case MACRO_END:
				/*closes macro writing*/
				tbl->isMacroOpen = 0;
				/*macro is locked from rewriting forever*/
				macro_lock(tbl, macro_name);
				memset(macro_name,'\0',MAX_MACRO_NAME);
				break;
			case MACRO_EXPAND:
				expandMacro(tbl, macro_name);
				break;
			case LINE_INSIDE:

				if (tbl->isMacroOpen == 1) {
					if ((loadMacroTable(tbl, macro_name, buffer)) == 0)
						report_error("MACRO TABLE FAILED TO LOAD", line_count, MAC, CRIT, 0);
				}
				break;
			case LINE_OUTSIDE:
				/*write to after file*/
				if(dupNameExistsInTable(tbl, buffer) == 1){
					expandMacro(tbl, buffer);
					return;
				}

				fprintf(fptr_after, "%s", buffer);

				break;
			case MACRO_ERROR:
				report_error(ERR_MACRO_NAME_WRONG,line_count,MAC , NON_CRIT , 0);
			break;
			default:
				break;
		}


		buffer = buffer_orig;

	}
	free(buffer); /* Free allocated memory */
	free(macro_name);
	free(pos);

}

/*line- fgets scan , macro_name-in charge of tranfering macro name, sym_tbl - symbols are scanned for */
int typeofline(macro_table_t *tbl, char *line, char *macro_name, symbol_table_t *sym_tbl) {
	char *buffer = calloc(LINE_LENGTH, sizeof(char));
	char *typeofline_buffer = buffer;
	char *start = calloc(MAX_MACRO_NAME+1, sizeof(char));
	int pos = 0;


	strcpy(buffer, line);
	/*removes white space from the front*/

	if (sscanf(buffer, "%s%n", start, &pos) == 1) {

		start[strlen(start)]='\0';

		if (checkMacroStart(buffer, start, macro_name, pos, sym_tbl, tbl))
			return MACRO_START;
		else if (checkMacroEnd(buffer, start, pos))
			return MACRO_END;
		else if (checkMacroExpand(tbl, line, start, macro_name, pos))
			return MACRO_EXPAND;

		else if (tbl->isMacroOpen == 1)
			return LINE_INSIDE;
		else
			return LINE_OUTSIDE;

		buffer = typeofline_buffer;
	}
	memset(buffer, '\0', LINE_LENGTH * sizeof(char));
	memset(start, '\0',   MAX_MACRO_NAME+1* sizeof(char));
	free(buffer);
	free(start);


}


int checkMacroStart(char *buffer, char *start, char *macro_name, int pos, symbol_table_t *sym_tbl, macro_table_t *tbl) {

	char *str = buffer;
	char macro_n[MAX_MACRO_NAME];
	int compLength = 0;
	memset(macro_n, '\0', sizeof(macro_n));

	if (strncmp(MACRO_START_WORD, start, MACRO_START_LEN) == 0) {

		str  = advance_buffer_if_possible(str,start);


		if (sscanf(str, "%s%n", macro_n, &pos) == 1) {/*check for actual macro name*/
			/*change the position*/
			switch (checkLegalName(macro_n, ALPHANUM_COMBINED)) {
				case 0:
					report_error(ERR_MACRO_NAME_WRONG, line_count, MAC, CRIT, 0);
					return 0;
				case 2:
					return 0 ;
				case 1:
					break;
			}

			if ((dupNameExistsInTable(tbl, macro_n) == 1) || (macro_name_duplicate(macro_n) == 1)){
				report_error(ERR_MACRO_NAME_DUP, line_count, MAC, CRIT, 0);    /*critical error*/
			return 0;
		}

			if (strlen(macro_n) >= MAX_MACRO_NAME) {
				report_error(ERR_MACRO_NAME_LONG, line_count, MAC, CRIT, 0);/*critical error*/
				return 0;
			}
			compLength = (strlen(str) > strlen(macro_n)  ) ? 1:0;

			/*macro named identified check if contiuation of buffer is empty*/
			if (compLength &&  isEmptyOrWhitespaceFromEnd(str) == 1) {
				report_error(ERR_START_MACRO_DEF, line_count, MAC, CRIT, 0);  /* critical wait for macro check*/
				return 0;
			}
			/*final filtering for isMacroStart, test for duplicates*/

			strncpy(macro_name, macro_n, strlen(macro_n));/*only place in use by typeofline*/
			strncpy(tbl->last_macro, macro_n , strlen(macro_n));

			return 1;/*line with name is correct*/
		}

	}
	str =buffer;
	return 0;
}

int checkMacroEnd(char *buffer, char *start, int pos) {
	char *str = buffer;
	if (strncmp(MACRO_END_WORD, start, MACRO_END_LEN) == 0) {
		/*ending macro has to have a line by itself*/
		str = advance_buffer_if_possible(str, start);
		if (isEmptyOrWhitespaceFromEnd(str) == 0) {
			report_error(ERR_MACRO_END, line_count, MAC, CRIT, 0);
			return 0;
		}
		return 1;
	}

	str = buffer;
	return 0;
}

/*check if  ready for expansion i.e macro_name is in table , and table not empty*/
int checkMacroExpand(macro_table_t *tbl, char *buffer, char *start, char *macro_name, int pos) {
	char *str = buffer;
	int len = strlen(start);

	if (tbl->isMacroOpen == 0 && tbl->size > 0 && (dupNameExistsInTable(tbl, start) == 1)) {
		strncpy(macro_name, start, len);
		strncpy(tbl->last_macro, macro_name, len);
		/*macro name is the single word allowed on macro expand*/
		if (isEmptyOrWhitespaceFromEnd(str) == 0)
			return 1;
		else {
			/*macro name is the single word allowed on macro expand*/
			str = buffer;
			report_error(ERR_MACRO_EXPAND, line_count, MAC, CRIT, 0);
			return 0;
		}

	}
	str = buffer;
	return 0;
}


/*todo add more error checking use ifeof*/
int checkEOFInBuffer(char *buffer) {
	char *localPtr = buffer;

	while (*localPtr != '\0') {
		if (*localPtr == EOF)
			return 1; /* EOF marker found*/
		localPtr++; /*Move to the next character*/
	}
	return 0; /*Eof Not Found*/
}

/*checks only opcode and directive*/
/*symbols will be checked in 2nd pass*/
int macro_name_duplicate(char *macro_name) {
	int j = 0, LEN = 0;
	char *opcode_names[16] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp",
	                          "bne", "red", "prn", "jsr", "rts", "stop"};
	char *directives[4] = {".data", ".string", ".entry", ".extern"};


	LEN = strlen(macro_name);
	for (j = 0; j < 16; ++j) {
		if (strcmp(macro_name, opcode_names[j]) == 0)
			return 1;
	}
	LEN = strlen(macro_name);
	for (j = 0; j < 4; ++j) {
		if (strcmp(macro_name, directives[j]) == 0)
			return 1;
	}


	return 0;
}

int isHeadOfSentenceValid(macro_table_t *tbl, char *buffer) {
	char *str = buffer;
	char start[MAX_MACRO_NAME];
	int len = strlen(str);
	if (sscanf(str, "%s", start) == 1) {
			strcat(start,"\0");
		if(strncmp(start, MACRO_START_WORD, MACRO_START_LEN) == 0 || strncmp(start, MACRO_END_WORD, MACRO_END_LEN) == 0 ){
			return 0;
		}

		if ((str[len - 1] != ':') && str[0]!='.' && (dupNameExistsInTable(tbl, start) == 0 && isRestOfLineEmpty(str))) {
			report_error(ERR_LINE_UNRECOGNIZED, line_count, MAC, CRIT, 0);
			return 1;
		}
	}
	return 0;
}


