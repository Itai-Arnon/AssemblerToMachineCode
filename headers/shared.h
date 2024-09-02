

#ifndef M14_SHARED_H
#define M14_SHARED_H
#include <stdlib.h>

#define MAX_15Bit_NUM 16382
#define MIN_15Bit_NUM -16383
#define MAX_12Bit_NUM 2047
#define MIN_12Bit_NUM -2048

#define LINE_LENGTH 80
#define SET_BUFFER_LENGTH 85

#define MAX_MACRO_NAME 31
#define MAX_FILE_NAME 64
#define MAX_DIRECTIVE_NAME 8

/*macros*/
#define MAX_SYMBOL_NAME 31
#define MACRO_START_WORD "macr"
#define MACRO_END_WORD "endmacr"
#define MACRO_END_LEN 7
#define MACRO_START_LEN 4

/*second passs*/
#define MAX_WORDS 4096

/*text add in */
/*#define PATH_BASE "c:\\OPU\\20465\\m\\m14\\"*/          /*TODO change when moving to linux*/
#define PATH_BASE "..\\"                                           /*ALternate PATH_BASE */
#define EXT_BEFORE ".txt" /*TODO change to appropriate extension once in linux*/
#define EXT_AFTER "asse.txt"
#define START_PRE ".as"
#define END_PRE   ".am"




/*writing Macro Error and File Errors*/


/*file problems*/
#define ERR_LINE_LENGTH "Line is too long"
#define ERR_ILLEGAL_CHAR "Line has an Illegal char"
#define ERR_FILE_BEFORE ".as File Failed To Open"
#define ERR_FILE_AFTER ".am File Failed To Create"
#define ERR_FILE_OBJ_EXT_ENT "Error in creating OB EXT or ENT file"
#define ERR_NO_FILES "No Files Inserted"
#define ERR_FILE_DESTINATION "Error with Destination file or files"
#define ERR_FILE_SOURCE "Error with Source file or Destination File"
#define ERR_FILE_GENERAL "Error with Source file or Destination File. If in existence we will now move to the next file"

/*macro related*/
#define ERR_WRITING_MACRO  "Failed to write Macro"
#define ERR_MACRO_NAME "Macro Name Missing, Failed to Expand Macro"
#define ERR_MACRO_NAME_RETRIEVE "Error Retrieving From Macro Table"
#define ERR_MACRO_TABLE_GENERAL_ERROR "General Macro Table Error"
#define ERR_MACRO_PERMISSION "Error  Macro Permission Mistake"
#define ERR_MACRO_NODE_CREATION_FAILED "Macro Node Allocation Failed"
#define ERR_MACRO_NAME_LONG "Macro Name Too Long"
#define ERR_MACRO_NAME_WRONG "Potential Macro Name Illegal"
#define ERR_START_MACRO_DEF "Error In Macro Start Definition"
#define ERR_MACRO_END "Error In Macro End Definition"
#define ERR_MACRO_EXPAND "Error In Macro Expand Definition"
#define ERR_MACRO_NAME_DUP "Error Duplicate Macro Name or Macro name is an opcode, a Directive"
#define ERR_MACRO_NOT_FOUND "Cannot find Macro"
#define ERR_MACRO_NAME_OP_DIRECT_SYMBOL "Macro name is a opcode, a Directive or a Symbol"
#define ERR_GENERAL_ERROR  "General Macro Failure, something in the process failed"

/* symbol table related , symbol.c */
#define ERR_DUPLICATE_SYMBOL_NAME  "DUPLICATE SYMBOL NAME"
#define ERR_FAIL_CREATE_SYMBOL_TBL "Symbol Table Creation Failed"
#define ERR_FAIL_CREATE_SYMBOL "Symbol  Creating Symbol Node Failed"
#define ERR_SYMBOL_NOT_FOUND "Symbol Not Found" /*non critial*/



/* parser: op code  directive related errors*/
#define ERR_OP_CODE_RECOGNITION "OP CODE NOT RECOGNIZED"
#define ERR_OP_CODE_FAILED_STRUCTURE "OP Code structure Unrecognized or Not Legal"
#define ERR_OP_CODE_REGISTRY_ILLEGAL "One or both OP Code register isn't legal"
#define ERR_REGISTRY_ILLEGAL "Failed to Recognize Registry, might be a wrong value "

#define ERR_LINE_UNRECOGNIZED "Line Not Recognized as OP_CODE or DIRECTIVE or Symbol Or Macro"
/* directive related errors*/
#define ERR_DIRECTIVE_RECOGNITION "Directive Not Recognized"
#define ERR_FAILED_TO_CONVERT_NUMBER "Failed to convert Number"
#define ERR_DATA_DIRECTIVE_NUMBER "Data Directive Number or Numbers Invalid"
#define ERR_STRING_QUOTATION_MISSING "String Quotations missing or characters invalid"
#define ERR_STRING_INVALID  "String Line not recognized"
#define ERR_EXTERN_ENTRY_ILLEGAL "Illegal Extern or Entry Line"


/*word_table*/
/*pass one and two*/
#define ERR_WORD_TABLE_ALLOCATION "Word Table Creation or Allocation Failed"
#define ERR_GENERAL_FIRST_PASS_ERROR "General First Pass Error"
#define ERR_TOO_MANY_WORDS "Table size has surpassed 4096 words"
#define WAR_MEMORY_NOT_CONFIGURED "Warning Memory Not Configured"
#define ERR_MEMORY_NOT_CONFIGURED "Error Symbol Memory Not Configure "
#define WAR_EXTERN_ENTRY_SYMBOL "Warning: Symbol is Extern or Entry"
#define ERR_EXTERN_SYMBOL "Symbol for Extern Does Not Exist or other Errors"
#define ERR_ENTRY_SYMBOL_PROB "Symbol for Entry Missing or other Errors"
#define ERR_EXTERN_SYMBOL_DUP "Extern Symbol With The Same Name Of A Regular Symbol"


/*report error uses this to identify in whihc file an error occured*/
typedef enum{ AS ,MAC , MACL , SYM , PARS ,FIRST ,SECOND ,UTIL }file_t;

/*type for 4 diretives - used mainly in parser*/
typedef enum {
	DATA,
	STRING,
	ENTRY,
	EXTERN,
}directive_cmd_t;

/*16 opcodes meant to identify lines in the files*/
typedef enum {
	mov, cmp, add, sub, lea, clr, not, inc, dec, jmp,
	bne, red, prn, jsr, rts, stop
} op_code_t;

/*macros enums used in preprocessor , for switch purposes*/
typedef enum{MACRO_START, MACRO_END , MACRO_EXPAND , LINE_INSIDE , LINE_OUTSIDE, EMPTY_LINE , MACRO_ERROR }MACRO_STATE_T;
/*utils - enums for macros func check legal name, ALPHA allows only ALPHA chars,
 * ALPHANUM both type & ALPHANUM_COMB begins w/ alpha char and continues with a combinations*/
typedef enum{ALPHA,ALPHANUM, ALPHANUM_COMBINED}check_legal_name;

/*specifies what kind of line->word each line would be*/
typedef enum {E, R,	A } ARE_T;

/*update the addresss of  a symbol or not*/
typedef  enum{NO, YES }update_address_t;
/*convertORcheckStringToNum from utils, check if string is changeable to int
 * Convert: converts  from string to int*/
typedef enum {CONVERT  , CHECK} convert_func_t;


/*symbols MIDDLE Direct type symbols Head-ones that are upfront */
typedef enum{  MIDDLE, HEAD}symbol_loci_t;

/*line->word loci of regiters in the word*/
typedef enum{DESTINATION , SOURCE}operand_t;

/*Report ERROR  CRIT -terminates and free table NON_CRIT is only for warnings*/
typedef enum{CRIT,NON_CRIT}err_type_t;

/*type of registries use to indetify opcode operands*/
typedef enum { _IMMEDIATE = 0 , _DIRECT  ,_INDIRECT  , _REGULAR  , _ERROR   , _TBD /*5*/ }type_of_register_t;

/*the four type of symbol types*/
typedef enum{ _INSTRUCTION , _DATA, _ENTRY , _EXTERN }memory_t;

#endif
