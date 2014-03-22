/* File: parser.h
* --------------
* This file provides constants and type definitions that will
* are used and/or exported by the yacc-generated parser.
*/

#ifndef PARSER_H
#define PARSER_H

// here we need to include things needed for the yylval union
// (types, classes, constants, etc.)

#include "scanner.h"            

// Next, we want to get the exported defines for the token codes and
// typedef for YYSTYPE and exported global variable yylval.  These
// definitions are generated and written to the y.tab.h header file. But
// because that header does not have any protection against being
// re-included and those definitions are also present in the y.tab.c,
// we can get into trouble if we don't take precaution to not include if
// we are compiling y.tab.c, which we use the YYBISON symbol for. 
// Managing C headers can be such a mess! 


//
#include <J_Symbol_Fwd_Decl.h>
//
#include <J_UI/J_UI_String.h>
#ifndef YYBISON                
#include "y.tab.h"              
#endif

#define YY_NO_UNISTD_H
#ifdef _DEBUG


#define YYDEBUG 1
#endif

namespace jomike {
extern j_symbol_component* g_input_line;
}

int yyparse();              // Defined in the generated y.tab.c file
void InitParser();          // Defined in parser.y

#endif //PARSER_H
