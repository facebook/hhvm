/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse xhpparse
#define yylex   xhplex
#define yyerror xhperror
#define yylval  xhplval
#define yychar  xhpchar
#define yydebug xhpdebug
#define yynerrs xhpnerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_REQUIRE_ONCE = 258,
     T_REQUIRE = 259,
     T_EVAL = 260,
     T_INCLUDE_ONCE = 261,
     T_INCLUDE = 262,
     T_LOGICAL_OR = 263,
     T_LOGICAL_XOR = 264,
     T_LOGICAL_AND = 265,
     T_PRINT = 266,
     T_SR_EQUAL = 267,
     T_SL_EQUAL = 268,
     T_XOR_EQUAL = 269,
     T_OR_EQUAL = 270,
     T_AND_EQUAL = 271,
     T_MOD_EQUAL = 272,
     T_CONCAT_EQUAL = 273,
     T_DIV_EQUAL = 274,
     T_MUL_EQUAL = 275,
     T_MINUS_EQUAL = 276,
     T_PLUS_EQUAL = 277,
     T_BOOLEAN_OR = 278,
     T_BOOLEAN_AND = 279,
     T_IS_NOT_IDENTICAL = 280,
     T_IS_IDENTICAL = 281,
     T_IS_NOT_EQUAL = 282,
     T_IS_EQUAL = 283,
     T_IS_GREATER_OR_EQUAL = 284,
     T_IS_SMALLER_OR_EQUAL = 285,
     T_SR = 286,
     T_SL = 287,
     T_INSTANCEOF = 288,
     T_UNSET_CAST = 289,
     T_BOOL_CAST = 290,
     T_OBJECT_CAST = 291,
     T_ARRAY_CAST = 292,
     T_BINARY_CAST = 293,
     T_UNICODE_CAST = 294,
     T_STRING_CAST = 295,
     T_DOUBLE_CAST = 296,
     T_INT_CAST = 297,
     T_DEC = 298,
     T_INC = 299,
     T_CLONE = 300,
     T_NEW = 301,
     T_EXIT = 302,
     T_IF = 303,
     T_ELSEIF = 304,
     T_ELSE = 305,
     T_ENDIF = 306,
     T_LNUMBER = 307,
     T_DNUMBER = 308,
     T_STRING = 309,
     T_STRING_VARNAME = 310,
     T_VARIABLE = 311,
     T_NUM_STRING = 312,
     T_INLINE_HTML = 313,
     T_CHARACTER = 314,
     T_BAD_CHARACTER = 315,
     T_ENCAPSED_AND_WHITESPACE = 316,
     T_CONSTANT_ENCAPSED_STRING = 317,
     T_BACKTICKS_EXPR = 318,
     T_ECHO = 319,
     T_DO = 320,
     T_WHILE = 321,
     T_ENDWHILE = 322,
     T_FOR = 323,
     T_ENDFOR = 324,
     T_FOREACH = 325,
     T_ENDFOREACH = 326,
     T_DECLARE = 327,
     T_ENDDECLARE = 328,
     T_AS = 329,
     T_SWITCH = 330,
     T_ENDSWITCH = 331,
     T_CASE = 332,
     T_DEFAULT = 333,
     T_BREAK = 334,
     T_CONTINUE = 335,
     T_GOTO = 336,
     T_FUNCTION = 337,
     T_CONST = 338,
     T_RETURN = 339,
     T_TRY = 340,
     T_CATCH = 341,
     T_THROW = 342,
     T_USE = 343,
     T_GLOBAL = 344,
     T_PUBLIC = 345,
     T_PROTECTED = 346,
     T_PRIVATE = 347,
     T_FINAL = 348,
     T_ABSTRACT = 349,
     T_STATIC = 350,
     T_VAR = 351,
     T_UNSET = 352,
     T_ISSET = 353,
     T_EMPTY = 354,
     T_HALT_COMPILER = 355,
     T_CLASS = 356,
     T_INTERFACE = 357,
     T_EXTENDS = 358,
     T_IMPLEMENTS = 359,
     T_OBJECT_OPERATOR = 360,
     T_DOUBLE_ARROW = 361,
     T_LIST = 362,
     T_ARRAY = 363,
     T_CLASS_C = 364,
     T_METHOD_C = 365,
     T_FUNC_C = 366,
     T_LINE = 367,
     T_FILE = 368,
     T_COMMENT = 369,
     T_DOC_COMMENT = 370,
     T_OPEN_TAG = 371,
     T_OPEN_TAG_WITH_ECHO = 372,
     T_OPEN_TAG_FAKE = 373,
     T_CLOSE_TAG = 374,
     T_WHITESPACE = 375,
     T_START_HEREDOC = 376,
     T_END_HEREDOC = 377,
     T_HEREDOC = 378,
     T_DOLLAR_OPEN_CURLY_BRACES = 379,
     T_CURLY_OPEN = 380,
     T_PAAMAYIM_NEKUDOTAYIM = 381,
     T_BINARY_DOUBLE = 382,
     T_BINARY_HEREDOC = 383,
     T_NAMESPACE = 384,
     T_NS_C = 385,
     T_DIR = 386,
     T_NS_SEPARATOR = 387,
     T_XHP_WHITESPACE = 388,
     T_XHP_TEXT = 389,
     T_XHP_LT_DIV = 390,
     T_XHP_LT_DIV_GT = 391,
     T_XHP_ATTRIBUTE = 392,
     T_XHP_CATEGORY = 393,
     T_XHP_CHILDREN = 394,
     T_XHP_ANY = 395,
     T_XHP_EMPTY = 396,
     T_XHP_PCDATA = 397,
     T_XHP_COLON = 398,
     T_XHP_HYPHEN = 399,
     T_XHP_BOOLEAN = 400,
     T_XHP_NUMBER = 401,
     T_XHP_ARRAY = 402,
     T_XHP_STRING = 403,
     T_XHP_ENUM = 404,
     T_XHP_FLOAT = 405,
     T_XHP_REQUIRED = 406
   };
#endif
/* Tokens.  */
#define T_REQUIRE_ONCE 258
#define T_REQUIRE 259
#define T_EVAL 260
#define T_INCLUDE_ONCE 261
#define T_INCLUDE 262
#define T_LOGICAL_OR 263
#define T_LOGICAL_XOR 264
#define T_LOGICAL_AND 265
#define T_PRINT 266
#define T_SR_EQUAL 267
#define T_SL_EQUAL 268
#define T_XOR_EQUAL 269
#define T_OR_EQUAL 270
#define T_AND_EQUAL 271
#define T_MOD_EQUAL 272
#define T_CONCAT_EQUAL 273
#define T_DIV_EQUAL 274
#define T_MUL_EQUAL 275
#define T_MINUS_EQUAL 276
#define T_PLUS_EQUAL 277
#define T_BOOLEAN_OR 278
#define T_BOOLEAN_AND 279
#define T_IS_NOT_IDENTICAL 280
#define T_IS_IDENTICAL 281
#define T_IS_NOT_EQUAL 282
#define T_IS_EQUAL 283
#define T_IS_GREATER_OR_EQUAL 284
#define T_IS_SMALLER_OR_EQUAL 285
#define T_SR 286
#define T_SL 287
#define T_INSTANCEOF 288
#define T_UNSET_CAST 289
#define T_BOOL_CAST 290
#define T_OBJECT_CAST 291
#define T_ARRAY_CAST 292
#define T_BINARY_CAST 293
#define T_UNICODE_CAST 294
#define T_STRING_CAST 295
#define T_DOUBLE_CAST 296
#define T_INT_CAST 297
#define T_DEC 298
#define T_INC 299
#define T_CLONE 300
#define T_NEW 301
#define T_EXIT 302
#define T_IF 303
#define T_ELSEIF 304
#define T_ELSE 305
#define T_ENDIF 306
#define T_LNUMBER 307
#define T_DNUMBER 308
#define T_STRING 309
#define T_STRING_VARNAME 310
#define T_VARIABLE 311
#define T_NUM_STRING 312
#define T_INLINE_HTML 313
#define T_CHARACTER 314
#define T_BAD_CHARACTER 315
#define T_ENCAPSED_AND_WHITESPACE 316
#define T_CONSTANT_ENCAPSED_STRING 317
#define T_BACKTICKS_EXPR 318
#define T_ECHO 319
#define T_DO 320
#define T_WHILE 321
#define T_ENDWHILE 322
#define T_FOR 323
#define T_ENDFOR 324
#define T_FOREACH 325
#define T_ENDFOREACH 326
#define T_DECLARE 327
#define T_ENDDECLARE 328
#define T_AS 329
#define T_SWITCH 330
#define T_ENDSWITCH 331
#define T_CASE 332
#define T_DEFAULT 333
#define T_BREAK 334
#define T_CONTINUE 335
#define T_GOTO 336
#define T_FUNCTION 337
#define T_CONST 338
#define T_RETURN 339
#define T_TRY 340
#define T_CATCH 341
#define T_THROW 342
#define T_USE 343
#define T_GLOBAL 344
#define T_PUBLIC 345
#define T_PROTECTED 346
#define T_PRIVATE 347
#define T_FINAL 348
#define T_ABSTRACT 349
#define T_STATIC 350
#define T_VAR 351
#define T_UNSET 352
#define T_ISSET 353
#define T_EMPTY 354
#define T_HALT_COMPILER 355
#define T_CLASS 356
#define T_INTERFACE 357
#define T_EXTENDS 358
#define T_IMPLEMENTS 359
#define T_OBJECT_OPERATOR 360
#define T_DOUBLE_ARROW 361
#define T_LIST 362
#define T_ARRAY 363
#define T_CLASS_C 364
#define T_METHOD_C 365
#define T_FUNC_C 366
#define T_LINE 367
#define T_FILE 368
#define T_COMMENT 369
#define T_DOC_COMMENT 370
#define T_OPEN_TAG 371
#define T_OPEN_TAG_WITH_ECHO 372
#define T_OPEN_TAG_FAKE 373
#define T_CLOSE_TAG 374
#define T_WHITESPACE 375
#define T_START_HEREDOC 376
#define T_END_HEREDOC 377
#define T_HEREDOC 378
#define T_DOLLAR_OPEN_CURLY_BRACES 379
#define T_CURLY_OPEN 380
#define T_PAAMAYIM_NEKUDOTAYIM 381
#define T_BINARY_DOUBLE 382
#define T_BINARY_HEREDOC 383
#define T_NAMESPACE 384
#define T_NS_C 385
#define T_DIR 386
#define T_NS_SEPARATOR 387
#define T_XHP_WHITESPACE 388
#define T_XHP_TEXT 389
#define T_XHP_LT_DIV 390
#define T_XHP_LT_DIV_GT 391
#define T_XHP_ATTRIBUTE 392
#define T_XHP_CATEGORY 393
#define T_XHP_CHILDREN 394
#define T_XHP_ANY 395
#define T_XHP_EMPTY 396
#define T_XHP_PCDATA 397
#define T_XHP_COLON 398
#define T_XHP_HYPHEN 399
#define T_XHP_BOOLEAN 400
#define T_XHP_NUMBER 401
#define T_XHP_ARRAY 402
#define T_XHP_STRING 403
#define T_XHP_ENUM 404
#define T_XHP_FLOAT 405
#define T_XHP_REQUIRED 406




/* Copy the first part of user declarations.  */
#line 18 "parser.y"

#include "xhp.hpp"
// PHP's if/else rules use right reduction rather than left reduction which
// means while parsing nested if/else's the stack grows until it the last
// statement is read. This is annoying, particularly because of a quirk in
// bison.
// http://www.gnu.org/software/bison/manual/html_node/Memory-Management.html
// Apparently if you compile a bison parser with g++ it can no longer grow
// the stack. The work around is to just make your initial stack ridiculously
// large. Unfortunately that increases memory usage while parsing which is
// dumb. Anyway, putting a TODO here to fix PHP's if/else grammar.
#define YYINITDEPTH 500
#line 32 "parser.y"

#undef yyextra
#define yyextra static_cast<yy_extra_type*>(xhpget_extra(yyscanner))
#undef yylineno
#define yylineno yyextra->first_lineno
#define cr(s) code_rope(s, yylineno)
#define push_state(s) xhp_new_push_state(s, (struct yyguts_t*) yyscanner)
#define pop_state() xhp_new_pop_state((struct yyguts_t*) yyscanner)
#define set_state(s) xhp_set_state(s, (struct yyguts_t*) yyscanner)
using namespace std;

static void yyerror(void* yyscanner, void* _, const char* error) {
  if (yyextra->terminated) {
    return;
  }
  yyextra->terminated = true;
  yyextra->error = error;
}

static void replacestr(string &source, const string &find, const string &rep) {
  size_t j;
  while ((j = source.find(find)) != std::string::npos) {
    source.replace(j, find.length(), rep);
  }
}



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 457 "parser.yacc.cpp"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   7860

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  179
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  158
/* YYNRULES -- Number of rules.  */
#define YYNRULES  483
/* YYNRULES -- Number of states.  */
#define YYNSTATES  957

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   406

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    48,   178,     2,   176,    47,    31,     2,
     171,   172,    45,    42,     8,    43,    44,    46,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    26,   173,
      36,    13,    37,    25,    51,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    63,     2,   177,    30,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   174,    29,   175,    50,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     9,    10,    11,    12,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    27,    28,
      32,    33,    34,    35,    38,    39,    40,    41,    49,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,     9,    11,    15,    17,    19,
      21,    26,    30,    36,    41,    45,    48,    52,    54,    56,
      60,    63,    68,    74,    79,    82,    83,    85,    87,    89,
      94,    96,    99,   101,   103,   105,   109,   117,   128,   134,
     142,   152,   158,   161,   165,   168,   172,   175,   179,   183,
     187,   191,   195,   197,   200,   206,   215,   224,   230,   232,
     246,   250,   254,   256,   257,   259,   262,   271,   273,   277,
     279,   281,   283,   284,   286,   296,   304,   311,   313,   316,
     319,   320,   323,   325,   326,   329,   330,   333,   335,   339,
     340,   343,   345,   348,   350,   355,   357,   362,   364,   369,
     373,   379,   383,   388,   393,   399,   400,   406,   411,   413,
     415,   417,   422,   423,   430,   431,   439,   440,   443,   444,
     448,   450,   451,   454,   458,   464,   469,   474,   480,   488,
     495,   496,   498,   500,   502,   503,   505,   507,   510,   514,
     518,   523,   527,   529,   531,   534,   539,   543,   549,   551,
     555,   558,   559,   563,   566,   567,   577,   579,   583,   585,
     587,   588,   590,   592,   595,   597,   599,   601,   603,   605,
     607,   611,   617,   619,   623,   629,   634,   638,   640,   641,
     643,   647,   649,   656,   660,   665,   672,   676,   679,   683,
     687,   691,   695,   699,   703,   707,   711,   715,   719,   723,
     726,   729,   732,   735,   739,   743,   747,   751,   755,   759,
     763,   767,   771,   775,   779,   783,   787,   791,   795,   799,
     802,   805,   808,   811,   815,   819,   823,   827,   831,   835,
     839,   843,   847,   851,   857,   862,   864,   867,   870,   873,
     876,   879,   882,   885,   888,   891,   894,   897,   899,   904,
     906,   909,   919,   930,   932,   933,   938,   942,   947,   949,
     952,   957,   964,   970,   977,   984,   991,   998,  1003,  1005,
    1007,  1011,  1014,  1016,  1020,  1023,  1025,  1027,  1032,  1034,
    1037,  1038,  1041,  1042,  1045,  1049,  1050,  1054,  1056,  1058,
    1060,  1062,  1064,  1066,  1068,  1070,  1072,  1074,  1076,  1078,
    1080,  1084,  1087,  1090,  1093,  1098,  1100,  1104,  1106,  1108,
    1110,  1114,  1117,  1119,  1120,  1123,  1124,  1126,  1132,  1136,
    1140,  1142,  1144,  1146,  1148,  1150,  1152,  1158,  1160,  1163,
    1164,  1168,  1172,  1173,  1175,  1178,  1182,  1186,  1188,  1190,
    1192,  1194,  1197,  1199,  1204,  1209,  1211,  1213,  1218,  1219,
    1221,  1223,  1225,  1230,  1235,  1237,  1239,  1243,  1245,  1248,
    1252,  1254,  1256,  1261,  1262,  1263,  1266,  1272,  1276,  1280,
    1282,  1289,  1294,  1299,  1302,  1307,  1312,  1315,  1318,  1323,
    1326,  1329,  1331,  1335,  1339,  1343,  1345,  1347,  1351,  1356,
    1360,  1364,  1366,  1369,  1371,  1374,  1375,  1377,  1380,  1384,
    1386,  1387,  1388,  1394,  1395,  1398,  1402,  1403,  1408,  1409,
    1410,  1416,  1417,  1419,  1420,  1424,  1425,  1428,  1429,  1433,
    1434,  1438,  1439,  1443,  1445,  1449,  1453,  1455,  1459,  1463,
    1465,  1466,  1467,  1468,  1479,  1480,  1485,  1487,  1491,  1496,
    1499,  1501,  1503,  1505,  1507,  1509,  1511,  1512,  1513,  1520,
    1522,  1524,  1528,  1531,  1534,  1535,  1537,  1538,  1539,  1544,
    1547,  1552,  1553,  1558,  1560,  1562,  1564,  1568,  1573,  1578,
    1583,  1585,  1587,  1590,  1593,  1596,  1600,  1604,  1606,  1608,
    1611,  1614,  1617,  1620
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     180,     0,    -1,   181,    -1,   181,   183,    -1,    -1,    73,
      -1,   182,   151,    73,    -1,   189,    -1,   196,    -1,   197,
      -1,   119,   171,   172,   173,    -1,   148,   182,   173,    -1,
     148,   182,   174,   181,   175,    -1,   148,   174,   181,   175,
      -1,   107,   184,   173,    -1,   186,   173,    -1,   184,     8,
     185,    -1,   185,    -1,   182,    -1,   182,    93,    73,    -1,
     151,   182,    -1,   151,   182,    93,    73,    -1,   186,     8,
      73,    13,   256,    -1,   102,    73,    13,   256,    -1,   187,
     188,    -1,    -1,   189,    -1,   196,    -1,   197,    -1,   119,
     171,   172,   173,    -1,   190,    -1,    73,    26,    -1,   135,
      -1,   136,    -1,   137,    -1,   174,   187,   175,    -1,    67,
     171,   262,   172,   189,   217,   219,    -1,    67,   171,   262,
     172,    26,   187,   218,   220,    70,   173,    -1,    85,   171,
     262,   172,   216,    -1,    84,   189,    85,   171,   262,   172,
     173,    -1,    87,   171,   240,   173,   240,   173,   240,   172,
     209,    -1,    94,   171,   262,   172,   213,    -1,    98,   173,
      -1,    98,   262,   173,    -1,    99,   173,    -1,    99,   262,
     173,    -1,   103,   173,    -1,   103,   242,   173,    -1,   103,
     266,   173,    -1,   108,   226,   173,    -1,   114,   228,   173,
      -1,    83,   239,   173,    -1,    77,    -1,   262,   173,    -1,
     116,   171,   194,   172,   173,    -1,    89,   171,   266,    93,
     208,   207,   172,   210,    -1,    89,   171,   242,    93,   266,
     207,   172,   210,    -1,    91,   171,   212,   172,   211,    -1,
     173,    -1,   104,   174,   187,   175,   105,   171,   248,    75,
     172,   174,   187,   175,   191,    -1,   106,   262,   173,    -1,
     100,    73,   173,    -1,   192,    -1,    -1,   193,    -1,   192,
     193,    -1,   105,   171,   248,    75,   172,   174,   187,   175,
      -1,   195,    -1,   194,     8,   195,    -1,   266,    -1,   199,
      -1,   200,    -1,    -1,    31,    -1,   243,   198,    73,   171,
     221,   172,   174,   187,   175,    -1,   201,    73,   202,   205,
     174,   229,   175,    -1,   203,    73,   204,   174,   229,   175,
      -1,   120,    -1,   113,   120,    -1,   112,   120,    -1,    -1,
     122,   248,    -1,   121,    -1,    -1,   122,   206,    -1,    -1,
     123,   206,    -1,   248,    -1,   206,     8,   248,    -1,    -1,
     125,   208,    -1,   266,    -1,    31,   266,    -1,   189,    -1,
      26,   187,    88,   173,    -1,   189,    -1,    26,   187,    90,
     173,    -1,   189,    -1,    26,   187,    92,   173,    -1,    73,
      13,   256,    -1,   212,     8,    73,    13,   256,    -1,   174,
     214,   175,    -1,   174,   173,   214,   175,    -1,    26,   214,
      95,   173,    -1,    26,   173,   214,    95,   173,    -1,    -1,
     214,    96,   262,   215,   187,    -1,   214,    97,   215,   187,
      -1,    26,    -1,   173,    -1,   189,    -1,    26,   187,    86,
     173,    -1,    -1,   217,    68,   171,   262,   172,   189,    -1,
      -1,   218,    68,   171,   262,   172,    26,   187,    -1,    -1,
      69,   189,    -1,    -1,    69,    26,   187,    -1,   222,    -1,
      -1,   223,    75,    -1,   223,    31,    75,    -1,   223,    31,
      75,    13,   256,    -1,   223,    75,    13,   256,    -1,   222,
       8,   223,    75,    -1,   222,     8,   223,    31,    75,    -1,
     222,     8,   223,    31,    75,    13,   256,    -1,   222,     8,
     223,    75,    13,   256,    -1,    -1,   248,    -1,   127,    -1,
     225,    -1,    -1,   242,    -1,   266,    -1,    31,   264,    -1,
     225,     8,   242,    -1,   225,     8,   266,    -1,   225,     8,
      31,   264,    -1,   226,     8,   227,    -1,   227,    -1,    75,
      -1,   176,   263,    -1,   176,   174,   262,   175,    -1,   228,
       8,    75,    -1,   228,     8,    75,    13,   256,    -1,    75,
      -1,    75,    13,   256,    -1,   229,   230,    -1,    -1,   233,
     237,   173,    -1,   238,   173,    -1,    -1,   234,   243,   231,
     198,    73,   171,   221,   172,   232,    -1,   173,    -1,   174,
     187,   175,    -1,   235,    -1,   115,    -1,    -1,   235,    -1,
     236,    -1,   235,   236,    -1,   109,    -1,   110,    -1,   111,
      -1,   114,    -1,   113,    -1,   112,    -1,   237,     8,    75,
      -1,   237,     8,    75,    13,   256,    -1,    75,    -1,    75,
      13,   256,    -1,   238,     8,    73,    13,   256,    -1,   102,
      73,    13,   256,    -1,   239,     8,   262,    -1,   262,    -1,
      -1,   241,    -1,   241,     8,   262,    -1,   262,    -1,   126,
     171,   282,   172,    13,   262,    -1,   266,    13,   262,    -1,
     266,    13,    31,   266,    -1,   266,    13,    31,    65,   249,
     254,    -1,    65,   249,   254,    -1,    64,   262,    -1,   266,
      24,   262,    -1,   266,    23,   262,    -1,   266,    22,   262,
      -1,   266,    21,   262,    -1,   266,    20,   262,    -1,   266,
      19,   262,    -1,   266,    18,   262,    -1,   266,    17,   262,
      -1,   266,    16,   262,    -1,   266,    15,   262,    -1,   266,
      14,   262,    -1,   265,    62,    -1,    62,   265,    -1,   265,
      61,    -1,    61,   265,    -1,   262,    27,   262,    -1,   262,
      28,   262,    -1,   262,     9,   262,    -1,   262,    11,   262,
      -1,   262,    10,   262,    -1,   262,    29,   262,    -1,   262,
      31,   262,    -1,   262,    30,   262,    -1,   262,    44,   262,
      -1,   262,    42,   262,    -1,   262,    43,   262,    -1,   262,
      45,   262,    -1,   262,    46,   262,    -1,   262,    47,   262,
      -1,   262,    41,   262,    -1,   262,    40,   262,    -1,    42,
     262,    -1,    43,   262,    -1,    48,   262,    -1,    50,   262,
      -1,   262,    33,   262,    -1,   262,    32,   262,    -1,   262,
      35,   262,    -1,   262,    34,   262,    -1,   262,    36,   262,
      -1,   262,    39,   262,    -1,   262,    37,   262,    -1,   262,
      38,   262,    -1,   262,    49,   249,    -1,   171,   262,   172,
      -1,   262,    25,   262,    26,   262,    -1,   262,    25,    26,
     262,    -1,   286,    -1,    60,   262,    -1,    59,   262,    -1,
      58,   262,    -1,    57,   262,    -1,    56,   262,    -1,    55,
     262,    -1,    54,   262,    -1,    53,   262,    -1,    52,   262,
      -1,    66,   253,    -1,    51,   262,    -1,   258,    -1,   127,
     171,   284,   172,    -1,    82,    -1,    12,   262,    -1,   243,
     198,   171,   221,   172,   244,   174,   187,   175,    -1,   114,
     243,   198,   171,   221,   172,   244,   174,   187,   175,    -1,
     101,    -1,    -1,   107,   171,   245,   172,    -1,   245,     8,
      75,    -1,   245,     8,    31,    75,    -1,    75,    -1,    31,
      75,    -1,   182,   171,   224,   172,    -1,   148,   151,   182,
     171,   224,   172,    -1,   151,   182,   171,   224,   172,    -1,
     247,   145,    73,   171,   224,   172,    -1,   272,   145,    73,
     171,   224,   172,    -1,   272,   145,   270,   171,   224,   172,
      -1,   247,   145,   270,   171,   224,   172,    -1,   270,   171,
     224,   172,    -1,   114,    -1,   182,    -1,   148,   151,   182,
      -1,   151,   182,    -1,   182,    -1,   148,   151,   182,    -1,
     151,   182,    -1,   247,    -1,   250,    -1,   274,   124,   278,
     251,    -1,   274,    -1,   251,   252,    -1,    -1,   124,   278,
      -1,    -1,   171,   172,    -1,   171,   262,   172,    -1,    -1,
     171,   224,   172,    -1,    71,    -1,    72,    -1,    81,    -1,
     131,    -1,   132,    -1,   150,    -1,   128,    -1,   129,    -1,
     130,    -1,   149,    -1,   142,    -1,   255,    -1,   182,    -1,
     148,   151,   182,    -1,   151,   182,    -1,    42,   256,    -1,
      43,   256,    -1,   127,   171,   259,   172,    -1,   257,    -1,
     247,   145,    73,    -1,    74,    -1,   288,    -1,   182,    -1,
     148,   151,   182,    -1,   151,   182,    -1,   255,    -1,    -1,
     261,   260,    -1,    -1,     8,    -1,   261,     8,   256,   125,
     256,    -1,   261,     8,   256,    -1,   256,   125,   256,    -1,
     256,    -1,   263,    -1,   242,    -1,   266,    -1,   266,    -1,
     266,    -1,   273,   124,   278,   269,   267,    -1,   273,    -1,
     267,   268,    -1,    -1,   124,   278,   269,    -1,   171,   224,
     172,    -1,    -1,   275,    -1,   281,   275,    -1,   247,   145,
     270,    -1,   272,   145,   270,    -1,   275,    -1,   274,    -1,
     246,    -1,   275,    -1,   281,   275,    -1,   271,    -1,   275,
      63,   277,   177,    -1,   275,   174,   262,   175,    -1,   276,
      -1,    75,    -1,   176,   174,   262,   175,    -1,    -1,   262,
      -1,   279,    -1,   270,    -1,   279,    63,   277,   177,    -1,
     279,   174,   262,   175,    -1,   280,    -1,    73,    -1,   174,
     262,   175,    -1,   176,    -1,   281,   176,    -1,   282,     8,
     283,    -1,   283,    -1,   266,    -1,   126,   171,   282,   172,
      -1,    -1,    -1,   285,   260,    -1,   285,     8,   262,   125,
     262,    -1,   285,     8,   262,    -1,   262,   125,   262,    -1,
     262,    -1,   285,     8,   262,   125,    31,   264,    -1,   285,
       8,    31,   264,    -1,   262,   125,    31,   264,    -1,    31,
     264,    -1,   117,   171,   287,   172,    -1,   118,   171,   266,
     172,    -1,     7,   262,    -1,     6,   262,    -1,     5,   171,
     262,   172,    -1,     4,   262,    -1,     3,   262,    -1,   266,
      -1,   287,     8,   266,    -1,   247,   145,    73,    -1,   272,
     145,    73,    -1,   289,    -1,   290,    -1,   291,   295,   292,
      -1,   293,   299,    46,    37,    -1,   293,   299,    37,    -1,
     154,   308,    37,    -1,   155,    -1,    36,   306,    -1,   153,
      -1,   294,   153,    -1,    -1,   294,    -1,   295,   296,    -1,
     295,   296,   294,    -1,   289,    -1,    -1,    -1,   174,   297,
     262,   175,   298,    -1,    -1,   299,   300,    -1,   310,    13,
     301,    -1,    -1,   178,   302,   305,   178,    -1,    -1,    -1,
     174,   303,   262,   304,   175,    -1,    -1,   294,    -1,    -1,
     307,   316,   318,    -1,    -1,   309,   316,    -1,    -1,   311,
     317,   318,    -1,    -1,   313,   317,   318,    -1,    -1,   315,
     316,   318,    -1,    73,    -1,   316,   162,    73,    -1,   316,
     163,    73,    -1,    73,    -1,   317,   162,    73,    -1,   317,
     163,    73,    -1,   152,    -1,    -1,    -1,    -1,   201,    26,
     306,   202,   205,   174,   319,   229,   320,   175,    -1,    -1,
     156,   321,   322,   173,    -1,   323,    -1,   322,     8,   323,
      -1,   324,   310,   328,   329,    -1,   162,   306,    -1,   167,
      -1,   164,    -1,   165,    -1,   166,    -1,   247,    -1,   115,
      -1,    -1,    -1,   168,   174,   325,   327,   326,   175,    -1,
     169,    -1,   255,    -1,   327,     8,   255,    -1,    13,   255,
      -1,    13,    73,    -1,    -1,   170,    -1,    -1,    -1,   157,
     330,   331,   173,    -1,    47,   312,    -1,   331,     8,    47,
     312,    -1,    -1,   158,   332,   333,   173,    -1,   334,    -1,
     159,    -1,   160,    -1,   171,   335,   172,    -1,   171,   335,
     172,    45,    -1,   171,   335,   172,    25,    -1,   171,   335,
     172,    42,    -1,   334,    -1,   336,    -1,   336,    45,    -1,
     336,    25,    -1,   336,    42,    -1,   335,     8,   335,    -1,
     335,    29,   335,    -1,   159,    -1,   161,    -1,   162,   314,
      -1,    47,   314,    -1,   162,   306,    -1,   162,   306,    -1,
     262,    63,   277,   177,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   199,   199,   205,   208,   214,   215,   221,   222,   223,
     224,   227,   230,   233,   236,   239,   245,   248,   252,   253,
     256,   259,   265,   268,   274,   277,   283,   284,   285,   286,
     292,   293,   296,   297,   298,   304,   307,   310,   313,   316,
     319,   322,   325,   328,   331,   334,   337,   340,   343,   346,
     349,   352,   355,   356,   359,   362,   365,   368,   371,   372,
     375,   378,   384,   385,   391,   392,   398,   404,   405,   411,
     415,   419,   423,   426,   430,   436,   439,   445,   446,   449,
     455,   458,   464,   468,   471,   477,   480,   486,   487,   493,
     496,   502,   503,   509,   510,   516,   517,   523,   524,   530,
     533,   539,   542,   545,   548,   554,   557,   560,   566,   567,
     571,   572,   578,   581,   587,   590,   596,   599,   605,   608,
     614,   615,   621,   624,   627,   630,   633,   636,   639,   642,
     648,   651,   654,   660,   661,   667,   668,   669,   672,   675,
     678,   684,   687,   691,   692,   695,   701,   704,   707,   708,
     714,   717,   723,   726,   729,   729,   739,   740,   746,   747,
     753,   756,   762,   763,   769,   770,   771,   772,   773,   774,
     778,   781,   784,   785,   791,   794,   800,   803,   807,   810,
     815,   818,   822,   825,   828,   831,   834,   837,   840,   843,
     846,   849,   852,   855,   858,   861,   864,   867,   870,   873,
     876,   879,   882,   885,   888,   891,   894,   897,   900,   903,
     906,   909,   912,   915,   918,   921,   924,   927,   930,   933,
     936,   939,   942,   945,   948,   951,   954,   957,   960,   963,
     966,   969,   972,   975,   978,   981,   982,   985,   988,   991,
     994,   997,  1000,  1003,  1006,  1009,  1012,  1015,  1016,  1019,
    1020,  1023,  1026,  1032,  1035,  1037,  1043,  1046,  1049,  1050,
    1056,  1059,  1062,  1065,  1068,  1071,  1074,  1077,  1083,  1084,
    1085,  1088,  1094,  1095,  1098,  1104,  1105,  1109,  1112,  1116,
    1119,  1125,  1131,  1134,  1137,  1143,  1146,  1152,  1153,  1154,
    1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1166,  1167,
    1168,  1171,  1174,  1177,  1180,  1183,  1187,  1193,  1194,  1195,
    1196,  1199,  1202,  1206,  1209,  1213,  1216,  1220,  1223,  1226,
    1229,  1233,  1234,  1238,  1242,  1246,  1250,  1253,  1257,  1260,
    1266,  1272,  1275,  1281,  1282,  1288,  1291,  1297,  1301,  1302,
    1306,  1307,  1310,  1314,  1317,  1320,  1324,  1325,  1331,  1334,
    1338,  1339,  1343,  1346,  1349,  1353,  1354,  1360,  1361,  1367,
    1370,  1374,  1375,  1378,  1384,  1387,  1391,  1394,  1397,  1400,
    1401,  1404,  1407,  1410,  1416,  1419,  1422,  1425,  1428,  1431,
    1434,  1440,  1441,  1447,  1450,  1460,  1467,  1468,  1480,  1493,
    1502,  1520,  1532,  1539,  1543,  1550,  1553,  1557,  1561,  1568,
    1569,  1572,  1569,  1583,  1587,  1593,  1599,  1599,  1602,  1602,
    1602,  1608,  1611,  1619,  1619,  1626,  1626,  1633,  1633,  1640,
    1640,  1648,  1648,  1655,  1660,  1663,  1669,  1674,  1677,  1683,
    1684,  1689,  1694,  1689,  1717,  1717,  1726,  1727,  1731,  1737,
    1745,  1748,  1751,  1754,  1757,  1760,  1763,  1763,  1763,  1766,
    1772,  1776,  1783,  1787,  1791,  1797,  1800,  1807,  1807,  1819,
    1822,  1829,  1829,  1837,  1840,  1843,  1849,  1852,  1855,  1858,
    1864,  1865,  1868,  1871,  1874,  1877,  1880,  1886,  1889,  1892,
    1895,  1902,  1911,  1925
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_REQUIRE_ONCE", "T_REQUIRE", "T_EVAL",
  "T_INCLUDE_ONCE", "T_INCLUDE", "','", "T_LOGICAL_OR", "T_LOGICAL_XOR",
  "T_LOGICAL_AND", "T_PRINT", "'='", "T_SR_EQUAL", "T_SL_EQUAL",
  "T_XOR_EQUAL", "T_OR_EQUAL", "T_AND_EQUAL", "T_MOD_EQUAL",
  "T_CONCAT_EQUAL", "T_DIV_EQUAL", "T_MUL_EQUAL", "T_MINUS_EQUAL",
  "T_PLUS_EQUAL", "'?'", "':'", "T_BOOLEAN_OR", "T_BOOLEAN_AND", "'|'",
  "'^'", "'&'", "T_IS_NOT_IDENTICAL", "T_IS_IDENTICAL", "T_IS_NOT_EQUAL",
  "T_IS_EQUAL", "'<'", "'>'", "T_IS_GREATER_OR_EQUAL",
  "T_IS_SMALLER_OR_EQUAL", "T_SR", "T_SL", "'+'", "'-'", "'.'", "'*'",
  "'/'", "'%'", "'!'", "T_INSTANCEOF", "'~'", "'@'", "T_UNSET_CAST",
  "T_BOOL_CAST", "T_OBJECT_CAST", "T_ARRAY_CAST", "T_BINARY_CAST",
  "T_UNICODE_CAST", "T_STRING_CAST", "T_DOUBLE_CAST", "T_INT_CAST",
  "T_DEC", "T_INC", "'['", "T_CLONE", "T_NEW", "T_EXIT", "T_IF",
  "T_ELSEIF", "T_ELSE", "T_ENDIF", "T_LNUMBER", "T_DNUMBER", "T_STRING",
  "T_STRING_VARNAME", "T_VARIABLE", "T_NUM_STRING", "T_INLINE_HTML",
  "T_CHARACTER", "T_BAD_CHARACTER", "T_ENCAPSED_AND_WHITESPACE",
  "T_CONSTANT_ENCAPSED_STRING", "T_BACKTICKS_EXPR", "T_ECHO", "T_DO",
  "T_WHILE", "T_ENDWHILE", "T_FOR", "T_ENDFOR", "T_FOREACH",
  "T_ENDFOREACH", "T_DECLARE", "T_ENDDECLARE", "T_AS", "T_SWITCH",
  "T_ENDSWITCH", "T_CASE", "T_DEFAULT", "T_BREAK", "T_CONTINUE", "T_GOTO",
  "T_FUNCTION", "T_CONST", "T_RETURN", "T_TRY", "T_CATCH", "T_THROW",
  "T_USE", "T_GLOBAL", "T_PUBLIC", "T_PROTECTED", "T_PRIVATE", "T_FINAL",
  "T_ABSTRACT", "T_STATIC", "T_VAR", "T_UNSET", "T_ISSET", "T_EMPTY",
  "T_HALT_COMPILER", "T_CLASS", "T_INTERFACE", "T_EXTENDS", "T_IMPLEMENTS",
  "T_OBJECT_OPERATOR", "T_DOUBLE_ARROW", "T_LIST", "T_ARRAY", "T_CLASS_C",
  "T_METHOD_C", "T_FUNC_C", "T_LINE", "T_FILE", "T_COMMENT",
  "T_DOC_COMMENT", "T_OPEN_TAG", "T_OPEN_TAG_WITH_ECHO", "T_OPEN_TAG_FAKE",
  "T_CLOSE_TAG", "T_WHITESPACE", "T_START_HEREDOC", "T_END_HEREDOC",
  "T_HEREDOC", "T_DOLLAR_OPEN_CURLY_BRACES", "T_CURLY_OPEN",
  "T_PAAMAYIM_NEKUDOTAYIM", "T_BINARY_DOUBLE", "T_BINARY_HEREDOC",
  "T_NAMESPACE", "T_NS_C", "T_DIR", "T_NS_SEPARATOR", "T_XHP_WHITESPACE",
  "T_XHP_TEXT", "T_XHP_LT_DIV", "T_XHP_LT_DIV_GT", "T_XHP_ATTRIBUTE",
  "T_XHP_CATEGORY", "T_XHP_CHILDREN", "T_XHP_ANY", "T_XHP_EMPTY",
  "T_XHP_PCDATA", "T_XHP_COLON", "T_XHP_HYPHEN", "T_XHP_BOOLEAN",
  "T_XHP_NUMBER", "T_XHP_ARRAY", "T_XHP_STRING", "T_XHP_ENUM",
  "T_XHP_FLOAT", "T_XHP_REQUIRED", "'('", "')'", "';'", "'{'", "'}'",
  "'$'", "']'", "'\"'", "$accept", "start", "top_statement_list",
  "namespace_name", "top_statement", "use_declarations", "use_declaration",
  "constant_declaration", "inner_statement_list", "inner_statement",
  "statement", "unticked_statement", "additional_catches",
  "non_empty_additional_catches", "additional_catch", "unset_variables",
  "unset_variable", "function_declaration_statement",
  "class_declaration_statement", "is_reference",
  "unticked_function_declaration_statement",
  "unticked_class_declaration_statement", "class_entry_type",
  "extends_from", "interface_entry", "interface_extends_list",
  "implements_list", "interface_list", "foreach_optional_arg",
  "foreach_variable", "for_statement", "foreach_statement",
  "declare_statement", "declare_list", "switch_case_list", "case_list",
  "case_separator", "while_statement", "elseif_list", "new_elseif_list",
  "else_single", "new_else_single", "parameter_list",
  "non_empty_parameter_list", "optional_class_type",
  "function_call_parameter_list", "non_empty_function_call_parameter_list",
  "global_var_list", "global_var", "static_var_list",
  "class_statement_list", "class_statement", "@1", "method_body",
  "variable_modifiers", "method_modifiers", "non_empty_member_modifiers",
  "member_modifier", "class_variable_declaration",
  "class_constant_declaration", "echo_expr_list", "for_expr",
  "non_empty_for_expr", "expr_without_variable", "function",
  "lexical_vars", "lexical_var_list", "function_call", "class_name",
  "fully_qualified_class_name", "class_name_reference",
  "dynamic_class_name_reference", "dynamic_class_name_variable_properties",
  "dynamic_class_name_variable_property", "exit_expr", "ctor_arguments",
  "common_scalar", "static_scalar", "static_class_constant", "scalar",
  "static_array_pair_list", "possible_comma",
  "non_empty_static_array_pair_list", "expr", "r_variable", "w_variable",
  "rw_variable", "variable", "variable_properties", "variable_property",
  "method_or_not", "variable_without_objects", "static_member",
  "variable_class_name", "base_variable_with_function_calls",
  "base_variable", "reference_variable", "compound_variable", "dim_offset",
  "object_property", "object_dim_list", "variable_name",
  "simple_indirect_reference", "assignment_list",
  "assignment_list_element", "array_pair_list",
  "non_empty_array_pair_list", "internal_functions_in_yacc",
  "isset_variables", "class_constant", "xhp_tag_expression",
  "xhp_singleton", "xhp_tag_open", "xhp_tag_close", "xhp_tag_start",
  "xhp_literal_text", "xhp_children", "xhp_child", "@2", "@3",
  "xhp_attributes", "xhp_attribute", "xhp_attribute_value", "@4", "@5",
  "@6", "xhp_attribute_quoted_value", "xhp_label_immediate", "@7",
  "xhp_label_no_space", "@8", "xhp_label_pass", "@9",
  "xhp_label_pass_immediate", "@10", "xhp_label", "@11", "xhp_label_",
  "xhp_label_pass_", "xhp_whitespace_hack", "@12", "@13", "@14",
  "xhp_attribute_decls", "xhp_attribute_decl", "xhp_attribute_decl_type",
  "@15", "@16", "xhp_attribute_enum", "xhp_attribute_default",
  "xhp_attribute_is_required", "@17", "xhp_category_list", "@18",
  "xhp_children_decl", "xhp_children_paren_expr", "xhp_children_decl_expr",
  "xhp_children_decl_tag", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,    44,   263,
     264,   265,   266,    61,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,    63,    58,   278,   279,   124,
      94,    38,   280,   281,   282,   283,    60,    62,   284,   285,
     286,   287,    43,    45,    46,    42,    47,    37,    33,   288,
     126,    64,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,    91,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   389,   390,   391,   392,   393,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,   405,
     406,    40,    41,    59,   123,   125,    36,    93,    34
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   179,   180,   181,   181,   182,   182,   183,   183,   183,
     183,   183,   183,   183,   183,   183,   184,   184,   185,   185,
     185,   185,   186,   186,   187,   187,   188,   188,   188,   188,
     189,   189,   189,   189,   189,   190,   190,   190,   190,   190,
     190,   190,   190,   190,   190,   190,   190,   190,   190,   190,
     190,   190,   190,   190,   190,   190,   190,   190,   190,   190,
     190,   190,   191,   191,   192,   192,   193,   194,   194,   195,
     196,   197,   198,   198,   199,   200,   200,   201,   201,   201,
     202,   202,   203,   204,   204,   205,   205,   206,   206,   207,
     207,   208,   208,   209,   209,   210,   210,   211,   211,   212,
     212,   213,   213,   213,   213,   214,   214,   214,   215,   215,
     216,   216,   217,   217,   218,   218,   219,   219,   220,   220,
     221,   221,   222,   222,   222,   222,   222,   222,   222,   222,
     223,   223,   223,   224,   224,   225,   225,   225,   225,   225,
     225,   226,   226,   227,   227,   227,   228,   228,   228,   228,
     229,   229,   230,   230,   231,   230,   232,   232,   233,   233,
     234,   234,   235,   235,   236,   236,   236,   236,   236,   236,
     237,   237,   237,   237,   238,   238,   239,   239,   240,   240,
     241,   241,   242,   242,   242,   242,   242,   242,   242,   242,
     242,   242,   242,   242,   242,   242,   242,   242,   242,   242,
     242,   242,   242,   242,   242,   242,   242,   242,   242,   242,
     242,   242,   242,   242,   242,   242,   242,   242,   242,   242,
     242,   242,   242,   242,   242,   242,   242,   242,   242,   242,
     242,   242,   242,   242,   242,   242,   242,   242,   242,   242,
     242,   242,   242,   242,   242,   242,   242,   242,   242,   242,
     242,   242,   242,   243,   244,   244,   245,   245,   245,   245,
     246,   246,   246,   246,   246,   246,   246,   246,   247,   247,
     247,   247,   248,   248,   248,   249,   249,   250,   250,   251,
     251,   252,   253,   253,   253,   254,   254,   255,   255,   255,
     255,   255,   255,   255,   255,   255,   255,   255,   256,   256,
     256,   256,   256,   256,   256,   256,   257,   258,   258,   258,
     258,   258,   258,   259,   259,   260,   260,   261,   261,   261,
     261,   262,   262,   263,   264,   265,   266,   266,   267,   267,
     268,   269,   269,   270,   270,   271,   271,   272,   273,   273,
     274,   274,   274,   275,   275,   275,   276,   276,   277,   277,
     278,   278,   279,   279,   279,   280,   280,   281,   281,   282,
     282,   283,   283,   283,   284,   284,   285,   285,   285,   285,
     285,   285,   285,   285,   286,   286,   286,   286,   286,   286,
     286,   287,   287,   288,   288,   242,   289,   289,   290,   291,
     292,   292,   293,   294,   294,   295,   295,   295,   295,   296,
     297,   298,   296,   299,   299,   300,   302,   301,   303,   304,
     301,   305,   305,   307,   306,   309,   308,   311,   310,   313,
     312,   315,   314,   316,   316,   316,   317,   317,   317,   318,
     318,   319,   320,   197,   321,   230,   322,   322,   323,   323,
     324,   324,   324,   324,   324,   324,   325,   326,   324,   324,
     327,   327,   328,   328,   328,   329,   329,   330,   230,   331,
     331,   332,   230,   333,   333,   333,   334,   334,   334,   334,
     335,   335,   335,   335,   335,   335,   335,   336,   336,   336,
     336,   247,   248,   242
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     0,     1,     3,     1,     1,     1,
       4,     3,     5,     4,     3,     2,     3,     1,     1,     3,
       2,     4,     5,     4,     2,     0,     1,     1,     1,     4,
       1,     2,     1,     1,     1,     3,     7,    10,     5,     7,
       9,     5,     2,     3,     2,     3,     2,     3,     3,     3,
       3,     3,     1,     2,     5,     8,     8,     5,     1,    13,
       3,     3,     1,     0,     1,     2,     8,     1,     3,     1,
       1,     1,     0,     1,     9,     7,     6,     1,     2,     2,
       0,     2,     1,     0,     2,     0,     2,     1,     3,     0,
       2,     1,     2,     1,     4,     1,     4,     1,     4,     3,
       5,     3,     4,     4,     5,     0,     5,     4,     1,     1,
       1,     4,     0,     6,     0,     7,     0,     2,     0,     3,
       1,     0,     2,     3,     5,     4,     4,     5,     7,     6,
       0,     1,     1,     1,     0,     1,     1,     2,     3,     3,
       4,     3,     1,     1,     2,     4,     3,     5,     1,     3,
       2,     0,     3,     2,     0,     9,     1,     3,     1,     1,
       0,     1,     1,     2,     1,     1,     1,     1,     1,     1,
       3,     5,     1,     3,     5,     4,     3,     1,     0,     1,
       3,     1,     6,     3,     4,     6,     3,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     5,     4,     1,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     1,     4,     1,
       2,     9,    10,     1,     0,     4,     3,     4,     1,     2,
       4,     6,     5,     6,     6,     6,     6,     4,     1,     1,
       3,     2,     1,     3,     2,     1,     1,     4,     1,     2,
       0,     2,     0,     2,     3,     0,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     2,     2,     2,     4,     1,     3,     1,     1,     1,
       3,     2,     1,     0,     2,     0,     1,     5,     3,     3,
       1,     1,     1,     1,     1,     1,     5,     1,     2,     0,
       3,     3,     0,     1,     2,     3,     3,     1,     1,     1,
       1,     2,     1,     4,     4,     1,     1,     4,     0,     1,
       1,     1,     4,     4,     1,     1,     3,     1,     2,     3,
       1,     1,     4,     0,     0,     2,     5,     3,     3,     1,
       6,     4,     4,     2,     4,     4,     2,     2,     4,     2,
       2,     1,     3,     3,     3,     1,     1,     3,     4,     3,
       3,     1,     2,     1,     2,     0,     1,     2,     3,     1,
       0,     0,     5,     0,     2,     3,     0,     4,     0,     0,
       5,     0,     1,     0,     3,     0,     2,     0,     3,     0,
       3,     0,     3,     1,     3,     3,     1,     3,     3,     1,
       0,     0,     0,    10,     0,     4,     1,     3,     4,     2,
       1,     1,     1,     1,     1,     1,     0,     0,     6,     1,
       1,     3,     2,     2,     0,     1,     0,     0,     4,     2,
       4,     0,     4,     1,     1,     1,     3,     4,     4,     4,
       1,     1,     2,     2,     2,     3,     3,     1,     1,     2,
       2,     2,     2,     4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       4,     0,     2,     1,     0,     0,     0,     0,     0,     0,
     413,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   282,
       0,   287,   288,     5,   307,   346,    52,   289,   249,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   253,
       0,     0,     0,     0,     0,     0,     0,     0,   268,     0,
       0,     0,     0,    77,    82,     0,     0,   293,   294,   295,
     290,   291,    32,    33,    34,   297,     0,   296,   292,     0,
     413,     0,    58,    25,   357,   309,     3,     0,     7,    30,
       8,     9,    70,    71,     0,     0,   322,    72,   339,     0,
     312,   247,     0,   321,     0,   323,     0,   342,     0,   327,
     338,   340,   345,     0,   235,   308,   385,   386,   395,   403,
       5,   268,     0,    72,   380,   379,     0,   377,   376,   250,
     392,     0,   219,   220,   221,   222,   246,   244,   243,   242,
     241,   240,   239,   238,   237,   236,   268,     0,     0,   269,
       0,   202,   325,     0,   200,   187,     0,     0,   269,   275,
     285,   276,     0,   278,   340,     0,     0,   245,     0,    31,
       0,   177,     0,     0,   178,     0,     0,     0,    42,     0,
      44,     0,     0,     0,    46,   322,     0,   323,    25,     0,
       0,    18,     0,    17,   143,     0,     0,   142,    79,    78,
     148,     0,    72,     0,     0,     0,     0,   363,   364,     0,
       4,     0,   311,   481,     0,     0,     0,     0,   134,     0,
      15,   413,    80,    83,    73,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   348,    53,   201,   199,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   134,
       0,     0,   348,     0,   358,   341,   393,   396,     0,   417,
       0,     0,   423,   430,     0,   271,     0,     0,     0,   271,
       0,   134,   186,     0,     0,   341,   283,     0,     0,     0,
      51,     0,     0,     0,   179,   181,   322,   323,     0,     0,
       0,    43,    45,    61,     0,    47,    48,     0,    60,    20,
       0,     0,    14,     0,   144,   323,     0,    49,     0,     0,
      50,     0,     0,    67,    69,   381,     0,     0,     0,     0,
     361,     0,   360,     0,   369,     0,   315,   310,     0,    11,
       4,   134,   232,     0,    35,    24,    26,    27,    28,     0,
       6,     0,     0,   133,   322,   323,     0,    80,     0,    85,
       0,     0,     0,   130,   383,   335,   333,     0,   205,   207,
     206,     0,     0,   203,   204,   208,   210,   209,   224,   223,
     226,   225,   227,   229,   230,   228,   218,   217,   212,   213,
     211,   214,   215,   216,   231,   349,     0,     0,   183,   198,
     197,   196,   195,   194,   193,   192,   191,   190,   189,   188,
       0,   384,   336,   355,     0,   351,   332,   350,   354,     0,
       0,   394,   415,   391,   400,   399,   387,   397,   389,     0,
     404,     0,     0,   378,   429,     0,     0,   414,   270,     0,
       0,   270,   335,     0,   336,   280,   284,     0,   176,     0,
       0,   178,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   299,     0,   298,    23,   305,     0,
       0,    19,    16,     0,   141,   149,   146,   130,     0,     0,
       0,   374,   375,    10,   363,   363,     0,   373,   324,     0,
     248,   316,   365,   134,    13,     0,     0,     0,   347,   137,
     260,     0,     0,    85,     0,     0,   413,   272,    81,     0,
       0,    84,    87,   151,   130,   132,     0,   120,     0,   131,
     134,   134,   334,   234,     0,   483,     0,   184,   267,   134,
     134,     0,   134,   329,   348,     0,   343,   344,     0,     0,
       0,   398,   388,     0,   426,   430,   424,   425,   286,   277,
      25,   112,     0,    25,   110,    38,     0,   180,    89,     0,
      89,    91,    99,     0,    25,    97,    57,   105,   105,    41,
     302,   303,   313,     0,   301,     0,     0,    21,   145,     0,
       0,    68,    54,   382,     0,   359,     0,     0,   368,     0,
     367,     0,    12,   262,     0,     0,   322,   323,    22,     0,
       0,   274,   482,    86,   151,     0,   160,     0,   254,   130,
       0,   122,     0,     0,   233,   285,     0,     0,   356,     0,
     326,     0,     0,   390,   416,     0,   408,   406,   405,     0,
       0,   418,     0,   279,   114,   116,     0,     0,   178,     0,
       0,    92,     0,     0,     0,   105,     0,   105,     0,   320,
       0,   315,   300,   306,     0,   147,   254,   362,   182,   372,
     371,     0,   261,    29,   140,   431,   273,   160,    88,     0,
     164,   165,   166,   169,   168,   167,   159,   434,   457,   461,
      76,   150,     0,     0,   158,   162,     0,     0,     0,     0,
       0,   123,     0,   263,   266,   185,   264,   265,   331,     0,
     328,   352,   353,   401,     0,   411,   427,   428,   281,   118,
       0,     0,    36,    39,     0,     0,    90,     0,     0,   100,
       0,     0,     0,     0,     0,     0,   101,     0,   304,   316,
     314,     0,     0,     0,   366,   151,    75,     0,     0,     0,
       0,   172,     0,   154,   163,     0,   153,    25,     0,    25,
       0,   126,     0,   125,   332,   402,   409,   412,     0,     0,
       0,     0,     0,   117,   111,     0,    25,    95,    56,    55,
      98,     0,   103,     0,   108,   109,    25,   102,   319,   318,
       0,    25,   370,   160,     0,   445,   413,   441,   442,   443,
     440,     0,   449,   444,     0,   436,   417,   419,     0,   464,
     465,     0,     0,   463,     0,     0,   152,    72,     0,     0,
       0,   258,     0,     0,   127,     0,   124,   330,     0,   407,
       0,    25,     0,     0,    25,    93,    40,     0,   104,    25,
     107,     0,     0,     0,     0,   175,   439,   446,     0,   435,
     454,   459,     0,     0,   458,   421,   477,   478,   421,   470,
       0,   471,   462,   173,   170,     0,     0,    74,   259,     0,
     255,   251,     0,   129,   410,     0,   119,    37,     0,     0,
       0,   106,   317,    25,   252,   433,     0,   437,     0,   456,
     430,   419,   480,     0,   479,     0,     0,   466,   473,   474,
     472,     0,     0,   174,     0,   256,   128,     0,   113,     0,
      96,     0,   450,   447,   453,   452,   455,   438,   420,   460,
     430,   475,   476,   468,   469,   467,   171,   130,   257,    25,
      94,    63,     0,     0,   422,     0,   115,     0,    59,    62,
      64,   451,   448,     0,     0,    65,   156,    25,   155,     0,
       0,     0,   157,     0,    25,     0,    66
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,    85,    86,   192,   193,    87,   215,   355,
     356,    89,   938,   939,   940,   332,   333,   357,   358,   225,
      92,    93,    94,   369,    95,   371,   520,   521,   650,   570,
     836,   778,   576,   309,   579,   656,   786,   565,   645,   719,
     722,   771,   526,   527,   528,   362,   363,   196,   197,   201,
     616,   691,   817,   948,   692,   693,   694,   695,   752,   696,
     170,   303,   304,    96,   123,   699,   822,    98,    99,   529,
     160,   161,   559,   643,   167,   292,   100,   477,   478,   101,
     660,   502,   661,   102,   103,   497,   104,   105,   630,   710,
     543,   106,   107,   108,   109,   110,   111,   112,   406,   426,
     427,   428,   113,   341,   342,   345,   346,   114,   336,   115,
     116,   117,   118,   436,   119,   277,   278,   437,   550,   765,
     279,   440,   638,   715,   714,   828,   768,   130,   131,   548,
     549,   441,   442,   851,   852,   892,   893,   283,   555,   447,
     745,   844,   748,   804,   805,   806,   886,   933,   913,   889,
     917,   749,   808,   750,   812,   859,   860,   861
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -570
static const yytype_int16 yypact[] =
{
    -570,    53,  1978,  -570,  6540,  6540,  -110,  6540,  6540,  6540,
    -570,  6540,  6540,  6540,  6540,  6540,  6540,  6540,  6540,  6540,
    6540,  6540,  6540,  6540,  6540,   587,   587,  6540,   595,   -48,
     -46,  -570,  -570,    71,  -570,  -570,  -570,  -570,  -570,  6540,
    4809,    65,    70,    95,   117,   119,  4958,  5090,   108,  -570,
     122,  5222,    46,  6540,   -24,   -44,   200,   222,    11,   183,
     198,   218,   242,  -570,  -570,   259,   260,  -570,  -570,  -570,
    -570,  -570,  -570,  -570,  -570,  -570,    96,  -570,  -570,   219,
    -570,  6540,  -570,  -570,   258,   151,  -570,    16,  -570,  -570,
    -570,  -570,  -570,  -570,    43,   360,  -570,   405,  -570,   293,
    -570,  -570,  6858,  -570,   276,  1364,   268,  -570,   295,   317,
    -570,     9,  -570,    63,  -570,  -570,  -570,  -570,   290,  -570,
    -570,   350,   307,   405,  7647,  7647,  6540,  7647,  7647,  7761,
    -570,   381,   396,   396,    72,   396,   396,   396,   396,   396,
     396,   396,   396,   396,   396,   396,  -570,   310,   219,   -25,
     323,  -570,  -570,   327,  -570,  -570,   318,   219,   325,   329,
     304,  -570,   334,   357,    66,    63,  5354,  -570,  6540,  -570,
      18,  7647,   397,  6540,  6540,  6540,   410,  6540,  -570,  6899,
    -570,  6960,   311,   475,  -570,   316,  7647,   388,  -570,  7005,
     219,   -18,    20,  -570,  -570,   487,    21,  -570,  -570,  -570,
     477,    24,   405,   587,   587,   587,   322,   568,  5486,   219,
    -570,   -60,   204,  -570,  7046,  2127,  6540,   426,  5618,   427,
    -570,  -570,   379,   384,  -570,   -34,    -8,  6540,  6540,  6540,
    5749,  6540,  6540,  6540,  6540,  6540,  6540,  6540,  6540,  6540,
    6540,  6540,  6540,  6540,  6540,  6540,  6540,  6540,  6540,  6540,
    6540,  6540,   595,  6540,  -570,  -570,  -570,  5881,  6540,  6540,
    6540,  6540,  6540,  6540,  6540,  6540,  6540,  6540,  6540,  5618,
      47,     8,  6540,  6540,   258,     3,  -570,   351,    60,    39,
     337,  7107,  -570,   171,   219,    81,    55,    68,   219,   325,
      69,  5618,  -570,    69,     8,     5,  -570,  7152,  7194,  6540,
    -570,   338,  7253,   340,   503,  7647,   428,   882,   507,    30,
    7298,  -570,  -570,  -570,   907,  -570,  -570,  2276,  -570,    -6,
     450,   -24,  -570,  6540,  -570,  -570,   -44,  -570,   907,   457,
    -570,   366,    35,  -570,  -570,  -570,    36,   367,   365,   369,
    -570,    38,  -570,   587,  7486,   371,   533,   215,  1680,  -570,
    -570,  5618,  -570,   385,  -570,  -570,  -570,  -570,  -570,  1246,
    -570,   587,   383,   545,    44,    87,   544,   379,   216,   440,
     216,   390,   398,   173,   399,   400,     5,    63,  7687,  7724,
    7761,  6540,  7592,  7797,  1398,  1666,  5011,  1526,  1630,  1630,
    1630,  1630,  1427,  1427,  1427,  1427,   482,   482,   417,   417,
     417,    72,    72,    72,  -570,  7647,   391,   266,  7761,  7761,
    7761,  7761,  7761,  7761,  7761,  7761,  7761,  7761,  7761,  7761,
     395,   402,   403,  -570,  6540,  -570,   404,    31,  -570,   406,
    1313,  -570,  -570,  -570,  -570,  -570,  -570,   290,  -570,   540,
    -570,   566,   508,  -570,  -570,   511,   514,  -570,   127,   399,
     402,   325,  -570,   418,  -570,  -570,  -570,  4064,  7647,  6540,
    4213,  6540,  6540,   587,   211,   907,   516,  4362,     1,   907,
     907,   425,   446,   219,   -61,   458,  -570,  -570,  -570,   502,
     535,  -570,  -570,  1595,  -570,  -570,   596,   173,   587,   439,
     587,  -570,  -570,  -570,   568,   568,   602,  -570,  -570,  6013,
    -570,  6145,  -570,  5618,  -570,  1829,   444,   445,  -570,  -570,
    -570,  6277,   907,   440,   467,   219,  -570,   325,  -570,   216,
     452,   612,  -570,  -570,   173,  -570,   459,   616,    42,  -570,
    5618,  5618,     5,  1484,  6540,  -570,   595,  -570,  -570,  5618,
    5618,  6690,  5618,  -570,  6540,  6540,  -570,  -570,   592,   381,
    6540,   351,  -570,   109,  -570,   229,  -570,  -570,  -570,   506,
    -570,  -570,  7340,  -570,  -570,  -570,   460,  7647,   509,   587,
     509,  -570,  -570,   624,  -570,  -570,  -570,   466,   471,  -570,
    -570,  -570,   907,   219,     2,   567,   480,  -570,  -570,   907,
     474,  -570,  -570,  -570,    50,  -570,  6540,   587,  7647,   587,
    7545,   476,  -570,  -570,   481,   587,    52,   142,  -570,   485,
     219,   325,  -570,   612,  -570,   216,   700,   492,   559,   181,
     594,   654,   500,   501,  1484,   304,   504,   505,  -570,   512,
     550,   519,  6731,  -570,   -74,  6772,  -570,  -570,  -570,   605,
     606,  -570,     8,  -570,  3915,   303,   513,  2425,  6540,   211,
     517,  -570,   521,   907,  2574,  -570,   210,  -570,    -4,   555,
     526,   691,    92,  -570,   216,  -570,   559,  -570,  7761,  -570,
    -570,  6409,  -570,  -570,  -570,  -570,   325,   977,  -570,   627,
    -570,  -570,  -570,  -570,  -570,  -570,  -570,  -570,  -570,  -570,
    -570,  -570,   628,   350,   641,  -570,    25,   530,   534,   532,
      49,   694,   907,  -570,  -570,  -570,  -570,  -570,  -570,     8,
    -570,  -570,  -570,  -570,  6540,   290,  -570,  -570,  -570,   308,
     537,  4809,  -570,  -570,   538,   541,  -570,  4511,  4511,  -570,
     539,   324,   552,  6540,    -1,   172,  -570,   907,  -570,   907,
    -570,   635,   549,   587,  7647,  -570,  -570,   701,   382,   671,
     256,   720,    26,  -570,  -570,   661,  -570,  -570,   121,  -570,
     662,   726,   907,  -570,   404,  -570,  7647,   351,   578,   589,
     732,   698,  6540,  -570,  -570,  4660,  -570,  -570,  -570,  -570,
    -570,   591,  -570,  6813,  -570,  -570,  -570,  -570,  -570,   644,
     600,  -570,  -570,   991,   907,  -570,  -570,  -570,  -570,  -570,
    -570,   607,  -570,  -570,    27,  -570,  -570,  -570,    28,  -570,
    -570,    89,   601,  -570,   907,   704,  -570,   405,   763,  2723,
     705,  -570,    56,  2872,   769,   907,  -570,  -570,   610,  -570,
    6540,  -570,   613,  7399,  -570,  -570,  -570,  3021,  -570,  -570,
    3915,   907,   614,  3170,   618,  -570,   714,  -570,   382,  -570,
     776,  -570,   508,   744,  -570,  -570,  -570,  -570,  -570,  -570,
      34,    73,  -570,  -570,   782,   723,   907,  -570,  -570,   202,
    -570,  -570,   907,  -570,  -570,  7444,  3915,  -570,  4809,  3319,
     626,  3915,  -570,  -570,  -570,  -570,   790,  -570,   932,   630,
     229,  -570,  -570,   381,  -570,    89,    89,   240,  -570,  -570,
    -570,   907,   632,  -570,   729,  -570,  -570,   775,  -570,   633,
    -570,  3468,  -570,   797,  -570,  -570,  -570,  -570,  -570,  -570,
     171,   778,  -570,  -570,  -570,  -570,  -570,   173,  -570,  -570,
    -570,   713,   790,   645,  -570,   650,  3915,   652,  -570,   713,
    -570,  -570,  -570,   221,   216,  -570,  -570,  -570,  -570,   751,
    3617,   655,  -570,   656,  -570,  3766,  -570
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -570,  -570,  -180,   -17,  -570,  -570,   518,  -570,  -181,  -570,
       0,  -570,  -570,  -570,  -111,  -570,   341,    10,    13,  -120,
    -570,  -570,  -570,   465,  -570,  -570,   320,   315,   267,   191,
    -570,   115,  -570,  -570,  -570,  -393,    61,  -570,  -570,  -570,
    -570,  -570,  -483,  -570,   226,  -212,  -570,  -570,   520,  -570,
    -569,  -570,  -570,  -570,  -570,  -570,  -570,   154,  -570,  -570,
    -570,  -439,  -570,    83,    -2,   184,  -570,  -570,    22,  -352,
    -233,  -570,  -570,  -570,  -570,   227,   271,   -47,  -570,  -570,
    -570,   190,  -570,   947,   664,  -344,   359,  1046,  -570,  -570,
     100,  -216,  -570,   193,  -570,   -22,   -14,  -570,  -249,  -274,
    -570,  -570,    23,   372,   370,  -570,  -570,  -570,  -570,  -570,
     590,  -570,  -570,  -570,  -570,  -416,  -570,  -570,  -570,  -570,
    -570,  -570,  -570,  -570,  -570,  -570,  -570,   -79,  -570,  -570,
    -570,    67,  -570,   -19,  -570,    29,  -570,  -536,    15,  -550,
    -570,  -570,  -570,  -570,    32,  -570,  -570,  -570,  -570,  -570,
    -570,  -570,  -570,  -570,  -570,   126,  -471,  -570
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -482
static const yytype_int16 yytable[] =
{
      97,   213,    88,   280,   590,   641,   163,   317,   149,   149,
     375,   158,    90,   634,   164,    91,   518,   509,   522,   404,
     455,   551,   566,   429,   219,   784,   299,   577,   321,   326,
     348,   194,   329,   755,   815,   848,   853,   191,   466,   372,
     172,   617,   895,   488,   490,   677,   495,   150,   150,   120,
     159,   165,  -135,     3,   422,   425,   202,   420,   495,   211,
    -138,   126,   212,   896,   869,   374,   272,    35,   272,   221,
     375,   422,   272,   620,   452,   320,   438,   454,   425,   453,
     760,   423,   331,    35,  -269,   439,   200,   480,   445,   446,
     217,   217,   733,   734,   544,  -136,    10,   169,   898,   275,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,    49,   349,   350,   899,   222,   621,   900,   202,
     421,   252,    35,   166,   761,   168,   217,   190,   449,   272,
      35,   285,   195,   217,   185,   253,   855,   373,    35,   506,
     289,   450,   367,    35,    35,   217,   218,  -271,  -325,  -325,
    -139,   295,   820,   217,  -337,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   522,    84,   120,
     505,   736,   785,   319,  -334,   578,   793,   273,   149,   273,
    -333,   182,   424,   273,    84,   658,   149,   149,   149,   220,
     149,   300,   347,   322,   327,   183,   821,   330,   756,   816,
     849,   854,   467,  -325,  -325,   545,   897,   489,   491,   725,
     496,  -337,   376,    97,   432,   433,  -135,   150,   153,   153,
     188,   162,   667,    84,  -138,   150,   150,   150,   870,   150,
     163,    84,   217,   904,   434,   158,   173,  -270,   164,   274,
     273,   174,   569,   217,    84,    84,   120,   209,   856,   377,
     857,   858,   351,   669,   120,   670,   376,   376,   306,  -136,
     811,   674,   731,   678,   735,   923,   175,   448,   733,   734,
     210,   451,   376,   376,   159,   165,   376,   905,   217,   376,
     376,   485,   924,   636,   120,   925,    35,   637,   176,   120,
     177,   601,   120,   377,   377,   631,  -269,   474,   503,   767,
     525,   364,   217,   625,   191,   732,   733,   734,   525,   377,
     377,   474,   741,   377,  -139,    97,   377,   377,   622,   623,
     198,   514,   218,   444,   515,   146,   149,   626,   627,   514,
     629,   536,   515,   445,   446,   516,   475,   255,   256,   120,
     918,    35,   199,   516,   149,  -121,    97,   787,    88,  -271,
     475,   517,   364,   517,   203,   217,   517,   920,    90,   147,
    -270,    91,   148,   532,   514,   150,   217,   515,   718,   204,
     934,   720,   721,    80,   364,   351,   769,   770,   516,   644,
     146,   444,   647,   150,   151,   154,   503,    84,   153,   205,
     149,   639,   640,   654,   946,   947,   153,   153,   153,   792,
     153,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   206,   147,   809,   810,   148,   572,   781,
     733,   734,   580,   581,   921,   922,   425,   811,    80,   150,
     207,   208,   216,   223,   364,   764,   224,   612,   226,   269,
     270,   271,    84,   276,   935,   162,   149,   149,   474,  -325,
    -325,    49,   474,   474,   282,   120,   584,   561,   209,   253,
     564,   284,   249,   250,   251,   608,   252,   575,   286,   288,
     517,   149,   287,   149,   290,   291,   217,   149,   149,   293,
     253,   294,   301,   308,   313,   150,   150,   475,   314,   315,
     328,   475,   475,   425,   338,   474,   146,   795,   611,   360,
     366,   368,   517,    97,   431,    88,   370,   517,   373,   459,
     150,   462,   150,   461,   163,    90,   150,   150,    91,   158,
     465,   463,   164,   481,   246,   247,   248,   249,   250,   251,
     156,   252,   486,   157,   475,   659,   153,   487,   493,   492,
     494,   501,   665,   500,   796,   253,   797,   798,   799,   800,
     801,   802,   149,   511,   153,   510,   507,   512,   159,   165,
     120,   316,    35,   519,   523,   474,   662,   538,   535,   524,
     530,   531,   474,   539,   540,   542,   819,   552,   823,   553,
     149,   554,   149,   546,   556,   476,   364,   557,   149,   573,
     558,   150,   949,   676,   606,   837,   582,   583,   517,   476,
     153,   146,   517,   585,   475,   840,   729,   586,   587,   589,
     843,   475,   592,   364,   364,   596,   603,   604,   610,   150,
     615,   150,   364,   364,   619,   364,   614,   150,   376,   633,
     642,   618,   149,   648,   649,   147,   474,   653,   148,   655,
     663,   120,    97,    35,   657,    97,   666,   517,   672,    80,
     876,   664,    97,   879,   673,   763,   153,   153,   881,   675,
     120,   323,    35,    84,   697,   377,   698,   702,   120,   701,
      35,   150,   703,   704,   709,   475,   706,   707,   716,   717,
     737,   153,   146,   153,   708,   474,   723,   153,   153,   727,
     788,   753,   789,   728,   339,   376,   711,   865,   738,   739,
     747,   146,   911,   751,   757,   758,   759,   762,   772,   146,
     790,   774,   780,   775,   794,   826,   147,   846,   807,   148,
     474,   773,   474,   791,   475,   782,   149,   777,   777,   162,
      80,   158,   377,   814,   818,   147,   476,   824,   148,   825,
     476,   476,  -161,   156,    84,   474,   157,   845,   936,    80,
     680,   681,   682,   683,   684,   685,   829,    80,   831,   475,
     830,   475,   153,    84,   838,   150,   950,   863,   832,   841,
     803,    84,   842,   955,   862,   835,   866,   474,   873,   864,
     868,   847,   872,   476,   475,   874,   877,  -481,   883,   888,
     153,   891,   153,   885,   882,   901,   902,   474,   153,   910,
     916,   929,   679,   927,   928,   932,   930,   896,   474,   680,
     681,   682,   683,   684,   685,   686,   475,    97,   937,   903,
     942,    97,   943,   944,   474,   906,   951,   953,   945,   591,
     954,   158,   513,   609,   613,    97,   475,   652,    97,   482,
     726,    97,   153,   779,   839,   700,   484,   475,   754,   474,
     742,   740,   705,   476,   926,   474,   687,   688,   689,   324,
     476,    31,    32,   475,   827,   595,   594,   890,   435,     0,
     803,    37,   919,   850,    97,   690,   813,    97,   908,    97,
     887,     0,     0,     0,   474,     0,     0,   894,   475,     0,
       0,     0,     0,     0,   475,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,     0,     0,    97,
     517,     0,     0,     0,     0,     0,     0,     0,    67,    68,
      69,    70,    71,   475,   476,     0,     0,   517,     0,     0,
       0,     0,    75,     0,    97,     0,   153,     0,     0,    77,
      78,     0,     0,  -325,  -325,     0,     0,     0,    97,   469,
     470,   124,   125,    97,   127,   128,   129,     0,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,     0,   476,   155,   464,     0,     0,    31,    32,
     120,     0,     0,     0,     0,     0,   171,     0,    37,     0,
       0,     0,     0,   179,   181,     0,     0,     0,   186,     0,
     189,     0,     0,    31,    32,   914,     0,     0,   476,     0,
     476,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,   146,     0,     0,     0,     0,     0,     0,   214,     0,
       0,     0,     0,   476,   471,    67,    68,    69,    70,    71,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
       0,     0,     0,     0,     0,   472,    77,    78,   473,     0,
      67,    68,    69,    70,    71,   476,     0,     0,     0,    80,
       0,   152,   152,   281,    75,     0,     0,     0,     0,   679,
       0,    77,    78,     0,     0,   476,   680,   681,   682,   683,
     684,   685,   686,   679,     0,     0,   476,   187,     0,     0,
     680,   681,   682,   683,   684,   685,   686,     0,     0,     0,
       0,     0,   476,   297,     0,   298,     0,     0,     0,     0,
     302,   305,   186,     0,   310,     0,     0,     0,     0,     0,
       0,     0,     0,   687,   688,   689,     0,   476,     0,     0,
       0,     0,     0,   476,     0,     0,     0,   687,   688,   689,
       0,     0,   746,     0,     0,   344,     0,   912,     0,   915,
       0,     0,     0,   359,     0,   186,  -432,     0,     0,     0,
       0,     0,   476,     0,   378,   379,   380,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,     0,
     405,     0,     0,   941,   408,   409,   410,   411,   412,   413,
     414,   415,   416,   417,   418,   419,   186,     0,     0,   405,
     430,   307,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   186,     0,
       0,   325,     0,     0,     0,     0,   458,     0,     0,   334,
     335,   337,     0,   340,     0,   227,   228,   229,     0,     0,
       0,     0,     0,     0,   365,     0,     0,     0,     0,     0,
     483,   230,     0,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,     0,   252,     0,     0,   186,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   253,
       0,     0,     0,     0,     0,   365,     0,     0,     0,     0,
       0,     0,   227,   228,   229,     0,     0,     0,   533,     0,
       0,     0,     0,     0,     0,     0,     0,   365,   230,     0,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,     0,   252,     0,     0,     0,     0,     0,     0,     0,
       0,   541,     0,     0,     0,     0,   253,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   498,
       0,     0,     0,     0,     0,     0,     0,   365,     0,     0,
       0,     0,     0,     0,     0,     0,   562,   498,   305,   567,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   508,     0,     0,     0,  -325,  -325,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   598,   252,   600,     0,
     186,     0,     0,   537,     0,     0,     0,     0,   186,     0,
       0,   253,     0,  -482,  -482,  -482,  -482,   244,   245,   246,
     247,   248,   249,   250,   251,     0,   252,   186,   186,     0,
       0,   624,     0,     0,     0,     0,   186,   186,   547,   186,
     253,   405,   632,     0,     0,     0,     0,   635,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   568,
     571,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,     0,   252,   334,     0,   593,     0,     0,     0,
     340,   340,     0,   668,     0,     0,     0,   253,     0,   365,
       0,     0,     0,     0,     0,     0,     0,   607,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,     0,   252,   365,   365,     0,     0,
       0,     0,     0,     0,     0,   365,   365,     0,   365,   253,
       0,     0,     0,     0,     0,   305,     0,     0,     0,     0,
       0,     0,     0,     0,   227,   228,   229,     0,     0,     0,
       0,     0,     0,     0,     0,   651,     0,     0,   744,     0,
     230,     0,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   498,   252,   498,     0,     0,     0,     0,
       0,   498,     0,     0,     0,     0,     0,     0,   253,     0,
       0,   766,  -482,  -482,  -482,  -482,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,     0,   252,
     783,     0,     0,     4,     5,     6,     7,     8,     0,     0,
       0,     0,     9,   253,     0,   571,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,     0,   252,    10,     0,     0,   833,
       0,     0,    11,    12,     0,     0,     0,     0,    13,   253,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,     0,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,    41,     0,    42,     0,    43,
     588,    44,     0,     0,    45,     0,     0,   875,    46,    47,
      48,    49,    50,    51,    52,     0,    53,    54,    55,   498,
       0,     0,    56,    57,    58,     0,    59,    60,    61,    62,
      63,    64,     0,     0,     0,     0,    65,    66,    67,    68,
      69,    70,    71,     0,     0,    72,    73,    74,     0,     0,
       0,     0,    75,     0,     0,     0,     0,     0,    76,    77,
      78,    79,     4,     5,     6,     7,     8,     0,     0,     0,
       0,     9,    80,     0,     0,     0,     0,     0,     0,     0,
       0,    81,     0,    82,    83,   504,    84,     0,     0,     0,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,    13,     0,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,     0,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,    41,     0,    42,     0,    43,     0,
      44,     0,     0,    45,     0,     0,     0,    46,    47,    48,
      49,    50,    51,    52,     0,    53,    54,    55,     0,     0,
       0,    56,    57,    58,     0,    59,    60,    61,    62,    63,
      64,     0,     0,     0,     0,    65,    66,    67,    68,    69,
      70,    71,     0,     0,    72,    73,    74,     0,     0,     0,
       0,    75,     0,     0,     0,     0,     0,    76,    77,    78,
      79,     4,     5,     6,     7,     8,     0,     0,     0,     0,
       9,    80,     0,     0,     0,     0,     0,     0,     0,     0,
      81,     0,    82,    83,   602,    84,     0,     0,     0,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,    13,     0,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,     0,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,    41,     0,    42,     0,    43,     0,    44,
       0,     0,    45,     0,     0,     0,    46,    47,    48,    49,
      50,    51,    52,     0,    53,    54,    55,     0,     0,     0,
      56,    57,    58,     0,    59,    60,    61,    62,    63,    64,
       0,     0,     0,     0,    65,    66,    67,    68,    69,    70,
      71,     0,     0,    72,    73,    74,     0,     0,     0,     0,
      75,     0,     0,     0,     0,     0,    76,    77,    78,    79,
       4,     5,     6,     7,     8,     0,     0,     0,     0,     9,
      80,     0,     0,     0,     0,     0,     0,     0,     0,    81,
       0,    82,    83,     0,    84,     0,     0,     0,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,    13,     0,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
       0,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,    41,     0,    42,     0,    43,     0,    44,     0,
       0,    45,     0,     0,     0,    46,    47,    48,    49,     0,
      51,    52,     0,    53,     0,    55,     0,     0,     0,    56,
      57,    58,     0,    59,    60,    61,   353,    63,    64,     0,
       0,     0,     0,    65,    66,    67,    68,    69,    70,    71,
       0,     0,    72,    73,    74,     0,     0,     0,     0,    75,
       0,     0,     0,     0,     0,   122,    77,    78,    79,     4,
       5,     6,     7,     8,     0,     0,     0,     0,     9,    80,
       0,     0,     0,     0,     0,     0,     0,     0,    81,     0,
      82,    83,   354,    84,     0,     0,     0,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,    13,     0,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,     0,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,    41,     0,    42,     0,    43,     0,    44,     0,     0,
      45,     0,     0,     0,    46,    47,    48,    49,     0,    51,
      52,     0,    53,     0,    55,     0,     0,     0,    56,    57,
      58,     0,    59,    60,    61,   353,    63,    64,     0,     0,
       0,     0,    65,    66,    67,    68,    69,    70,    71,     0,
       0,    72,    73,    74,     0,     0,     0,     0,    75,     0,
       0,     0,     0,     0,   122,    77,    78,    79,     4,     5,
       6,     7,     8,     0,     0,     0,     0,     9,    80,     0,
       0,     0,     0,     0,     0,     0,     0,    81,     0,    82,
      83,   479,    84,     0,     0,     0,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,    13,     0,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,     0,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
      41,   724,    42,     0,    43,     0,    44,     0,     0,    45,
       0,     0,     0,    46,    47,    48,    49,     0,    51,    52,
       0,    53,     0,    55,     0,     0,     0,    56,    57,    58,
       0,    59,    60,    61,   353,    63,    64,     0,     0,     0,
       0,    65,    66,    67,    68,    69,    70,    71,     0,     0,
      72,    73,    74,     0,     0,     0,     0,    75,     0,     0,
       0,     0,     0,   122,    77,    78,    79,     4,     5,     6,
       7,     8,     0,     0,     0,     0,     9,    80,     0,     0,
       0,     0,     0,     0,     0,     0,    81,     0,    82,    83,
       0,    84,     0,     0,     0,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,    13,     0,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,     0,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
       0,    36,     0,     0,     0,    37,    38,    39,    40,    41,
       0,    42,     0,    43,     0,    44,   730,     0,    45,     0,
       0,     0,    46,    47,    48,    49,     0,    51,    52,     0,
      53,     0,    55,     0,     0,     0,    56,    57,    58,     0,
      59,    60,    61,   353,    63,    64,     0,     0,     0,     0,
      65,    66,    67,    68,    69,    70,    71,     0,     0,    72,
      73,    74,     0,     0,     0,     0,    75,     0,     0,     0,
       0,     0,   122,    77,    78,    79,     4,     5,     6,     7,
       8,     0,     0,     0,     0,     9,    80,     0,     0,     0,
       0,     0,     0,     0,     0,    81,     0,    82,    83,     0,
      84,     0,     0,     0,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,    13,     0,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,     0,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,     0,     0,     0,    37,    38,    39,    40,    41,     0,
      42,     0,    43,     0,    44,     0,     0,    45,     0,     0,
       0,    46,    47,    48,    49,     0,    51,    52,     0,    53,
       0,    55,     0,     0,     0,    56,    57,    58,     0,    59,
      60,    61,   353,    63,    64,     0,     0,     0,     0,    65,
      66,    67,    68,    69,    70,    71,     0,     0,    72,    73,
      74,     0,     0,     0,     0,    75,     0,     0,     0,     0,
       0,   122,    77,    78,    79,     4,     5,     6,     7,     8,
       0,     0,     0,     0,     9,    80,     0,     0,     0,     0,
       0,     0,     0,     0,    81,     0,    82,    83,   867,    84,
       0,     0,     0,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
      13,     0,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,     0,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,     0,    36,
       0,     0,     0,    37,    38,    39,    40,    41,     0,    42,
       0,    43,     0,    44,     0,     0,    45,     0,     0,     0,
      46,    47,    48,    49,     0,    51,    52,     0,    53,     0,
      55,     0,     0,     0,    56,    57,    58,     0,    59,    60,
      61,   353,    63,    64,     0,     0,     0,     0,    65,    66,
      67,    68,    69,    70,    71,     0,     0,    72,    73,    74,
       0,     0,     0,     0,    75,     0,     0,     0,     0,     0,
     122,    77,    78,    79,     4,     5,     6,     7,     8,     0,
       0,     0,     0,     9,    80,     0,     0,     0,     0,     0,
       0,     0,     0,    81,     0,    82,    83,   871,    84,     0,
       0,     0,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,    13,
       0,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,     0,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,    41,     0,    42,     0,
      43,   880,    44,     0,     0,    45,     0,     0,     0,    46,
      47,    48,    49,     0,    51,    52,     0,    53,     0,    55,
       0,     0,     0,    56,    57,    58,     0,    59,    60,    61,
     353,    63,    64,     0,     0,     0,     0,    65,    66,    67,
      68,    69,    70,    71,     0,     0,    72,    73,    74,     0,
       0,     0,     0,    75,     0,     0,     0,     0,     0,   122,
      77,    78,    79,     4,     5,     6,     7,     8,     0,     0,
       0,     0,     9,    80,     0,     0,     0,     0,     0,     0,
       0,     0,    81,     0,    82,    83,     0,    84,     0,     0,
       0,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,    13,     0,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,     0,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,    41,     0,    42,     0,    43,
       0,    44,     0,     0,    45,     0,     0,     0,    46,    47,
      48,    49,     0,    51,    52,     0,    53,     0,    55,     0,
       0,     0,    56,    57,    58,     0,    59,    60,    61,   353,
      63,    64,     0,     0,     0,     0,    65,    66,    67,    68,
      69,    70,    71,     0,     0,    72,    73,    74,     0,     0,
       0,     0,    75,     0,     0,     0,     0,     0,   122,    77,
      78,    79,     4,     5,     6,     7,     8,     0,     0,     0,
       0,     9,    80,     0,     0,     0,     0,     0,     0,     0,
       0,    81,     0,    82,    83,   884,    84,     0,     0,     0,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,    13,     0,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,     0,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,    41,     0,    42,   909,    43,     0,
      44,     0,     0,    45,     0,     0,     0,    46,    47,    48,
      49,     0,    51,    52,     0,    53,     0,    55,     0,     0,
       0,    56,    57,    58,     0,    59,    60,    61,   353,    63,
      64,     0,     0,     0,     0,    65,    66,    67,    68,    69,
      70,    71,     0,     0,    72,    73,    74,     0,     0,     0,
       0,    75,     0,     0,     0,     0,     0,   122,    77,    78,
      79,     4,     5,     6,     7,     8,     0,     0,     0,     0,
       9,    80,     0,     0,     0,     0,     0,     0,     0,     0,
      81,     0,    82,    83,     0,    84,     0,     0,     0,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,    13,     0,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,     0,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,    41,     0,    42,     0,    43,     0,    44,
       0,     0,    45,     0,     0,     0,    46,    47,    48,    49,
       0,    51,    52,     0,    53,     0,    55,     0,     0,     0,
      56,    57,    58,     0,    59,    60,    61,   353,    63,    64,
       0,     0,     0,     0,    65,    66,    67,    68,    69,    70,
      71,     0,     0,    72,    73,    74,     0,     0,     0,     0,
      75,     0,     0,     0,     0,     0,   122,    77,    78,    79,
       4,     5,     6,     7,     8,     0,     0,     0,     0,     9,
      80,     0,     0,     0,     0,     0,     0,     0,     0,    81,
       0,    82,    83,   931,    84,     0,     0,     0,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,    13,     0,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
       0,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,    41,     0,    42,     0,    43,     0,    44,     0,
       0,    45,     0,     0,     0,    46,    47,    48,    49,     0,
      51,    52,     0,    53,     0,    55,     0,     0,     0,    56,
      57,    58,     0,    59,    60,    61,   353,    63,    64,     0,
       0,     0,     0,    65,    66,    67,    68,    69,    70,    71,
       0,     0,    72,    73,    74,     0,     0,     0,     0,    75,
       0,     0,     0,     0,     0,   122,    77,    78,    79,     4,
       5,     6,     7,     8,     0,     0,     0,     0,     9,    80,
       0,     0,     0,     0,     0,     0,     0,     0,    81,     0,
      82,    83,   952,    84,     0,     0,     0,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,    13,     0,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,     0,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,    41,     0,    42,     0,    43,     0,    44,     0,     0,
      45,     0,     0,     0,    46,    47,    48,    49,     0,    51,
      52,     0,    53,     0,    55,     0,     0,     0,    56,    57,
      58,     0,    59,    60,    61,   353,    63,    64,     0,     0,
       0,     0,    65,    66,    67,    68,    69,    70,    71,     0,
       0,    72,    73,    74,     0,     0,     0,     0,    75,     0,
       0,     0,     0,     0,   122,    77,    78,    79,     4,     5,
       6,     7,     8,     0,     0,     0,     0,     9,    80,     0,
       0,     0,     0,     0,     0,     0,     0,    81,     0,    82,
      83,   956,    84,     0,     0,     0,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,    13,     0,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,     0,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
      41,     0,    42,     0,    43,     0,    44,     0,     0,    45,
       0,     0,     0,    46,    47,    48,    49,     0,    51,    52,
       0,    53,     0,    55,     0,     0,     0,    56,    57,    58,
       0,    59,    60,    61,   353,    63,    64,     0,     0,     0,
       0,    65,    66,    67,    68,    69,    70,    71,     0,     0,
      72,    73,    74,     0,     0,     0,     0,    75,     0,     0,
       0,     0,     0,   122,    77,    78,    79,     4,     5,     6,
       7,     8,     0,     0,     0,     0,     9,    80,     0,     0,
       0,     0,     0,     0,     0,     0,    81,     0,    82,    83,
     560,    84,     0,     0,     0,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,    13,     0,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,     0,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
       0,    36,     0,     0,     0,    37,    38,    39,    40,    41,
       0,    42,     0,    43,     0,    44,     0,     0,    45,     0,
       0,     0,    46,    47,    48,    49,     0,    51,    52,     0,
      53,     0,    55,     0,     0,     0,     0,     0,    58,     0,
      59,    60,    61,     0,     0,     0,     0,     0,     0,     0,
      65,    66,    67,    68,    69,    70,    71,     0,     0,    72,
      73,    74,     0,     0,     0,     0,    75,     0,     0,     0,
       0,     0,   122,    77,    78,    79,     4,     5,     6,     7,
       8,     0,     0,     0,     0,     9,    80,     0,     0,     0,
       0,     0,     0,     0,     0,    81,     0,    82,    83,   563,
      84,     0,     0,     0,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,    13,     0,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,     0,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,     0,     0,     0,    37,    38,    39,    40,    41,     0,
      42,     0,    43,     0,    44,     0,     0,    45,     0,     0,
       0,    46,    47,    48,    49,     0,    51,    52,     0,    53,
       0,    55,     0,     0,     0,     0,     0,    58,     0,    59,
      60,    61,     0,     0,     0,     0,     0,     0,     0,    65,
      66,    67,    68,    69,    70,    71,     0,     0,    72,    73,
      74,     0,     0,     0,     0,    75,     0,     0,     0,     0,
       0,   122,    77,    78,    79,     4,     5,     6,     7,     8,
       0,     0,     0,     0,     9,    80,     0,     0,     0,     0,
       0,     0,     0,     0,    81,     0,    82,    83,   574,    84,
       0,     0,     0,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
      13,     0,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,     0,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,     0,    36,
       0,     0,     0,    37,    38,    39,    40,    41,     0,    42,
       0,    43,     0,    44,     0,     0,    45,     0,     0,     0,
      46,    47,    48,    49,     0,    51,    52,     0,    53,     0,
      55,     0,     0,     0,     0,     0,    58,     0,    59,    60,
      61,     0,     0,     0,     0,     0,     0,     0,    65,    66,
      67,    68,    69,    70,    71,     0,     0,    72,    73,    74,
       0,     0,     0,     0,    75,     0,     0,     0,     0,     0,
     122,    77,    78,    79,     4,     5,     6,     7,     8,     0,
       0,     0,     0,     9,    80,     0,     0,     0,     0,     0,
       0,     0,     0,    81,     0,    82,    83,   776,    84,     0,
       0,     0,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,    13,
       0,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,     0,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,    41,     0,    42,     0,
      43,     0,    44,     0,     0,    45,     0,     0,     0,    46,
      47,    48,    49,     0,    51,    52,     0,    53,     0,    55,
       0,     0,     0,     0,     0,    58,     0,    59,    60,    61,
       0,     0,     0,     0,     0,     0,     0,    65,    66,    67,
      68,    69,    70,    71,     0,     0,    72,    73,    74,     0,
       0,     0,     0,    75,     0,     0,     0,     0,     0,   122,
      77,    78,    79,     4,     5,     6,     7,     8,     0,     0,
       0,     0,     9,    80,     0,     0,     0,     0,     0,     0,
       0,     0,    81,     0,    82,    83,   834,    84,     0,     0,
       0,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,    13,     0,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,     0,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,    41,     0,    42,     0,    43,
       0,    44,     0,     0,    45,     0,     0,     0,    46,    47,
      48,    49,     0,    51,    52,     0,    53,     0,    55,     0,
       0,     0,     0,     0,    58,     0,    59,    60,    61,     0,
       0,     0,     0,     0,     0,     0,    65,    66,    67,    68,
      69,    70,    71,     0,     0,    72,    73,    74,     0,     0,
       0,     0,    75,     0,     0,     0,     0,     0,   122,    77,
      78,    79,     4,     5,     6,     7,     8,     0,     0,     0,
       0,     9,    80,     0,     0,     0,     0,     0,     0,     0,
       0,    81,     0,    82,    83,     0,    84,     0,     0,     0,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,    13,     0,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,     0,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,    41,     0,    42,     0,    43,     0,
      44,     0,     0,    45,     0,     0,     0,    46,    47,    48,
      49,     0,    51,    52,     0,    53,     0,    55,     0,     0,
       0,     0,     0,    58,     0,    59,    60,    61,     0,     0,
       0,     0,     0,     0,     0,    65,    66,    67,    68,    69,
      70,    71,     0,     0,    72,    73,    74,     0,     0,     0,
       0,    75,     0,     0,     0,     0,     0,   122,    77,    78,
      79,     4,     5,     6,     7,     8,     0,     0,     0,     0,
       9,    80,     0,     0,     0,     0,     0,     0,     0,     0,
      81,     0,    82,    83,     0,    84,     0,     0,     0,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,    13,     0,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,     0,    27,    28,    29,     0,     0,     0,     0,    31,
      32,   120,    34,    35,     0,     0,     0,     0,     0,    37,
      38,     0,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,    49,
     252,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   121,     0,   253,    60,    61,     0,     0,     0,
       0,     0,     0,     0,    65,    66,    67,    68,    69,    70,
      71,     0,     0,     4,     5,     6,     7,     8,     0,     0,
      75,     0,     9,     0,     0,     0,   122,    77,    78,    79,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      80,     0,     0,     0,     0,     0,    10,     0,     0,    81,
       0,   178,    11,    12,    84,     0,     0,     0,    13,     0,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,     0,    27,    28,    29,     0,     0,     0,
       0,    31,    32,   120,    34,    35,     0,     0,     0,     0,
       0,    37,    38,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    49,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   121,     0,     0,    60,    61,     0,
       0,     0,     0,     0,     0,     0,    65,    66,    67,    68,
      69,    70,    71,     0,     0,     4,     5,     6,     7,     8,
       0,     0,    75,     0,     9,     0,     0,     0,   122,    77,
      78,    79,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    80,     0,     0,     0,     0,     0,    10,     0,
       0,    81,     0,   180,    11,    12,    84,     0,     0,     0,
      13,     0,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,     0,    27,    28,    29,     0,
       0,     0,     0,    31,    32,   120,    34,    35,     0,     0,
       0,     0,     0,    37,    38,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    49,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   121,     0,     0,    60,
      61,     0,     0,     0,     0,     0,     0,     0,    65,    66,
      67,    68,    69,    70,    71,     0,     0,     4,     5,     6,
       7,     8,     0,     0,    75,     0,     9,     0,     0,     0,
     122,    77,    78,    79,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,     0,     0,
      10,     0,     0,    81,     0,   184,    11,    12,    84,     0,
       0,     0,    13,     0,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,     0,    27,    28,
      29,     0,     0,     0,     0,    31,    32,   120,    34,    35,
       0,     0,     0,     0,     0,    37,    38,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   121,     0,
       0,    60,    61,     0,     0,     0,     0,     0,     0,     0,
      65,    66,    67,    68,    69,    70,    71,     0,     0,     4,
       5,     6,     7,     8,     0,     0,    75,     0,     9,     0,
       0,     0,   122,    77,    78,    79,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    80,   343,     0,     0,
       0,     0,    10,     0,     0,    81,   296,     0,    11,    12,
      84,     0,     0,     0,    13,     0,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,     0,
      27,    28,    29,     0,     0,     0,     0,    31,    32,   120,
      34,    35,     0,     0,     0,     0,     0,    37,    38,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    49,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     121,     0,     0,    60,    61,     0,     0,     0,     0,     0,
       0,     0,    65,    66,    67,    68,    69,    70,    71,     0,
       0,     4,     5,     6,     7,     8,     0,     0,    75,     0,
       9,     0,     0,     0,   122,    77,    78,    79,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    80,   361,
       0,     0,     0,     0,    10,     0,     0,    81,     0,     0,
      11,    12,    84,     0,     0,     0,    13,     0,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,     0,    27,    28,    29,     0,     0,     0,     0,    31,
      32,   120,    34,    35,     0,     0,     0,     0,     0,    37,
      38,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   121,     0,     0,    60,    61,     0,     0,     0,
       0,     0,     0,     0,    65,    66,    67,    68,    69,    70,
      71,     0,     4,     5,     6,     7,     8,     0,     0,     0,
      75,     9,     0,     0,     0,     0,   122,    77,    78,    79,
       0,     0,     0,     0,     0,   381,     0,     0,     0,     0,
      80,     0,     0,     0,     0,    10,     0,     0,     0,    81,
       0,    11,    12,     0,    84,     0,     0,    13,     0,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,     0,    27,    28,    29,     0,     0,     0,     0,
      31,    32,   120,    34,    35,     0,     0,     0,     0,     0,
      37,    38,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   121,     0,     0,    60,    61,     0,     0,
       0,     0,     0,     0,     0,    65,    66,    67,    68,    69,
      70,    71,     0,     0,     4,     5,     6,     7,     8,     0,
       0,    75,     0,     9,     0,     0,     0,   122,    77,    78,
      79,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    80,   407,     0,     0,     0,     0,    10,     0,     0,
      81,     0,     0,    11,    12,    84,     0,     0,     0,    13,
       0,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,     0,    27,    28,    29,     0,     0,
       0,     0,    31,    32,   120,    34,    35,     0,     0,     0,
       0,     0,    37,    38,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    49,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   121,     0,     0,    60,    61,
       0,     0,     0,     0,     0,     0,     0,    65,    66,    67,
      68,    69,    70,    71,     0,     0,     4,     5,     6,     7,
       8,     0,     0,    75,     0,     9,     0,     0,     0,   122,
      77,    78,    79,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    80,   597,     0,     0,     0,     0,    10,
       0,     0,    81,     0,     0,    11,    12,    84,     0,     0,
       0,    13,     0,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,     0,    27,    28,    29,
       0,     0,     0,     0,    31,    32,   120,    34,    35,     0,
       0,     0,     0,     0,    37,    38,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   121,     0,     0,
      60,    61,     0,     0,     0,     0,     0,     0,     0,    65,
      66,    67,    68,    69,    70,    71,     0,     0,     4,     5,
       6,     7,     8,     0,     0,    75,     0,     9,     0,     0,
       0,   122,    77,    78,    79,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    80,   599,     0,     0,     0,
       0,    10,     0,     0,    81,     0,     0,    11,    12,    84,
       0,     0,     0,    13,     0,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,     0,    27,
      28,    29,     0,     0,     0,     0,    31,    32,   120,    34,
      35,     0,     0,     0,     0,     0,    37,    38,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    49,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   121,
       0,     0,    60,    61,     0,     0,     0,     0,     0,     0,
       0,    65,    66,    67,    68,    69,    70,    71,     0,     0,
       4,     5,     6,     7,     8,     0,     0,    75,     0,     9,
       0,     0,     0,   122,    77,    78,    79,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    80,   605,     0,
       0,     0,     0,    10,     0,     0,    81,     0,     0,    11,
      12,    84,     0,     0,     0,    13,     0,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
       0,    27,    28,    29,     0,     0,     0,     0,    31,    32,
     120,    34,    35,     0,     0,     0,     0,     0,    37,    38,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    49,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   121,     0,     0,    60,    61,     0,     0,     0,     0,
       0,     0,     0,    65,    66,    67,    68,    69,    70,    71,
       0,     0,     4,     5,     6,     7,     8,     0,     0,    75,
       0,     9,     0,     0,     0,   122,    77,    78,    79,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    80,
     743,     0,     0,     0,     0,    10,     0,     0,    81,     0,
       0,    11,    12,    84,     0,     0,     0,    13,     0,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,     0,    27,    28,    29,     0,     0,     0,     0,
      31,    32,   120,    34,    35,     0,     0,     0,     0,     0,
      37,    38,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   121,     0,     0,    60,    61,     0,     0,
       0,     0,     0,     0,     0,    65,    66,    67,    68,    69,
      70,    71,     0,     4,     5,     6,     7,     8,     0,     0,
       0,    75,     9,     0,     0,     0,     0,   122,    77,    78,
      79,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,    10,     0,     0,     0,
      81,     0,    11,    12,     0,    84,     0,     0,    13,     0,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,     0,    27,    28,    29,     0,     0,     0,
       0,    31,    32,   120,    34,    35,     0,     0,     0,     0,
       0,    37,    38,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    49,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   121,     0,     0,    60,    61,     0,
       0,     0,     0,     0,     0,     0,    65,    66,    67,    68,
      69,    70,    71,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,     0,     0,   122,    77,
      78,    79,     0,     0,     0,     0,     0,     0,     0,   227,
     228,   229,    80,     0,     0,     0,     0,     0,     0,     0,
       0,    81,     0,     0,     0,   230,    84,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,     0,   252,
     227,   228,   229,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   253,     0,     0,   230,     0,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,     0,
     252,   227,   228,   229,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   253,     0,     0,   230,     0,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
       0,   252,   227,   228,   229,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   253,     0,     0,   230,   784,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,     0,   252,     0,     0,   628,     0,   227,   228,   229,
       0,     0,     0,     0,     0,     0,   253,     0,     0,     0,
       0,     0,     0,   230,     0,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   712,   252,   227,   228,
     229,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   253,     0,     0,   230,     0,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   713,   252,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   253,     0,     0,     0,     0,     0,     0,   227,
     228,   229,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   230,   785,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,     0,   252,
       0,     0,     0,     0,   227,   228,   229,     0,     0,     0,
       0,     0,     0,   253,     0,     0,     0,     0,     0,     0,
     230,   254,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,     0,   252,   227,   228,   229,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   253,     0,
       0,   230,   311,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,     0,   252,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   253,
       0,     0,     0,     0,     0,     0,   227,   228,   229,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   230,   312,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,     0,   252,     0,     0,     0,
       0,   227,   228,   229,     0,     0,     0,     0,     0,     0,
     253,     0,     0,     0,     0,     0,     0,   230,   318,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
       0,   252,     0,   227,   228,   229,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   253,     0,     0,   352,   230,
       0,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,     0,   252,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   253,     0,     0,
       0,     0,   227,   228,   229,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   230,   443,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,     0,   252,     0,     0,     0,     0,   227,   228,   229,
       0,     0,     0,     0,     0,     0,   253,     0,     0,     0,
       0,     0,     0,   230,   456,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,     0,   252,     0,   227,
     228,   229,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   253,     0,     0,     0,   230,   457,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,     0,   252,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   253,     0,     0,     0,     0,   227,   228,
     229,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   230,   460,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,     0,   252,     0,
       0,     0,     0,   227,   228,   229,     0,     0,     0,     0,
       0,     0,   253,     0,     0,     0,     0,     0,     0,   230,
     468,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,     0,   252,     0,   227,   228,   229,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   253,     0,     0,
       0,   230,   646,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,     0,   252,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   253,
       0,     0,     0,     0,   227,   228,   229,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     230,   878,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   251,     0,   252,     0,     0,     0,     0,     0,
       0,   227,   228,   229,     0,     0,     0,     0,   253,     0,
       0,   499,     0,     0,     0,     0,   907,   230,   534,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
       0,   252,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   253,   227,   228,   229,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     671,     0,   230,     0,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,     0,   252,   228,   229,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     253,     0,   230,     0,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   229,   252,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   230,
     253,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,   242,   243,   244,   245,   246,   247,   248,   249,
     250,   251,     0,   252,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   230,   253,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,     0,
     252,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   253,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,     0,   252,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     253
};

static const yytype_int16 yycheck[] =
{
       2,    80,     2,   123,   487,   555,    28,   188,    25,    26,
     226,    28,     2,   549,    28,     2,   368,   361,   370,   252,
     294,   437,   461,   272,     8,    26,     8,    26,     8,     8,
     210,    75,     8,     8,     8,     8,     8,    54,     8,    73,
      40,   524,     8,     8,     8,   614,     8,    25,    26,    73,
      28,    28,     8,     0,   270,   271,    58,   269,     8,    76,
       8,   171,    79,    29,     8,    73,    63,    75,    63,    26,
     286,   287,    63,    31,   290,    93,    37,   293,   294,   291,
      31,    73,   202,    75,   145,    46,    75,    93,   162,   163,
     151,   151,    96,    97,    63,     8,    36,    26,    25,   113,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,   101,   173,   174,    42,    73,    75,    45,   121,
      73,    49,    75,   171,    75,   171,   151,   151,    73,    63,
      75,   148,   176,   151,    51,    63,    47,   171,    75,   351,
     157,    73,   221,    75,    75,   151,   171,   145,    61,    62,
       8,   165,    31,   151,   145,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,   519,   176,    73,
     350,   175,   173,   190,   171,   174,   745,   174,   195,   174,
     171,    73,   174,   174,   176,   578,   203,   204,   205,   173,
     207,   173,   209,   173,   173,    73,    75,   173,   173,   173,
     173,   173,   172,    61,    62,   174,   172,   172,   172,   648,
     172,   145,   226,   215,   154,   155,   172,   195,    25,    26,
     174,    28,   172,   176,   172,   203,   204,   205,   172,   207,
     252,   176,   151,    31,   174,   252,   171,   145,   252,   176,
     174,   171,    31,   151,   176,   176,    73,   151,   159,   226,
     161,   162,   171,   597,    73,   599,   270,   271,   175,   172,
     171,   605,   655,   615,   657,    25,   171,   284,    96,    97,
     174,   288,   286,   287,   252,   252,   290,    75,   151,   293,
     294,   328,    42,   174,    73,    45,    75,   178,   171,    73,
     171,   503,    73,   270,   271,   544,   145,   314,   171,   715,
     127,   218,   151,   536,   321,    95,    96,    97,   127,   286,
     287,   328,   664,   290,   172,   317,   293,   294,   530,   531,
     120,   148,   171,   152,   151,   114,   343,   539,   540,   148,
     542,    65,   151,   162,   163,   162,   314,    61,    62,    73,
     890,    75,   120,   162,   361,   172,   348,   175,   348,   145,
     328,   368,   269,   370,   171,   151,   373,   893,   348,   148,
     145,   348,   151,   377,   148,   343,   151,   151,   642,   171,
     920,    68,    69,   162,   291,   171,    68,    69,   162,   560,
     114,   152,   563,   361,    25,    26,   171,   176,   195,   171,
     407,   162,   163,   574,   173,   174,   203,   204,   205,   743,
     207,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,   171,   148,   159,   160,   151,   465,    95,
      96,    97,   469,   470,   895,   896,   642,   171,   162,   407,
     171,   171,   174,    73,   351,   709,    31,   516,   145,   171,
     145,   124,   176,   153,   927,   252,   463,   464,   465,    61,
      62,   101,   469,   470,    73,    73,   473,   457,   151,    63,
     460,   151,    45,    46,    47,   512,    49,   467,   145,   151,
     487,   488,   145,   490,   145,   171,   151,   494,   495,   145,
      63,   124,    85,    73,   173,   463,   464,   465,    13,   173,
      13,   469,   470,   709,   172,   512,   114,   115,   515,    73,
      73,   122,   519,   505,   153,   505,   122,   524,   171,   171,
     488,     8,   490,   173,   536,   505,   494,   495,   505,   536,
      13,    93,   536,    73,    42,    43,    44,    45,    46,    47,
     148,    49,    75,   151,   512,   582,   343,   171,   173,   172,
     171,     8,   589,   172,   162,    63,   164,   165,   166,   167,
     168,   169,   569,     8,   361,   172,   171,    13,   536,   536,
      73,   173,    75,   123,   174,   582,   583,   172,   177,   171,
     171,   171,   589,   171,   171,   171,   757,    37,   759,    13,
     597,    73,   599,   177,    73,   314,   503,    73,   605,    73,
     172,   569,   944,   610,   511,   776,   171,   151,   615,   328,
     407,   114,   619,   145,   582,   786,   653,   105,    73,    13,
     791,   589,   173,   530,   531,    13,   172,   172,   151,   597,
       8,   599,   539,   540,     8,   542,   174,   605,   642,    37,
     124,   172,   649,   173,   125,   148,   653,    13,   151,   173,
      73,    73,   644,    75,   173,   647,   172,   664,   172,   162,
     831,   171,   654,   834,   173,   702,   463,   464,   839,   174,
      73,   174,    75,   176,   172,   642,   107,    13,    73,    75,
      75,   649,   172,   172,   124,   653,   172,   172,    73,    73,
     125,   488,   114,   490,   172,   702,   173,   494,   495,   172,
     737,   693,   739,   172,   126,   709,   177,   817,   172,     8,
      73,   114,   883,    75,   174,   171,   174,    13,   171,   114,
      75,   173,   173,   172,    13,   762,   148,   796,    47,   151,
     737,   721,   739,   174,   702,   173,   743,   727,   728,   536,
     162,   748,   709,    13,    73,   148,   465,    75,   151,    13,
     469,   470,   101,   148,   176,   762,   151,   794,   929,   162,
     109,   110,   111,   112,   113,   114,   178,   162,    26,   737,
     171,   739,   569,   176,   173,   743,   947,   814,    70,   125,
     748,   176,   172,   954,   173,   775,    13,   794,   825,    75,
      75,   174,    13,   512,   762,   175,   173,    73,   174,    13,
     597,    47,   599,   175,   841,    13,    73,   814,   605,   173,
     170,    26,   102,   171,    75,     8,   173,    29,   825,   109,
     110,   111,   112,   113,   114,   115,   794,   819,   105,   866,
     175,   823,   172,   171,   841,   872,    75,   172,   939,   488,
     174,   848,   367,   513,   519,   837,   814,   570,   840,   321,
     649,   843,   649,   728,   783,   619,   326,   825,   694,   866,
     666,   661,   625,   582,   901,   872,   156,   157,   158,   195,
     589,    71,    72,   841,   764,   495,   494,   852,   278,    -1,
     848,    81,   891,   806,   876,   175,   750,   879,   878,   881,
     848,    -1,    -1,    -1,   901,    -1,    -1,   858,   866,    -1,
      -1,    -1,    -1,    -1,   872,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    -1,    -1,   911,
     927,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   128,   129,
     130,   131,   132,   901,   653,    -1,    -1,   944,    -1,    -1,
      -1,    -1,   142,    -1,   936,    -1,   743,    -1,    -1,   149,
     150,    -1,    -1,    61,    62,    -1,    -1,    -1,   950,    42,
      43,     4,     5,   955,     7,     8,     9,    -1,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    -1,   702,    27,    93,    -1,    -1,    71,    72,
      73,    -1,    -1,    -1,    -1,    -1,    39,    -1,    81,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    51,    -1,
      53,    -1,    -1,    71,    72,    73,    -1,    -1,   737,    -1,
     739,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,   762,   127,   128,   129,   130,   131,   132,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   142,
      -1,    -1,    -1,    -1,    -1,   148,   149,   150,   151,    -1,
     128,   129,   130,   131,   132,   794,    -1,    -1,    -1,   162,
      -1,    25,    26,   126,   142,    -1,    -1,    -1,    -1,   102,
      -1,   149,   150,    -1,    -1,   814,   109,   110,   111,   112,
     113,   114,   115,   102,    -1,    -1,   825,    51,    -1,    -1,
     109,   110,   111,   112,   113,   114,   115,    -1,    -1,    -1,
      -1,    -1,   841,   166,    -1,   168,    -1,    -1,    -1,    -1,
     173,   174,   175,    -1,   177,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   156,   157,   158,    -1,   866,    -1,    -1,
      -1,    -1,    -1,   872,    -1,    -1,    -1,   156,   157,   158,
      -1,    -1,   175,    -1,    -1,   208,    -1,   886,    -1,   888,
      -1,    -1,    -1,   216,    -1,   218,   175,    -1,    -1,    -1,
      -1,    -1,   901,    -1,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,    -1,
     253,    -1,    -1,   932,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,    -1,    -1,   272,
     273,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   291,    -1,
      -1,   195,    -1,    -1,    -1,    -1,   299,    -1,    -1,   203,
     204,   205,    -1,   207,    -1,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,   218,    -1,    -1,    -1,    -1,    -1,
     323,    25,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,   351,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,
      -1,    -1,    -1,    -1,    -1,   269,    -1,    -1,    -1,    -1,
      -1,    -1,     9,    10,    11,    -1,    -1,    -1,   381,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   291,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   424,    -1,    -1,    -1,    -1,    63,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,   343,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   351,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   459,   361,   461,   462,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    61,    62,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,   499,    49,   501,    -1,
     503,    -1,    -1,   407,    -1,    -1,    -1,    -1,   511,    -1,
      -1,    63,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,   530,   531,    -1,
      -1,   534,    -1,    -1,    -1,    -1,   539,   540,   175,   542,
      63,   544,   545,    -1,    -1,    -1,    -1,   550,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   463,
     464,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,   488,    -1,   490,    -1,    -1,    -1,
     494,   495,    -1,   596,    -1,    -1,    -1,    63,    -1,   503,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   511,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,   530,   531,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   539,   540,    -1,   542,    63,
      -1,    -1,    -1,    -1,    -1,   648,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   569,    -1,    -1,   671,    -1,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,   597,    49,   599,    -1,    -1,    -1,    -1,
      -1,   605,    -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,
      -1,   714,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
     733,    -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    12,    63,    -1,   649,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    36,    -1,    -1,   772,
      -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,    63,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    -1,    64,    65,    66,    67,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    -1,    77,    -1,    -1,
      -1,    81,    82,    83,    84,    85,    -1,    87,    -1,    89,
     175,    91,    -1,    -1,    94,    -1,    -1,   830,    98,    99,
     100,   101,   102,   103,   104,    -1,   106,   107,   108,   743,
      -1,    -1,   112,   113,   114,    -1,   116,   117,   118,   119,
     120,   121,    -1,    -1,    -1,    -1,   126,   127,   128,   129,
     130,   131,   132,    -1,    -1,   135,   136,   137,    -1,    -1,
      -1,    -1,   142,    -1,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   171,    -1,   173,   174,   175,   176,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    -1,    64,    65,    66,    67,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    -1,    77,    -1,    -1,    -1,
      81,    82,    83,    84,    85,    -1,    87,    -1,    89,    -1,
      91,    -1,    -1,    94,    -1,    -1,    -1,    98,    99,   100,
     101,   102,   103,   104,    -1,   106,   107,   108,    -1,    -1,
      -1,   112,   113,   114,    -1,   116,   117,   118,   119,   120,
     121,    -1,    -1,    -1,    -1,   126,   127,   128,   129,   130,
     131,   132,    -1,    -1,   135,   136,   137,    -1,    -1,    -1,
      -1,   142,    -1,    -1,    -1,    -1,    -1,   148,   149,   150,
     151,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      12,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     171,    -1,   173,   174,   175,   176,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    -1,    64,    65,    66,    67,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    -1,    77,    -1,    -1,    -1,    81,
      82,    83,    84,    85,    -1,    87,    -1,    89,    -1,    91,
      -1,    -1,    94,    -1,    -1,    -1,    98,    99,   100,   101,
     102,   103,   104,    -1,   106,   107,   108,    -1,    -1,    -1,
     112,   113,   114,    -1,   116,   117,   118,   119,   120,   121,
      -1,    -1,    -1,    -1,   126,   127,   128,   129,   130,   131,
     132,    -1,    -1,   135,   136,   137,    -1,    -1,    -1,    -1,
     142,    -1,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,
     162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   171,
      -1,   173,   174,    -1,   176,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      -1,    64,    65,    66,    67,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    -1,    77,    -1,    -1,    -1,    81,    82,
      83,    84,    85,    -1,    87,    -1,    89,    -1,    91,    -1,
      -1,    94,    -1,    -1,    -1,    98,    99,   100,   101,    -1,
     103,   104,    -1,   106,    -1,   108,    -1,    -1,    -1,   112,
     113,   114,    -1,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,    -1,   126,   127,   128,   129,   130,   131,   132,
      -1,    -1,   135,   136,   137,    -1,    -1,    -1,    -1,   142,
      -1,    -1,    -1,    -1,    -1,   148,   149,   150,   151,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,   162,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   171,    -1,
     173,   174,   175,   176,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    -1,
      64,    65,    66,    67,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    -1,    77,    -1,    -1,    -1,    81,    82,    83,
      84,    85,    -1,    87,    -1,    89,    -1,    91,    -1,    -1,
      94,    -1,    -1,    -1,    98,    99,   100,   101,    -1,   103,
     104,    -1,   106,    -1,   108,    -1,    -1,    -1,   112,   113,
     114,    -1,   116,   117,   118,   119,   120,   121,    -1,    -1,
      -1,    -1,   126,   127,   128,   129,   130,   131,   132,    -1,
      -1,   135,   136,   137,    -1,    -1,    -1,    -1,   142,    -1,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    12,   162,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   171,    -1,   173,
     174,   175,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    36,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    -1,    64,
      65,    66,    67,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    -1,    77,    -1,    -1,    -1,    81,    82,    83,    84,
      85,    86,    87,    -1,    89,    -1,    91,    -1,    -1,    94,
      -1,    -1,    -1,    98,    99,   100,   101,    -1,   103,   104,
      -1,   106,    -1,   108,    -1,    -1,    -1,   112,   113,   114,
      -1,   116,   117,   118,   119,   120,   121,    -1,    -1,    -1,
      -1,   126,   127,   128,   129,   130,   131,   132,    -1,    -1,
     135,   136,   137,    -1,    -1,    -1,    -1,   142,    -1,    -1,
      -1,    -1,    -1,   148,   149,   150,   151,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    12,   162,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   171,    -1,   173,   174,
      -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      36,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    -1,    64,    65,
      66,    67,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      -1,    77,    -1,    -1,    -1,    81,    82,    83,    84,    85,
      -1,    87,    -1,    89,    -1,    91,    92,    -1,    94,    -1,
      -1,    -1,    98,    99,   100,   101,    -1,   103,   104,    -1,
     106,    -1,   108,    -1,    -1,    -1,   112,   113,   114,    -1,
     116,   117,   118,   119,   120,   121,    -1,    -1,    -1,    -1,
     126,   127,   128,   129,   130,   131,   132,    -1,    -1,   135,
     136,   137,    -1,    -1,    -1,    -1,   142,    -1,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,   162,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   171,    -1,   173,   174,    -1,
     176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,
      -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    -1,    64,    65,    66,
      67,    -1,    -1,    -1,    71,    72,    73,    74,    75,    -1,
      77,    -1,    -1,    -1,    81,    82,    83,    84,    85,    -1,
      87,    -1,    89,    -1,    91,    -1,    -1,    94,    -1,    -1,
      -1,    98,    99,   100,   101,    -1,   103,   104,    -1,   106,
      -1,   108,    -1,    -1,    -1,   112,   113,   114,    -1,   116,
     117,   118,   119,   120,   121,    -1,    -1,    -1,    -1,   126,
     127,   128,   129,   130,   131,   132,    -1,    -1,   135,   136,
     137,    -1,    -1,    -1,    -1,   142,    -1,    -1,    -1,    -1,
      -1,   148,   149,   150,   151,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    12,   162,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   171,    -1,   173,   174,   175,   176,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    -1,    64,    65,    66,    67,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    -1,    77,
      -1,    -1,    -1,    81,    82,    83,    84,    85,    -1,    87,
      -1,    89,    -1,    91,    -1,    -1,    94,    -1,    -1,    -1,
      98,    99,   100,   101,    -1,   103,   104,    -1,   106,    -1,
     108,    -1,    -1,    -1,   112,   113,   114,    -1,   116,   117,
     118,   119,   120,   121,    -1,    -1,    -1,    -1,   126,   127,
     128,   129,   130,   131,   132,    -1,    -1,   135,   136,   137,
      -1,    -1,    -1,    -1,   142,    -1,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    12,   162,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   171,    -1,   173,   174,   175,   176,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    -1,    64,    65,    66,    67,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    -1,    77,    -1,
      -1,    -1,    81,    82,    83,    84,    85,    -1,    87,    -1,
      89,    90,    91,    -1,    -1,    94,    -1,    -1,    -1,    98,
      99,   100,   101,    -1,   103,   104,    -1,   106,    -1,   108,
      -1,    -1,    -1,   112,   113,   114,    -1,   116,   117,   118,
     119,   120,   121,    -1,    -1,    -1,    -1,   126,   127,   128,
     129,   130,   131,   132,    -1,    -1,   135,   136,   137,    -1,
      -1,    -1,    -1,   142,    -1,    -1,    -1,    -1,    -1,   148,
     149,   150,   151,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    12,   162,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   171,    -1,   173,   174,    -1,   176,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,
      -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    -1,    64,    65,    66,    67,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    -1,    77,    -1,    -1,
      -1,    81,    82,    83,    84,    85,    -1,    87,    -1,    89,
      -1,    91,    -1,    -1,    94,    -1,    -1,    -1,    98,    99,
     100,   101,    -1,   103,   104,    -1,   106,    -1,   108,    -1,
      -1,    -1,   112,   113,   114,    -1,   116,   117,   118,   119,
     120,   121,    -1,    -1,    -1,    -1,   126,   127,   128,   129,
     130,   131,   132,    -1,    -1,   135,   136,   137,    -1,    -1,
      -1,    -1,   142,    -1,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   171,    -1,   173,   174,   175,   176,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    -1,    64,    65,    66,    67,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    -1,    77,    -1,    -1,    -1,
      81,    82,    83,    84,    85,    -1,    87,    88,    89,    -1,
      91,    -1,    -1,    94,    -1,    -1,    -1,    98,    99,   100,
     101,    -1,   103,   104,    -1,   106,    -1,   108,    -1,    -1,
      -1,   112,   113,   114,    -1,   116,   117,   118,   119,   120,
     121,    -1,    -1,    -1,    -1,   126,   127,   128,   129,   130,
     131,   132,    -1,    -1,   135,   136,   137,    -1,    -1,    -1,
      -1,   142,    -1,    -1,    -1,    -1,    -1,   148,   149,   150,
     151,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      12,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     171,    -1,   173,   174,    -1,   176,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    -1,    64,    65,    66,    67,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    -1,    77,    -1,    -1,    -1,    81,
      82,    83,    84,    85,    -1,    87,    -1,    89,    -1,    91,
      -1,    -1,    94,    -1,    -1,    -1,    98,    99,   100,   101,
      -1,   103,   104,    -1,   106,    -1,   108,    -1,    -1,    -1,
     112,   113,   114,    -1,   116,   117,   118,   119,   120,   121,
      -1,    -1,    -1,    -1,   126,   127,   128,   129,   130,   131,
     132,    -1,    -1,   135,   136,   137,    -1,    -1,    -1,    -1,
     142,    -1,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,
     162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   171,
      -1,   173,   174,   175,   176,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      -1,    64,    65,    66,    67,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    -1,    77,    -1,    -1,    -1,    81,    82,
      83,    84,    85,    -1,    87,    -1,    89,    -1,    91,    -1,
      -1,    94,    -1,    -1,    -1,    98,    99,   100,   101,    -1,
     103,   104,    -1,   106,    -1,   108,    -1,    -1,    -1,   112,
     113,   114,    -1,   116,   117,   118,   119,   120,   121,    -1,
      -1,    -1,    -1,   126,   127,   128,   129,   130,   131,   132,
      -1,    -1,   135,   136,   137,    -1,    -1,    -1,    -1,   142,
      -1,    -1,    -1,    -1,    -1,   148,   149,   150,   151,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,   162,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   171,    -1,
     173,   174,   175,   176,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    -1,
      64,    65,    66,    67,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    -1,    77,    -1,    -1,    -1,    81,    82,    83,
      84,    85,    -1,    87,    -1,    89,    -1,    91,    -1,    -1,
      94,    -1,    -1,    -1,    98,    99,   100,   101,    -1,   103,
     104,    -1,   106,    -1,   108,    -1,    -1,    -1,   112,   113,
     114,    -1,   116,   117,   118,   119,   120,   121,    -1,    -1,
      -1,    -1,   126,   127,   128,   129,   130,   131,   132,    -1,
      -1,   135,   136,   137,    -1,    -1,    -1,    -1,   142,    -1,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    12,   162,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   171,    -1,   173,
     174,   175,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    36,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    -1,    64,
      65,    66,    67,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    -1,    77,    -1,    -1,    -1,    81,    82,    83,    84,
      85,    -1,    87,    -1,    89,    -1,    91,    -1,    -1,    94,
      -1,    -1,    -1,    98,    99,   100,   101,    -1,   103,   104,
      -1,   106,    -1,   108,    -1,    -1,    -1,   112,   113,   114,
      -1,   116,   117,   118,   119,   120,   121,    -1,    -1,    -1,
      -1,   126,   127,   128,   129,   130,   131,   132,    -1,    -1,
     135,   136,   137,    -1,    -1,    -1,    -1,   142,    -1,    -1,
      -1,    -1,    -1,   148,   149,   150,   151,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    12,   162,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   171,    -1,   173,   174,
      26,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      36,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    -1,    64,    65,
      66,    67,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      -1,    77,    -1,    -1,    -1,    81,    82,    83,    84,    85,
      -1,    87,    -1,    89,    -1,    91,    -1,    -1,    94,    -1,
      -1,    -1,    98,    99,   100,   101,    -1,   103,   104,    -1,
     106,    -1,   108,    -1,    -1,    -1,    -1,    -1,   114,    -1,
     116,   117,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     126,   127,   128,   129,   130,   131,   132,    -1,    -1,   135,
     136,   137,    -1,    -1,    -1,    -1,   142,    -1,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,   162,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   171,    -1,   173,   174,    26,
     176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,
      -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    -1,    64,    65,    66,
      67,    -1,    -1,    -1,    71,    72,    73,    74,    75,    -1,
      77,    -1,    -1,    -1,    81,    82,    83,    84,    85,    -1,
      87,    -1,    89,    -1,    91,    -1,    -1,    94,    -1,    -1,
      -1,    98,    99,   100,   101,    -1,   103,   104,    -1,   106,
      -1,   108,    -1,    -1,    -1,    -1,    -1,   114,    -1,   116,
     117,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   126,
     127,   128,   129,   130,   131,   132,    -1,    -1,   135,   136,
     137,    -1,    -1,    -1,    -1,   142,    -1,    -1,    -1,    -1,
      -1,   148,   149,   150,   151,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    12,   162,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   171,    -1,   173,   174,    26,   176,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    -1,    64,    65,    66,    67,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    -1,    77,
      -1,    -1,    -1,    81,    82,    83,    84,    85,    -1,    87,
      -1,    89,    -1,    91,    -1,    -1,    94,    -1,    -1,    -1,
      98,    99,   100,   101,    -1,   103,   104,    -1,   106,    -1,
     108,    -1,    -1,    -1,    -1,    -1,   114,    -1,   116,   117,
     118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   126,   127,
     128,   129,   130,   131,   132,    -1,    -1,   135,   136,   137,
      -1,    -1,    -1,    -1,   142,    -1,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    12,   162,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   171,    -1,   173,   174,    26,   176,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    -1,    64,    65,    66,    67,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    -1,    77,    -1,
      -1,    -1,    81,    82,    83,    84,    85,    -1,    87,    -1,
      89,    -1,    91,    -1,    -1,    94,    -1,    -1,    -1,    98,
      99,   100,   101,    -1,   103,   104,    -1,   106,    -1,   108,
      -1,    -1,    -1,    -1,    -1,   114,    -1,   116,   117,   118,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   126,   127,   128,
     129,   130,   131,   132,    -1,    -1,   135,   136,   137,    -1,
      -1,    -1,    -1,   142,    -1,    -1,    -1,    -1,    -1,   148,
     149,   150,   151,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    12,   162,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   171,    -1,   173,   174,    26,   176,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,
      -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    -1,    64,    65,    66,    67,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    -1,    77,    -1,    -1,
      -1,    81,    82,    83,    84,    85,    -1,    87,    -1,    89,
      -1,    91,    -1,    -1,    94,    -1,    -1,    -1,    98,    99,
     100,   101,    -1,   103,   104,    -1,   106,    -1,   108,    -1,
      -1,    -1,    -1,    -1,   114,    -1,   116,   117,   118,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   126,   127,   128,   129,
     130,   131,   132,    -1,    -1,   135,   136,   137,    -1,    -1,
      -1,    -1,   142,    -1,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   171,    -1,   173,   174,    -1,   176,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    -1,    64,    65,    66,    67,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    -1,    77,    -1,    -1,    -1,
      81,    82,    83,    84,    85,    -1,    87,    -1,    89,    -1,
      91,    -1,    -1,    94,    -1,    -1,    -1,    98,    99,   100,
     101,    -1,   103,   104,    -1,   106,    -1,   108,    -1,    -1,
      -1,    -1,    -1,   114,    -1,   116,   117,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   126,   127,   128,   129,   130,
     131,   132,    -1,    -1,   135,   136,   137,    -1,    -1,    -1,
      -1,   142,    -1,    -1,    -1,    -1,    -1,   148,   149,   150,
     151,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      12,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     171,    -1,   173,   174,    -1,   176,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    -1,    64,    65,    66,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,    81,
      82,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,   101,
      49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    63,   117,   118,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   126,   127,   128,   129,   130,   131,
     132,    -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,
     142,    -1,    12,    -1,    -1,    -1,   148,   149,   150,   151,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     162,    -1,    -1,    -1,    -1,    -1,    36,    -1,    -1,   171,
      -1,   173,    42,    43,   176,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    -1,    64,    65,    66,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    -1,    -1,    -1,    -1,
      -1,    81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   114,    -1,    -1,   117,   118,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   126,   127,   128,   129,
     130,   131,   132,    -1,    -1,     3,     4,     5,     6,     7,
      -1,    -1,   142,    -1,    12,    -1,    -1,    -1,   148,   149,
     150,   151,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,    36,    -1,
      -1,   171,    -1,   173,    42,    43,   176,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    -1,    64,    65,    66,    -1,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    -1,    -1,
      -1,    -1,    -1,    81,    82,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,
     118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   126,   127,
     128,   129,   130,   131,   132,    -1,    -1,     3,     4,     5,
       6,     7,    -1,    -1,   142,    -1,    12,    -1,    -1,    -1,
     148,   149,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,
      36,    -1,    -1,   171,    -1,   173,    42,    43,   176,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    -1,    64,    65,
      66,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      -1,    -1,    -1,    -1,    -1,    81,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,
      -1,   117,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     126,   127,   128,   129,   130,   131,   132,    -1,    -1,     3,
       4,     5,     6,     7,    -1,    -1,   142,    -1,    12,    -1,
      -1,    -1,   148,   149,   150,   151,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   162,    31,    -1,    -1,
      -1,    -1,    36,    -1,    -1,   171,   172,    -1,    42,    43,
     176,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    -1,
      64,    65,    66,    -1,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    -1,    -1,    -1,    -1,    -1,    81,    82,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     114,    -1,    -1,   117,   118,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   126,   127,   128,   129,   130,   131,   132,    -1,
      -1,     3,     4,     5,     6,     7,    -1,    -1,   142,    -1,
      12,    -1,    -1,    -1,   148,   149,   150,   151,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   162,    31,
      -1,    -1,    -1,    -1,    36,    -1,    -1,   171,    -1,    -1,
      42,    43,   176,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    -1,    64,    65,    66,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,    81,
      82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,   117,   118,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   126,   127,   128,   129,   130,   131,
     132,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
     142,    12,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,    -1,    -1,
     162,    -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,   171,
      -1,    42,    43,    -1,   176,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    -1,    64,    65,    66,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,
      81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   114,    -1,    -1,   117,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   126,   127,   128,   129,   130,
     131,   132,    -1,    -1,     3,     4,     5,     6,     7,    -1,
      -1,   142,    -1,    12,    -1,    -1,    -1,   148,   149,   150,
     151,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   162,    31,    -1,    -1,    -1,    -1,    36,    -1,    -1,
     171,    -1,    -1,    42,    43,   176,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    -1,    64,    65,    66,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    -1,    -1,    -1,
      -1,    -1,    81,    82,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,   118,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   126,   127,   128,
     129,   130,   131,   132,    -1,    -1,     3,     4,     5,     6,
       7,    -1,    -1,   142,    -1,    12,    -1,    -1,    -1,   148,
     149,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   162,    31,    -1,    -1,    -1,    -1,    36,
      -1,    -1,   171,    -1,    -1,    42,    43,   176,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    -1,    64,    65,    66,
      -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,    -1,
      -1,    -1,    -1,    -1,    81,    82,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,
     117,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   126,
     127,   128,   129,   130,   131,   132,    -1,    -1,     3,     4,
       5,     6,     7,    -1,    -1,   142,    -1,    12,    -1,    -1,
      -1,   148,   149,   150,   151,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   162,    31,    -1,    -1,    -1,
      -1,    36,    -1,    -1,   171,    -1,    -1,    42,    43,   176,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    -1,    64,
      65,    66,    -1,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    -1,    -1,    -1,    -1,    -1,    81,    82,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,
      -1,    -1,   117,   118,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   126,   127,   128,   129,   130,   131,   132,    -1,    -1,
       3,     4,     5,     6,     7,    -1,    -1,   142,    -1,    12,
      -1,    -1,    -1,   148,   149,   150,   151,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   162,    31,    -1,
      -1,    -1,    -1,    36,    -1,    -1,   171,    -1,    -1,    42,
      43,   176,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      -1,    64,    65,    66,    -1,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    -1,    -1,    -1,    -1,    -1,    81,    82,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,    -1,    -1,   117,   118,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   126,   127,   128,   129,   130,   131,   132,
      -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,   142,
      -1,    12,    -1,    -1,    -1,   148,   149,   150,   151,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   162,
      31,    -1,    -1,    -1,    -1,    36,    -1,    -1,   171,    -1,
      -1,    42,    43,   176,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    -1,    64,    65,    66,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,
      81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   114,    -1,    -1,   117,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   126,   127,   128,   129,   130,
     131,   132,    -1,     3,     4,     5,     6,     7,    -1,    -1,
      -1,   142,    12,    -1,    -1,    -1,    -1,   148,   149,   150,
     151,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   162,    -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,
     171,    -1,    42,    43,    -1,   176,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    -1,    64,    65,    66,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    -1,    -1,    -1,    -1,
      -1,    81,    82,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   114,    -1,    -1,   117,   118,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   126,   127,   128,   129,
     130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   142,    -1,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,
      10,    11,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   171,    -1,    -1,    -1,    25,   176,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    63,    -1,    -1,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    63,    -1,    -1,    25,    -1,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,   175,    -1,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,
      -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,   175,    49,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    63,    -1,    -1,    25,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,   175,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    63,    -1,    -1,    -1,    -1,    -1,    -1,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,   173,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    -1,    -1,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    63,    -1,    -1,    -1,    -1,    -1,    -1,
      25,   173,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,
      -1,    25,   173,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,
      -1,    -1,    -1,    -1,    -1,    -1,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,   173,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,
      -1,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      63,    -1,    -1,    -1,    -1,    -1,    -1,    25,   173,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,   172,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,
      -1,    -1,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,   172,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    -1,    -1,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,
      -1,    -1,    -1,    25,   172,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    63,    -1,    -1,    -1,    25,   172,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    63,    -1,    -1,    -1,    -1,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,   172,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    -1,    -1,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    63,    -1,    -1,    -1,    -1,    -1,    -1,    25,
     172,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,
      -1,    25,   172,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,
      -1,    -1,    -1,    -1,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,   172,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,    -1,
      -1,     9,    10,    11,    -1,    -1,    -1,    -1,    63,    -1,
      -1,   125,    -1,    -1,    -1,    -1,   172,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    63,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     125,    -1,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      63,    -1,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    11,    49,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      63,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    63,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    63,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      63
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   180,   181,     0,     3,     4,     5,     6,     7,    12,
      36,    42,    43,    48,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    64,    65,    66,
      67,    71,    72,    73,    74,    75,    77,    81,    82,    83,
      84,    85,    87,    89,    91,    94,    98,    99,   100,   101,
     102,   103,   104,   106,   107,   108,   112,   113,   114,   116,
     117,   118,   119,   120,   121,   126,   127,   128,   129,   130,
     131,   132,   135,   136,   137,   142,   148,   149,   150,   151,
     162,   171,   173,   174,   176,   182,   183,   186,   189,   190,
     196,   197,   199,   200,   201,   203,   242,   243,   246,   247,
     255,   258,   262,   263,   265,   266,   270,   271,   272,   273,
     274,   275,   276,   281,   286,   288,   289,   290,   291,   293,
      73,   114,   148,   243,   262,   262,   171,   262,   262,   262,
     306,   307,   262,   262,   262,   262,   262,   262,   262,   262,
     262,   262,   262,   262,   262,   262,   114,   148,   151,   182,
     247,   265,   266,   272,   265,   262,   148,   151,   182,   247,
     249,   250,   272,   274,   275,   281,   171,   253,   171,    26,
     239,   262,   189,   171,   171,   171,   171,   171,   173,   262,
     173,   262,    73,    73,   173,   242,   262,   266,   174,   262,
     151,   182,   184,   185,    75,   176,   226,   227,   120,   120,
      75,   228,   243,   171,   171,   171,   171,   171,   171,   151,
     174,   182,   182,   306,   262,   187,   174,   151,   171,     8,
     173,    26,    73,    73,    31,   198,   145,     9,    10,    11,
      25,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    49,    63,   173,    61,    62,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,   171,
     145,   124,    63,   174,   176,   275,   153,   294,   295,   299,
     198,   262,    73,   316,   151,   182,   145,   145,   151,   182,
     145,   171,   254,   145,   124,   275,   172,   262,   262,     8,
     173,    85,   262,   240,   241,   262,   242,   266,    73,   212,
     262,   173,   173,   173,    13,   173,   173,   187,   173,   182,
      93,     8,   173,   174,   263,   266,     8,   173,    13,     8,
     173,   198,   194,   195,   266,   266,   287,   266,   172,   126,
     266,   282,   283,    31,   262,   284,   285,   182,   181,   173,
     174,   171,   172,   119,   175,   188,   189,   196,   197,   262,
      73,    31,   224,   225,   242,   266,    73,   306,   122,   202,
     122,   204,    73,   171,    73,   270,   275,   281,   262,   262,
     262,    26,   262,   262,   262,   262,   262,   262,   262,   262,
     262,   262,   262,   262,   262,   262,   262,   262,   262,   262,
     262,   262,   262,   262,   249,   262,   277,    31,   262,   262,
     262,   262,   262,   262,   262,   262,   262,   262,   262,   262,
     224,    73,   270,    73,   174,   270,   278,   279,   280,   277,
     262,   153,   154,   155,   174,   289,   292,   296,    37,    46,
     300,   310,   311,   172,   152,   162,   163,   318,   182,    73,
      73,   182,   270,   224,   270,   278,   172,   172,   262,   171,
     172,   173,     8,    93,    93,    13,     8,   172,   172,    42,
      43,   127,   148,   151,   182,   247,   255,   256,   257,   175,
      93,    73,   185,   262,   227,   256,    75,   171,     8,   172,
       8,   172,   172,   173,   171,     8,   172,   264,   266,   125,
     172,     8,   260,   171,   175,   181,   224,   171,   175,   264,
     172,     8,    13,   202,   148,   151,   162,   182,   248,   123,
     205,   206,   248,   174,   171,   127,   221,   222,   223,   248,
     171,   171,   275,   262,    26,   177,    65,   266,   172,   171,
     171,   262,   171,   269,    63,   174,   177,   175,   308,   309,
     297,   294,    37,    13,    73,   317,    73,    73,   172,   251,
      26,   189,   262,    26,   189,   216,   240,   262,   266,    31,
     208,   266,   256,    73,    26,   189,   211,    26,   174,   213,
     256,   256,   171,   151,   182,   145,   105,    73,   175,    13,
     221,   195,   173,   266,   282,   283,    13,    31,   262,    31,
     262,   224,   175,   172,   172,    31,   242,   266,   256,   205,
     151,   182,   306,   206,   174,     8,   229,   221,   172,     8,
      31,    75,   224,   224,   262,   249,   224,   224,   175,   224,
     267,   277,   262,    37,   316,   262,   174,   178,   301,   162,
     163,   318,   124,   252,   187,   217,   172,   187,   173,   125,
     207,   266,   207,    13,   187,   173,   214,   173,   214,   256,
     259,   261,   182,    73,   171,   256,   172,   172,   262,   264,
     264,   125,   172,   173,   264,   174,   182,   229,   248,   102,
     109,   110,   111,   112,   113,   114,   115,   156,   157,   158,
     175,   230,   233,   234,   235,   236,   238,   172,   107,   244,
     223,    75,    13,   172,   172,   254,   172,   172,   172,   124,
     268,   177,   175,   175,   303,   302,    73,    73,   278,   218,
      68,    69,   219,   173,    86,   240,   208,   172,   172,   256,
      92,   214,    95,    96,    97,   214,   175,   125,   172,     8,
     260,   248,   244,    31,   262,   319,   175,    73,   321,   330,
     332,    75,   237,   243,   236,     8,   173,   174,   171,   174,
      31,    75,    13,   256,   278,   298,   262,   294,   305,    68,
      69,   220,   171,   189,   173,   172,    26,   189,   210,   210,
     173,    95,   173,   262,    26,   173,   215,   175,   256,   256,
      75,   174,   264,   229,    13,   115,   162,   164,   165,   166,
     167,   168,   169,   247,   322,   323,   324,    47,   331,   159,
     160,   171,   333,   334,    13,     8,   173,   231,    73,   187,
      31,    75,   245,   187,    75,    13,   256,   269,   304,   178,
     171,    26,    70,   262,    26,   189,   209,   187,   173,   215,
     187,   125,   172,   187,   320,   256,   306,   174,     8,   173,
     310,   312,   313,     8,   173,    47,   159,   161,   162,   334,
     335,   336,   173,   256,    75,   198,    13,   175,    75,     8,
     172,   175,    13,   256,   175,   262,   187,   173,   172,   187,
      90,   187,   256,   174,   175,   175,   325,   323,    13,   328,
     317,    47,   314,   315,   314,     8,    29,   172,    25,    42,
      45,    13,    73,   256,    31,    75,   256,   172,   189,    88,
     173,   187,   255,   327,    73,   255,   170,   329,   318,   312,
     316,   335,   335,    25,    42,    45,   256,   171,    75,    26,
     173,   175,     8,   326,   318,   221,   187,   105,   191,   192,
     193,   255,   175,   172,   171,   193,   173,   174,   232,   248,
     187,    75,   175,   172,   174,   187,   175
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (yyscanner, root, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, yyscanner)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, yyscanner, root); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void* yyscanner, code_rope* root)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yyscanner, root)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    void* yyscanner;
    code_rope* root;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yyscanner);
  YYUSE (root);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void* yyscanner, code_rope* root)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yyscanner, root)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    void* yyscanner;
    code_rope* root;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yyscanner, root);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule, void* yyscanner, code_rope* root)
#else
static void
yy_reduce_print (yyvsp, yyrule, yyscanner, root)
    YYSTYPE *yyvsp;
    int yyrule;
    void* yyscanner;
    code_rope* root;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       , yyscanner, root);
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule, yyscanner, root); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, void* yyscanner, code_rope* root)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yyscanner, root)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    void* yyscanner;
    code_rope* root;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yyscanner);
  YYUSE (root);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void* yyscanner, code_rope* root);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void* yyscanner, code_rope* root)
#else
int
yyparse (yyscanner, root)
    void* yyscanner;
    code_rope* root;
#endif
#endif
{
  /* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 199 "parser.y"
    {
    *root = (yyvsp[(1) - (1)]);
  ;}
    break;

  case 3:
#line 205 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 4:
#line 208 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 6:
#line 215 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 10:
#line 224 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 11:
#line 227 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + " " + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 12:
#line 230 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (5)]) + " " + (yyvsp[(2) - (5)]) + (yyvsp[(3) - (5)]) + (yyvsp[(4) - (5)]) + (yyvsp[(5) - (5)]);
  ;}
    break;

  case 13:
#line 233 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 14:
#line 236 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + " " + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 15:
#line 239 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 16:
#line 245 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 19:
#line 253 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 20:
#line 256 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 21:
#line 259 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + " " + (yyvsp[(3) - (4)]) + " " + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 22:
#line 265 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (5)]) + (yyvsp[(2) - (5)]) + (yyvsp[(3) - (5)]) + (yyvsp[(4) - (5)]) + (yyvsp[(5) - (5)]);
  ;}
    break;

  case 23:
#line 268 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + " " + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 24:
#line 274 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 25:
#line 277 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 29:
#line 286 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 31:
#line 293 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 34:
#line 298 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 35:
#line 304 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 36:
#line 307 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (7)]) + (yyvsp[(2) - (7)]) + (yyvsp[(3) - (7)]) + (yyvsp[(4) - (7)]) + (yyvsp[(5) - (7)]) + (yyvsp[(6) - (7)]) + (yyvsp[(7) - (7)]);
  ;}
    break;

  case 37:
#line 310 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (10)]) + (yyvsp[(2) - (10)]) + (yyvsp[(3) - (10)]) + (yyvsp[(4) - (10)]) + (yyvsp[(5) - (10)]) + (yyvsp[(6) - (10)]) + (yyvsp[(7) - (10)]) + (yyvsp[(8) - (10)]) + (yyvsp[(9) - (10)]) + (yyvsp[(10) - (10)]);
  ;}
    break;

  case 38:
#line 313 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (5)]) + (yyvsp[(2) - (5)]) + (yyvsp[(3) - (5)]) + (yyvsp[(4) - (5)]) + (yyvsp[(5) - (5)]);
  ;}
    break;

  case 39:
#line 316 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (7)]) + " " + (yyvsp[(2) - (7)]) + (yyvsp[(3) - (7)]) + (yyvsp[(4) - (7)]) + (yyvsp[(5) - (7)]) + (yyvsp[(6) - (7)]) + (yyvsp[(7) - (7)]);
  ;}
    break;

  case 40:
#line 319 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (9)]) + (yyvsp[(2) - (9)]) + (yyvsp[(3) - (9)]) + (yyvsp[(4) - (9)]) + (yyvsp[(5) - (9)]) + (yyvsp[(6) - (9)]) + (yyvsp[(7) - (9)]) + (yyvsp[(8) - (9)]) + (yyvsp[(9) - (9)]);
  ;}
    break;

  case 41:
#line 322 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (5)]) + (yyvsp[(2) - (5)]) + (yyvsp[(3) - (5)]) + (yyvsp[(4) - (5)]) + (yyvsp[(5) - (5)]);
  ;}
    break;

  case 42:
#line 325 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 43:
#line 328 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + " " + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 44:
#line 331 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 45:
#line 334 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + " " + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 46:
#line 337 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 47:
#line 340 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + " " + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 48:
#line 343 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + " " + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 49:
#line 346 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + " " + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 50:
#line 349 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + " " + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 51:
#line 352 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + " " + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 53:
#line 356 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 54:
#line 359 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (5)]) + (yyvsp[(2) - (5)]) + (yyvsp[(3) - (5)]) + (yyvsp[(4) - (5)]) + (yyvsp[(5) - (5)]);
  ;}
    break;

  case 55:
#line 362 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (8)]) + (yyvsp[(2) - (8)]) + (yyvsp[(3) - (8)]) + " " + (yyvsp[(4) - (8)]) + " " + (yyvsp[(5) - (8)]) + (yyvsp[(6) - (8)]) + (yyvsp[(7) - (8)]) + (yyvsp[(8) - (8)]);
  ;}
    break;

  case 56:
#line 365 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (8)]) + (yyvsp[(2) - (8)]) + (yyvsp[(3) - (8)]) + " " + (yyvsp[(4) - (8)]) + " " + (yyvsp[(5) - (8)]) + (yyvsp[(6) - (8)]) + (yyvsp[(7) - (8)]) + (yyvsp[(8) - (8)]);
  ;}
    break;

  case 57:
#line 368 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (5)]) + (yyvsp[(2) - (5)]) + (yyvsp[(3) - (5)]) + (yyvsp[(4) - (5)]) + (yyvsp[(5) - (5)]);
  ;}
    break;

  case 59:
#line 372 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (13)]) + (yyvsp[(2) - (13)]) + (yyvsp[(3) - (13)]) + (yyvsp[(4) - (13)]) + (yyvsp[(5) - (13)]) + (yyvsp[(6) - (13)]) + (yyvsp[(7) - (13)]) + " " + (yyvsp[(8) - (13)]) + (yyvsp[(9) - (13)]) + (yyvsp[(10) - (13)]) + (yyvsp[(11) - (13)]) + (yyvsp[(12) - (13)]) + (yyvsp[(13) - (13)]);
  ;}
    break;

  case 60:
#line 375 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + " " + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 61:
#line 378 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + " " + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 63:
#line 385 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 65:
#line 392 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 66:
#line 398 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (8)]) + (yyvsp[(2) - (8)]) + (yyvsp[(3) - (8)]) + " " + (yyvsp[(4) - (8)]) + (yyvsp[(5) - (8)]) + (yyvsp[(6) - (8)]) + (yyvsp[(7) - (8)]) + (yyvsp[(8) - (8)]);
  ;}
    break;

  case 68:
#line 405 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 72:
#line 423 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 74:
#line 430 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (9)]) + " " + (yyvsp[(2) - (9)]) + (yyvsp[(3) - (9)]) + (yyvsp[(4) - (9)]) + (yyvsp[(5) - (9)]) + (yyvsp[(6) - (9)]) + (yyvsp[(7) - (9)]) + (yyvsp[(8) - (9)]) + (yyvsp[(9) - (9)]);
  ;}
    break;

  case 75:
#line 436 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (7)]) + " " + (yyvsp[(2) - (7)]) + (yyvsp[(3) - (7)]) + (yyvsp[(4) - (7)]) + (yyvsp[(5) - (7)]) + (yyvsp[(6) - (7)]) + (yyvsp[(7) - (7)]);
  ;}
    break;

  case 76:
#line 439 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (6)]) + " " + (yyvsp[(2) - (6)]) + (yyvsp[(3) - (6)]) + (yyvsp[(4) - (6)]) + (yyvsp[(5) - (6)]) + (yyvsp[(6) - (6)]);
  ;}
    break;

  case 78:
#line 446 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + " " + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 79:
#line 449 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + " " + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 80:
#line 455 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 81:
#line 458 "parser.y"
    {
    (yyval) = " " + (yyvsp[(1) - (2)]) + " " + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 83:
#line 468 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 84:
#line 471 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + " " + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 85:
#line 477 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 86:
#line 480 "parser.y"
    {
    (yyval) = " " + (yyvsp[(1) - (2)]) + " " + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 88:
#line 487 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 89:
#line 493 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 90:
#line 496 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 92:
#line 503 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 94:
#line 510 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 96:
#line 517 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 98:
#line 524 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 99:
#line 530 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 100:
#line 533 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (5)]) + (yyvsp[(2) - (5)]) + (yyvsp[(3) - (5)]) + (yyvsp[(4) - (5)]) + (yyvsp[(5) - (5)]);
  ;}
    break;

  case 101:
#line 539 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 102:
#line 542 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 103:
#line 545 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 104:
#line 548 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (5)]) + (yyvsp[(2) - (5)]) + (yyvsp[(3) - (5)]) + (yyvsp[(4) - (5)]) + (yyvsp[(5) - (5)]);
  ;}
    break;

  case 105:
#line 554 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 106:
#line 557 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (5)]) + (yyvsp[(2) - (5)]) + " " + (yyvsp[(3) - (5)]) + (yyvsp[(4) - (5)]) + (yyvsp[(5) - (5)]);
  ;}
    break;

  case 107:
#line 560 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 111:
#line 572 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 112:
#line 578 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 113:
#line 581 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (6)]) + (yyvsp[(2) - (6)]) + (yyvsp[(3) - (6)]) + (yyvsp[(4) - (6)]) + (yyvsp[(5) - (6)]) + (yyvsp[(6) - (6)]);
  ;}
    break;

  case 114:
#line 587 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 115:
#line 590 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (7)]) + (yyvsp[(2) - (7)]) + (yyvsp[(3) - (7)]) + (yyvsp[(4) - (7)]) + (yyvsp[(5) - (7)]) + (yyvsp[(6) - (7)]) + (yyvsp[(7) - (7)]);
  ;}
    break;

  case 116:
#line 596 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 117:
#line 599 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + " " + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 118:
#line 605 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 119:
#line 608 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 121:
#line 615 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 122:
#line 621 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 123:
#line 624 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 124:
#line 627 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (5)]) + (yyvsp[(2) - (5)]) + (yyvsp[(3) - (5)]) + (yyvsp[(4) - (5)]) + (yyvsp[(5) - (5)]);
  ;}
    break;

  case 125:
#line 630 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 126:
#line 633 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 127:
#line 636 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (5)]) + (yyvsp[(2) - (5)]) + (yyvsp[(3) - (5)]) + (yyvsp[(4) - (5)]) + (yyvsp[(5) - (5)]);
  ;}
    break;

  case 128:
#line 639 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (7)]) + (yyvsp[(2) - (7)]) + (yyvsp[(3) - (7)]) + (yyvsp[(4) - (7)]) + (yyvsp[(5) - (7)]) + (yyvsp[(6) - (7)]) + (yyvsp[(7) - (7)]);
  ;}
    break;

  case 129:
#line 642 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (6)]) + (yyvsp[(2) - (6)]) + (yyvsp[(3) - (6)]) + (yyvsp[(4) - (6)]) + (yyvsp[(5) - (6)]) + (yyvsp[(6) - (6)]);
  ;}
    break;

  case 130:
#line 648 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 131:
#line 651 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (1)]) + " ";
  ;}
    break;

  case 132:
#line 654 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (1)]) + " ";
  ;}
    break;

  case 134:
#line 661 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 137:
#line 669 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 138:
#line 672 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 139:
#line 675 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 140:
#line 678 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 141:
#line 684 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 144:
#line 692 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 145:
#line 695 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 146:
#line 701 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 147:
#line 704 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (5)]) + (yyvsp[(2) - (5)]) + (yyvsp[(3) - (5)]) + (yyvsp[(4) - (5)]) + (yyvsp[(5) - (5)]);
  ;}
    break;

  case 149:
#line 708 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 150:
#line 714 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 151:
#line 717 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 152:
#line 723 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + " " + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 153:
#line 726 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 154:
#line 729 "parser.y"
    {
    yyextra->old_expecting_xhp_class_statements = yyextra->expecting_xhp_class_statements;
    yyextra->expecting_xhp_class_statements = false;
  ;}
    break;

  case 155:
#line 732 "parser.y"
    {
    yyextra->expecting_xhp_class_statements = yyextra->old_expecting_xhp_class_statements;
    (yyval) = (yyvsp[(1) - (9)]) + (yyvsp[(2) - (9)]) + " " + (yyvsp[(4) - (9)]) + (yyvsp[(5) - (9)]) + (yyvsp[(6) - (9)]) + (yyvsp[(7) - (9)]) + (yyvsp[(8) - (9)]) + (yyvsp[(9) - (9)]);
  ;}
    break;

  case 157:
#line 740 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 159:
#line 747 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (1)]) + " ";
  ;}
    break;

  case 160:
#line 753 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 161:
#line 756 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (1)]) + " ";
  ;}
    break;

  case 163:
#line 763 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + " " + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 170:
#line 778 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 171:
#line 781 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (5)]) + (yyvsp[(2) - (5)]) + (yyvsp[(3) - (5)]) + (yyvsp[(4) - (5)]) + (yyvsp[(5) - (5)]);
  ;}
    break;

  case 173:
#line 785 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 174:
#line 791 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (5)]) + (yyvsp[(2) - (5)]) + (yyvsp[(3) - (5)]) + (yyvsp[(4) - (5)]) + (yyvsp[(5) - (5)]);
  ;}
    break;

  case 175:
#line 794 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + " " + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 176:
#line 800 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 178:
#line 807 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 180:
#line 815 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 182:
#line 822 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (6)]) + (yyvsp[(2) - (6)]) + (yyvsp[(3) - (6)]) + (yyvsp[(4) - (6)]) + (yyvsp[(5) - (6)]) + (yyvsp[(6) - (6)]);
  ;}
    break;

  case 183:
#line 825 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 184:
#line 828 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 185:
#line 831 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (6)]) + (yyvsp[(2) - (6)]) + (yyvsp[(3) - (6)]) + (yyvsp[(4) - (6)]) + " " + (yyvsp[(5) - (6)]) + (yyvsp[(6) - (6)]);
  ;}
    break;

  case 186:
#line 834 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + " " + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 187:
#line 837 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + " " + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 188:
#line 840 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 189:
#line 843 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 190:
#line 846 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 191:
#line 849 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 192:
#line 852 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 193:
#line 855 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 194:
#line 858 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 195:
#line 861 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 196:
#line 864 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 197:
#line 867 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 198:
#line 870 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 199:
#line 873 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 200:
#line 876 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 201:
#line 879 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 202:
#line 882 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 203:
#line 885 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 204:
#line 888 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 205:
#line 891 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + " " +  (yyvsp[(2) - (3)]) + " " + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 206:
#line 894 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + " " + (yyvsp[(2) - (3)]) + " " + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 207:
#line 897 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + " " + (yyvsp[(2) - (3)]) + " " + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 208:
#line 900 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 209:
#line 903 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 210:
#line 906 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 211:
#line 909 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 212:
#line 912 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 213:
#line 915 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 214:
#line 918 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 215:
#line 921 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 216:
#line 924 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 217:
#line 927 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 218:
#line 930 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 219:
#line 933 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 220:
#line 936 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 221:
#line 939 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 222:
#line 942 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 223:
#line 945 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 224:
#line 948 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 225:
#line 951 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 226:
#line 954 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 227:
#line 957 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 228:
#line 960 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 229:
#line 963 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 230:
#line 966 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 231:
#line 969 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + " " + (yyvsp[(2) - (3)]) + " " + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 232:
#line 972 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 233:
#line 975 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (5)]) + (yyvsp[(2) - (5)]) + (yyvsp[(3) - (5)]) + (yyvsp[(4) - (5)]) + (yyvsp[(5) - (5)]);
  ;}
    break;

  case 234:
#line 978 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 236:
#line 982 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 237:
#line 985 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 238:
#line 988 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 239:
#line 991 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 240:
#line 994 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 241:
#line 997 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 242:
#line 1000 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 243:
#line 1003 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 244:
#line 1006 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 245:
#line 1009 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 246:
#line 1012 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 248:
#line 1016 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 250:
#line 1020 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + " " + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 251:
#line 1023 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (9)]) + (yyvsp[(2) - (9)]) + (yyvsp[(3) - (9)]) + (yyvsp[(4) - (9)]) + (yyvsp[(5) - (9)]) + (yyvsp[(6) - (9)]) + (yyvsp[(7) - (9)]) + (yyvsp[(8) - (9)]) + (yyvsp[(9) - (9)]);
  ;}
    break;

  case 252:
#line 1026 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (10)]) + " " + (yyvsp[(2) - (10)]) + (yyvsp[(3) - (10)]) + (yyvsp[(4) - (10)]) + (yyvsp[(5) - (10)]) + (yyvsp[(6) - (10)]) + (yyvsp[(7) - (10)]) + (yyvsp[(8) - (10)]) + (yyvsp[(9) - (10)]) + (yyvsp[(10) - (10)]);
  ;}
    break;

  case 255:
#line 1037 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 256:
#line 1043 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 257:
#line 1046 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 259:
#line 1050 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 260:
#line 1056 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 261:
#line 1059 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (6)]) + (yyvsp[(2) - (6)]) + (yyvsp[(3) - (6)]) + (yyvsp[(4) - (6)]) + (yyvsp[(5) - (6)]) + (yyvsp[(6) - (6)]);
  ;}
    break;

  case 262:
#line 1062 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (5)]) + (yyvsp[(2) - (5)]) + (yyvsp[(3) - (5)]) + (yyvsp[(4) - (5)]) + (yyvsp[(5) - (5)]);
  ;}
    break;

  case 263:
#line 1065 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (6)]) + (yyvsp[(2) - (6)]) + (yyvsp[(3) - (6)]) + (yyvsp[(4) - (6)]) + (yyvsp[(5) - (6)]) + (yyvsp[(6) - (6)]);
  ;}
    break;

  case 264:
#line 1068 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (6)]) + (yyvsp[(2) - (6)]) + (yyvsp[(3) - (6)]) + (yyvsp[(4) - (6)]) + (yyvsp[(5) - (6)]) + (yyvsp[(6) - (6)]);
  ;}
    break;

  case 265:
#line 1071 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (6)]) + (yyvsp[(2) - (6)]) + (yyvsp[(3) - (6)]) + (yyvsp[(4) - (6)]) + (yyvsp[(5) - (6)]) + (yyvsp[(6) - (6)]);
  ;}
    break;

  case 266:
#line 1074 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (6)]) + (yyvsp[(2) - (6)]) + (yyvsp[(3) - (6)]) + (yyvsp[(4) - (6)]) + (yyvsp[(5) - (6)]) + (yyvsp[(6) - (6)]);
  ;}
    break;

  case 267:
#line 1077 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 270:
#line 1085 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 271:
#line 1088 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 273:
#line 1095 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 274:
#line 1098 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 277:
#line 1109 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 279:
#line 1116 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 280:
#line 1119 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 281:
#line 1125 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 282:
#line 1131 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 283:
#line 1134 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 284:
#line 1137 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 285:
#line 1143 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 286:
#line 1146 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 300:
#line 1168 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 301:
#line 1171 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 302:
#line 1174 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 303:
#line 1177 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 304:
#line 1180 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 306:
#line 1187 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 310:
#line 1196 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 311:
#line 1199 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 313:
#line 1206 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 315:
#line 1213 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 317:
#line 1220 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (5)]) + (yyvsp[(2) - (5)]) + (yyvsp[(3) - (5)]) + (yyvsp[(4) - (5)]) + (yyvsp[(5) - (5)]);
  ;}
    break;

  case 318:
#line 1223 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 319:
#line 1226 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 326:
#line 1250 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (5)]) + (yyvsp[(2) - (5)]) + (yyvsp[(3) - (5)]) + (yyvsp[(4) - (5)]) + (yyvsp[(5) - (5)]);
  ;}
    break;

  case 328:
#line 1257 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 329:
#line 1260 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 330:
#line 1266 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 331:
#line 1272 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 332:
#line 1275 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 334:
#line 1282 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 335:
#line 1288 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 336:
#line 1291 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 341:
#line 1307 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 343:
#line 1314 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 344:
#line 1317 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]);
  ;}
    break;

  case 347:
#line 1325 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 348:
#line 1331 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 352:
#line 1343 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 353:
#line 1346 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 356:
#line 1354 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 358:
#line 1361 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 359:
#line 1367 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 362:
#line 1375 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 363:
#line 1378 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 364:
#line 1384 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 366:
#line 1391 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (5)]) + (yyvsp[(2) - (5)]) + (yyvsp[(3) - (5)]) + (yyvsp[(4) - (5)]) + (yyvsp[(5) - (5)]);
  ;}
    break;

  case 367:
#line 1394 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 368:
#line 1397 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 370:
#line 1401 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (6)]) + (yyvsp[(2) - (6)]) + (yyvsp[(3) - (6)]) + (yyvsp[(4) - (6)]) + (yyvsp[(5) - (6)]) + (yyvsp[(6) - (6)]);
  ;}
    break;

  case 371:
#line 1404 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 372:
#line 1407 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 373:
#line 1410 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 374:
#line 1416 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 375:
#line 1419 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 376:
#line 1422 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + " " + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 377:
#line 1425 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + " " + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 378:
#line 1428 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
  ;}
    break;

  case 379:
#line 1431 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + " " + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 380:
#line 1434 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + " " + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 382:
#line 1441 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 383:
#line 1447 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 384:
#line 1450 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 385:
#line 1460 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (1)]);
    yyextra->used = true;
  ;}
    break;

  case 387:
#line 1468 "parser.y"
    {
    if (yyextra->include_debug) {
      char line[16];
      sprintf(line, "%lu", (unsigned long)(yyvsp[(1) - (3)]).lineno());
      (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + "), __FILE__, " + line +")";
    } else {
      (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + "))";
    }
  ;}
    break;

  case 388:
#line 1480 "parser.y"
    {
    pop_state(); // XHP_ATTRS
    if (yyextra->include_debug) {
      char line[16];
      sprintf(line, "%lu", (unsigned long)(yyvsp[(1) - (4)]).lineno());
      (yyval) = (yyextra->emit_namespaces ? "new \\xhp_" : "new xhp_") + (yyvsp[(1) - (4)]) + "(array(" + (yyvsp[(2) - (4)]) + "), array(), __FILE__, " + line + ")";
    } else {
      (yyval) = (yyextra->emit_namespaces ? "new \\xhp_" : "new xhp_") + (yyvsp[(1) - (4)]) + "(array(" + (yyvsp[(2) - (4)]) + "), array())";
    }
  ;}
    break;

  case 389:
#line 1493 "parser.y"
    {
    pop_state(); // XHP_ATTRS
    push_state(XHP_CHILD_START);
    yyextra->pushTag((yyvsp[(1) - (3)]).c_str());
    (yyval) = (yyextra->emit_namespaces ? "new \\xhp_" : "new xhp_") + (yyvsp[(1) - (3)]) + "(array(" + (yyvsp[(2) - (3)]) + "), array(";
  ;}
    break;

  case 390:
#line 1502 "parser.y"
    {
    pop_state(); // XHP_CHILD_START
    if (yyextra->peekTag() != (yyvsp[(2) - (3)]).c_str()) {
      string e1 = (yyvsp[(2) - (3)]).c_str();
      string e2 = yyextra->peekTag();
      replacestr(e1, "__", ":");
      replacestr(e1, "_", "-");
      replacestr(e2, "__", ":");
      replacestr(e2, "_", "-");
      string e = "syntax error, mismatched tag </" + e1 + ">, expecting </" + e2 +">";
      yyerror(yyscanner, NULL, e.c_str());
      yyextra->terminated = true;
    }
    yyextra->popTag();
    if (yyextra->haveTag()) {
      set_state(XHP_CHILD_START);
    }
  ;}
    break;

  case 391:
#line 1520 "parser.y"
    {
    // empty end tag -- SGML SHORTTAG
    pop_state(); // XHP_CHILD_START
    yyextra->popTag();
    if (yyextra->haveTag()) {
      set_state(XHP_CHILD_START);
    }
    (yyval) = "))";
  ;}
    break;

  case 392:
#line 1532 "parser.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
  ;}
    break;

  case 393:
#line 1539 "parser.y"
    {
    (yyvsp[(1) - (1)]).strip_lines();
    (yyval) = (yyvsp[(1) - (1)]);
  ;}
    break;

  case 394:
#line 1543 "parser.y"
    {
    (yyvsp[(2) - (2)]).strip_lines();
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 395:
#line 1550 "parser.y"
    {
    (yyval) = "";
  ;}
    break;

  case 396:
#line 1553 "parser.y"
    {
    set_state(XHP_CHILD_START);
    (yyval) = "'" + (yyvsp[(1) - (1)]) + "',";
  ;}
    break;

  case 397:
#line 1557 "parser.y"
    {
    set_state(XHP_CHILD_START);
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]) + ",";
  ;}
    break;

  case 398:
#line 1561 "parser.y"
    {
    set_state(XHP_CHILD_START);
    (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + ",'" + (yyvsp[(3) - (3)]) + "',";
  ;}
    break;

  case 400:
#line 1569 "parser.y"
    {
    push_state(PHP);
    yyextra->pushStack();
  ;}
    break;

  case 401:
#line 1572 "parser.y"
    {
    pop_state();
    yyextra->popStack();
  ;}
    break;

  case 402:
#line 1575 "parser.y"
    {
    set_state(XHP_CHILD_START);
    (yyval) = (yyvsp[(3) - (5)]);
  ;}
    break;

  case 403:
#line 1583 "parser.y"
    {
    (yyval) = "";
    push_state(XHP_ATTRS);
  ;}
    break;

  case 404:
#line 1587 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (2)]) + (yyvsp[(2) - (2)]) + ",";
  ;}
    break;

  case 405:
#line 1593 "parser.y"
    {
    (yyval) = "'" + (yyvsp[(1) - (3)]) + "' => " + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 406:
#line 1599 "parser.y"
    { push_state(XHP_ATTR_VAL); ;}
    break;

  case 407:
#line 1599 "parser.y"
    {
    (yyval) = (yyvsp[(3) - (4)]);
  ;}
    break;

  case 408:
#line 1602 "parser.y"
    { push_state(PHP); ;}
    break;

  case 409:
#line 1602 "parser.y"
    { pop_state(); ;}
    break;

  case 410:
#line 1602 "parser.y"
    {
    (yyval) = (yyvsp[(3) - (5)]);
  ;}
    break;

  case 411:
#line 1608 "parser.y"
    {
    (yyval) = "''";
  ;}
    break;

  case 412:
#line 1611 "parser.y"
    {
    // XHP_ATTR_VAL is popped by the time this code runs
    (yyval) = "'" + (yyvsp[(1) - (1)]) + "'";
  ;}
    break;

  case 413:
#line 1619 "parser.y"
    { push_state(XHP_LABEL); ;}
    break;

  case 414:
#line 1619 "parser.y"
    {
    pop_state();
    (yyval) = (yyvsp[(2) - (3)]);
  ;}
    break;

  case 415:
#line 1626 "parser.y"
    { push_state(XHP_LABEL); ;}
    break;

  case 416:
#line 1626 "parser.y"
    {
    pop_state();
    (yyval) = (yyvsp[(2) - (2)]);
  ;}
    break;

  case 417:
#line 1633 "parser.y"
    { push_state(XHP_LABEL_WHITESPACE); ;}
    break;

  case 418:
#line 1633 "parser.y"
    {
    pop_state();
    (yyval) = (yyvsp[(2) - (3)]);
  ;}
    break;

  case 419:
#line 1640 "parser.y"
    { push_state(XHP_LABEL); ;}
    break;

  case 420:
#line 1640 "parser.y"
    {
    pop_state();
    (yyval) = (yyvsp[(2) - (3)]);
  ;}
    break;

  case 421:
#line 1648 "parser.y"
    { push_state(XHP_LABEL_WHITESPACE); ;}
    break;

  case 422:
#line 1648 "parser.y"
    {
    pop_state();
    (yyval) = (yyvsp[(2) - (3)]);
  ;}
    break;

  case 423:
#line 1655 "parser.y"
    {
    // XHP_LABEL is popped in the scanner on " ", ">", "/", or "="
    push_state(XHP_LABEL);
    (yyval) = (yyvsp[(1) - (1)]);
  ;}
    break;

  case 424:
#line 1660 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + "__" + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 425:
#line 1663 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + "_" + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 426:
#line 1669 "parser.y"
    {
    // XHP_LABEL is popped in the scanner on " ", ">", "/", or "="
    push_state(XHP_LABEL);
    (yyval) = (yyvsp[(1) - (1)]);
  ;}
    break;

  case 427:
#line 1674 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 428:
#line 1677 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 431:
#line 1689 "parser.y"
    {
    yyextra->expecting_xhp_class_statements = true;
    yyextra->attribute_decls = "";
    yyextra->attribute_inherit = "";
    yyextra->used_attributes = false;
  ;}
    break;

  case 432:
#line 1694 "parser.y"
    {
    yyextra->expecting_xhp_class_statements = false;
  ;}
    break;

  case 433:
#line 1696 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (10)]) + " xhp_" + (yyvsp[(3) - (10)]) + (yyvsp[(4) - (10)]) + (yyvsp[(5) - (10)]) + (yyvsp[(6) - (10)]) + (yyvsp[(8) - (10)]);
    if (yyextra->used_attributes) {
      (yyval) = (yyval) +
        "protected static function &__xhpAttributeDeclaration() {" +
          "static $_ = -1;" +
          "if ($_ === -1) {" +
            "$_ = array_merge(parent::__xhpAttributeDeclaration(), " +
              yyextra->attribute_inherit +
              "array(" + yyextra->attribute_decls + "));" +
          "}" +
          "return $_;"
        "}";
    }
    (yyval) = (yyval) + (yyvsp[(10) - (10)]);
    yyextra->used = true;
  ;}
    break;

  case 434:
#line 1717 "parser.y"
    { push_state(XHP_ATTR_TYPE_DECL); ;}
    break;

  case 435:
#line 1717 "parser.y"
    {
    pop_state();
    yyextra->used = true;
    yyextra->used_attributes = true;
    (yyval) = ""; // this will be injected when the class closes
  ;}
    break;

  case 436:
#line 1726 "parser.y"
    {;}
    break;

  case 437:
#line 1727 "parser.y"
    {;}
    break;

  case 438:
#line 1731 "parser.y"
    {
    (yyvsp[(1) - (4)]).strip_lines();
    (yyvsp[(2) - (4)]).strip_lines();
    yyextra->attribute_decls = yyextra->attribute_decls +
      "'" + (yyvsp[(2) - (4)]) + "'=>array(" + (yyvsp[(1) - (4)]) + "," + (yyvsp[(3) - (4)]) + ", " + (yyvsp[(4) - (4)]) + "),";
  ;}
    break;

  case 439:
#line 1737 "parser.y"
    {
    (yyvsp[(2) - (2)]).strip_lines();
    yyextra->attribute_inherit = yyextra->attribute_inherit +
      (yyextra->emit_namespaces ? "\\xhp_" : "xhp_") + (yyvsp[(2) - (2)]) + "::__xhpAttributeDeclaration(),";
  ;}
    break;

  case 440:
#line 1745 "parser.y"
    {
    (yyval) = "1, null";
  ;}
    break;

  case 441:
#line 1748 "parser.y"
    {
    (yyval) = "2, null";
  ;}
    break;

  case 442:
#line 1751 "parser.y"
    {
    (yyval) = "3, null";
  ;}
    break;

  case 443:
#line 1754 "parser.y"
    {
    (yyval) = "4, null";
  ;}
    break;

  case 444:
#line 1757 "parser.y"
    {
    (yyval) = "5, '" + (yyvsp[(1) - (1)]) + "'";
  ;}
    break;

  case 445:
#line 1760 "parser.y"
    {
    (yyval) = "6, null";
  ;}
    break;

  case 446:
#line 1763 "parser.y"
    { push_state(PHP); ;}
    break;

  case 447:
#line 1763 "parser.y"
    { pop_state(); ;}
    break;

  case 448:
#line 1763 "parser.y"
    {
    (yyval) = "7, array(" + (yyvsp[(4) - (6)]) + ")";
  ;}
    break;

  case 449:
#line 1766 "parser.y"
    {
    (yyval) = "8, null";
  ;}
    break;

  case 450:
#line 1772 "parser.y"
    {
    (yyvsp[(1) - (1)]).strip_lines();
    (yyval) = (yyvsp[(1) - (1)]);
  ;}
    break;

  case 451:
#line 1776 "parser.y"
    {
    (yyvsp[(3) - (3)]).strip_lines();
    (yyval) = (yyvsp[(1) - (3)]) + ", " + (yyvsp[(3) - (3)]);
  ;}
    break;

  case 452:
#line 1783 "parser.y"
    {
    (yyvsp[(2) - (2)]).strip_lines();
    (yyval) = (yyvsp[(2) - (2)]);
  ;}
    break;

  case 453:
#line 1787 "parser.y"
    {
    (yyvsp[(2) - (2)]).strip_lines();
    (yyval) = (yyvsp[(2) - (2)]);
  ;}
    break;

  case 454:
#line 1791 "parser.y"
    {
    (yyval) = "null";
  ;}
    break;

  case 455:
#line 1797 "parser.y"
    {
    (yyval) = "1";
  ;}
    break;

  case 456:
#line 1800 "parser.y"
    {
    (yyval) = "0";
  ;}
    break;

  case 457:
#line 1807 "parser.y"
    { push_state(PHP_NO_RESERVED_WORDS_PERSIST); ;}
    break;

  case 458:
#line 1807 "parser.y"
    {
    pop_state();
    yyextra->used = true;
    (yyval) =
      "protected function &__xhpCategoryDeclaration() {" +
         code_rope("static $_ = array(") + (yyvsp[(3) - (4)]) + ");" +
        "return $_;" +
      "}";
  ;}
    break;

  case 459:
#line 1819 "parser.y"
    {
    (yyval) = "'" + (yyvsp[(2) - (2)]) + "' => 1";
  ;}
    break;

  case 460:
#line 1822 "parser.y"
    {
    (yyval) = (yyvsp[(1) - (4)]) + ",'" + (yyvsp[(4) - (4)]) + "' => 1";
  ;}
    break;

  case 461:
#line 1829 "parser.y"
    { push_state(XHP_CHILDREN_DECL); ;}
    break;

  case 462:
#line 1829 "parser.y"
    {
    // XHP_CHILDREN_DECL is popped in the scanner on ';'
    yyextra->used = true;
    (yyval) = "protected function &__xhpChildrenDeclaration() {" + (yyvsp[(3) - (4)]) + "}";
  ;}
    break;

  case 463:
#line 1837 "parser.y"
    {
    (yyval) = "static $_ = " + (yyvsp[(1) - (1)]) + "; return $_;";
  ;}
    break;

  case 464:
#line 1840 "parser.y"
    {
    (yyval) = "static $_ = 1; return $_;";
  ;}
    break;

  case 465:
#line 1843 "parser.y"
    {
    (yyval) = "static $_ = 0; return $_;";
  ;}
    break;

  case 466:
#line 1849 "parser.y"
    {
    (yyval) = "array(0, 5, " + (yyvsp[(2) - (3)]) + ")";
  ;}
    break;

  case 467:
#line 1852 "parser.y"
    {
    (yyval) = "array(1, 5, " + (yyvsp[(2) - (4)]) + ")";
  ;}
    break;

  case 468:
#line 1855 "parser.y"
    {
    (yyval) = "array(2, 5, " + (yyvsp[(2) - (4)]) + ")";
  ;}
    break;

  case 469:
#line 1858 "parser.y"
    {
    (yyval) = "array(3, 5, " + (yyvsp[(2) - (4)]) + ")";
  ;}
    break;

  case 471:
#line 1865 "parser.y"
    {
    (yyval) = "array(0, " + (yyvsp[(1) - (1)]) + ")";
  ;}
    break;

  case 472:
#line 1868 "parser.y"
    {
    (yyval) = "array(1, " + (yyvsp[(1) - (2)]) + ")";
  ;}
    break;

  case 473:
#line 1871 "parser.y"
    {
    (yyval) = "array(2, " + (yyvsp[(1) - (2)]) + ")";
  ;}
    break;

  case 474:
#line 1874 "parser.y"
    {
    (yyval) = "array(3, " + (yyvsp[(1) - (2)]) + ")";
  ;}
    break;

  case 475:
#line 1877 "parser.y"
    {
    (yyval) = "array(4, " + (yyvsp[(1) - (3)]) + "," + (yyvsp[(3) - (3)]) + ")";
  ;}
    break;

  case 476:
#line 1880 "parser.y"
    {
    (yyval) = "array(5, " + (yyvsp[(1) - (3)]) + "," + (yyvsp[(3) - (3)]) + ")";
  ;}
    break;

  case 477:
#line 1886 "parser.y"
    {
    (yyval) = "1, null";
  ;}
    break;

  case 478:
#line 1889 "parser.y"
    {
    (yyval) = "2, null";
  ;}
    break;

  case 479:
#line 1892 "parser.y"
    {
    (yyval) = (yyextra->emit_namespaces ? "3, \'\\\\xhp_" + (yyvsp[(2) - (2)]) + "\'" : "3, \'xhp_" + (yyvsp[(2) - (2)]) + "\'");
  ;}
    break;

  case 480:
#line 1895 "parser.y"
    {
    (yyval) = "4, \'" + (yyvsp[(2) - (2)]) + "\'";
  ;}
    break;

  case 481:
#line 1902 "parser.y"
    {
    pop_state();
    push_state(PHP);
    yyextra->used = true;
    (yyval) = (yyextra->emit_namespaces ? "\\xhp_" : "xhp_") + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 482:
#line 1911 "parser.y"
    {
    pop_state();
    push_state(PHP);
    yyextra->used = true;
    (yyval) = (yyextra->emit_namespaces ? "\\xhp_" : "xhp_") + (yyvsp[(2) - (2)]);
  ;}
    break;

  case 483:
#line 1925 "parser.y"
    {
    if (yyextra->idx_expr) {
      yyextra->used = true;
      (yyval) = (yyextra->emit_namespaces ? "\\__xhp_idx(" : "__xhp_idx(") + (yyvsp[(1) - (4)]) + ", " + (yyvsp[(3) - (4)]) + ")";
    } else {
      (yyval) = (yyvsp[(1) - (4)]) + (yyvsp[(2) - (4)]) + (yyvsp[(3) - (4)]) + (yyvsp[(4) - (4)]);
    }
  ;}
    break;


/* Line 1267 of yacc.c.  */
#line 6660 "parser.yacc.cpp"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (yyscanner, root, YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yyscanner, root, yymsg);
	  }
	else
	  {
	    yyerror (yyscanner, root, YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, yyscanner, root);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yyscanner, root);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (yyscanner, root, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, yyscanner, root);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yyscanner, root);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 1936 "parser.y"


const char* yytokname(int tok) {
  if (tok < 255) {
    return NULL;
  }
  return yytname[YYTRANSLATE(tok)];
}

