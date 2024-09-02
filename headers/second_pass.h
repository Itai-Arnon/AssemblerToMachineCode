

#ifndef M14_SECOND_PASS_H
#define M14_SECOND_PASS_H


#include "shared.h"
#include "symbols.h"
#include "first_pass.h"
#include "macro_list.h"

/*main function| checks for symbol problems and other address*/
void second_pass(symbol_table_t *, symbol_table_t *,word_table_t *, word_table_t *, char * , int);



/*check if there are duplicate symbols in macro table*/
void checkSymbolsUnique(macro_table_t *, symbol_table_t *sym_table);

int convertToOctal(int num);

/*returns 1 for the first symbol w/o correct value*/
symbol_t *firstSymbolMissingValue(symbol_table_t *table);
/*2nd pass check of opcode*/
/*entry has a specific table , all entry are move to it*/
int moveSymbolsToEntry(symbol_table_t *sym_tbl, symbol_table_t *entrySTable);

/*Check if symbols of type ARE_T E have the same name as symbols of type ARE_T A or R */
int checkExternCollisions(symbol_table_t *exTable , symbol_table_t *otherTable);

/*adds existing symbol to another table */
int addSymbolToTable(symbol_table_t *table, symbol_t *_symbol);

/*all words that are relocatable addresses are updated*/
void updateRelocatableLines(word_table_t *table);

/*adds value to memory adresses by memory type| the boudary is when the data table started*/
void addConstantToSymbols(symbol_table_t *, word_table_t *, memory_t , int );

/*print to file ps.ent or stdout  entry information - use a specific symbol table */
void checkSymbolsUnique(macro_table_t *macro_table, symbol_table_t *sym_table);

/*count the number of Extern element in a table */
int countExternInTables(word_table_t *table);


/*printing table in octal */
void printTableToFile(word_table_t *, word_table_t *, char * );

/*prints the Externals and their IC and DC locations*/
void printExternTable(word_table_t *table1, word_table_t *table2, char *filename);

/*print to file ps.ext or stdout  entry information - use a specific symbol table */
void printEntryTable(symbol_table_t *table, char *filename);

void print_symbol_table(symbol_table_t *sym_tbl);

/*print the word in binary form*/
void printBinary(unsigned short num);

/*basic print of word_table */
void printTable(word_table_t *table);

#endif /*M14_SECOND_PASS_H*/
