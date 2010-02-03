//
// $Id: yaccmm.hh,v 1.15 2004/03/09 02:38:01 cholm Exp $
//
//  yaccmm.hh
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
#ifndef YLMM_yaccmm
#define YLMM_yaccmm
/**  @file   ylmm/yaccmm.hh
     @author Christian Holm
     @date   Mon Sep 30 00:08:22 2002
     @brief  Declaration of parser class. */

/** @defgroup yacc Semantic Parser Classes.
    These classes are used by the semantic parser classes, and wraps
    the @b Bison generated C function. */
// #if !defined(YYBISON) && !defined(YYBYACC) && !defined(YYV7YACC)
// # error Only for inclussion in Yacc input file
// #endif
#ifndef YLMM_PARSER_CLASS
# error Parser class not defined, please define YLMM_PARSER_CLASS
#endif
#ifndef YLMM_basic_parser
# include <util/ylmm/basic_parser.hh>
#endif

//____________________________________________________________________
/** Define the token type to be the argument of the basic_parser
    template */
#ifndef YYSTYPE
# define YYSTYPE YLMM_PARSER_CLASS::token_type
#endif

//____________________________________________________________________
/** (Re)define the location type */
#ifdef YYLTYPE
# undef YYLTYPE
#endif
#define YYLTYPE YLMM_PARSER_CLASS::location_type

//____________________________________________________________________
#if !defined(YYDEBUG)
static int yydebug;
#endif

//____________________________________________________________________
/** Static pointer to the parser. */
static YLMM_PARSER_CLASS *_parser;

//____________________________________________________________________
/** Forward decl. */
extern int yydebug;

//____________________________________________________________________
/** Overload yyparse to always have a argument-less and and argumented
    version, so that we can always call it with arguments, no matter
    what. */
#ifdef YYPARSE_PARAM
int yyparse(void* YYPARSE_PARAM);
static int yyparse() { return yyparse(0); }
#else
int yyparse();
static int yyparse(void*) { return yyparse(); }
#endif

namespace ylmm
{
//____________________________________________________________________
/** Specialisation of some basic_parser methods for this grammar. */
template <>
bool
ylmm::basic_parser<YLMM_PARSER_CLASS::token_type,
		   YLMM_PARSER_CLASS::location_type,
		   YLMM_PARSER_CLASS::parser_id,
		   YLMM_PARSER_CLASS::lock_type>::tracing() const
{
#ifdef YYDEBUG
  return yydebug != 0 ? true : false;
#else
  return false;
#endif
}

//____________________________________________________________________
/** Specialisation of some basic_parser methods for this grammar. */
template <>
bool
ylmm::basic_parser<YLMM_PARSER_CLASS::token_type,
		   YLMM_PARSER_CLASS::location_type,
		   YLMM_PARSER_CLASS::parser_id,
		   YLMM_PARSER_CLASS::lock_type>::tracing(bool t)
{
#ifdef YYDEBUG
  yydebug = (t ? 1 : 0);
#endif
  return t;
}

//____________________________________________________________________
/** Specialisation of some basic_parser methods for this grammar. */
template <>
int
ylmm::basic_parser<YLMM_PARSER_CLASS::token_type,
		   YLMM_PARSER_CLASS::location_type,
		   YLMM_PARSER_CLASS::parser_id,
		   YLMM_PARSER_CLASS::lock_type>::parse(void* arg)
{
  _parser = static_cast<YLMM_PARSER_CLASS*>(this);
  return yyparse(arg);
}

}
//____________________________________________________________________
/** (Re)define YYPRINT to call parser member function print.
    @param f FILE argument, which is ignored.
    @param l The lookahead token.
    @param t The token */
#ifdef YYPRINT
# undef YYPRINT
#endif
#define YYPRINT(f,l,t)    _parser->trace(l,t)

//____________________________________________________________________
/** (Re)define YYPPRINTF to call parser member function verbose.
    @param f FILE argument (ignored)
    @param ... Variadic arguments sent directly to verbose. */
#ifdef YYFPRINTF
# undef YYFPRINTF
#endif
#if defined(YLMM_CXXCPP_STD_VARIADIC) || (defined(__GNUC__) && __GNUC__ >= 3)
# define YYFPRINTF(f,...)  _parser->message(__VA_ARGS__)
#elif defined(YLMM_CXXCPP_GNU_VARIADIC) || (defined(__GNUC__) && __GNUC__ < 3)
# define YYFPRINTF(f,a...)  _parser->message(a)
#else
# ifndef __CSTDARG__
#  include <cstdarg>
# endif
# ifndef __CSTDIO__
#  include <cstdio>
# endif
namespace
{
  void bump_fprint(FILE*, const char* f, ...)
  {
    va_list ap;
    va_start(ap, f);
    static char buf[1024];
# ifdef HAVE_VSNPRINTF
    vsnprintf(buf, 1024, f, ap);
# else
    vsprintf(buf, f, ap);
# endif
    _parser->message(buf);
  }
}
# define YYFPRINTF bump_fprintf
#endif

//____________________________________________________________________
/** (Re)define YYFFPRINTF to call parser member function trace.
    @param f FILE argument (ignored)
    @param l Lock-ahead character
    @param t the token.
    @param ... Variadic arguments sent directly to trace. */
#ifdef YYFFPRINTF
# undef YYFFPRINTF
#endif
#if defined(YLMM_CXXCPP_STD_VARIADIC) || (defined(__GNUC__) && __GNUC__ >= 3)
# define YYFFPRINTF(f,...) _parser->message(__VA_ARGS__)
#elif defined(YLMM_CXXCPP_GNU_VARIADIC) || (defined(__GNUC__) && __GNUC__ < 3)
# define YYFFPRINTF(f,a...) _parser->message(a)
#else
# define YYFFPRINTF(f,l,t) _parser->trace(l,t)
#endif

//____________________________________________________________________
/** (Re)define yyerror to call parser member function fatal. */
#ifdef yyerror
# undef yyerror
#endif
#define yyerror           _parser->fatal

//____________________________________________________________________
/** (Re)define the default location action.
    The default action for location information.
    This uses the member function `default' from the location class.
    A subclass of the location class may override this if needed.
    @param current The location to assign to.
    @param rhs The location array
    @param n Index into the location array. */
#ifdef YYLLOC_DEFAULT
# undef YYLLOC_DEFAULT
#endif
#define YYLLOC_DEFAULT(current, rhs, n)  \
  current.first((rhs)[1]);               \
  current.last((rhs)[n]);                \

//____________________________________________________________________
/** Scanner interface function (without locations).
    This function is overloaded for with and without location
    information, so that if the parser uses the location information,
    we don't need to redefine anything.  Quite nifty IMHO.
    @param token  Pointer to the token to set.
    @param param  Pointer to user defined argument.
    @return The new token value. */
template <typename Semantic>
static int yylex(Semantic* token, void* param=0)
{
  _parser->token_addr(token);
  int ret  = _parser->scan(param);
  return ret;
}

//____________________________________________________________________
/** Scanner interface function (with locations).
    This function is overloaded for with and without location
    information, so that if the parser uses the location information,
    we don't need to redefine anything.  Quite nifty IMHO.
    @param token  Pointer to the token to set.
    @param loc    Pointer to the location to set.
    @param param  Pointer to user defined argument.
    @return The new token value. */
template <typename Semantic, typename Location>
static int yylex(Semantic* token, Location* loc, void* param=0)
{
  _parser->where_addr(loc);
  int ret = yylex<Semantic>(token, param);
  return ret;
}

//____________________________________________________________________
/** If we're not defining a pure parser, we better set it up, so that
    we can have a non-templated version of yylex, that automatically
    sends the arguments on. Note that we can not define these
    unconditionally, as it would mean that we would have to define
    yylval somewhere, and we can't just do it for one, as we need to
    distinigish the cases. */
#ifdef YLMM_LEX_STATIC
extern YYSTYPE yylval;
static int yylex() { return yylex<YYSTYPE>(&yylval, 0); }
#elif defined(YLMM_LEX_STATIC_LOCATION)
extern YYSTYPE yylval; \
extern YYLTYPE yylloc; \
static int yylex() { return yylex<YYSTYPE,YYLTYPE>(&yylval,&yylloc,0);}
#endif


#endif // YLMM_yaccmm
//
// EOF
//


