#include <ctype.h>
#include <string.h>
#include "headers/utils.h"
#include "headers/global_vars.h"
#include "headers/assembler.h"

int isRestOfLineEmpty(char *line) {
	char *s_check = line;
	int idx = 0;
	int LEN = nonNullTerminatedLength(s_check);
	while (isspace(*s_check++) && *s_check);
	if (*s_check) {
		return 0;
	}
	else
	s_check++;
	*s_check = '\0';
	s_check =line; /*resets the string */
	return 1;

}


int findSeperator(char *buffer, char sep) {
	char *str = buffer;
	int LEN = strlen(str);
	int i, j;
	i = j = 0;
	for (i = 0; str[i] < LEN; i++) {
		if (str[i] == '\n' && i < (LEN - 1)) {
			while (isspace(*str++));
			if (!*str)
				return 1;
		}
	}
	return 0;
}

int nonNullTerminatedLength(char *arr) {
	int count = 0;
	while (arr[++count] != 0);
	return count;
}


int is_char_separator(char c, char separators[], int separators_amount) {
	int i;
	for (i = 0; i < separators_amount; i++) {
		if (c == separators[i])
			return 1;
	}

	return 0;
}

int extra_char_at_end(char line[], int loc) {
	while (line[loc] == ' ' || line[loc] == '\t')
		loc++;

	if (line[loc] == '\0')
		return 0;

	return 1;
}


char *removeColon(char *symbol_name) {
	int length = strlen(symbol_name);
	char *symbol_name_c = malloc(sizeof(char));
	if (symbol_name[length] == ':') {
		strncat(symbol_name_c, symbol_name, length - 1);
		return symbol_name_c;
	}
	return symbol_name;
}


/*removes white spaces from noth sides*/




int checkLegalName(char *str, check_legal_name type) {
	char *s_str = str;
	int i = 0;
	int len = nonNullTerminatedLength(s_str);
	/*before we chech the macro name. we hace to consider  directives and symbols with colon at their end*/
	/*they will be tested later for validity*/
	if (s_str[0] == '.' || s_str[len - 1] == ':') {
		return 2;
	}
	if (!isalpha(s_str[0])) return 0;
	switch (type) {
		case ALPHA:
			while (i < (len - 1) && isalpha(s_str[i])) ++i;
			break;
		case ALPHANUM:
			while (i < (len - 1) && isalpha(s_str[i])) ++i;
			/*check condition where after the alphabet there are digits which is acceptable*/
			while (i < (len - 1) && isdigit(s_str[i])) ++i;
			break;


		case ALPHANUM_COMBINED:
			/*check condition where after the alphabet there are digits which is acceptable*/
			while (i < (len - 1) && isalnum(s_str[i])) ++i;
			break;

		default:
			break;
	}
	if (isalnum(s_str[i])) {
		*(s_str + i + 1)  = '\0';
		s_str = str;
		return i == (len - 1) ? 1 : 0;
	}
	else
		return 0;
}

/*meant only for frontal removal of whitspace*/
char *removeFrontalWhitespace(char *buffer) {
	while (isspace(*buffer++) && *buffer);
	return buffer;
}

char *strstrip(char *s) {
	int LEN;
	char *end;
	LEN = strlen(s);
	if (!LEN)
		return s;
	end = s + LEN - 1;
	while (end >= s && isspace(*end))
		end--;
	*(end + 1) ='\0';

	while (*s && isspace(*s))
		s++;

	return s;
}

int countCommas(char *str) {
	int count = 0;
	int i;
	for (i = 0; str[i] != '\0'; i++) {
		if (str[i] == ',') {
			count++;
		}
	}
	return count;
}


int convertOrCheckStringToNum(char *str, convert_func_t type) {
	char *endtoken;
	unsigned long num = strtol(str, &endtoken, 10);
	switch (type) {
		case 0:
			if (num == 0)
				return 0;

			if (*endtoken != '\0')   {
				report_error(ERR_FAILED_TO_CONVERT_NUMBER, line_count, UTIL, CRIT, 0);
			} else
				return (int) num;
			break;
		case 1:
			if (*endtoken != '\0') {
				return 0;
			} else
				return 1;
	}
}


/*count the numbers in a string to check valditiy of .DATA directive */
int countNumbersInString(char *str) {
	int count = 0;
	int i = 0;
	int inNumber = 0;

	while (str[i] != '\0') {
		/* Check if the current character is a digit or a minus sign */
		if (isdigit(str[i]) || (str[i] == '-' && isdigit(str[i + 1]))) {
			if (!inNumber) {
				count++;
				inNumber = 1;
			}
		} else {
			inNumber = 0;
		}
		i++;
	}
	return count;
}

/*under the premise that in the string there is no whitespace before or after*/
int checkQuotes(char *str) { /*todo erase */
	/*check if string is not empty*/
	int len = strlen(str);
	if (len < 2) {
		return 0;
	} else len = len - 2;

	return 0;
}

int isEmptyOrWhitespaceFromEnd(char *str) {
	if (str == NULL) {
		return 1; /* Consider NULL as empty or whitespace */
	}

	char *end = str + strlen(str) - 1;

	while (end >= str) {
		if (!isspace(*end)) {
			return 0; /* Found a non-whitespace character */
		}
		end--;
		*(end+1)='\n';

	}

	return 1; /* Only whitespace characters found */
}


char *exchangeExtensionNoMalloc(char *filename, const char *old_ext, const char *new_ext) {
	char *pos = NULL;
	size_t filename_len = strlen(filename);
	size_t old_ext_len = strlen(old_ext);


	/* Find the last occurrence of old_ext in filename **/
	if (filename_len >= old_ext_len) {
		pos = strstr(filename + filename_len - old_ext_len, old_ext);
	}

	/** If old_ext is found at the end of the filename, replace it with new_ext **/
	if (pos != NULL && strcmp(pos, old_ext) == 0) {
		strcpy(pos, new_ext);
	} else {
		/** If old_ext is not found, append new_ext to the filename **/
		strcat(filename, new_ext);
	}

	return filename;
}

char *exchangeExtension(const char *filename, const char *old_ext, const char *new_ext) {
	char *pos = NULL;
	size_t filename_len = strlen(filename);
	size_t old_ext_len = strlen(old_ext);
	size_t new_ext_len = strlen(new_ext);
	size_t new_filename_len = filename_len + new_ext_len - old_ext_len + 1; /* +1 for null terminator */

	/* Allocate memory for the new filename */
	char *new_filename = (char *)malloc(new_filename_len);
	if (new_filename == NULL) {
		report_error(ERR_FILE_DESTINATION,line_count,SECOND,CRIT,0);
		return NULL;
	}
	/* Copy the original filename to the new filename */
	strcpy(new_filename, filename);

	/* Find the last occurrence of old_ext in filename */
	if (filename_len >= old_ext_len) {
		pos = strstr(new_filename + filename_len - old_ext_len, old_ext);
	}

	/* If old_ext is found at the end of the filename, replace it with new_ext */
	if (pos != NULL && strcmp(pos, old_ext) == 0) {
		strcpy(pos, new_ext);
	} else {
		/* If old_ext is not found, append new_ext to the filename */
		strcat(new_filename, new_ext);
	}

	return new_filename;
}


int check_string_bounds(const char *str, size_t length) {
	size_t i ;
	for ( i = 0; i < length; i++) {
		if (!isprint(str[i])) {
			fprintf(stderr, "Out of bounds character found at position %zu: %c\n", i, str[i]);
			return 1;
		}
	}
	return 0;
}