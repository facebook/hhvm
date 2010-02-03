//
// $Id: lexmm.hh,v 1.12 2004/03/09 02:38:01 cholm Exp $
//
//  lexmm.hh
//  Copyright (C) 2002 Christian Holm Christensen <cholm@nbi.dk>
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public License
//  as published by the Free Software Foundation; either version 2.1
//  of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free
//  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
//  02111-1307 USA
//
#ifndef YLMM_lexmm
#define YLMM_lexmm
/**  @file   ylmm/lexmm.hh
     @author Christian Holm
     @date   Mon Sep 30 00:08:22 2002
     @brief  Declaration of parser class. */

/** @defgroup lex Lexical Scanner Classes.
    These classes are used by the lexical scanner classes, and wraps
    the @b Flex generated C function. */

// #if !defined(FLEX_SCANNER) && !defined(YYV7LEX)
// # error Only for inclussion in Flex input file
// #endif
#ifndef YLMM_SCANNER_CLASS
# error Scanner class not defined, please define YLMM_SCANNER_CLASS
#endif

#ifdef YYV7LEX
int yylook();
int yyback(int* p, int m);
# define YY_START long(yybgin)
# ifndef yyterminate
#  define yyterminate() return 0
# endif
# ifdef input
#  undef input
# endif
# ifdef unput
#  undef unput
# endif
# ifdef output
#  undef output
# endif
# define YLMM_YYTEXT extern char yytext[]
#else
# define YLMM_YYTEXT extern char* yytext
#endif

//____________________________________________________________________
/** Forward decl. */
YLMM_YYTEXT;
extern int yyleng;
int yylex();
static YLMM_SCANNER_CLASS * _scanner;

namespace ylmm
{
//____________________________________________________________________
/** Specialisations of some member functions of basic_scanner */
template <>
int
ylmm::basic_scanner<YLMM_SCANNER_CLASS::token_type,
		    YLMM_SCANNER_CLASS::location_type,
		    YLMM_SCANNER_CLASS::scanner_id,
		    YLMM_SCANNER_CLASS::lock_type>::start_condition() const
{
  return YY_START;
}

//____________________________________________________________________
/** Specialisations of some member functions of basic_scanner */
template <>
int
ylmm::basic_scanner<YLMM_SCANNER_CLASS::token_type,
		    YLMM_SCANNER_CLASS::location_type,
		    YLMM_SCANNER_CLASS::scanner_id,
		    YLMM_SCANNER_CLASS::lock_type>::scan()
{
  _scanner = static_cast<YLMM_SCANNER_CLASS*>(this);
  return _type = yylex();
}

//____________________________________________________________________
/** Specialisations of some member functions of basic_scanner */
template <>
const char*
ylmm::basic_scanner<YLMM_SCANNER_CLASS::token_type,
		    YLMM_SCANNER_CLASS::location_type,
		    YLMM_SCANNER_CLASS::scanner_id,
		    YLMM_SCANNER_CLASS::lock_type>::text() const
{
  return yytext;
}

//____________________________________________________________________
/** Specialisations of some member functions of basic_scanner */
template <>
int
ylmm::basic_scanner<YLMM_SCANNER_CLASS::token_type,
		    YLMM_SCANNER_CLASS::location_type,
		    YLMM_SCANNER_CLASS::scanner_id,
		    YLMM_SCANNER_CLASS::lock_type>::length() const
{
  return yyleng;
}

}
//____________________________________________________________________
/** Forward calls to class */
extern "C" {
  static int yywrap()
  {
    return _scanner->wrap();
  }
}

//____________________________________________________________________
/** Forwards calls to the scanner class. */
#ifdef YY_FATAL_ERROR
# undef YY_FATAL_ERROR
#endif
#define YY_FATAL_ERROR(msg) _scanner->fatal(msg)

//____________________________________________________________________
/** Forwards calls to the scanner class. */
#ifdef YY_INPUT
# undef YY_INPUT
#endif
#define YY_INPUT(buf,result,max) _scanner->read(buf,result,max)

//____________________________________________________________________
/** Forwards calls to scanner class. */
#ifdef ECHO
# undef ECHO
# define ECHO _scanner->echo(yytext, yyleng)
#endif

//____________________________________________________________________
/** Forward calls to the scanner class */
#ifdef YYFPRINF
# undef YYFPRINTF
# define YYFPRINTF _scanner->message
#endif
#ifdef LEXDEBUG
static void message_forward(FILE*, const char* f, ...)
{
  va_list ap;
  va_start(ap, f);
  static char buf[1024];
  vsprintf(buf, f, ap);
  _scanner->message(buf);
  va_end(ap);
}
static void message_forward(const char* s)
{
  _scanner->message(s);
}
# undef LEXDEBUG
# define MSG_FORWARD message_forward
# ifdef YYDPRINTF
#  undef YYDPRINTF
# endif
# define YYDPRINTF(x) MSG_FORWARD x
# ifdef YYDPRINTS
#  undef YYDPRINTS
# endif
# define YYDPRINTC(x) _scanner->echo(x)
# ifdef YYDPRINTS
#  undef YYDPRINTS
# endif
# define YYDPRINTS(x) message_forward(x)
#endif


#ifndef FLEX_SCANNER
//____________________________________________________________________
/** Forward calls to the scanner class */
#define input  _scanner->read

//____________________________________________________________________
/** Forward calls to the scanner class */
#define unput    _scanner->putback
//____________________________________________________________________
/** Forward calls to the scanner class */
#define output   _scanner->echo
#endif

#endif
//____________________________________________________________________
//
// EOF
//

