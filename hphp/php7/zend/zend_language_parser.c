/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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
#define YYBISON_VERSION "2.7"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         zendparse
#define yylex           zendlex
#define yyerror         zenderror
#define yylval          zendlval
#define yychar          zendchar
#define yydebug         zenddebug
#define yynerrs         zendnerrs

/* Copy the first part of user declarations.  */
/* Line 371 of yacc.c  */
#line 1 "zend_language_parser.y"

/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2017 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   |          Nikita Popov <nikic@php.net>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#include "zend.h"
#include "zend_language_scanner.h"
#include "zend_ast.h"


#define YYSIZE_T size_t
#define yytnamerr zend_yytnamerr
static YYSIZE_T zend_yytnamerr(char*, const char*);

#define YYERROR_VERBOSE
#define YYSTYPE zend_parser_stack_elem

#ifdef _MSC_VER
#define YYMALLOC malloc
#define YYFREE free
#endif


/* Line 371 of yacc.c  */
#line 117 "zend_language_parser.c"

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "zend_language_parser.h".  */
#ifndef YY_ZEND_ZEND_LANGUAGE_PARSER_H_INCLUDED
# define YY_ZEND_ZEND_LANGUAGE_PARSER_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int zenddebug;
#endif
/* "%code requires" blocks.  */
/* Line 387 of yacc.c  */
#line 46 "zend_language_parser.y"



/* Line 387 of yacc.c  */
#line 153 "zend_language_parser.c"

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     END = 0,
     T_REQUIRE_ONCE = 258,
     T_REQUIRE = 259,
     T_EVAL = 260,
     T_INCLUDE_ONCE = 261,
     T_INCLUDE = 262,
     T_LOGICAL_OR = 263,
     T_LOGICAL_XOR = 264,
     T_LOGICAL_AND = 265,
     T_PRINT = 266,
     T_YIELD = 267,
     T_DOUBLE_ARROW = 268,
     T_YIELD_FROM = 269,
     T_POW_EQUAL = 270,
     T_SR_EQUAL = 271,
     T_SL_EQUAL = 272,
     T_XOR_EQUAL = 273,
     T_OR_EQUAL = 274,
     T_AND_EQUAL = 275,
     T_MOD_EQUAL = 276,
     T_CONCAT_EQUAL = 277,
     T_DIV_EQUAL = 278,
     T_MUL_EQUAL = 279,
     T_MINUS_EQUAL = 280,
     T_PLUS_EQUAL = 281,
     T_COALESCE = 282,
     T_BOOLEAN_OR = 283,
     T_BOOLEAN_AND = 284,
     T_SPACESHIP = 285,
     T_IS_NOT_IDENTICAL = 286,
     T_IS_IDENTICAL = 287,
     T_IS_NOT_EQUAL = 288,
     T_IS_EQUAL = 289,
     T_IS_GREATER_OR_EQUAL = 290,
     T_IS_SMALLER_OR_EQUAL = 291,
     T_SR = 292,
     T_SL = 293,
     T_INSTANCEOF = 294,
     T_UNSET_CAST = 295,
     T_BOOL_CAST = 296,
     T_OBJECT_CAST = 297,
     T_ARRAY_CAST = 298,
     T_STRING_CAST = 299,
     T_DOUBLE_CAST = 300,
     T_INT_CAST = 301,
     T_DEC = 302,
     T_INC = 303,
     T_POW = 304,
     T_CLONE = 305,
     T_NEW = 306,
     T_NOELSE = 307,
     T_ELSEIF = 308,
     T_ELSE = 309,
     T_ENDIF = 310,
     T_PUBLIC = 311,
     T_PROTECTED = 312,
     T_PRIVATE = 313,
     T_FINAL = 314,
     T_ABSTRACT = 315,
     T_STATIC = 316,
     T_LNUMBER = 317,
     T_DNUMBER = 318,
     T_STRING = 319,
     T_VARIABLE = 320,
     T_INLINE_HTML = 321,
     T_ENCAPSED_AND_WHITESPACE = 322,
     T_CONSTANT_ENCAPSED_STRING = 323,
     T_STRING_VARNAME = 324,
     T_NUM_STRING = 325,
     T_EXIT = 326,
     T_IF = 327,
     T_ECHO = 328,
     T_DO = 329,
     T_WHILE = 330,
     T_ENDWHILE = 331,
     T_FOR = 332,
     T_ENDFOR = 333,
     T_FOREACH = 334,
     T_ENDFOREACH = 335,
     T_DECLARE = 336,
     T_ENDDECLARE = 337,
     T_AS = 338,
     T_SWITCH = 339,
     T_ENDSWITCH = 340,
     T_CASE = 341,
     T_DEFAULT = 342,
     T_BREAK = 343,
     T_CONTINUE = 344,
     T_GOTO = 345,
     T_FUNCTION = 346,
     T_CONST = 347,
     T_RETURN = 348,
     T_TRY = 349,
     T_CATCH = 350,
     T_FINALLY = 351,
     T_THROW = 352,
     T_USE = 353,
     T_INSTEADOF = 354,
     T_GLOBAL = 355,
     T_VAR = 356,
     T_UNSET = 357,
     T_ISSET = 358,
     T_EMPTY = 359,
     T_HALT_COMPILER = 360,
     T_CLASS = 361,
     T_TRAIT = 362,
     T_INTERFACE = 363,
     T_EXTENDS = 364,
     T_IMPLEMENTS = 365,
     T_OBJECT_OPERATOR = 366,
     T_LIST = 367,
     T_ARRAY = 368,
     T_CALLABLE = 369,
     T_LINE = 370,
     T_FILE = 371,
     T_DIR = 372,
     T_CLASS_C = 373,
     T_TRAIT_C = 374,
     T_METHOD_C = 375,
     T_FUNC_C = 376,
     T_COMMENT = 377,
     T_DOC_COMMENT = 378,
     T_OPEN_TAG = 379,
     T_OPEN_TAG_WITH_ECHO = 380,
     T_CLOSE_TAG = 381,
     T_WHITESPACE = 382,
     T_START_HEREDOC = 383,
     T_END_HEREDOC = 384,
     T_DOLLAR_OPEN_CURLY_BRACES = 385,
     T_CURLY_OPEN = 386,
     T_PAAMAYIM_NEKUDOTAYIM = 387,
     T_NAMESPACE = 388,
     T_NS_C = 389,
     T_NS_SEPARATOR = 390,
     T_ELLIPSIS = 391,
     T_ERROR = 392
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int zendparse (void *YYPARSE_PARAM);
#else
int zendparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int zendparse (void);
#else
int zendparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_ZEND_ZEND_LANGUAGE_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

/* Line 390 of yacc.c  */
#line 327 "zend_language_parser.c"

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
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(N) (N)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
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
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   7393

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  166
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  138
/* YYNRULES -- Number of rules.  */
#define YYNRULES  498
/* YYNRULES -- Number of states.  */
#define YYNSTATES  945

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   392

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    54,   164,     2,   165,    53,    36,     2,
     157,   158,    51,    48,     8,    49,    50,    52,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    30,   159,
      42,    16,    43,    29,    57,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    68,     2,   162,    35,     2,   163,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   160,    34,   161,    56,     2,     2,     2,
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
       5,     6,     7,     9,    10,    11,    12,    13,    14,    15,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    31,    32,    33,    37,    38,    39,    40,    41,
      44,    45,    46,    47,    55,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    13,    15,    17,
      19,    21,    23,    25,    27,    29,    31,    33,    35,    37,
      39,    41,    43,    45,    47,    49,    51,    53,    55,    57,
      59,    61,    63,    65,    67,    69,    71,    73,    75,    77,
      79,    81,    83,    85,    87,    89,    91,    93,    95,    97,
      99,   101,   103,   105,   107,   109,   111,   113,   115,   117,
     119,   121,   123,   125,   127,   129,   131,   133,   135,   137,
     139,   141,   143,   145,   147,   149,   151,   153,   155,   157,
     160,   161,   163,   167,   169,   173,   176,   178,   180,   182,
     184,   186,   191,   195,   196,   203,   204,   210,   214,   219,
     223,   228,   232,   234,   236,   243,   251,   258,   266,   267,
     269,   273,   275,   279,   281,   285,   287,   289,   292,   294,
     298,   300,   303,   307,   309,   312,   313,   315,   317,   319,
     321,   323,   328,   332,   334,   336,   342,   350,   360,   366,
     370,   374,   378,   382,   386,   390,   392,   395,   401,   409,
     419,   420,   427,   429,   436,   440,   444,   447,   448,   458,
     460,   464,   465,   470,   472,   476,   478,   492,   493,   495,
     496,   498,   499,   510,   511,   521,   523,   526,   528,   530,
     531,   539,   540,   549,   550,   553,   554,   557,   558,   561,
     563,   566,   571,   575,   577,   582,   584,   589,   591,   596,
     600,   605,   610,   616,   617,   623,   628,   630,   632,   634,
     639,   645,   652,   654,   658,   665,   673,   677,   684,   686,
     687,   689,   693,   698,   705,   706,   708,   710,   713,   715,
     717,   719,   720,   723,   726,   730,   732,   736,   738,   741,
     745,   747,   749,   753,   755,   757,   761,   764,   765,   769,
     774,   778,   791,   793,   797,   799,   802,   806,   808,   811,
     814,   817,   821,   825,   829,   834,   838,   840,   842,   846,
     848,   852,   854,   856,   857,   859,   861,   864,   866,   868,
     870,   872,   874,   876,   880,   882,   885,   890,   894,   896,
     901,   906,   910,   912,   914,   915,   917,   921,   923,   924,
     934,   938,   941,   948,   954,   958,   963,   966,   970,   974,
     978,   982,   986,   990,   994,   998,  1002,  1006,  1010,  1014,
    1017,  1020,  1023,  1026,  1030,  1034,  1038,  1042,  1046,  1050,
    1054,  1058,  1062,  1066,  1070,  1074,  1078,  1082,  1086,  1090,
    1094,  1097,  1100,  1103,  1106,  1110,  1114,  1118,  1122,  1126,
    1130,  1134,  1138,  1142,  1146,  1150,  1152,  1158,  1163,  1167,
    1169,  1172,  1175,  1178,  1181,  1184,  1187,  1190,  1193,  1196,
    1198,  1202,  1205,  1207,  1210,  1215,  1218,  1232,  1247,  1249,
    1250,  1251,  1252,  1254,  1255,  1260,  1264,  1266,  1268,  1271,
    1274,  1279,  1284,  1287,  1289,  1291,  1293,  1295,  1296,  1300,
    1301,  1303,  1305,  1306,  1308,  1313,  1317,  1319,  1321,  1323,
    1325,  1327,  1329,  1331,  1333,  1335,  1337,  1339,  1343,  1346,
    1350,  1354,  1356,  1358,  1360,  1364,  1368,  1370,  1372,  1373,
    1375,  1377,  1379,  1383,  1385,  1387,  1391,  1393,  1395,  1400,
    1405,  1410,  1415,  1417,  1419,  1421,  1425,  1427,  1432,  1435,
    1439,  1443,  1445,  1450,  1455,  1459,  1463,  1467,  1469,  1473,
    1475,  1477,  1481,  1483,  1485,  1486,  1488,  1492,  1494,  1498,
    1500,  1505,  1508,  1515,  1520,  1523,  1526,  1528,  1531,  1533,
    1538,  1542,  1546,  1550,  1557,  1561,  1563,  1565,  1568,  1570,
    1575,  1580,  1583,  1586,  1591,  1594,  1597,  1599,  1603
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     167,     0,    -1,   171,    -1,     7,    -1,     6,    -1,     5,
      -1,     4,    -1,     3,    -1,     9,    -1,    10,    -1,    11,
      -1,    55,    -1,    70,    -1,    69,    -1,    90,    -1,    91,
      -1,    72,    -1,    73,    -1,    74,    -1,    92,    -1,    93,
      -1,    94,    -1,    95,    -1,    96,    -1,    97,    -1,    98,
      -1,    99,    -1,   100,    -1,   101,    -1,   102,    -1,   113,
      -1,   114,    -1,   115,    -1,   116,    -1,   117,    -1,   118,
      -1,   119,    -1,   120,    -1,   121,    -1,   122,    -1,   123,
      -1,   108,    -1,   109,    -1,   110,    -1,   111,    -1,   112,
      -1,    12,    -1,    13,    -1,   131,    -1,   103,    -1,   104,
      -1,   105,    -1,   106,    -1,   107,    -1,   132,    -1,   133,
      -1,   128,    -1,   129,    -1,   152,    -1,   126,    -1,   127,
      -1,   125,    -1,   137,    -1,   138,    -1,   140,    -1,   139,
      -1,   134,    -1,   135,    -1,   136,    -1,   153,    -1,   168,
      -1,    80,    -1,    79,    -1,    78,    -1,    77,    -1,    76,
      -1,    75,    -1,    83,    -1,   169,    -1,   171,   174,    -1,
      -1,    83,    -1,   172,   154,    83,    -1,   172,    -1,   152,
     154,   172,    -1,   154,   172,    -1,   190,    -1,   197,    -1,
     200,    -1,   205,    -1,   207,    -1,   124,   157,   158,   159,
      -1,   152,   172,   159,    -1,    -1,   152,   172,   175,   160,
     171,   161,    -1,    -1,   152,   176,   160,   171,   161,    -1,
     117,   179,   159,    -1,   117,   177,   178,   159,    -1,   117,
     183,   159,    -1,   117,   177,   183,   159,    -1,   111,   187,
     159,    -1,   110,    -1,   111,    -1,   172,   154,   160,   182,
     180,   161,    -1,   154,   172,   154,   160,   182,   180,   161,
      -1,   172,   154,   160,   181,   180,   161,    -1,   154,   172,
     154,   160,   181,   180,   161,    -1,    -1,     8,    -1,   181,
       8,   184,    -1,   184,    -1,   182,     8,   185,    -1,   185,
      -1,   183,     8,   186,    -1,   186,    -1,   185,    -1,   177,
     185,    -1,   172,    -1,   172,   102,    83,    -1,   185,    -1,
     154,   185,    -1,   187,     8,   257,    -1,   257,    -1,   188,
     189,    -1,    -1,   190,    -1,   197,    -1,   200,    -1,   205,
      -1,   207,    -1,   124,   157,   158,   159,    -1,   160,   188,
     161,    -1,   221,    -1,   223,    -1,    94,   157,   282,   158,
     219,    -1,    93,   190,    94,   157,   282,   158,   159,    -1,
      96,   157,   260,   159,   260,   159,   260,   158,   213,    -1,
     103,   157,   282,   158,   216,    -1,   107,   283,   159,    -1,
     108,   283,   159,    -1,   112,   283,   159,    -1,   119,   234,
     159,    -1,    80,   236,   159,    -1,    92,   258,   159,    -1,
      85,    -1,   282,   159,    -1,   121,   157,   195,   158,   159,
      -1,    98,   157,   282,   102,   212,   158,   214,    -1,    98,
     157,   282,   102,   212,    14,   212,   158,   214,    -1,    -1,
     100,   157,   187,   158,   191,   215,    -1,   159,    -1,   113,
     160,   188,   161,   192,   194,    -1,   116,   282,   159,    -1,
     109,    83,   159,    -1,    83,    30,    -1,    -1,   192,   114,
     157,   193,    84,   158,   160,   188,   161,    -1,   173,    -1,
     193,    34,   173,    -1,    -1,   115,   160,   188,   161,    -1,
     196,    -1,   195,     8,   196,    -1,   288,    -1,   266,   269,
      83,   267,   157,   224,   158,   230,   268,   160,   188,   161,
     268,    -1,    -1,    36,    -1,    -1,   155,    -1,    -1,   203,
     125,   201,    83,   209,   211,   267,   160,   238,   161,    -1,
      -1,   125,   202,    83,   209,   211,   267,   160,   238,   161,
      -1,   204,    -1,   203,   204,    -1,    79,    -1,    78,    -1,
      -1,   126,   206,    83,   267,   160,   238,   161,    -1,    -1,
     127,   208,    83,   210,   267,   160,   238,   161,    -1,    -1,
     128,   173,    -1,    -1,   128,   240,    -1,    -1,   129,   240,
      -1,   288,    -1,    36,   288,    -1,   131,   157,   294,   158,
      -1,    68,   294,   162,    -1,   190,    -1,    30,   188,    97,
     159,    -1,   190,    -1,    30,   188,    99,   159,    -1,   190,
      -1,    30,   188,   101,   159,    -1,   160,   217,   161,    -1,
     160,   159,   217,   161,    -1,    30,   217,   104,   159,    -1,
      30,   159,   217,   104,   159,    -1,    -1,   217,   105,   282,
     218,   188,    -1,   217,   106,   218,   188,    -1,    30,    -1,
     159,    -1,   190,    -1,    30,   188,    95,   159,    -1,    91,
     157,   282,   158,   190,    -1,   220,    72,   157,   282,   158,
     190,    -1,   220,    -1,   220,    73,   190,    -1,    91,   157,
     282,   158,    30,   188,    -1,   222,    72,   157,   282,   158,
      30,   188,    -1,   222,    74,   159,    -1,   222,    73,    30,
     188,    74,   159,    -1,   225,    -1,    -1,   226,    -1,   225,
       8,   226,    -1,   227,   198,   199,    84,    -1,   227,   198,
     199,    84,    16,   282,    -1,    -1,   228,    -1,   229,    -1,
      29,   229,    -1,   132,    -1,   133,    -1,   173,    -1,    -1,
      30,   228,    -1,   157,   158,    -1,   157,   232,   158,    -1,
     233,    -1,   232,     8,   233,    -1,   282,    -1,   155,   282,
      -1,   234,     8,   235,    -1,   235,    -1,   289,    -1,   236,
       8,   237,    -1,   237,    -1,    84,    -1,    84,    16,   282,
      -1,   238,   239,    -1,    -1,   249,   253,   159,    -1,   250,
     111,   255,   159,    -1,   117,   240,   241,    -1,   250,   266,
     269,   170,   267,   157,   224,   158,   230,   268,   248,   268,
      -1,   173,    -1,   240,     8,   173,    -1,   159,    -1,   160,
     161,    -1,   160,   242,   161,    -1,   243,    -1,   242,   243,
      -1,   244,   159,    -1,   245,   159,    -1,   247,   118,   240,
      -1,   246,   102,    83,    -1,   246,   102,   168,    -1,   246,
     102,   252,   170,    -1,   246,   102,   252,    -1,   170,    -1,
     247,    -1,   173,   151,   170,    -1,   159,    -1,   160,   188,
     161,    -1,   251,    -1,   120,    -1,    -1,   251,    -1,   252,
      -1,   251,   252,    -1,    75,    -1,    76,    -1,    77,    -1,
      80,    -1,    79,    -1,    78,    -1,   253,     8,   254,    -1,
     254,    -1,    84,   267,    -1,    84,    16,   282,   267,    -1,
     255,     8,   256,    -1,   256,    -1,   170,    16,   282,   267,
      -1,    83,    16,   282,   267,    -1,   258,     8,   259,    -1,
     259,    -1,   282,    -1,    -1,   261,    -1,   261,     8,   282,
      -1,   282,    -1,    -1,   125,   263,   278,   209,   211,   267,
     160,   238,   161,    -1,    70,   275,   278,    -1,    70,   262,
      -1,   131,   157,   294,   158,    16,   282,    -1,    68,   294,
     162,    16,   282,    -1,   288,    16,   282,    -1,   288,    16,
      36,   288,    -1,    69,   282,    -1,   288,    28,   282,    -1,
     288,    27,   282,    -1,   288,    26,   282,    -1,   288,    17,
     282,    -1,   288,    25,   282,    -1,   288,    24,   282,    -1,
     288,    23,   282,    -1,   288,    22,   282,    -1,   288,    21,
     282,    -1,   288,    20,   282,    -1,   288,    19,   282,    -1,
     288,    18,   282,    -1,   288,    66,    -1,    66,   288,    -1,
     288,    65,    -1,    65,   288,    -1,   282,    32,   282,    -1,
     282,    33,   282,    -1,   282,     9,   282,    -1,   282,    11,
     282,    -1,   282,    10,   282,    -1,   282,    34,   282,    -1,
     282,    36,   282,    -1,   282,    35,   282,    -1,   282,    50,
     282,    -1,   282,    48,   282,    -1,   282,    49,   282,    -1,
     282,    51,   282,    -1,   282,    67,   282,    -1,   282,    52,
     282,    -1,   282,    53,   282,    -1,   282,    47,   282,    -1,
     282,    46,   282,    -1,    48,   282,    -1,    49,   282,    -1,
      54,   282,    -1,    56,   282,    -1,   282,    39,   282,    -1,
     282,    38,   282,    -1,   282,    41,   282,    -1,   282,    40,
     282,    -1,   282,    42,   282,    -1,   282,    45,   282,    -1,
     282,    43,   282,    -1,   282,    44,   282,    -1,   282,    37,
     282,    -1,   282,    55,   275,    -1,   157,   282,   158,    -1,
     264,    -1,   282,    29,   282,    30,   282,    -1,   282,    29,
      30,   282,    -1,   282,    31,   282,    -1,   301,    -1,    64,
     282,    -1,    63,   282,    -1,    62,   282,    -1,    61,   282,
      -1,    60,   282,    -1,    59,   282,    -1,    58,   282,    -1,
      90,   276,    -1,    57,   282,    -1,   280,    -1,   163,   277,
     163,    -1,    12,   282,    -1,    13,    -1,    13,   282,    -1,
      13,   282,    14,   282,    -1,    15,   282,    -1,   266,   269,
     267,   157,   224,   158,   270,   230,   268,   160,   188,   161,
     268,    -1,    80,   266,   269,   267,   157,   224,   158,   270,
     230,   268,   160,   188,   161,   268,    -1,   110,    -1,    -1,
      -1,    -1,    36,    -1,    -1,   117,   157,   271,   158,    -1,
     271,     8,   272,    -1,   272,    -1,    84,    -1,    36,    84,
      -1,   173,   231,    -1,   274,   151,   292,   231,    -1,   284,
     151,   292,   231,    -1,   286,   231,    -1,    80,    -1,   173,
      -1,   274,    -1,   291,    -1,    -1,   157,   283,   158,    -1,
      -1,    86,    -1,   298,    -1,    -1,   231,    -1,   132,   157,
     294,   158,    -1,    68,   294,   162,    -1,    87,    -1,    81,
      -1,    82,    -1,   134,    -1,   135,    -1,   136,    -1,   138,
      -1,   139,    -1,   140,    -1,   153,    -1,   137,    -1,   147,
      86,   148,    -1,   147,   148,    -1,   164,   298,   164,    -1,
     147,   298,   148,    -1,   279,    -1,   281,    -1,   173,    -1,
     274,   151,   170,    -1,   284,   151,   170,    -1,   288,    -1,
     265,    -1,    -1,   282,    -1,   285,    -1,   288,    -1,   157,
     282,   158,    -1,   279,    -1,   287,    -1,   157,   282,   158,
      -1,   279,    -1,   289,    -1,   285,    68,   283,   162,    -1,
     281,    68,   283,   162,    -1,   285,   160,   282,   161,    -1,
     285,   130,   293,   231,    -1,   273,    -1,   287,    -1,   290,
      -1,   285,   130,   293,    -1,    84,    -1,   165,   160,   282,
     161,    -1,   165,   289,    -1,   274,   151,   289,    -1,   284,
     151,   289,    -1,   289,    -1,   291,    68,   283,   162,    -1,
     291,   160,   282,   161,    -1,   291,   130,   293,    -1,   274,
     151,   289,    -1,   291,   151,   289,    -1,   170,    -1,   160,
     282,   161,    -1,   289,    -1,    83,    -1,   160,   282,   161,
      -1,   289,    -1,   296,    -1,    -1,   297,    -1,   296,     8,
     295,    -1,   295,    -1,   282,    14,   282,    -1,   282,    -1,
     282,    14,    36,   288,    -1,    36,   288,    -1,   282,    14,
     131,   157,   294,   158,    -1,   131,   157,   294,   158,    -1,
     298,   299,    -1,   298,    86,    -1,   299,    -1,    86,   299,
      -1,    84,    -1,    84,    68,   300,   162,    -1,    84,   130,
      83,    -1,   149,   282,   161,    -1,   149,    88,   161,    -1,
     149,    88,    68,   282,   162,   161,    -1,   150,   288,   161,
      -1,    83,    -1,    89,    -1,    49,    89,    -1,    84,    -1,
     122,   157,   302,   158,    -1,   123,   157,   282,   158,    -1,
       7,   282,    -1,     6,   282,    -1,     5,   157,   282,   158,
      -1,     4,   282,    -1,     3,   282,    -1,   303,    -1,   302,
       8,   303,    -1,   282,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   263,   263,   267,   267,   267,   267,   267,   267,   267,
     267,   268,   268,   268,   268,   268,   268,   268,   268,   268,
     268,   268,   268,   269,   269,   269,   269,   269,   269,   269,
     269,   269,   269,   270,   270,   270,   270,   270,   270,   270,
     270,   270,   270,   271,   271,   271,   271,   271,   271,   271,
     271,   271,   271,   271,   272,   272,   272,   272,   272,   272,
     272,   272,   273,   273,   273,   273,   273,   273,   273,   273,
     277,   278,   278,   278,   278,   278,   278,   282,   283,   291,
     292,   296,   297,   301,   302,   303,   307,   308,   309,   310,
     311,   312,   316,   319,   319,   322,   322,   325,   326,   327,
     328,   329,   333,   334,   338,   340,   345,   347,   351,   353,
     357,   359,   364,   366,   371,   373,   378,   379,   383,   385,
     390,   391,   395,   396,   400,   403,   408,   409,   410,   411,
     412,   413,   420,   421,   422,   423,   425,   427,   429,   431,
     432,   433,   434,   435,   436,   437,   438,   439,   440,   442,
     446,   445,   449,   450,   452,   453,   454,   459,   460,   465,
     466,   470,   471,   475,   476,   480,   484,   491,   492,   496,
     497,   501,   501,   504,   504,   510,   511,   515,   516,   520,
     520,   526,   526,   532,   533,   537,   538,   542,   543,   547,
     548,   549,   550,   554,   555,   559,   560,   564,   565,   569,
     570,   571,   572,   576,   577,   579,   584,   585,   590,   591,
     596,   599,   605,   606,   611,   614,   620,   621,   627,   628,
     633,   635,   640,   642,   648,   649,   653,   654,   658,   659,
     660,   664,   665,   669,   670,   674,   676,   681,   682,   686,
     687,   691,   697,   698,   702,   703,   708,   711,   716,   718,
     720,   722,   729,   730,   734,   735,   736,   740,   742,   747,
     748,   752,   757,   759,   761,   763,   768,   770,   774,   779,
     780,   784,   785,   789,   790,   795,   796,   801,   802,   803,
     804,   805,   806,   810,   811,   815,   817,   822,   823,   827,
     831,   835,   836,   839,   843,   844,   848,   849,   853,   853,
     863,   865,   870,   872,   874,   876,   878,   879,   881,   883,
     885,   887,   889,   891,   893,   895,   897,   899,   901,   903,
     904,   905,   906,   907,   909,   911,   913,   915,   917,   918,
     919,   920,   921,   922,   923,   924,   925,   926,   927,   928,
     929,   930,   931,   932,   933,   935,   937,   939,   941,   943,
     945,   947,   949,   951,   953,   954,   955,   957,   959,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   983,   991,   995,
     999,  1003,  1004,  1008,  1009,  1013,  1014,  1018,  1019,  1023,
    1025,  1027,  1029,  1034,  1037,  1041,  1042,  1046,  1047,  1052,
    1053,  1054,  1059,  1060,  1065,  1066,  1067,  1071,  1072,  1073,
    1074,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1084,
    1085,  1086,  1087,  1091,  1092,  1094,  1099,  1100,  1104,  1105,
    1109,  1113,  1114,  1115,  1119,  1120,  1121,  1125,  1127,  1129,
    1131,  1133,  1135,  1139,  1141,  1143,  1148,  1149,  1150,  1154,
    1156,  1161,  1163,  1165,  1167,  1169,  1171,  1176,  1177,  1178,
    1182,  1183,  1184,  1188,  1193,  1194,  1198,  1200,  1205,  1207,
    1209,  1211,  1213,  1216,  1222,  1224,  1226,  1228,  1233,  1235,
    1238,  1241,  1243,  1245,  1248,  1252,  1253,  1254,  1255,  1260,
    1261,  1262,  1264,  1266,  1268,  1270,  1275,  1276,  1281
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "$undefined",
  "\"require_once (T_REQUIRE_ONCE)\"", "\"require (T_REQUIRE)\"",
  "\"eval (T_EVAL)\"", "\"include_once (T_INCLUDE_ONCE)\"",
  "\"include (T_INCLUDE)\"", "','", "\"or (T_LOGICAL_OR)\"",
  "\"xor (T_LOGICAL_XOR)\"", "\"and (T_LOGICAL_AND)\"",
  "\"print (T_PRINT)\"", "\"yield (T_YIELD)\"", "\"=> (T_DOUBLE_ARROW)\"",
  "\"yield from (T_YIELD_FROM)\"", "'='", "\"**= (T_POW_EQUAL)\"",
  "\">>= (T_SR_EQUAL)\"", "\"<<= (T_SL_EQUAL)\"", "\"^= (T_XOR_EQUAL)\"",
  "\"|= (T_OR_EQUAL)\"", "\"&= (T_AND_EQUAL)\"", "\"%= (T_MOD_EQUAL)\"",
  "\".= (T_CONCAT_EQUAL)\"", "\"/= (T_DIV_EQUAL)\"",
  "\"*= (T_MUL_EQUAL)\"", "\"-= (T_MINUS_EQUAL)\"",
  "\"+= (T_PLUS_EQUAL)\"", "'?'", "':'", "\"?? (T_COALESCE)\"",
  "\"|| (T_BOOLEAN_OR)\"", "\"&& (T_BOOLEAN_AND)\"", "'|'", "'^'", "'&'",
  "\"<=> (T_SPACESHIP)\"", "\"!== (T_IS_NOT_IDENTICAL)\"",
  "\"=== (T_IS_IDENTICAL)\"", "\"!= (T_IS_NOT_EQUAL)\"",
  "\"== (T_IS_EQUAL)\"", "'<'", "'>'", "\">= (T_IS_GREATER_OR_EQUAL)\"",
  "\"<= (T_IS_SMALLER_OR_EQUAL)\"", "\">> (T_SR)\"", "\"<< (T_SL)\"",
  "'+'", "'-'", "'.'", "'*'", "'/'", "'%'", "'!'",
  "\"instanceof (T_INSTANCEOF)\"", "'~'", "'@'",
  "\"(unset) (T_UNSET_CAST)\"", "\"(bool) (T_BOOL_CAST)\"",
  "\"(object) (T_OBJECT_CAST)\"", "\"(array) (T_ARRAY_CAST)\"",
  "\"(string) (T_STRING_CAST)\"", "\"(double) (T_DOUBLE_CAST)\"",
  "\"(int) (T_INT_CAST)\"", "\"-- (T_DEC)\"", "\"++ (T_INC)\"",
  "\"** (T_POW)\"", "'['", "\"clone (T_CLONE)\"", "\"new (T_NEW)\"",
  "T_NOELSE", "\"elseif (T_ELSEIF)\"", "\"else (T_ELSE)\"",
  "\"endif (T_ENDIF)\"", "\"public (T_PUBLIC)\"",
  "\"protected (T_PROTECTED)\"", "\"private (T_PRIVATE)\"",
  "\"final (T_FINAL)\"", "\"abstract (T_ABSTRACT)\"",
  "\"static (T_STATIC)\"", "\"integer number (T_LNUMBER)\"",
  "\"floating-point number (T_DNUMBER)\"", "\"identifier (T_STRING)\"",
  "\"variable (T_VARIABLE)\"", "T_INLINE_HTML",
  "\"quoted-string and whitespace (T_ENCAPSED_AND_WHITESPACE)\"",
  "\"quoted-string (T_CONSTANT_ENCAPSED_STRING)\"",
  "\"variable name (T_STRING_VARNAME)\"", "\"number (T_NUM_STRING)\"",
  "\"exit (T_EXIT)\"", "\"if (T_IF)\"", "\"echo (T_ECHO)\"",
  "\"do (T_DO)\"", "\"while (T_WHILE)\"", "\"endwhile (T_ENDWHILE)\"",
  "\"for (T_FOR)\"", "\"endfor (T_ENDFOR)\"", "\"foreach (T_FOREACH)\"",
  "\"endforeach (T_ENDFOREACH)\"", "\"declare (T_DECLARE)\"",
  "\"enddeclare (T_ENDDECLARE)\"", "\"as (T_AS)\"",
  "\"switch (T_SWITCH)\"", "\"endswitch (T_ENDSWITCH)\"",
  "\"case (T_CASE)\"", "\"default (T_DEFAULT)\"", "\"break (T_BREAK)\"",
  "\"continue (T_CONTINUE)\"", "\"goto (T_GOTO)\"",
  "\"function (T_FUNCTION)\"", "\"const (T_CONST)\"",
  "\"return (T_RETURN)\"", "\"try (T_TRY)\"", "\"catch (T_CATCH)\"",
  "\"finally (T_FINALLY)\"", "\"throw (T_THROW)\"", "\"use (T_USE)\"",
  "\"insteadof (T_INSTEADOF)\"", "\"global (T_GLOBAL)\"",
  "\"var (T_VAR)\"", "\"unset (T_UNSET)\"", "\"isset (T_ISSET)\"",
  "\"empty (T_EMPTY)\"", "\"__halt_compiler (T_HALT_COMPILER)\"",
  "\"class (T_CLASS)\"", "\"trait (T_TRAIT)\"",
  "\"interface (T_INTERFACE)\"", "\"extends (T_EXTENDS)\"",
  "\"implements (T_IMPLEMENTS)\"", "\"-> (T_OBJECT_OPERATOR)\"",
  "\"list (T_LIST)\"", "\"array (T_ARRAY)\"", "\"callable (T_CALLABLE)\"",
  "\"__LINE__ (T_LINE)\"", "\"__FILE__ (T_FILE)\"", "\"__DIR__ (T_DIR)\"",
  "\"__CLASS__ (T_CLASS_C)\"", "\"__TRAIT__ (T_TRAIT_C)\"",
  "\"__METHOD__ (T_METHOD_C)\"", "\"__FUNCTION__ (T_FUNC_C)\"",
  "\"comment (T_COMMENT)\"", "\"doc comment (T_DOC_COMMENT)\"",
  "\"open tag (T_OPEN_TAG)\"",
  "\"open tag with echo (T_OPEN_TAG_WITH_ECHO)\"",
  "\"close tag (T_CLOSE_TAG)\"", "\"whitespace (T_WHITESPACE)\"",
  "\"heredoc start (T_START_HEREDOC)\"", "\"heredoc end (T_END_HEREDOC)\"",
  "\"${ (T_DOLLAR_OPEN_CURLY_BRACES)\"", "\"{$ (T_CURLY_OPEN)\"",
  "\":: (T_PAAMAYIM_NEKUDOTAYIM)\"", "\"namespace (T_NAMESPACE)\"",
  "\"__NAMESPACE__ (T_NS_C)\"", "\"\\\\ (T_NS_SEPARATOR)\"",
  "\"... (T_ELLIPSIS)\"", "T_ERROR", "'('", "')'", "';'", "'{'", "'}'",
  "']'", "'`'", "'\"'", "'$'", "$accept", "start",
  "reserved_non_modifiers", "semi_reserved", "identifier",
  "top_statement_list", "namespace_name", "name", "top_statement", "$@1",
  "$@2", "use_type", "group_use_declaration",
  "mixed_group_use_declaration", "possible_comma",
  "inline_use_declarations", "unprefixed_use_declarations",
  "use_declarations", "inline_use_declaration",
  "unprefixed_use_declaration", "use_declaration", "const_list",
  "inner_statement_list", "inner_statement", "statement", "$@3",
  "catch_list", "catch_name_list", "finally_statement", "unset_variables",
  "unset_variable", "function_declaration_statement", "is_reference",
  "is_variadic", "class_declaration_statement", "@4", "@5",
  "class_modifiers", "class_modifier", "trait_declaration_statement", "@6",
  "interface_declaration_statement", "@7", "extends_from",
  "interface_extends_list", "implements_list", "foreach_variable",
  "for_statement", "foreach_statement", "declare_statement",
  "switch_case_list", "case_list", "case_separator", "while_statement",
  "if_stmt_without_else", "if_stmt", "alt_if_stmt_without_else",
  "alt_if_stmt", "parameter_list", "non_empty_parameter_list", "parameter",
  "optional_type", "type_expr", "type", "return_type", "argument_list",
  "non_empty_argument_list", "argument", "global_var_list", "global_var",
  "static_var_list", "static_var", "class_statement_list",
  "class_statement", "name_list", "trait_adaptations",
  "trait_adaptation_list", "trait_adaptation", "trait_precedence",
  "trait_alias", "trait_method_reference",
  "absolute_trait_method_reference", "method_body", "variable_modifiers",
  "method_modifiers", "non_empty_member_modifiers", "member_modifier",
  "property_list", "property", "class_const_list", "class_const_decl",
  "const_decl", "echo_expr_list", "echo_expr", "for_exprs",
  "non_empty_for_exprs", "anonymous_class", "@8", "new_expr",
  "expr_without_variable", "function", "backup_doc_comment",
  "backup_fn_flags", "returns_ref", "lexical_vars", "lexical_var_list",
  "lexical_var", "function_call", "class_name", "class_name_reference",
  "exit_expr", "backticks_expr", "ctor_arguments", "dereferencable_scalar",
  "scalar", "constant", "expr", "optional_expr", "variable_class_name",
  "dereferencable", "callable_expr", "callable_variable", "variable",
  "simple_variable", "static_member", "new_variable", "member_name",
  "property_name", "array_pair_list", "possible_array_pair",
  "non_empty_array_pair_list", "array_pair", "encaps_list", "encaps_var",
  "encaps_var_offset", "internal_functions_in_yacc", "isset_variables",
  "isset_variable", YY_NULL
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,    44,   263,
     264,   265,   266,   267,   268,   269,    61,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,    63,
      58,   282,   283,   284,   124,    94,    38,   285,   286,   287,
     288,   289,    60,    62,   290,   291,   292,   293,    43,    45,
      46,    42,    47,    37,    33,   294,   126,    64,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,    91,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   389,   390,   391,   392,    40,    41,    59,
     123,   125,    93,    96,    34,    36
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   166,   167,   168,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
     169,   169,   169,   169,   169,   169,   169,   170,   170,   171,
     171,   172,   172,   173,   173,   173,   174,   174,   174,   174,
     174,   174,   174,   175,   174,   176,   174,   174,   174,   174,
     174,   174,   177,   177,   178,   178,   179,   179,   180,   180,
     181,   181,   182,   182,   183,   183,   184,   184,   185,   185,
     186,   186,   187,   187,   188,   188,   189,   189,   189,   189,
     189,   189,   190,   190,   190,   190,   190,   190,   190,   190,
     190,   190,   190,   190,   190,   190,   190,   190,   190,   190,
     191,   190,   190,   190,   190,   190,   190,   192,   192,   193,
     193,   194,   194,   195,   195,   196,   197,   198,   198,   199,
     199,   201,   200,   202,   200,   203,   203,   204,   204,   206,
     205,   208,   207,   209,   209,   210,   210,   211,   211,   212,
     212,   212,   212,   213,   213,   214,   214,   215,   215,   216,
     216,   216,   216,   217,   217,   217,   218,   218,   219,   219,
     220,   220,   221,   221,   222,   222,   223,   223,   224,   224,
     225,   225,   226,   226,   227,   227,   228,   228,   229,   229,
     229,   230,   230,   231,   231,   232,   232,   233,   233,   234,
     234,   235,   236,   236,   237,   237,   238,   238,   239,   239,
     239,   239,   240,   240,   241,   241,   241,   242,   242,   243,
     243,   244,   245,   245,   245,   245,   246,   246,   247,   248,
     248,   249,   249,   250,   250,   251,   251,   252,   252,   252,
     252,   252,   252,   253,   253,   254,   254,   255,   255,   256,
     257,   258,   258,   259,   260,   260,   261,   261,   263,   262,
     264,   264,   265,   265,   265,   265,   265,   265,   265,   265,
     265,   265,   265,   265,   265,   265,   265,   265,   265,   265,
     265,   265,   265,   265,   265,   265,   265,   265,   265,   265,
     265,   265,   265,   265,   265,   265,   265,   265,   265,   265,
     265,   265,   265,   265,   265,   265,   265,   265,   265,   265,
     265,   265,   265,   265,   265,   265,   265,   265,   265,   265,
     265,   265,   265,   265,   265,   265,   265,   265,   265,   265,
     265,   265,   265,   265,   265,   265,   265,   265,   266,   267,
     268,   269,   269,   270,   270,   271,   271,   272,   272,   273,
     273,   273,   273,   274,   274,   275,   275,   276,   276,   277,
     277,   277,   278,   278,   279,   279,   279,   280,   280,   280,
     280,   280,   280,   280,   280,   280,   280,   280,   280,   280,
     280,   280,   280,   281,   281,   281,   282,   282,   283,   283,
     284,   285,   285,   285,   286,   286,   286,   287,   287,   287,
     287,   287,   287,   288,   288,   288,   289,   289,   289,   290,
     290,   291,   291,   291,   291,   291,   291,   292,   292,   292,
     293,   293,   293,   294,   295,   295,   296,   296,   297,   297,
     297,   297,   297,   297,   298,   298,   298,   298,   299,   299,
     299,   299,   299,   299,   299,   300,   300,   300,   300,   301,
     301,   301,   301,   301,   301,   301,   302,   302,   303
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       0,     1,     3,     1,     3,     2,     1,     1,     1,     1,
       1,     4,     3,     0,     6,     0,     5,     3,     4,     3,
       4,     3,     1,     1,     6,     7,     6,     7,     0,     1,
       3,     1,     3,     1,     3,     1,     1,     2,     1,     3,
       1,     2,     3,     1,     2,     0,     1,     1,     1,     1,
       1,     4,     3,     1,     1,     5,     7,     9,     5,     3,
       3,     3,     3,     3,     3,     1,     2,     5,     7,     9,
       0,     6,     1,     6,     3,     3,     2,     0,     9,     1,
       3,     0,     4,     1,     3,     1,    13,     0,     1,     0,
       1,     0,    10,     0,     9,     1,     2,     1,     1,     0,
       7,     0,     8,     0,     2,     0,     2,     0,     2,     1,
       2,     4,     3,     1,     4,     1,     4,     1,     4,     3,
       4,     4,     5,     0,     5,     4,     1,     1,     1,     4,
       5,     6,     1,     3,     6,     7,     3,     6,     1,     0,
       1,     3,     4,     6,     0,     1,     1,     2,     1,     1,
       1,     0,     2,     2,     3,     1,     3,     1,     2,     3,
       1,     1,     3,     1,     1,     3,     2,     0,     3,     4,
       3,    12,     1,     3,     1,     2,     3,     1,     2,     2,
       2,     3,     3,     3,     4,     3,     1,     1,     3,     1,
       3,     1,     1,     0,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     3,     1,     2,     4,     3,     1,     4,
       4,     3,     1,     1,     0,     1,     3,     1,     0,     9,
       3,     2,     6,     5,     3,     4,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     1,     5,     4,     3,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     1,
       3,     2,     1,     2,     4,     2,    13,    14,     1,     0,
       0,     0,     1,     0,     4,     3,     1,     1,     2,     2,
       4,     4,     2,     1,     1,     1,     1,     0,     3,     0,
       1,     1,     0,     1,     4,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     2,     3,
       3,     1,     1,     1,     3,     3,     1,     1,     0,     1,
       1,     1,     3,     1,     1,     3,     1,     1,     4,     4,
       4,     4,     1,     1,     1,     3,     1,     4,     2,     3,
       3,     1,     4,     4,     3,     3,     3,     1,     3,     1,
       1,     3,     1,     1,     0,     1,     3,     1,     3,     1,
       4,     2,     6,     4,     2,     2,     1,     2,     1,     4,
       3,     3,     3,     6,     3,     1,     1,     2,     1,     4,
       4,     2,     2,     4,     2,     2,     1,     3,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
      80,     0,     2,     1,     0,     0,     0,     0,     0,     0,
     372,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   464,     0,     0,   178,
     177,   393,   407,   408,    81,   446,   145,   406,   397,     0,
       0,     0,     0,     0,     0,     0,     0,   428,   428,     0,
     378,     0,   428,     0,     0,     0,     0,     0,     0,     0,
       0,   173,   179,   181,     0,     0,   409,   410,   411,   416,
     412,   413,   414,     0,    95,   415,     0,     0,   152,   125,
     399,     0,     0,    83,   423,    79,    86,    87,    88,     0,
     175,    89,    90,   212,   133,     0,   134,   355,   427,   381,
     442,     0,   421,   369,   422,     0,     0,   430,     0,   443,
     426,   437,   444,   359,   393,    81,     0,   381,   495,   494,
       0,   492,   491,   371,   373,   375,   340,   341,   342,   343,
     368,   366,   365,   364,   363,   362,   361,   360,   464,   393,
       0,   433,     0,   322,   320,     0,     0,   469,     0,   467,
     463,   465,   306,   298,   394,   301,   395,   402,   451,   396,
     244,     0,   243,   381,   156,   428,   367,     0,     0,   292,
     293,     0,     0,   294,     0,     0,     0,   429,     0,     0,
       0,     0,     0,   123,     0,   125,     0,   102,   103,     0,
     118,     0,     0,     0,   120,   115,     0,   240,   241,     0,
       0,     0,     0,     0,     0,     0,   464,   464,   478,     0,
     418,     0,     0,     0,   476,     0,    93,     0,    85,     0,
       0,   400,     0,   401,     0,     0,     0,   448,     0,     0,
     389,   171,   176,     0,     0,     0,     0,     0,   382,   379,
       0,   428,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   146,     0,   428,     0,     0,   392,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     321,   319,   379,     0,     0,     0,     0,   431,   464,     0,
     405,   464,   402,     0,   403,   300,   428,     0,     0,     0,
       0,     0,   143,   379,     0,     0,     0,   144,     0,     0,
       0,   295,   297,     0,     0,     0,   139,   140,   155,     0,
       0,   101,   141,     0,   154,   118,   121,     0,     0,     0,
     118,     0,     0,    97,     0,    99,     0,   142,     0,   163,
     431,   498,     0,   496,     0,     0,   183,   379,   185,     0,
       0,     0,     0,   417,   477,     0,     0,   431,   475,   420,
     474,    84,    92,     0,    80,   354,     0,   132,   124,   126,
     127,   128,   129,   130,   370,   419,     0,    82,     0,   233,
       0,   235,   237,     0,     0,   213,     0,   125,   216,   379,
       0,     7,     6,     5,     4,     3,     8,     9,    10,    46,
      47,    11,    13,    12,    16,    17,    18,    76,    75,    74,
      73,    72,    71,    77,    14,    15,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    49,    50,    51,
      52,    53,    41,    42,    43,    44,    45,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    61,    59,
      60,    56,    57,    48,    54,    55,    66,    67,    68,    62,
      63,    65,    64,    58,    69,     0,    70,    78,   424,   449,
       0,     0,   325,   327,   326,     0,     0,   358,   323,   324,
     328,   330,   329,   352,   345,   344,   347,   346,   348,   350,
     351,   349,   339,   338,   332,   333,   331,   334,   336,   337,
     353,   335,   425,   450,     0,     0,   460,     0,   462,   445,
       0,     0,   304,   310,   318,   317,   316,   315,   314,   313,
     312,   311,   309,   308,   307,   493,   374,   405,   432,     0,
       0,     0,   468,     0,   466,   183,   455,     0,   454,   456,
       0,   245,   242,     0,   398,     0,   291,     0,     0,   294,
       0,     0,   150,     0,   379,   122,   157,     0,   119,     0,
     118,     0,    98,   100,     0,   118,   114,   239,     0,     0,
       0,   489,   490,    91,     0,   187,     0,     0,   379,     0,
     404,     0,   485,   488,   486,     0,   480,     0,   482,   481,
     484,    80,     0,     0,   447,   238,     0,   234,   183,     0,
       0,     0,     0,   224,     0,   390,   439,   357,     0,   391,
     438,     0,   441,   440,   305,   473,   431,   464,   303,   187,
     452,   453,   224,   125,   210,     0,   125,   208,   135,     0,
     296,     0,   464,     0,     0,   431,     0,   203,   203,   138,
     290,   161,     0,     0,   108,   111,   116,     0,     0,   164,
     147,   497,   184,     0,   379,   247,   252,   186,     0,     0,
     487,   479,     0,     0,    96,     0,   236,   187,     0,     0,
       0,   224,     0,   228,   229,   230,     0,   218,   220,   167,
     225,   226,   458,   356,   461,     0,   379,     0,   214,     0,
       0,   294,   431,     0,   464,     0,     0,   125,   197,   151,
     203,     0,   203,     0,     0,     0,   153,   108,   117,   109,
       0,     0,   108,   113,   188,     0,   273,     0,   247,   302,
       0,    94,   131,   379,   211,   125,   217,     0,   227,   383,
     224,   168,   169,   472,     0,   383,   136,     0,     0,   405,
       0,     0,   125,   195,   148,     0,     0,     0,     0,     0,
       0,   199,     0,   125,     0,   110,   106,   108,   109,     0,
     247,   277,   278,   279,   282,   281,   280,     0,   272,   180,
     246,     0,     0,   274,   275,   253,   273,   483,     0,   215,
     231,     0,   231,   221,   170,     0,   247,   231,   209,     0,
     191,     0,     0,     0,     0,   201,     0,   206,   207,   125,
     200,   159,     0,     0,   107,     0,   112,   104,   273,     0,
     379,     0,   284,     0,   381,   276,   182,   247,     0,   380,
       0,   380,   222,   273,   380,   125,   193,   137,   149,     0,
     198,   202,   125,   205,     0,     0,   162,   105,   174,   254,
       0,   250,     0,   285,     0,   248,     0,     0,   288,     0,
     273,   232,     0,     0,   387,     0,   386,     0,     0,   299,
       0,     0,   196,   204,   160,     0,    81,    58,   255,   266,
       0,     0,   257,     0,     0,     0,   267,   379,   283,     0,
       0,   249,   379,   172,   125,   388,     0,   384,   125,   223,
     125,     0,   125,     0,   256,   258,   259,   260,     0,     0,
     286,   379,   287,     0,     0,   385,     0,     0,   194,     0,
     268,   262,   263,   265,   261,   289,   224,   380,   380,   380,
     158,   264,     0,   166,   376,   377,   231,   380,     0,   269,
     125,   380,     0,   251,   270
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,   476,   477,   856,     2,    83,    84,    85,   373,
     217,   653,   341,   192,   720,   654,   722,   193,   655,   194,
     195,   182,   220,   378,   379,   646,   651,   812,   716,   348,
     349,   380,   742,   795,   381,   393,   203,    89,    90,   382,
     204,   383,   205,   585,   588,   664,   644,   837,   754,   709,
     649,   711,   809,   638,    93,    94,    95,    96,   686,   687,
     688,   689,   690,   691,   829,   304,   390,   391,   196,   197,
     161,   162,   726,   780,   667,   851,   881,   882,   883,   884,
     885,   886,   941,   781,   782,   783,   784,   821,   822,   857,
     858,   183,   168,   169,   320,   321,   155,   302,    97,    98,
     117,   400,   862,   239,   792,   865,   866,   100,   101,   157,
     166,   222,   305,   102,   103,   104,   105,   178,   106,   107,
     108,   109,   110,   111,   112,   159,   480,   519,   148,   149,
     150,   151,   213,   214,   595,   113,   352,   353
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -714
static const yytype_int16 yypact[] =
{
    -714,   109,  1211,  -714,  5560,  5560,   -38,  5560,  5560,  5560,
    5560,  5560,  5560,  5560,  5560,  5560,  5560,  5560,  5560,  5560,
    5560,  5560,  5560,  5560,   355,   355,  4888,  5560,    14,  -714,
    -714,   224,  -714,  -714,   108,  -714,  -714,  -714,   -12,    -9,
    5560,  4613,    -5,    79,   182,   184,   195,  5560,  5560,   273,
    -714,   281,  5560,   206,  5560,     6,     8,   212,   214,   219,
     221,  -714,  -714,  -714,   222,   225,  -714,  -714,  -714,  -714,
    -714,  -714,  -714,   442,    58,  -714,   298,  5560,  -714,  -714,
     244,   291,   157,   243,   -37,  -714,  -714,  -714,  -714,   358,
    -714,  -714,  -714,   166,  -714,    62,  -714,  -714,  -714,   376,
    -714,   279,   257,  -714,   364,  6620,   293,     3,   307,   308,
     429,  -714,  -714,  -714,   363,  -714,   320,   376,  7326,  7326,
    5560,  7326,  7326,  1522,  1196,  1522,   411,   411,    48,   411,
     411,   411,   411,   411,   411,   411,   411,   411,  4888,  -714,
    5560,   324,   364,    23,    23,   355,   325,  7232,   326,  -714,
     485,  -714,  -714,  -714,  -714,  -714,   348,   307,  -714,   229,
     486,    35,  -714,   376,  -714,  5560,  -714,  5560,    38,  -714,
    7326,   407,  5560,  5560,  5560,   281,  5560,  7326,   345,   349,
     351,   497,    41,  -714,   356,  -714,  6667,  -714,  -714,   298,
     -14,   211,   357,    47,  -714,  -714,    54,  -714,  -714,   355,
    5560,  5560,   360,   436,   450,   451,  4888,  4888,    25,   336,
    -714,  5000,   355,   527,  -714,   298,   -31,   375,   243,  6714,
    1374,   196,   380,   382,   196,   235,  5560,  -714,   462,  4776,
    -714,  -714,  -714,   389,  4613,   391,   520,   399,  -714,   479,
    3493,  5560,  5560,  5560,  5560,  5112,  5560,  5560,  5560,  5560,
    5560,  5560,  5560,  5560,  5560,  5560,  5560,  5560,  5560,  5560,
    5560,  5560,  5560,  5560,  5560,  5560,  5560,  5560,  5560,   495,
    5560,  -714,  3493,  5560,    75,  5560,  -714,  5224,  5560,  5560,
    5560,  5560,  5560,  5560,  5560,  5560,  5560,  5560,  5560,  5560,
    -714,  -714,  -714,  6761,  5560,   403,  6808,    31,  4888,  5336,
     550,  4888,   307,     8,  -714,  -714,  5560,    75,     8,  5560,
    5560,   487,  -714,  -714,   414,  6855,  5560,  -714,   412,  6902,
     415,   568,  7326,  7184,    59,  6949,  -714,  -714,  -714,  5560,
     281,  -714,  -714,  1537,  -714,   213,  -714,   494,   -13,   298,
     255,   423,    57,  -714,   218,  -714,     8,  -714,    64,  -714,
      66,  7326,    68,  -714,  6996,   424,   453,  -714,   467,   440,
     441,   422,   519,  -714,  -714,    -4,  6041,   445,  -714,  -714,
    -714,   243,  -714,   448,  -714,   274,   455,  -714,  -714,  -714,
    -714,  -714,  -714,  -714,  -714,  -714,  6088,  -714,  5560,  -714,
      69,  -714,  7326,   532,  5560,  -714,  5560,  -714,  -714,  -714,
     460,  -714,  -714,  -714,  -714,  -714,  -714,  -714,  -714,  -714,
    -714,  -714,  -714,  -714,  -714,  -714,  -714,  -714,  -714,  -714,
    -714,  -714,  -714,  -714,  -714,  -714,  -714,  -714,  -714,  -714,
    -714,  -714,  -714,  -714,  -714,  -714,  -714,  -714,  -714,  -714,
    -714,  -714,  -714,  -714,  -714,  -714,  -714,  -714,  -714,  -714,
    -714,  -714,  -714,  -714,  -714,  -714,  -714,  -714,  -714,  -714,
    -714,  -714,  -714,  -714,  -714,  -714,  -714,  -714,  -714,  -714,
    -714,  -714,  -714,  -714,  -714,  5560,  -714,  -714,   461,   463,
     307,   457,  1362,  1036,  1522,  5560,  7279,  1685,  1846,  2009,
    2172,  2332,  2494,  2657,  2657,  2657,  2657,  2657,   800,   800,
     800,   800,   613,   613,   685,   685,   685,    48,    48,    48,
    -714,   411,   461,   463,   307,   459,  -714,  5560,  -714,   307,
    6136,   355,  1522,  1522,  1522,  1522,  1522,  1522,  1522,  1522,
    1522,  1522,  1522,  1522,  1522,  -714,  1522,  -714,   468,   466,
     355,   469,  7326,  5560,  -714,   453,  -714,   465,  -714,  -714,
    6183,  7326,  -714,   472,  -714,  3798,  -714,  5560,  3961,  5560,
    5560,   264,  -714,    15,  7326,  -714,  -714,    27,  -714,    19,
     259,    42,  -714,  -714,   298,   272,  -714,  -714,   355,   471,
    5560,  -714,  -714,  -714,    13,   506,   478,    13,  -714,   624,
    -714,   553,  -714,  -714,  -714,   481,  -714,  5560,  -714,  -714,
    -714,  -714,   885,   490,  -714,  7326,  5448,  -714,   453,  7043,
    7090,  1700,   496,   270,  6231,  -714,  -714,  1685,  5560,  -714,
    -714,  6278,  -714,  -714,    23,   624,    45,  4888,  1522,   506,
    -714,  -714,   270,  -714,  -714,  7137,  -714,  -714,  -714,   500,
    7326,   355,  4888,   499,    43,    71,  4124,   510,   511,  -714,
    -714,   129,    19,   298,   664,  -714,  -714,    49,   298,  -714,
    -714,  -714,  -714,    13,  -714,  -714,  -714,   671,   521,  5560,
    -714,  -714,  5994,  1048,  -714,   524,  -714,   506,  4613,   654,
     529,   270,   344,  -714,  -714,  -714,   528,   681,  -714,   655,
    -714,  -714,  -714,  1685,  -714,   535,  -714,   536,  3635,   538,
    1863,  5560,    73,   533,  4888,   264,  4287,  -714,  -714,  -714,
    -714,   386,  -714,    21,   545,   544,  -714,   664,  -714,    19,
     546,   298,   698,  -714,   671,   555,   383,    13,  -714,  1522,
     549,  -714,  -714,  -714,  -714,  -714,  -714,   561,  -714,   604,
      18,  -714,   575,   624,   571,   604,  -714,   574,   576,    76,
     583,   584,  -714,  -714,  -714,  2026,   418,   585,  5560,    78,
     249,  -714,    13,  -714,   582,  -714,  -714,   698,   298,   586,
    -714,  -714,  -714,  -714,  -714,  -714,  -714,    13,  -714,  -714,
    -714,   662,    11,   679,  -714,  -714,   476,  -714,   588,  3635,
     721,   603,   721,  -714,  -714,   677,  -714,   721,  -714,  4450,
    -714,  4287,  2189,   605,   606,  -714,  5710,  -714,  -714,  -714,
    -714,  -714,    65,  2352,  -714,   607,  -714,  -714,   649,    32,
     754,    60,  -714,  6350,   376,  -714,  -714,  -714,    18,  -714,
      77,  -714,   755,   713,  -714,  -714,  -714,  -714,  -714,   614,
    -714,  -714,  -714,  3635,    13,   616,  -714,  -714,  -714,  -714,
    5723,  -714,  5560,  -714,   662,  -714,   756,    61,  -714,  6350,
     719,  -714,   617,   692,  -714,    70,  -714,   618,  5560,  -714,
     622,  2515,  -714,  3635,  -714,   625,   682,   320,  -714,  -714,
     635,  5861,  -714,   644,   645,   703,   688,  7326,  -714,  5560,
    6350,  -714,  -714,  -714,  -714,  -714,    77,  -714,  -714,  7326,
    -714,   648,  -714,  6350,  -714,  -714,  -714,  -714,  6488,    13,
    -714,  7326,  -714,   651,  2678,  -714,  2841,  3004,  -714,  3167,
    -714,  -714,  -714,  6350,   671,  -714,   270,  -714,  -714,  -714,
    -714,  -714,   656,  -714,  -714,  -714,   721,  -714,   177,  -714,
    -714,  -714,  3330,  -714,  -714
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -714,  -714,   -99,  -714,  -236,  -369,    -1,   -27,  -714,  -714,
    -714,   758,  -714,  -714,  -399,   163,    95,   627,   100,  -183,
     480,   647,  -119,  -714,    39,  -714,  -714,  -714,  -714,  -714,
     242,     1,  -714,  -714,     5,  -714,  -714,  -714,   736,    26,
    -714,    30,  -714,  -427,  -714,  -534,   121,  -714,    28,  -714,
    -714,  -322,    22,  -714,  -714,  -714,  -714,  -714,  -622,  -714,
      87,  -714,    29,   149,  -713,   -47,  -714,   226,  -714,   488,
    -714,   526,  -665,  -714,  -613,  -714,  -714,   -46,  -714,  -714,
    -714,  -714,  -714,  -714,  -714,  -714,  -683,  -714,    10,  -714,
     -32,   530,  -714,   543,  -517,  -714,  -714,  -714,  -714,  -714,
      -2,  -253,  -522,  -115,   116,  -714,   -34,  -714,     7,   594,
    -714,  -714,   563,     9,  -714,   161,     4,    34,  -714,  -714,
    -714,  -714,   171,    55,  -714,  -714,   596,   566,  -100,   569,
    -714,  -714,   269,   316,  -714,  -714,  -714,   295
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -472
static const yytype_int16 yytable[] =
{
      99,   154,   292,    87,   478,   602,   336,    88,   118,   119,
     697,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,    91,   163,
     147,   152,    92,   141,   141,   156,   512,   230,   295,  -471,
     727,    86,   639,   311,   170,   647,   316,   682,   313,   330,
     724,   177,   177,  -470,   190,   344,   177,   705,   186,   737,
     553,   276,   346,   786,   597,   344,   333,   330,   854,   890,
     387,   273,   578,   216,  -165,   218,   580,   606,   896,   831,
     171,   219,   179,   158,   834,  -189,   184,  -190,   337,   115,
    -192,  -431,    35,   361,   139,   696,   115,   115,    35,   844,
     825,   115,   115,   269,   586,   818,   359,   360,   807,     3,
     387,   198,   163,   863,  -394,   270,   187,   188,   629,   120,
     229,    50,   823,   228,   293,   387,   758,   759,   372,   187,
     188,   833,   387,   274,   235,   236,   237,   227,   164,   153,
     338,   115,   147,   733,   296,   165,   612,   569,   167,   845,
     683,   684,   172,  -431,   141,   362,   336,   598,   516,    35,
     189,   864,   860,   275,   819,   116,   116,    76,    76,   177,
     116,   315,    76,    82,  -431,   648,   319,   322,   323,    82,
     325,   677,   761,  -431,   748,   142,   142,   652,   335,  -471,
     340,   849,   850,  -471,   312,   143,   144,   317,   539,   314,
     331,   706,   658,  -470,   351,   354,   345,  -470,   141,   721,
     147,   147,   215,   347,   371,   366,   573,   562,    99,   855,
     891,   141,   579,   937,  -165,   923,   581,   607,   897,  -189,
     386,  -190,   673,   392,  -192,   517,   173,   808,   233,   234,
      82,    35,   154,   714,   715,   177,   482,   483,   484,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,   502,   503,   504,   505,   506,
     507,   508,   509,   395,   511,   481,   156,   177,   611,   520,
     208,   522,   523,   524,   525,   526,   527,   528,   529,   530,
     531,   532,   533,   534,   115,   479,   924,   306,   536,   682,
     641,   115,   147,   542,   932,   147,   142,   515,   160,   867,
     177,   650,   870,   550,   551,   337,   297,   226,   764,   208,
     170,   368,    82,   769,   158,  -433,   713,   513,   208,   518,
     221,    99,   642,   564,    50,   668,   939,   940,   570,   174,
     547,   175,  -432,   575,   139,   211,   212,   115,    35,   223,
     225,    37,   176,   115,   758,   759,   180,   337,   546,   307,
     142,   337,   518,   549,   181,   339,   185,   567,   815,   199,
     350,   200,   574,   142,   337,   208,   201,   224,   202,   206,
     308,   115,   207,   367,   211,   212,   656,  -433,   756,   309,
     760,   336,   605,   211,   212,   643,    65,   228,   609,   385,
     610,   198,   683,   684,  -432,   933,   934,   935,  -433,   571,
     810,   725,   238,   657,  -436,   938,   116,  -433,    76,   943,
     208,   140,   116,   138,    76,  -432,   228,   115,  -219,    82,
     240,  -435,   241,   615,  -432,   139,    29,    30,   115,    35,
     211,   212,    37,   744,   272,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   771,   772,
     773,   774,   775,   776,   229,  -434,   208,   619,   368,   656,
     718,   591,   622,    50,   215,   723,   683,   684,   270,   614,
     788,  -436,   298,   231,   363,   211,   212,    65,   300,   617,
     757,   758,   759,   301,   290,   291,   116,  -431,    76,   303,
     777,   318,   310,   778,   326,   592,   593,   116,   327,    76,
     328,   594,   140,   329,   698,   332,   343,   700,   355,   356,
      82,   621,   804,   758,   759,   364,   208,   695,   209,   370,
     141,   211,   212,   357,   358,   374,   656,   364,   723,   370,
     364,   370,   703,   384,   779,   387,   394,   628,   396,   141,
     397,   771,   772,   773,   774,   775,   776,   662,   398,  -431,
     666,   635,   399,   322,   640,   537,   543,   853,   575,   557,
     141,   160,   554,   575,   559,   139,   560,   568,   115,    35,
    -431,   584,   572,   583,   351,   816,   685,   141,   755,  -431,
     210,   211,   212,   777,   634,   587,   778,   637,   589,   590,
      99,   672,   596,    87,   750,   685,   600,    88,   601,    99,
     392,   208,   603,   368,   879,   608,   789,   613,  -457,   616,
    -459,   620,   693,   892,   625,  -435,   627,   630,    91,   632,
     660,   147,    92,   802,   910,   663,   666,   826,   665,   913,
     669,    86,   670,   671,   813,   879,   147,   116,   675,    76,
     141,   575,   575,   681,   685,   685,   704,   575,   925,   701,
      82,   263,   264,   265,   266,   267,   268,   920,   269,   710,
     712,    99,   719,   729,    87,   369,   211,   212,    88,   727,
     270,   728,   142,   732,   735,   708,   739,   931,   736,   740,
     843,   741,   624,   743,   745,   749,    99,   746,    99,    91,
     785,   142,   762,    92,   763,   322,   768,   766,   147,   859,
     787,   626,    86,   685,   141,   770,   871,   734,   575,   790,
     575,   791,   142,   873,   771,   772,   773,   774,   775,   776,
     794,   796,   645,   798,   799,   811,   266,   267,   268,   142,
     269,   800,   801,   814,   805,   753,   820,   817,   827,   350,
     666,   828,   270,    99,   771,   772,   773,   774,   775,   776,
     830,   832,   806,  -271,   840,   841,   777,   575,   847,   778,
     852,   868,   889,   872,   875,   914,   895,   894,   898,   916,
     824,   917,   900,   919,   -77,   902,   903,    99,   771,   772,
     773,   774,   775,   776,   771,   772,   773,   774,   775,   776,
      99,   685,   142,   906,   907,   908,   909,   918,   926,   922,
     848,    99,   702,   191,   936,   717,   767,   874,   342,   765,
     659,   942,   324,   880,   576,   232,   751,   793,   842,   838,
     777,   738,   676,   778,   577,   905,   777,   552,   836,   778,
     753,    99,  -472,  -472,  -472,  -472,   261,   262,   263,   264,
     265,   266,   267,   268,   880,   269,   887,   861,   912,   556,
     565,   797,   915,   510,   888,   545,   142,   270,   514,    99,
     544,    99,   899,   548,   869,   661,   645,     0,     0,     0,
     893,     0,   666,     0,     0,     0,     0,     0,     4,     5,
       6,     7,     8,   911,     0,     0,     0,     9,    10,   685,
      11,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    99,     0,    99,    99,     0,    99,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
      99,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,     0,    26,    27,    28,     0,     0,     0,     0,
       0,     0,     0,    29,    30,    31,    32,    33,    34,    35,
      36,     0,    37,     0,     0,    38,    39,    40,    41,    42,
       0,    43,     0,    44,     0,    45,     0,     0,    46,     0,
       0,     0,    47,    48,    49,    50,    51,    52,    53,     0,
       0,    54,    55,     0,    56,     0,    57,    58,    59,    60,
      61,    62,    63,     0,     0,     0,    64,    65,     0,    66,
      67,    68,    69,    70,    71,    72,     0,     0,     0,     0,
       0,     0,    73,     0,     0,     0,     0,    74,    75,    76,
       0,     0,    77,     0,    78,    79,   674,   244,    80,    81,
      82,     4,     5,     6,     7,     8,     0,     0,     0,     0,
       9,    10,     0,    11,     0,   245,     0,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
       0,   269,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,   270,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,     0,    26,    27,    28,     0,
       0,     0,     0,     0,     0,     0,    29,    30,    31,    32,
      33,    34,    35,    36,     0,    37,     0,     0,    38,    39,
      40,    41,    42,     0,    43,     0,    44,     0,    45,     0,
       0,    46,     0,     0,     0,    47,    48,    49,    50,    51,
      52,    53,     0,     0,    54,    55,     0,    56,     0,    57,
      58,    59,    60,    61,    62,    63,     0,     0,     0,    64,
      65,     0,    66,    67,    68,    69,    70,    71,    72,     0,
       0,     0,     0,     0,     0,    73,     0,     0,     0,     0,
      74,    75,    76,     0,     0,    77,     0,    78,    79,   731,
     294,    80,    81,    82,     4,     5,     6,     7,     8,     0,
       0,     0,     0,     9,    10,   245,    11,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
       0,   269,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,   270,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,     0,    26,
      27,    28,     0,     0,     0,     0,     0,     0,     0,    29,
      30,    31,    32,    33,    34,    35,    36,     0,    37,     0,
       0,    38,    39,    40,    41,    42,     0,    43,     0,    44,
       0,    45,     0,     0,    46,     0,     0,     0,    47,    48,
      49,    50,    51,    52,    53,     0,     0,    54,    55,     0,
      56,     0,    57,    58,    59,    60,    61,    62,    63,     0,
       0,     0,    64,    65,     0,    66,    67,    68,    69,    70,
      71,    72,     0,     0,     0,     0,     0,     0,    73,     0,
       0,     0,     0,    74,    75,    76,     0,     0,    77,     0,
      78,    79,   243,   244,    80,    81,    82,     4,     5,     6,
       7,     8,     0,     0,     0,     0,     9,    10,     0,    11,
       0,   245,     0,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,     0,   269,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,   270,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,     0,    26,    27,    28,     0,     0,     0,     0,     0,
       0,     0,    29,    30,    31,    32,    33,    34,    35,    36,
       0,    37,     0,     0,    38,    39,    40,    41,    42,     0,
      43,     0,    44,     0,    45,     0,     0,    46,     0,     0,
       0,    47,    48,    49,    50,     0,    52,    53,     0,     0,
      54,     0,     0,    56,     0,    57,    58,    59,   376,    61,
      62,    63,     0,     0,     0,    64,    65,     0,    66,    67,
      68,    69,    70,    71,    72,     0,     0,     0,     0,     0,
       0,    73,     0,     0,     0,     0,   116,    75,    76,     0,
       0,    77,     0,    78,    79,   377,     0,    80,    81,    82,
       4,     5,     6,     7,     8,     0,     0,     0,     0,     9,
      10,   245,    11,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,     0,   269,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,   270,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,     0,    26,    27,    28,     0,     0,
       0,     0,     0,     0,     0,    29,    30,    31,    32,    33,
      34,    35,    36,     0,    37,     0,     0,    38,    39,    40,
      41,    42,     0,    43,     0,    44,     0,    45,     0,     0,
      46,     0,     0,     0,    47,    48,    49,    50,     0,    52,
      53,     0,     0,    54,     0,     0,    56,     0,    57,    58,
      59,   376,    61,    62,    63,     0,     0,     0,    64,    65,
       0,    66,    67,    68,    69,    70,    71,    72,     0,     0,
       0,     0,     0,     0,    73,     0,     0,     0,     0,   116,
      75,    76,     0,     0,    77,     0,    78,    79,   566,     0,
      80,    81,    82,     4,     5,     6,     7,     8,     0,     0,
       0,     0,     9,    10,     0,    11,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,     0,
     269,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,   270,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,     0,    26,    27,
      28,     0,     0,     0,   680,     0,     0,     0,    29,    30,
      31,    32,    33,    34,    35,    36,     0,    37,     0,     0,
      38,    39,    40,    41,    42,     0,    43,     0,    44,     0,
      45,     0,     0,    46,     0,     0,     0,    47,    48,    49,
      50,     0,    52,    53,     0,     0,    54,     0,     0,    56,
       0,    57,    58,    59,   376,    61,    62,    63,     0,     0,
       0,    64,    65,     0,    66,    67,    68,    69,    70,    71,
      72,     0,     0,     0,     0,     0,     0,    73,     0,     0,
       0,     0,   116,    75,    76,     0,     0,    77,     0,    78,
      79,     0,     0,    80,    81,    82,     4,     5,     6,     7,
       8,     0,     0,     0,     0,     9,    10,     0,    11,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
       0,   269,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,   270,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
       0,    26,    27,    28,     0,     0,     0,     0,     0,     0,
       0,    29,    30,    31,    32,    33,    34,    35,    36,     0,
      37,     0,     0,    38,    39,    40,    41,    42,   747,    43,
       0,    44,     0,    45,     0,     0,    46,     0,     0,     0,
      47,    48,    49,    50,     0,    52,    53,     0,     0,    54,
       0,     0,    56,     0,    57,    58,    59,   376,    61,    62,
      63,     0,     0,     0,    64,    65,     0,    66,    67,    68,
      69,    70,    71,    72,     0,     0,     0,     0,     0,     0,
      73,     0,     0,     0,     0,   116,    75,    76,     0,     0,
      77,     0,    78,    79,     0,     0,    80,    81,    82,     4,
       5,     6,     7,     8,     0,     0,     0,     0,     9,    10,
       0,    11,     0,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,     0,   269,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,   270,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,     0,     0,     0,
       0,     0,     0,     0,    29,    30,    31,    32,    33,    34,
      35,    36,     0,    37,     0,     0,    38,    39,    40,    41,
      42,     0,    43,     0,    44,     0,    45,   803,     0,    46,
       0,     0,     0,    47,    48,    49,    50,     0,    52,    53,
       0,     0,    54,     0,     0,    56,     0,    57,    58,    59,
     376,    61,    62,    63,     0,     0,     0,    64,    65,     0,
      66,    67,    68,    69,    70,    71,    72,     0,     0,     0,
       0,     0,     0,    73,     0,     0,     0,     0,   116,    75,
      76,     0,     0,    77,     0,    78,    79,     0,     0,    80,
      81,    82,     4,     5,     6,     7,     8,     0,     0,     0,
       0,     9,    10,     0,    11,     0,     0,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,     0,   269,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,   270,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,     0,    26,    27,    28,
       0,     0,     0,     0,     0,     0,     0,    29,    30,    31,
      32,    33,    34,    35,    36,     0,    37,     0,     0,    38,
      39,    40,    41,    42,     0,    43,     0,    44,   839,    45,
       0,     0,    46,     0,     0,     0,    47,    48,    49,    50,
       0,    52,    53,     0,     0,    54,     0,     0,    56,     0,
      57,    58,    59,   376,    61,    62,    63,     0,     0,     0,
      64,    65,     0,    66,    67,    68,    69,    70,    71,    72,
       0,     0,     0,     0,     0,     0,    73,     0,     0,     0,
       0,   116,    75,    76,     0,     0,    77,     0,    78,    79,
       0,     0,    80,    81,    82,     4,     5,     6,     7,     8,
       0,     0,     0,     0,     9,    10,     0,    11,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,     0,   269,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   270,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,     0,
      26,    27,    28,     0,     0,     0,     0,     0,     0,     0,
      29,    30,    31,    32,    33,    34,    35,    36,     0,    37,
       0,     0,    38,    39,    40,    41,    42,     0,    43,     0,
      44,     0,    45,     0,     0,    46,     0,     0,     0,    47,
      48,    49,    50,     0,    52,    53,     0,     0,    54,     0,
       0,    56,     0,    57,    58,    59,   376,    61,    62,    63,
       0,     0,     0,    64,    65,     0,    66,    67,    68,    69,
      70,    71,    72,     0,     0,     0,     0,     0,     0,    73,
       0,     0,     0,     0,   116,    75,    76,     0,     0,    77,
       0,    78,    79,   846,     0,    80,    81,    82,     4,     5,
       6,     7,     8,     0,     0,     0,     0,     9,    10,     0,
      11,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,   266,   267,   268,     0,   269,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   270,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,     0,    26,    27,    28,     0,     0,     0,     0,
       0,     0,     0,    29,    30,    31,    32,    33,    34,    35,
      36,     0,    37,     0,     0,    38,    39,    40,    41,    42,
       0,    43,   901,    44,     0,    45,     0,     0,    46,     0,
       0,     0,    47,    48,    49,    50,     0,    52,    53,     0,
       0,    54,     0,     0,    56,     0,    57,    58,    59,   376,
      61,    62,    63,     0,     0,     0,    64,    65,     0,    66,
      67,    68,    69,    70,    71,    72,     0,     0,     0,     0,
       0,     0,    73,     0,     0,     0,     0,   116,    75,    76,
       0,     0,    77,     0,    78,    79,     0,     0,    80,    81,
      82,     4,     5,     6,     7,     8,     0,     0,     0,     0,
       9,    10,     0,    11,  -472,  -472,  -472,  -472,  -472,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,     0,   269,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   270,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,     0,    26,    27,    28,     0,
       0,     0,     0,     0,     0,     0,    29,    30,    31,    32,
      33,    34,    35,    36,     0,    37,     0,     0,    38,    39,
      40,    41,    42,     0,    43,     0,    44,     0,    45,     0,
       0,    46,     0,     0,     0,    47,    48,    49,    50,     0,
      52,    53,     0,     0,    54,     0,     0,    56,     0,    57,
      58,    59,   376,    61,    62,    63,     0,     0,     0,    64,
      65,     0,    66,    67,    68,    69,    70,    71,    72,     0,
       0,     0,     0,     0,     0,    73,     0,     0,     0,     0,
     116,    75,    76,     0,     0,    77,     0,    78,    79,   927,
       0,    80,    81,    82,     4,     5,     6,     7,     8,     0,
       0,     0,     0,     9,    10,     0,    11,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,     0,    26,
      27,    28,     0,     0,     0,     0,     0,     0,     0,    29,
      30,    31,    32,    33,    34,    35,    36,     0,    37,     0,
       0,    38,    39,    40,    41,    42,     0,    43,     0,    44,
       0,    45,     0,     0,    46,     0,     0,     0,    47,    48,
      49,    50,     0,    52,    53,     0,     0,    54,     0,     0,
      56,     0,    57,    58,    59,   376,    61,    62,    63,     0,
       0,     0,    64,    65,     0,    66,    67,    68,    69,    70,
      71,    72,     0,     0,     0,     0,     0,     0,    73,     0,
       0,     0,     0,   116,    75,    76,     0,     0,    77,     0,
      78,    79,   928,     0,    80,    81,    82,     4,     5,     6,
       7,     8,     0,     0,     0,     0,     9,    10,     0,    11,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,     0,    26,    27,    28,     0,     0,     0,     0,     0,
       0,     0,    29,    30,    31,    32,    33,    34,    35,    36,
       0,    37,     0,     0,    38,    39,    40,    41,    42,     0,
      43,     0,    44,     0,    45,     0,     0,    46,     0,     0,
       0,    47,    48,    49,    50,     0,    52,    53,     0,     0,
      54,     0,     0,    56,     0,    57,    58,    59,   376,    61,
      62,    63,     0,     0,     0,    64,    65,     0,    66,    67,
      68,    69,    70,    71,    72,     0,     0,     0,     0,     0,
       0,    73,     0,     0,     0,     0,   116,    75,    76,     0,
       0,    77,     0,    78,    79,   929,     0,    80,    81,    82,
       4,     5,     6,     7,     8,     0,     0,     0,     0,     9,
      10,     0,    11,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,     0,    26,    27,    28,     0,     0,
       0,     0,     0,     0,     0,    29,    30,    31,    32,    33,
      34,    35,    36,     0,    37,     0,     0,    38,    39,    40,
      41,    42,     0,    43,     0,    44,     0,    45,     0,     0,
      46,     0,     0,     0,    47,    48,    49,    50,     0,    52,
      53,     0,     0,    54,     0,     0,    56,     0,    57,    58,
      59,   376,    61,    62,    63,     0,     0,     0,    64,    65,
       0,    66,    67,    68,    69,    70,    71,    72,     0,     0,
       0,     0,     0,     0,    73,     0,     0,     0,     0,   116,
      75,    76,     0,     0,    77,     0,    78,    79,   930,     0,
      80,    81,    82,     4,     5,     6,     7,     8,     0,     0,
       0,     0,     9,    10,     0,    11,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,     0,    26,    27,
      28,     0,     0,     0,     0,     0,     0,     0,    29,    30,
      31,    32,    33,    34,    35,    36,     0,    37,     0,     0,
      38,    39,    40,    41,    42,     0,    43,     0,    44,     0,
      45,     0,     0,    46,     0,     0,     0,    47,    48,    49,
      50,     0,    52,    53,     0,     0,    54,     0,     0,    56,
       0,    57,    58,    59,   376,    61,    62,    63,     0,     0,
       0,    64,    65,     0,    66,    67,    68,    69,    70,    71,
      72,     0,     0,     0,     0,     0,     0,    73,     0,     0,
       0,     0,   116,    75,    76,     0,     0,    77,     0,    78,
      79,   944,     0,    80,    81,    82,   401,   402,   403,   404,
     405,     0,   406,   407,   408,   409,   410,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   411,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   412,   413,     0,   414,   415,   416,   417,   418,
     419,   420,   421,   422,     0,     0,   423,    35,     0,     0,
       0,     0,     0,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,     0,   458,   459,
     460,   461,   462,     0,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,     0,     0,     0,     0,     4,     5,
       6,     7,     8,     0,     0,   473,   474,     9,    10,     0,
      11,     0,     0,   475,     0,     0,     0,     0,    82,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,     0,    26,    27,    28,     0,     0,     0,     0,
       0,     0,     0,    29,    30,    31,    32,    33,    34,    35,
      36,     0,    37,     0,     0,    38,    39,    40,    41,    42,
       0,    43,     0,    44,     0,    45,     0,     0,    46,     0,
       0,     0,    47,    48,    49,    50,     0,    52,    53,     0,
       0,    54,     0,     0,    56,     0,    57,    58,    59,   376,
      61,    62,    63,     0,     0,     0,    64,    65,     0,    66,
      67,    68,    69,    70,    71,    72,     0,     0,     0,     0,
       0,     0,    73,     0,     0,     0,     0,   116,    75,    76,
       0,     0,    77,     0,    78,    79,     0,     0,    80,    81,
      82,     4,     5,     6,     7,     8,     0,     0,     0,     0,
       9,    10,     0,    11,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   633,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,     0,    26,    27,    28,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    31,    32,
      33,    34,    35,    36,     0,    37,     0,     0,    38,    39,
      40,    41,    42,     0,    43,     0,    44,     0,    45,     0,
       0,    46,     0,     0,     0,    47,    48,    49,    50,     0,
      52,    53,     0,     0,    54,     0,     0,    56,     0,    57,
      58,    59,     0,     0,     0,     0,     0,     0,     0,    64,
      65,     0,    66,    67,    68,    69,    70,    71,    72,     0,
       0,     0,     0,     0,     0,    73,     0,     0,     0,     0,
     116,    75,    76,     0,     0,    77,     0,    78,    79,     0,
       0,    80,    81,    82,     4,     5,     6,     7,     8,     0,
       0,     0,     0,     9,    10,     0,    11,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   636,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,     0,    26,
      27,    28,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,    37,     0,
       0,    38,    39,    40,    41,    42,     0,    43,     0,    44,
       0,    45,     0,     0,    46,     0,     0,     0,    47,    48,
      49,    50,     0,    52,    53,     0,     0,    54,     0,     0,
      56,     0,    57,    58,    59,     0,     0,     0,     0,     0,
       0,     0,    64,    65,     0,    66,    67,    68,    69,    70,
      71,    72,     0,     0,     0,     0,     0,     0,    73,     0,
       0,     0,     0,   116,    75,    76,     0,     0,    77,     0,
      78,    79,     0,     0,    80,    81,    82,     4,     5,     6,
       7,     8,     0,     0,     0,     0,     9,    10,     0,    11,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   707,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,     0,    26,    27,    28,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    31,    32,    33,    34,    35,    36,
       0,    37,     0,     0,    38,    39,    40,    41,    42,     0,
      43,     0,    44,     0,    45,     0,     0,    46,     0,     0,
       0,    47,    48,    49,    50,     0,    52,    53,     0,     0,
      54,     0,     0,    56,     0,    57,    58,    59,     0,     0,
       0,     0,     0,     0,     0,    64,    65,     0,    66,    67,
      68,    69,    70,    71,    72,     0,     0,     0,     0,     0,
       0,    73,     0,     0,     0,     0,   116,    75,    76,     0,
       0,    77,     0,    78,    79,     0,     0,    80,    81,    82,
       4,     5,     6,     7,     8,     0,     0,     0,     0,     9,
      10,     0,    11,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   752,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,     0,    26,    27,    28,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    31,    32,    33,
      34,    35,    36,     0,    37,     0,     0,    38,    39,    40,
      41,    42,     0,    43,     0,    44,     0,    45,     0,     0,
      46,     0,     0,     0,    47,    48,    49,    50,     0,    52,
      53,     0,     0,    54,     0,     0,    56,     0,    57,    58,
      59,     0,     0,     0,     0,     0,     0,     0,    64,    65,
       0,    66,    67,    68,    69,    70,    71,    72,     0,     0,
       0,     0,     0,     0,    73,     0,     0,     0,     0,   116,
      75,    76,     0,     0,    77,     0,    78,    79,     0,     0,
      80,    81,    82,     4,     5,     6,     7,     8,     0,     0,
       0,     0,     9,    10,     0,    11,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     835,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,     0,    26,    27,
      28,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      31,    32,    33,    34,    35,    36,     0,    37,     0,     0,
      38,    39,    40,    41,    42,     0,    43,     0,    44,     0,
      45,     0,     0,    46,     0,     0,     0,    47,    48,    49,
      50,     0,    52,    53,     0,     0,    54,     0,     0,    56,
       0,    57,    58,    59,     0,     0,     0,     0,     0,     0,
       0,    64,    65,     0,    66,    67,    68,    69,    70,    71,
      72,     0,     0,     0,     0,     0,     0,    73,     0,     0,
       0,     0,   116,    75,    76,     0,     0,    77,     0,    78,
      79,     0,     0,    80,    81,    82,     4,     5,     6,     7,
       8,     0,     0,     0,     0,     9,    10,     0,    11,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
       0,    26,    27,    28,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    31,    32,    33,    34,    35,    36,     0,
      37,     0,     0,    38,    39,    40,    41,    42,     0,    43,
       0,    44,     0,    45,     0,     0,    46,     0,     0,     0,
      47,    48,    49,    50,     0,    52,    53,     0,     0,    54,
       0,     0,    56,     0,    57,    58,    59,     0,     0,     0,
       0,     0,     0,     0,    64,    65,     0,    66,    67,    68,
      69,    70,    71,    72,     0,     0,     0,     0,     0,     0,
      73,     0,     0,     0,     0,   116,    75,    76,     0,     0,
      77,     0,    78,    79,     0,     0,    80,    81,    82,     4,
       5,     6,     7,     8,     0,     0,     0,     0,     9,    10,
       0,    11,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   114,    32,    33,   115,
      35,     0,     0,    37,     0,     0,    38,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     4,     5,     6,     7,     8,     0,     0,    58,    59,
       9,    10,     0,    11,     0,     0,     0,    64,    65,     0,
      66,    67,    68,    69,    70,    71,    72,     0,     0,     0,
       0,     0,     0,    73,   145,     0,     0,     0,   116,    75,
      76,   388,     0,    77,   389,     0,    12,    13,     0,    80,
      81,    82,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,     0,    26,    27,    28,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   114,    32,
      33,   115,    35,     0,     0,    37,     0,     0,    38,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     4,     5,     6,     7,     8,     0,     0,
      58,    59,     9,    10,     0,    11,     0,     0,     0,   146,
      65,     0,    66,    67,    68,    69,    70,    71,    72,     0,
       0,     0,     0,     0,     0,    73,     0,     0,     0,     0,
     116,    75,    76,     0,     0,    77,     0,     0,    12,    13,
       0,    80,    81,    82,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,     0,    26,    27,
      28,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     114,    32,    33,   115,    35,     0,     0,    37,   365,     0,
      38,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     4,     5,     6,     7,     8,
       0,     0,    58,    59,     9,    10,     0,    11,     0,     0,
       0,    64,    65,     0,    66,    67,    68,    69,    70,    71,
      72,     0,   485,     0,     0,     0,     0,    73,     0,     0,
       0,     0,   116,    75,    76,     0,     0,    77,     0,     0,
      12,    13,     0,    80,    81,    82,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,     0,
      26,    27,    28,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   114,    32,    33,   115,    35,     0,     0,    37,
       0,     0,    38,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     4,     5,     6,
       7,     8,     0,     0,    58,    59,     9,    10,     0,    11,
       0,     0,     0,    64,    65,     0,    66,    67,    68,    69,
      70,    71,    72,     0,     0,     0,     0,     0,     0,    73,
     521,     0,     0,     0,   116,    75,    76,     0,     0,    77,
       0,     0,    12,    13,     0,    80,    81,    82,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,     0,    26,    27,    28,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   114,    32,    33,   115,    35,     0,
       0,    37,     0,     0,    38,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     4,
       5,     6,     7,     8,     0,     0,    58,    59,     9,    10,
       0,    11,     0,     0,     0,    64,    65,     0,    66,    67,
      68,    69,    70,    71,    72,     0,     0,     0,     0,     0,
       0,    73,   540,     0,     0,     0,   116,    75,    76,     0,
       0,    77,     0,     0,    12,    13,     0,    80,    81,    82,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   114,    32,    33,   115,
      35,     0,     0,    37,     0,     0,    38,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     4,     5,     6,     7,     8,     0,     0,    58,    59,
       9,    10,     0,    11,     0,     0,     0,   541,    65,     0,
      66,    67,    68,    69,    70,    71,    72,     0,     0,     0,
       0,     0,     0,    73,     0,     0,     0,     0,   116,    75,
      76,     0,     0,    77,     0,     0,    12,    13,     0,    80,
      81,    82,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,     0,    26,    27,    28,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   114,    32,
      33,   115,    35,     0,     0,    37,     0,     0,    38,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     4,     5,     6,     7,     8,     0,     0,
      58,    59,     9,    10,     0,    11,     0,     0,     0,    64,
      65,     0,    66,    67,    68,    69,    70,    71,    72,     0,
       0,     0,     0,     0,     0,    73,     0,     0,     0,     0,
     116,    75,    76,   388,     0,    77,     0,     0,    12,    13,
       0,    80,    81,    82,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,     0,    26,    27,
      28,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     114,    32,    33,   115,    35,     0,     0,    37,     0,     0,
      38,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    58,    59,     0,     0,     0,     0,     0,     0,
       0,    64,    65,     0,    66,    67,    68,    69,    70,    71,
      72,     0,     0,     0,     0,     0,     0,    73,     0,     0,
       0,     0,   116,    75,    76,     0,     0,    77,     0,   242,
     243,   244,     0,    80,    81,    82,   401,   402,   403,   404,
     405,     0,   406,   407,   408,   409,   410,     0,     0,   245,
     807,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,     0,   269,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   270,   411,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   412,   413,     0,   414,   415,   416,   417,   418,
     419,   420,   421,   422,     0,     0,   876,     0,     0,     0,
       0,     0,     0,   424,   425,   426,   427,   428,   429,   430,
     431,   432,   433,   434,   435,   436,   437,   438,   439,   440,
     441,   442,   443,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,     0,   458,   459,
     460,   461,   462,     0,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   401,   402,   403,   404,   405,   808,
     406,   407,   408,   409,   410,   877,   474,    76,     0,     0,
       0,     0,     0,     0,   878,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   411,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     412,   413,     0,   414,   415,   416,   417,   418,   419,   420,
     421,   422,     0,     0,   876,     0,     0,     0,     0,     0,
       0,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,     0,   458,   459,   460,   461,
     462,     0,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,     0,   242,   243,   244,     0,     0,     0,     0,
       0,     0,     0,   877,   474,    76,     0,     0,     0,     0,
       0,     0,   904,   245,     0,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,   266,   267,   268,     0,   269,
     242,   243,   244,     0,     0,     0,     0,     0,     0,     0,
       0,   270,     0,     0,     0,     0,     0,     0,     0,     0,
     245,     0,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,     0,   269,   242,   243,   244,
       0,     0,     0,     0,     0,     0,     0,     0,   270,     0,
       0,     0,     0,     0,     0,     0,     0,   245,     0,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,     0,   269,     0,   242,   243,   244,     0,     0,
       0,     0,     0,     0,     0,   270,   730,     0,     0,     0,
       0,     0,     0,     0,     0,   245,     0,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
       0,   269,   242,   243,   244,     0,     0,     0,     0,     0,
       0,     0,   599,   270,     0,     0,     0,     0,     0,     0,
       0,     0,   245,     0,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,     0,   269,     0,
     242,   243,   244,     0,     0,     0,     0,     0,     0,   604,
     270,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     245,     0,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,     0,   269,   242,   243,   244,
       0,     0,     0,     0,     0,     0,     0,   623,   270,     0,
       0,     0,     0,     0,     0,     0,     0,   245,     0,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,     0,   269,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   631,   270,     0,     0,     0,     0,
       0,     0,     0,   401,   402,   403,   404,   405,     0,   406,
     407,   408,   409,   410,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   692,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   411,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   412,
     413,     0,   414,   415,   416,   417,   418,   419,   420,   421,
     422,     0,     0,   423,     0,     0,     0,     0,     0,   694,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,     0,   458,   459,   460,   461,   462,
       0,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   401,   402,   403,   404,   405,     0,   406,   407,   408,
     409,   410,   473,   474,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   411,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   412,   413,     0,
     414,   415,   416,   771,   772,   773,   774,   775,   776,     0,
       0,   921,     0,     0,     0,     0,     0,     0,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,     0,   458,   459,   460,   461,   462,     0,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   242,
     243,   244,     0,     0,     0,     0,     0,     0,     0,     0,
     473,   474,     0,     0,     0,     0,     0,     0,     0,   245,
       0,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,     0,   269,   242,   243,   244,     0,
       0,     0,     0,     0,     0,     0,     0,   270,     0,     0,
       0,     0,     0,     0,     0,     0,   245,     0,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,     0,   269,   242,   243,   244,     0,     0,     0,     0,
       0,     0,     0,     0,   270,     0,     0,     0,     0,     0,
       0,     0,     0,   245,     0,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,   266,   267,   268,     0,   269,
     242,   243,   244,     0,     0,     0,     0,     0,     0,   271,
       0,   270,     0,     0,     0,     0,     0,     0,     0,     0,
     245,     0,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,     0,   269,   242,   243,   244,
       0,     0,     0,     0,     0,     0,   334,     0,   270,     0,
       0,     0,     0,     0,     0,     0,     0,   245,     0,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,     0,   269,   242,   243,   244,     0,     0,     0,
       0,     0,   375,     0,     0,   270,     0,     0,     0,     0,
       0,     0,     0,     0,   245,     0,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,     0,
     269,   242,   243,   244,     0,     0,     0,     0,     0,   535,
       0,     0,   270,     0,     0,     0,     0,     0,     0,     0,
       0,   245,     0,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,     0,   269,   242,   243,
     244,     0,     0,     0,     0,     0,   538,     0,     0,   270,
       0,     0,     0,     0,     0,     0,     0,     0,   245,     0,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,     0,   269,   242,   243,   244,     0,     0,
       0,     0,     0,   555,     0,     0,   270,     0,     0,     0,
       0,     0,     0,     0,     0,   245,     0,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
       0,   269,   242,   243,   244,     0,     0,     0,     0,     0,
     558,     0,     0,   270,     0,     0,     0,     0,     0,     0,
       0,     0,   245,     0,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,     0,   269,   242,
     243,   244,     0,     0,     0,     0,     0,   563,     0,     0,
     270,     0,     0,     0,     0,     0,     0,     0,     0,   245,
       0,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,     0,   269,   242,   243,   244,     0,
       0,     0,     0,     0,   582,     0,     0,   270,     0,     0,
       0,     0,     0,     0,     0,     0,   245,     0,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,     0,   269,   242,   243,   244,     0,     0,     0,     0,
       0,   678,     0,     0,   270,     0,     0,     0,     0,     0,
       0,     0,     0,   245,     0,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,   266,   267,   268,     0,   269,
       0,   242,   243,   244,     0,     0,   299,     0,   679,     0,
       0,   270,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   245,     0,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   561,   269,   242,   243,
     244,     0,     0,     0,     0,   699,     0,     0,     0,   270,
       0,     0,     0,     0,     0,     0,     0,     0,   245,   618,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,     0,   269,   242,   243,   244,     0,     0,
       0,     0,     0,     0,     0,     0,   270,     0,     0,     0,
       0,     0,     0,     0,     0,   245,     0,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
       0,   269,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   270
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-714)))

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-472)))

static const yytype_int16 yycheck[] =
{
       2,    28,   117,     2,   240,   374,   189,     2,     4,     5,
     632,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,     2,    31,
      26,    27,     2,    24,    25,    28,   272,    84,   138,     8,
       8,     2,   559,     8,    40,    30,     8,    29,   163,     8,
     663,    47,    48,     8,    55,     8,    52,    14,    54,   681,
     313,   108,     8,   728,    68,     8,   185,     8,     8,     8,
      83,    68,     8,    74,     8,    76,     8,     8,     8,   792,
      41,    77,    48,    28,   797,    14,    52,    14,   102,    83,
      14,    68,    84,    68,    80,   629,    83,    83,    84,    34,
     783,    83,    83,    55,   357,   770,   206,   207,    30,     0,
      83,    56,   114,    36,   151,    67,   110,   111,   545,   157,
     157,   110,   111,   154,   120,    83,   105,   106,   159,   110,
     111,   796,    83,   130,    72,    73,    74,    82,    30,   125,
     154,    83,   138,   677,   140,   157,   399,   160,   157,    84,
     132,   133,   157,   130,   145,   130,   339,   161,    83,    84,
     154,    84,   827,   160,   777,   152,   152,   154,   154,   165,
     152,   167,   154,   165,   151,   160,   172,   173,   174,   165,
     176,   608,   161,   160,   701,    24,    25,   160,   189,   158,
     191,   159,   160,   162,   159,    24,    25,   159,   298,   165,
     159,   158,   160,   158,   200,   201,   159,   162,   199,   160,
     206,   207,   154,   159,   215,   211,   159,   158,   220,   159,
     159,   212,   158,   936,   158,   908,   158,   158,   158,   158,
     226,   158,   601,   229,   158,   160,   157,   159,    72,    73,
     165,    84,   269,   114,   115,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   234,   270,   241,   269,   273,   397,   275,
      84,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,    83,   240,   909,    68,   294,    29,
      36,    83,   298,   299,   926,   301,   145,   273,    84,   831,
     306,   564,   834,   309,   310,   102,   145,   160,   717,    84,
     316,    86,   165,   722,   269,    68,   648,   272,    84,   274,
      86,   333,    68,   329,   110,   588,   159,   160,   339,   157,
     306,   157,    68,   344,    80,   149,   150,    83,    84,    80,
      81,    87,   157,    83,   105,   106,    83,   102,   303,   130,
     199,   102,   307,   308,    83,   154,   160,   154,   767,   157,
     199,   157,   154,   212,   102,    84,   157,    86,   157,   157,
     151,    83,   157,   212,   149,   150,   569,   130,   710,   160,
     712,   574,   388,   149,   150,   131,   132,   154,   394,   164,
     396,   346,   132,   133,   130,   927,   928,   929,   151,   154,
     161,   664,    36,   154,   157,   937,   152,   160,   154,   941,
      84,   157,   152,    68,   154,   151,   154,    83,   158,   165,
     151,   157,    68,   480,   160,    80,    78,    79,    83,    84,
     149,   150,    87,   696,   151,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    75,    76,
      77,    78,    79,    80,   157,   157,    84,   514,    86,   652,
     653,    49,   519,   110,   154,   658,   132,   133,    67,   475,
     733,   157,   157,   125,   148,   149,   150,   132,   162,   485,
     104,   105,   106,     8,    65,    66,   152,    68,   154,   151,
     117,    94,    16,   120,   159,    83,    84,   152,   159,   154,
     159,    89,   157,    16,   633,   159,   159,   636,   158,    83,
     165,   517,   104,   105,   106,   209,    84,   627,    86,   213,
     521,   149,   150,    83,    83,   160,   719,   221,   721,   223,
     224,   225,   642,   163,   161,    83,   157,   543,   157,   540,
      30,    75,    76,    77,    78,    79,    80,   584,   159,   130,
     587,   557,    83,   559,   560,   162,    16,   820,   569,   157,
     561,    84,   158,   574,   159,    80,     8,    83,    83,    84,
     151,   128,   159,   159,   580,   768,   613,   578,   707,   160,
     148,   149,   150,   117,   555,   128,   120,   558,   158,   158,
     602,   597,    83,   602,   704,   632,   161,   602,   160,   611,
     606,    84,   157,    86,   850,    83,   735,   157,   157,   162,
     157,   162,   618,   859,   158,   157,   157,   162,   602,   157,
     159,   627,   602,   752,   887,   129,   663,   161,   160,   892,
      16,   602,    89,   162,   763,   881,   642,   152,   158,   154,
     641,   652,   653,   157,   681,   682,   157,   658,   911,   159,
     165,    48,    49,    50,    51,    52,    53,   903,    55,   159,
     159,   673,     8,   669,   673,   148,   149,   150,   673,     8,
      67,   160,   521,   159,    30,   646,   158,   923,   159,     8,
     809,    36,   521,   158,   158,   162,   698,   159,   700,   673,
     727,   540,   157,   673,   160,   701,     8,   161,   704,   824,
     161,   540,   673,   740,   705,   160,   835,   678,   719,   158,
     721,   117,   561,   842,    75,    76,    77,    78,    79,    80,
     155,   160,   561,   159,   158,   762,    51,    52,    53,   578,
      55,   158,   158,   161,   159,   706,    84,   161,   160,   578,
     777,    30,    67,   755,    75,    76,    77,    78,    79,    80,
     157,    84,   758,    84,   159,   159,   117,   768,   161,   120,
      16,    16,    16,   159,   158,   894,    84,   160,   160,   898,
     782,   900,   160,   902,   102,   160,   151,   789,    75,    76,
      77,    78,    79,    80,    75,    76,    77,    78,    79,    80,
     802,   828,   641,   159,   159,   102,   118,   159,   157,   908,
     161,   813,   641,    55,   158,   652,   721,   844,   191,   719,
     578,   940,   175,   850,   344,    89,   705,   740,   806,   801,
     117,   682,   606,   120,   346,   881,   117,   311,   799,   120,
     801,   843,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,   881,    55,   852,   828,   890,   316,
     330,   745,   896,   269,   854,   302,   705,    67,   272,   871,
     301,   873,   868,   307,   161,   580,   705,    -1,    -1,    -1,
     161,    -1,   909,    -1,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,     7,   889,    -1,    -1,    -1,    12,    13,   926,
      15,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   914,    -1,   916,   917,    -1,   919,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,
     942,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    68,    69,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      85,    -1,    87,    -1,    -1,    90,    91,    92,    93,    94,
      -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,    -1,
      -1,    -1,   107,   108,   109,   110,   111,   112,   113,    -1,
      -1,   116,   117,    -1,   119,    -1,   121,   122,   123,   124,
     125,   126,   127,    -1,    -1,    -1,   131,   132,    -1,   134,
     135,   136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,
      -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,
      -1,    -1,   157,    -1,   159,   160,   161,    11,   163,   164,
     165,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      12,    13,    -1,    15,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    -1,    -1,    -1,    -1,    48,    49,    -1,    -1,
      -1,    -1,    54,    67,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    68,    69,    70,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    85,    -1,    87,    -1,    -1,    90,    91,
      92,    93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,
      -1,   103,    -1,    -1,    -1,   107,   108,   109,   110,   111,
     112,   113,    -1,    -1,   116,   117,    -1,   119,    -1,   121,
     122,   123,   124,   125,   126,   127,    -1,    -1,    -1,   131,
     132,    -1,   134,   135,   136,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,    -1,    -1,   157,    -1,   159,   160,   161,
      14,   163,   164,   165,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    12,    13,    29,    15,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      49,    -1,    -1,    67,    -1,    54,    -1,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    68,
      69,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    85,    -1,    87,    -1,
      -1,    90,    91,    92,    93,    94,    -1,    96,    -1,    98,
      -1,   100,    -1,    -1,   103,    -1,    -1,    -1,   107,   108,
     109,   110,   111,   112,   113,    -1,    -1,   116,   117,    -1,
     119,    -1,   121,   122,   123,   124,   125,   126,   127,    -1,
      -1,    -1,   131,   132,    -1,   134,   135,   136,   137,   138,
     139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,
      -1,    -1,    -1,   152,   153,   154,    -1,    -1,   157,    -1,
     159,   160,    10,    11,   163,   164,   165,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    12,    13,    -1,    15,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    -1,    -1,
      -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,    67,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    68,    69,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    85,
      -1,    87,    -1,    -1,    90,    91,    92,    93,    94,    -1,
      96,    -1,    98,    -1,   100,    -1,    -1,   103,    -1,    -1,
      -1,   107,   108,   109,   110,    -1,   112,   113,    -1,    -1,
     116,    -1,    -1,   119,    -1,   121,   122,   123,   124,   125,
     126,   127,    -1,    -1,    -1,   131,   132,    -1,   134,   135,
     136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,
      -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,    -1,
      -1,   157,    -1,   159,   160,   161,    -1,   163,   164,   165,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,
      13,    29,    15,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    67,
      -1,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    68,    69,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    85,    -1,    87,    -1,    -1,    90,    91,    92,
      93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,
     103,    -1,    -1,    -1,   107,   108,   109,   110,    -1,   112,
     113,    -1,    -1,   116,    -1,    -1,   119,    -1,   121,   122,
     123,   124,   125,   126,   127,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,    -1,    -1,   157,    -1,   159,   160,   161,    -1,
     163,   164,   165,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    12,    13,    -1,    15,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,
      -1,    -1,    67,    -1,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    68,    69,
      70,    -1,    -1,    -1,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    85,    -1,    87,    -1,    -1,
      90,    91,    92,    93,    94,    -1,    96,    -1,    98,    -1,
     100,    -1,    -1,   103,    -1,    -1,    -1,   107,   108,   109,
     110,    -1,   112,   113,    -1,    -1,   116,    -1,    -1,   119,
      -1,   121,   122,   123,   124,   125,   126,   127,    -1,    -1,
      -1,   131,   132,    -1,   134,   135,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,    -1,    -1,   157,    -1,   159,
     160,    -1,    -1,   163,   164,   165,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,    13,    -1,    15,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    49,    67,    -1,    -1,    -1,    54,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    68,    69,    70,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    85,    -1,
      87,    -1,    -1,    90,    91,    92,    93,    94,    95,    96,
      -1,    98,    -1,   100,    -1,    -1,   103,    -1,    -1,    -1,
     107,   108,   109,   110,    -1,   112,   113,    -1,    -1,   116,
      -1,    -1,   119,    -1,   121,   122,   123,   124,   125,   126,
     127,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,   153,   154,    -1,    -1,
     157,    -1,   159,   160,    -1,    -1,   163,   164,   165,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,    13,
      -1,    15,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    49,    67,    -1,    -1,    -1,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    68,    69,    70,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    85,    -1,    87,    -1,    -1,    90,    91,    92,    93,
      94,    -1,    96,    -1,    98,    -1,   100,   101,    -1,   103,
      -1,    -1,    -1,   107,   108,   109,   110,    -1,   112,   113,
      -1,    -1,   116,    -1,    -1,   119,    -1,   121,   122,   123,
     124,   125,   126,   127,    -1,    -1,    -1,   131,   132,    -1,
     134,   135,   136,   137,   138,   139,   140,    -1,    -1,    -1,
      -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,
     154,    -1,    -1,   157,    -1,   159,   160,    -1,    -1,   163,
     164,   165,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,    13,    -1,    15,    -1,    -1,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    67,
      -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    68,    69,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    85,    -1,    87,    -1,    -1,    90,
      91,    92,    93,    94,    -1,    96,    -1,    98,    99,   100,
      -1,    -1,   103,    -1,    -1,    -1,   107,   108,   109,   110,
      -1,   112,   113,    -1,    -1,   116,    -1,    -1,   119,    -1,
     121,   122,   123,   124,   125,   126,   127,    -1,    -1,    -1,
     131,   132,    -1,   134,   135,   136,   137,   138,   139,   140,
      -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,
      -1,   152,   153,   154,    -1,    -1,   157,    -1,   159,   160,
      -1,    -1,   163,   164,   165,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    12,    13,    -1,    15,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,
      48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      68,    69,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    85,    -1,    87,
      -1,    -1,    90,    91,    92,    93,    94,    -1,    96,    -1,
      98,    -1,   100,    -1,    -1,   103,    -1,    -1,    -1,   107,
     108,   109,   110,    -1,   112,   113,    -1,    -1,   116,    -1,
      -1,   119,    -1,   121,   122,   123,   124,   125,   126,   127,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,
      -1,    -1,    -1,    -1,   152,   153,   154,    -1,    -1,   157,
      -1,   159,   160,   161,    -1,   163,   164,   165,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    12,    13,    -1,
      15,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    68,    69,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      85,    -1,    87,    -1,    -1,    90,    91,    92,    93,    94,
      -1,    96,    97,    98,    -1,   100,    -1,    -1,   103,    -1,
      -1,    -1,   107,   108,   109,   110,    -1,   112,   113,    -1,
      -1,   116,    -1,    -1,   119,    -1,   121,   122,   123,   124,
     125,   126,   127,    -1,    -1,    -1,   131,   132,    -1,   134,
     135,   136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,
      -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,
      -1,    -1,   157,    -1,   159,   160,    -1,    -1,   163,   164,
     165,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      12,    13,    -1,    15,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    48,    49,    -1,    -1,
      -1,    -1,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    68,    69,    70,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    85,    -1,    87,    -1,    -1,    90,    91,
      92,    93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,
      -1,   103,    -1,    -1,    -1,   107,   108,   109,   110,    -1,
     112,   113,    -1,    -1,   116,    -1,    -1,   119,    -1,   121,
     122,   123,   124,   125,   126,   127,    -1,    -1,    -1,   131,
     132,    -1,   134,   135,   136,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,    -1,    -1,   157,    -1,   159,   160,   161,
      -1,   163,   164,   165,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    12,    13,    -1,    15,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    68,
      69,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    85,    -1,    87,    -1,
      -1,    90,    91,    92,    93,    94,    -1,    96,    -1,    98,
      -1,   100,    -1,    -1,   103,    -1,    -1,    -1,   107,   108,
     109,   110,    -1,   112,   113,    -1,    -1,   116,    -1,    -1,
     119,    -1,   121,   122,   123,   124,   125,   126,   127,    -1,
      -1,    -1,   131,   132,    -1,   134,   135,   136,   137,   138,
     139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,
      -1,    -1,    -1,   152,   153,   154,    -1,    -1,   157,    -1,
     159,   160,   161,    -1,   163,   164,   165,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    12,    13,    -1,    15,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    68,    69,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    85,
      -1,    87,    -1,    -1,    90,    91,    92,    93,    94,    -1,
      96,    -1,    98,    -1,   100,    -1,    -1,   103,    -1,    -1,
      -1,   107,   108,   109,   110,    -1,   112,   113,    -1,    -1,
     116,    -1,    -1,   119,    -1,   121,   122,   123,   124,   125,
     126,   127,    -1,    -1,    -1,   131,   132,    -1,   134,   135,
     136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,
      -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,    -1,
      -1,   157,    -1,   159,   160,   161,    -1,   163,   164,   165,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,
      13,    -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    -1,
      -1,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    68,    69,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    85,    -1,    87,    -1,    -1,    90,    91,    92,
      93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,
     103,    -1,    -1,    -1,   107,   108,   109,   110,    -1,   112,
     113,    -1,    -1,   116,    -1,    -1,   119,    -1,   121,   122,
     123,   124,   125,   126,   127,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,    -1,    -1,   157,    -1,   159,   160,   161,    -1,
     163,   164,   165,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    12,    13,    -1,    15,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,
      -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    68,    69,
      70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    85,    -1,    87,    -1,    -1,
      90,    91,    92,    93,    94,    -1,    96,    -1,    98,    -1,
     100,    -1,    -1,   103,    -1,    -1,    -1,   107,   108,   109,
     110,    -1,   112,   113,    -1,    -1,   116,    -1,    -1,   119,
      -1,   121,   122,   123,   124,   125,   126,   127,    -1,    -1,
      -1,   131,   132,    -1,   134,   135,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,    -1,    -1,   157,    -1,   159,
     160,   161,    -1,   163,   164,   165,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    70,    -1,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    -1,    -1,    83,    84,    -1,    -1,
      -1,    -1,    -1,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,    -1,   125,   126,
     127,   128,   129,    -1,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,     7,    -1,    -1,   152,   153,    12,    13,    -1,
      15,    -1,    -1,   160,    -1,    -1,    -1,    -1,   165,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    68,    69,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      85,    -1,    87,    -1,    -1,    90,    91,    92,    93,    94,
      -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,    -1,
      -1,    -1,   107,   108,   109,   110,    -1,   112,   113,    -1,
      -1,   116,    -1,    -1,   119,    -1,   121,   122,   123,   124,
     125,   126,   127,    -1,    -1,    -1,   131,   132,    -1,   134,
     135,   136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,
      -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,
      -1,    -1,   157,    -1,   159,   160,    -1,    -1,   163,   164,
     165,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      12,    13,    -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    -1,
      -1,    -1,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    68,    69,    70,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    80,    81,
      82,    83,    84,    85,    -1,    87,    -1,    -1,    90,    91,
      92,    93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,
      -1,   103,    -1,    -1,    -1,   107,   108,   109,   110,    -1,
     112,   113,    -1,    -1,   116,    -1,    -1,   119,    -1,   121,
     122,   123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,
     132,    -1,   134,   135,   136,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,    -1,    -1,   157,    -1,   159,   160,    -1,
      -1,   163,   164,   165,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    12,    13,    -1,    15,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      49,    -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    68,
      69,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    80,    81,    82,    83,    84,    85,    -1,    87,    -1,
      -1,    90,    91,    92,    93,    94,    -1,    96,    -1,    98,
      -1,   100,    -1,    -1,   103,    -1,    -1,    -1,   107,   108,
     109,   110,    -1,   112,   113,    -1,    -1,   116,    -1,    -1,
     119,    -1,   121,   122,   123,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   131,   132,    -1,   134,   135,   136,   137,   138,
     139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,
      -1,    -1,    -1,   152,   153,   154,    -1,    -1,   157,    -1,
     159,   160,    -1,    -1,   163,   164,   165,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    12,    13,    -1,    15,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    68,    69,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,    85,
      -1,    87,    -1,    -1,    90,    91,    92,    93,    94,    -1,
      96,    -1,    98,    -1,   100,    -1,    -1,   103,    -1,    -1,
      -1,   107,   108,   109,   110,    -1,   112,   113,    -1,    -1,
     116,    -1,    -1,   119,    -1,   121,   122,   123,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,
     136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,
      -1,   147,    -1,    -1,    -1,    -1,   152,   153,   154,    -1,
      -1,   157,    -1,   159,   160,    -1,    -1,   163,   164,   165,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,
      13,    -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    -1,
      -1,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    68,    69,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    80,    81,    82,
      83,    84,    85,    -1,    87,    -1,    -1,    90,    91,    92,
      93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,
     103,    -1,    -1,    -1,   107,   108,   109,   110,    -1,   112,
     113,    -1,    -1,   116,    -1,    -1,   119,    -1,   121,   122,
     123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,   139,   140,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,
     153,   154,    -1,    -1,   157,    -1,   159,   160,    -1,    -1,
     163,   164,   165,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    12,    13,    -1,    15,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,
      -1,    -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    68,    69,
      70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      80,    81,    82,    83,    84,    85,    -1,    87,    -1,    -1,
      90,    91,    92,    93,    94,    -1,    96,    -1,    98,    -1,
     100,    -1,    -1,   103,    -1,    -1,    -1,   107,   108,   109,
     110,    -1,   112,   113,    -1,    -1,   116,    -1,    -1,   119,
      -1,   121,   122,   123,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   131,   132,    -1,   134,   135,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,    -1,    -1,   157,    -1,   159,
     160,    -1,    -1,   163,   164,   165,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,    13,    -1,    15,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    49,    -1,    -1,    -1,    -1,    54,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    68,    69,    70,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    80,    81,    82,    83,    84,    85,    -1,
      87,    -1,    -1,    90,    91,    92,    93,    94,    -1,    96,
      -1,    98,    -1,   100,    -1,    -1,   103,    -1,    -1,    -1,
     107,   108,   109,   110,    -1,   112,   113,    -1,    -1,   116,
      -1,    -1,   119,    -1,   121,   122,   123,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,
     147,    -1,    -1,    -1,    -1,   152,   153,   154,    -1,    -1,
     157,    -1,   159,   160,    -1,    -1,   163,   164,   165,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,    13,
      -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    49,    -1,    -1,    -1,    -1,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    68,    69,    70,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    80,    81,    82,    83,
      84,    -1,    -1,    87,    -1,    -1,    90,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   110,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,     7,    -1,    -1,   122,   123,
      12,    13,    -1,    15,    -1,    -1,    -1,   131,   132,    -1,
     134,   135,   136,   137,   138,   139,   140,    -1,    -1,    -1,
      -1,    -1,    -1,   147,    36,    -1,    -1,    -1,   152,   153,
     154,   155,    -1,   157,   158,    -1,    48,    49,    -1,   163,
     164,   165,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    68,    69,    70,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    80,    81,
      82,    83,    84,    -1,    -1,    87,    -1,    -1,    90,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   110,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,
     122,   123,    12,    13,    -1,    15,    -1,    -1,    -1,   131,
     132,    -1,   134,   135,   136,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,    -1,    -1,   157,    -1,    -1,    48,    49,
      -1,   163,   164,   165,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    68,    69,
      70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      80,    81,    82,    83,    84,    -1,    -1,    87,    88,    -1,
      90,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     110,    -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,
      -1,    -1,   122,   123,    12,    13,    -1,    15,    -1,    -1,
      -1,   131,   132,    -1,   134,   135,   136,   137,   138,   139,
     140,    -1,    30,    -1,    -1,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,    -1,    -1,   157,    -1,    -1,
      48,    49,    -1,   163,   164,   165,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      68,    69,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    80,    81,    82,    83,    84,    -1,    -1,    87,
      -1,    -1,    90,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   110,    -1,    -1,    -1,    -1,     3,     4,     5,
       6,     7,    -1,    -1,   122,   123,    12,    13,    -1,    15,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,   139,   140,    -1,    -1,    -1,    -1,    -1,    -1,   147,
      36,    -1,    -1,    -1,   152,   153,   154,    -1,    -1,   157,
      -1,    -1,    48,    49,    -1,   163,   164,   165,    54,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    68,    69,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,    -1,
      -1,    87,    -1,    -1,    90,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   110,    -1,    -1,    -1,    -1,     3,
       4,     5,     6,     7,    -1,    -1,   122,   123,    12,    13,
      -1,    15,    -1,    -1,    -1,   131,   132,    -1,   134,   135,
     136,   137,   138,   139,   140,    -1,    -1,    -1,    -1,    -1,
      -1,   147,    36,    -1,    -1,    -1,   152,   153,   154,    -1,
      -1,   157,    -1,    -1,    48,    49,    -1,   163,   164,   165,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    68,    69,    70,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    80,    81,    82,    83,
      84,    -1,    -1,    87,    -1,    -1,    90,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   110,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,     7,    -1,    -1,   122,   123,
      12,    13,    -1,    15,    -1,    -1,    -1,   131,   132,    -1,
     134,   135,   136,   137,   138,   139,   140,    -1,    -1,    -1,
      -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,   152,   153,
     154,    -1,    -1,   157,    -1,    -1,    48,    49,    -1,   163,
     164,   165,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    68,    69,    70,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    80,    81,
      82,    83,    84,    -1,    -1,    87,    -1,    -1,    90,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   110,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,
     122,   123,    12,    13,    -1,    15,    -1,    -1,    -1,   131,
     132,    -1,   134,   135,   136,   137,   138,   139,   140,    -1,
      -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,    -1,
     152,   153,   154,   155,    -1,   157,    -1,    -1,    48,    49,
      -1,   163,   164,   165,    54,    -1,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    68,    69,
      70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      80,    81,    82,    83,    84,    -1,    -1,    87,    -1,    -1,
      90,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     110,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   122,   123,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   131,   132,    -1,   134,   135,   136,   137,   138,   139,
     140,    -1,    -1,    -1,    -1,    -1,    -1,   147,    -1,    -1,
      -1,    -1,   152,   153,   154,    -1,    -1,   157,    -1,     9,
      10,    11,    -1,   163,   164,   165,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    70,    -1,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    -1,    -1,    83,    -1,    -1,    -1,
      -1,    -1,    -1,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,    -1,   125,   126,
     127,   128,   129,    -1,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,     3,     4,     5,     6,     7,   159,
       9,    10,    11,    12,    13,   152,   153,   154,    -1,    -1,
      -1,    -1,    -1,    -1,   161,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    70,    -1,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    -1,    -1,    83,    -1,    -1,    -1,    -1,    -1,
      -1,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,    -1,   125,   126,   127,   128,
     129,    -1,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,    -1,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   152,   153,   154,    -1,    -1,    -1,    -1,
      -1,    -1,   161,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    -1,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,   162,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   161,    67,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    55,    -1,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,   161,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,    67,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   161,    67,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,     9,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      70,    -1,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    -1,    -1,    83,    -1,    -1,    -1,    -1,    -1,   161,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,    -1,   125,   126,   127,   128,   129,
      -1,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,     3,     4,     5,     6,     7,    -1,     9,    10,    11,
      12,    13,   152,   153,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    70,    -1,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    -1,
      -1,    83,    -1,    -1,    -1,    -1,    -1,    -1,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,    -1,   125,   126,   127,   128,   129,    -1,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     152,   153,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,   159,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,   158,    -1,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,   158,
      -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,   158,    -1,    -1,    67,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
     158,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    55,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,   158,    -1,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,   158,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
      -1,     9,    10,    11,    -1,    -1,    14,    -1,   158,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,   102,    55,     9,    10,
      11,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   167,   171,     0,     3,     4,     5,     6,     7,    12,
      13,    15,    48,    49,    54,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    68,    69,    70,    78,
      79,    80,    81,    82,    83,    84,    85,    87,    90,    91,
      92,    93,    94,    96,    98,   100,   103,   107,   108,   109,
     110,   111,   112,   113,   116,   117,   119,   121,   122,   123,
     124,   125,   126,   127,   131,   132,   134,   135,   136,   137,
     138,   139,   140,   147,   152,   153,   154,   157,   159,   160,
     163,   164,   165,   172,   173,   174,   190,   197,   200,   203,
     204,   205,   207,   220,   221,   222,   223,   264,   265,   266,
     273,   274,   279,   280,   281,   282,   284,   285,   286,   287,
     288,   289,   290,   301,    80,    83,   152,   266,   282,   282,
     157,   282,   282,   282,   282,   282,   282,   282,   282,   282,
     282,   282,   282,   282,   282,   282,   282,   282,    68,    80,
     157,   279,   281,   288,   288,    36,   131,   282,   294,   295,
     296,   297,   282,   125,   173,   262,   274,   275,   289,   291,
      84,   236,   237,   266,    30,   157,   276,   157,   258,   259,
     282,   190,   157,   157,   157,   157,   157,   282,   283,   283,
      83,    83,   187,   257,   283,   160,   282,   110,   111,   154,
     172,   177,   179,   183,   185,   186,   234,   235,   289,   157,
     157,   157,   157,   202,   206,   208,   157,   157,    84,    86,
     148,   149,   150,   298,   299,   154,   172,   176,   172,   282,
     188,    86,   277,   298,    86,   298,   160,   289,   154,   157,
     231,   125,   204,    72,    73,    72,    73,    74,    36,   269,
     151,    68,     9,    10,    11,    29,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    55,
      67,   159,   151,    68,   130,   160,   231,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      65,    66,   269,   282,    14,   294,   282,   288,   157,    14,
     162,     8,   263,   151,   231,   278,    68,   130,   151,   160,
      16,     8,   159,   269,   283,   282,     8,   159,    94,   282,
     260,   261,   282,   282,   187,   282,   159,   159,   159,    16,
       8,   159,   159,   188,   159,   172,   185,   102,   154,   154,
     172,   178,   183,   159,     8,   159,     8,   159,   195,   196,
     288,   282,   302,   303,   282,   158,    83,    83,    83,   294,
     294,    68,   130,   148,   299,    88,   282,   288,    86,   148,
     299,   172,   159,   175,   160,   158,   124,   161,   189,   190,
     197,   200,   205,   207,   163,   164,   282,    83,   155,   158,
     232,   233,   282,   201,   157,   190,   157,    30,   159,    83,
     267,     3,     4,     5,     6,     7,     9,    10,    11,    12,
      13,    55,    69,    70,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    83,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   125,   126,
     127,   128,   129,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   152,   153,   160,   168,   169,   170,   289,
     292,   283,   282,   282,   282,    30,   282,   282,   282,   282,
     282,   282,   282,   282,   282,   282,   282,   282,   282,   282,
     282,   282,   282,   282,   282,   282,   282,   282,   282,   282,
     275,   282,   170,   289,   292,   283,    83,   160,   289,   293,
     282,    36,   282,   282,   282,   282,   282,   282,   282,   282,
     282,   282,   282,   282,   282,   158,   282,   162,   158,   294,
      36,   131,   282,    16,   295,   278,   289,   283,   293,   289,
     282,   282,   237,   267,   158,   158,   259,   157,   158,   159,
       8,   102,   158,   158,   282,   257,   161,   154,    83,   160,
     172,   154,   159,   159,   154,   172,   186,   235,     8,   158,
       8,   158,   158,   159,   128,   209,   267,   128,   210,   158,
     158,    49,    83,    84,    89,   300,    83,    68,   161,   161,
     161,   160,   171,   157,   161,   282,     8,   158,    83,   282,
     282,   188,   267,   157,   282,   231,   162,   282,    30,   231,
     162,   282,   231,   161,   288,   158,   288,   157,   282,   209,
     162,   161,   157,    30,   190,   282,    30,   190,   219,   260,
     282,    36,    68,   131,   212,   288,   191,    30,   160,   216,
     267,   192,   160,   177,   181,   184,   185,   154,   160,   196,
     159,   303,   173,   129,   211,   160,   173,   240,   267,    16,
      89,   162,   282,   171,   161,   158,   233,   209,   158,   158,
      74,   157,    29,   132,   133,   173,   224,   225,   226,   227,
     228,   229,   161,   282,   161,   294,   211,   224,   188,   158,
     188,   159,   288,   294,   157,    14,   158,    30,   190,   215,
     159,   217,   159,   217,   114,   115,   194,   181,   185,     8,
     180,   160,   182,   185,   240,   267,   238,     8,   160,   282,
     162,   161,   159,   211,   190,    30,   159,   224,   229,   158,
       8,    36,   198,   158,   267,   158,   159,    95,   260,   162,
     294,   212,    30,   190,   214,   188,   217,   104,   105,   106,
     217,   161,   157,   160,   180,   184,   161,   182,     8,   180,
     160,    75,    76,    77,    78,    79,    80,   117,   120,   161,
     239,   249,   250,   251,   252,   173,   238,   161,   267,   188,
     158,   117,   270,   226,   155,   199,   160,   270,   159,   158,
     158,   158,   188,   101,   104,   159,   282,    30,   159,   218,
     161,   173,   193,   188,   161,   180,   185,   161,   238,   240,
      84,   253,   254,   111,   266,   252,   161,   160,    30,   230,
     157,   230,    84,   238,   230,    30,   190,   213,   214,    99,
     159,   159,   218,   188,    34,    84,   161,   161,   161,   159,
     160,   241,    16,   267,     8,   159,   170,   255,   256,   269,
     238,   228,   268,    36,    84,   271,   272,   268,    16,   161,
     268,   188,   159,   188,   173,   158,    83,   152,   161,   170,
     173,   242,   243,   244,   245,   246,   247,   282,   254,    16,
       8,   159,   170,   161,   160,    84,     8,   158,   160,   282,
     160,    97,   160,   151,   161,   243,   159,   159,   102,   118,
     267,   282,   256,   267,   188,   272,   188,   188,   159,   188,
     170,    83,   168,   252,   240,   267,   157,   161,   161,   161,
     161,   170,   224,   268,   268,   268,   158,   230,   268,   159,
     160,   248,   188,   268,   161
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
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))

/* Error token number */
#define YYTERROR	1
#define YYERRCODE	256


/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */
#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
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
		  Type, Value); \
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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
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
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
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
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
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

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULL, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {
      case 81: /* "integer number (T_LNUMBER)" */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3494 "zend_language_parser.c"
        break;
      case 82: /* "floating-point number (T_DNUMBER)" */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3501 "zend_language_parser.c"
        break;
      case 83: /* "identifier (T_STRING)" */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3508 "zend_language_parser.c"
        break;
      case 84: /* "variable (T_VARIABLE)" */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3515 "zend_language_parser.c"
        break;
      case 85: /* T_INLINE_HTML */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3522 "zend_language_parser.c"
        break;
      case 86: /* "quoted-string and whitespace (T_ENCAPSED_AND_WHITESPACE)" */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3529 "zend_language_parser.c"
        break;
      case 87: /* "quoted-string (T_CONSTANT_ENCAPSED_STRING)" */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3536 "zend_language_parser.c"
        break;
      case 88: /* "variable name (T_STRING_VARNAME)" */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3543 "zend_language_parser.c"
        break;
      case 89: /* "number (T_NUM_STRING)" */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3550 "zend_language_parser.c"
        break;
      case 170: /* identifier */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3557 "zend_language_parser.c"
        break;
      case 171: /* top_statement_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3564 "zend_language_parser.c"
        break;
      case 172: /* namespace_name */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3571 "zend_language_parser.c"
        break;
      case 173: /* name */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3578 "zend_language_parser.c"
        break;
      case 174: /* top_statement */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3585 "zend_language_parser.c"
        break;
      case 178: /* group_use_declaration */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3592 "zend_language_parser.c"
        break;
      case 179: /* mixed_group_use_declaration */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3599 "zend_language_parser.c"
        break;
      case 181: /* inline_use_declarations */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3606 "zend_language_parser.c"
        break;
      case 182: /* unprefixed_use_declarations */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3613 "zend_language_parser.c"
        break;
      case 183: /* use_declarations */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3620 "zend_language_parser.c"
        break;
      case 184: /* inline_use_declaration */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3627 "zend_language_parser.c"
        break;
      case 185: /* unprefixed_use_declaration */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3634 "zend_language_parser.c"
        break;
      case 186: /* use_declaration */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3641 "zend_language_parser.c"
        break;
      case 187: /* const_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3648 "zend_language_parser.c"
        break;
      case 188: /* inner_statement_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3655 "zend_language_parser.c"
        break;
      case 189: /* inner_statement */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3662 "zend_language_parser.c"
        break;
      case 190: /* statement */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3669 "zend_language_parser.c"
        break;
      case 192: /* catch_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3676 "zend_language_parser.c"
        break;
      case 193: /* catch_name_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3683 "zend_language_parser.c"
        break;
      case 194: /* finally_statement */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3690 "zend_language_parser.c"
        break;
      case 195: /* unset_variables */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3697 "zend_language_parser.c"
        break;
      case 196: /* unset_variable */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3704 "zend_language_parser.c"
        break;
      case 197: /* function_declaration_statement */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3711 "zend_language_parser.c"
        break;
      case 200: /* class_declaration_statement */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3718 "zend_language_parser.c"
        break;
      case 205: /* trait_declaration_statement */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3725 "zend_language_parser.c"
        break;
      case 207: /* interface_declaration_statement */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3732 "zend_language_parser.c"
        break;
      case 209: /* extends_from */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3739 "zend_language_parser.c"
        break;
      case 210: /* interface_extends_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3746 "zend_language_parser.c"
        break;
      case 211: /* implements_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3753 "zend_language_parser.c"
        break;
      case 212: /* foreach_variable */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3760 "zend_language_parser.c"
        break;
      case 213: /* for_statement */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3767 "zend_language_parser.c"
        break;
      case 214: /* foreach_statement */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3774 "zend_language_parser.c"
        break;
      case 215: /* declare_statement */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3781 "zend_language_parser.c"
        break;
      case 216: /* switch_case_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3788 "zend_language_parser.c"
        break;
      case 217: /* case_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3795 "zend_language_parser.c"
        break;
      case 219: /* while_statement */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3802 "zend_language_parser.c"
        break;
      case 220: /* if_stmt_without_else */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3809 "zend_language_parser.c"
        break;
      case 221: /* if_stmt */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3816 "zend_language_parser.c"
        break;
      case 222: /* alt_if_stmt_without_else */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3823 "zend_language_parser.c"
        break;
      case 223: /* alt_if_stmt */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3830 "zend_language_parser.c"
        break;
      case 224: /* parameter_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3837 "zend_language_parser.c"
        break;
      case 225: /* non_empty_parameter_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3844 "zend_language_parser.c"
        break;
      case 226: /* parameter */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3851 "zend_language_parser.c"
        break;
      case 227: /* optional_type */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3858 "zend_language_parser.c"
        break;
      case 228: /* type_expr */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3865 "zend_language_parser.c"
        break;
      case 229: /* type */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3872 "zend_language_parser.c"
        break;
      case 230: /* return_type */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3879 "zend_language_parser.c"
        break;
      case 231: /* argument_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3886 "zend_language_parser.c"
        break;
      case 232: /* non_empty_argument_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3893 "zend_language_parser.c"
        break;
      case 233: /* argument */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3900 "zend_language_parser.c"
        break;
      case 234: /* global_var_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3907 "zend_language_parser.c"
        break;
      case 235: /* global_var */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3914 "zend_language_parser.c"
        break;
      case 236: /* static_var_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3921 "zend_language_parser.c"
        break;
      case 237: /* static_var */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3928 "zend_language_parser.c"
        break;
      case 238: /* class_statement_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3935 "zend_language_parser.c"
        break;
      case 239: /* class_statement */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3942 "zend_language_parser.c"
        break;
      case 240: /* name_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3949 "zend_language_parser.c"
        break;
      case 241: /* trait_adaptations */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3956 "zend_language_parser.c"
        break;
      case 242: /* trait_adaptation_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3963 "zend_language_parser.c"
        break;
      case 243: /* trait_adaptation */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3970 "zend_language_parser.c"
        break;
      case 244: /* trait_precedence */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3977 "zend_language_parser.c"
        break;
      case 245: /* trait_alias */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3984 "zend_language_parser.c"
        break;
      case 246: /* trait_method_reference */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3991 "zend_language_parser.c"
        break;
      case 247: /* absolute_trait_method_reference */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 3998 "zend_language_parser.c"
        break;
      case 248: /* method_body */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4005 "zend_language_parser.c"
        break;
      case 253: /* property_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4012 "zend_language_parser.c"
        break;
      case 254: /* property */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4019 "zend_language_parser.c"
        break;
      case 255: /* class_const_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4026 "zend_language_parser.c"
        break;
      case 256: /* class_const_decl */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4033 "zend_language_parser.c"
        break;
      case 257: /* const_decl */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4040 "zend_language_parser.c"
        break;
      case 258: /* echo_expr_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4047 "zend_language_parser.c"
        break;
      case 259: /* echo_expr */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4054 "zend_language_parser.c"
        break;
      case 260: /* for_exprs */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4061 "zend_language_parser.c"
        break;
      case 261: /* non_empty_for_exprs */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4068 "zend_language_parser.c"
        break;
      case 262: /* anonymous_class */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4075 "zend_language_parser.c"
        break;
      case 264: /* new_expr */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4082 "zend_language_parser.c"
        break;
      case 265: /* expr_without_variable */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4089 "zend_language_parser.c"
        break;
      case 267: /* backup_doc_comment */
/* Line 1398 of yacc.c  */
#line 50 "zend_language_parser.y"
        { if (((*yyvaluep).str)) zend_string_release(((*yyvaluep).str)); };
/* Line 1398 of yacc.c  */
#line 4096 "zend_language_parser.c"
        break;
      case 270: /* lexical_vars */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4103 "zend_language_parser.c"
        break;
      case 271: /* lexical_var_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4110 "zend_language_parser.c"
        break;
      case 272: /* lexical_var */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4117 "zend_language_parser.c"
        break;
      case 273: /* function_call */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4124 "zend_language_parser.c"
        break;
      case 274: /* class_name */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4131 "zend_language_parser.c"
        break;
      case 275: /* class_name_reference */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4138 "zend_language_parser.c"
        break;
      case 276: /* exit_expr */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4145 "zend_language_parser.c"
        break;
      case 277: /* backticks_expr */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4152 "zend_language_parser.c"
        break;
      case 278: /* ctor_arguments */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4159 "zend_language_parser.c"
        break;
      case 279: /* dereferencable_scalar */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4166 "zend_language_parser.c"
        break;
      case 280: /* scalar */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4173 "zend_language_parser.c"
        break;
      case 281: /* constant */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4180 "zend_language_parser.c"
        break;
      case 282: /* expr */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4187 "zend_language_parser.c"
        break;
      case 283: /* optional_expr */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4194 "zend_language_parser.c"
        break;
      case 284: /* variable_class_name */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4201 "zend_language_parser.c"
        break;
      case 285: /* dereferencable */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4208 "zend_language_parser.c"
        break;
      case 286: /* callable_expr */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4215 "zend_language_parser.c"
        break;
      case 287: /* callable_variable */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4222 "zend_language_parser.c"
        break;
      case 288: /* variable */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4229 "zend_language_parser.c"
        break;
      case 289: /* simple_variable */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4236 "zend_language_parser.c"
        break;
      case 290: /* static_member */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4243 "zend_language_parser.c"
        break;
      case 291: /* new_variable */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4250 "zend_language_parser.c"
        break;
      case 292: /* member_name */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4257 "zend_language_parser.c"
        break;
      case 293: /* property_name */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4264 "zend_language_parser.c"
        break;
      case 294: /* array_pair_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4271 "zend_language_parser.c"
        break;
      case 295: /* possible_array_pair */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4278 "zend_language_parser.c"
        break;
      case 296: /* non_empty_array_pair_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4285 "zend_language_parser.c"
        break;
      case 297: /* array_pair */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4292 "zend_language_parser.c"
        break;
      case 298: /* encaps_list */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4299 "zend_language_parser.c"
        break;
      case 299: /* encaps_var */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4306 "zend_language_parser.c"
        break;
      case 300: /* encaps_var_offset */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4313 "zend_language_parser.c"
        break;
      case 301: /* internal_functions_in_yacc */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4320 "zend_language_parser.c"
        break;
      case 302: /* isset_variables */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4327 "zend_language_parser.c"
        break;
      case 303: /* isset_variable */
/* Line 1398 of yacc.c  */
#line 49 "zend_language_parser.y"
        { zend_ast_destroy(((*yyvaluep).ast)); };
/* Line 1398 of yacc.c  */
#line 4334 "zend_language_parser.c"
        break;

      default:
        break;
    }
}




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
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
/* The lookahead symbol.  */
int yychar;


#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
static YYSTYPE yyval_default;
# define YY_INITIAL_VALUE(Value) = Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval YY_INITIAL_VALUE(yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
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
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
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

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
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
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
/* Line 1792 of yacc.c  */
#line 263 "zend_language_parser.y"
    { CG(ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 77:
/* Line 1792 of yacc.c  */
#line 282 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 78:
/* Line 1792 of yacc.c  */
#line 283 "zend_language_parser.y"
    {
			zval zv;
			zend_lex_tstring(&zv);
			(yyval.ast) = zend_ast_create_zval(&zv);
		}
    break;

  case 79:
/* Line 1792 of yacc.c  */
#line 291 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (2)].ast), (yyvsp[(2) - (2)].ast)); }
    break;

  case 80:
/* Line 1792 of yacc.c  */
#line 292 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }
    break;

  case 81:
/* Line 1792 of yacc.c  */
#line 296 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 82:
/* Line 1792 of yacc.c  */
#line 297 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_append_str((yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 83:
/* Line 1792 of yacc.c  */
#line 301 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); (yyval.ast)->attr = ZEND_NAME_NOT_FQ; }
    break;

  case 84:
/* Line 1792 of yacc.c  */
#line 302 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(3) - (3)].ast); (yyval.ast)->attr = ZEND_NAME_RELATIVE; }
    break;

  case 85:
/* Line 1792 of yacc.c  */
#line 303 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (2)].ast); (yyval.ast)->attr = ZEND_NAME_FQ; }
    break;

  case 86:
/* Line 1792 of yacc.c  */
#line 307 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 87:
/* Line 1792 of yacc.c  */
#line 308 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 88:
/* Line 1792 of yacc.c  */
#line 309 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 89:
/* Line 1792 of yacc.c  */
#line 310 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 90:
/* Line 1792 of yacc.c  */
#line 311 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 91:
/* Line 1792 of yacc.c  */
#line 313 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_HALT_COMPILER,
			      zend_ast_create_zval_from_long(zend_get_scanned_file_offset()));
			  zend_stop_lexing(); }
    break;

  case 92:
/* Line 1792 of yacc.c  */
#line 317 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, (yyvsp[(2) - (3)].ast), NULL);
			  RESET_DOC_COMMENT(); }
    break;

  case 93:
/* Line 1792 of yacc.c  */
#line 319 "zend_language_parser.y"
    { RESET_DOC_COMMENT(); }
    break;

  case 94:
/* Line 1792 of yacc.c  */
#line 321 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, (yyvsp[(2) - (6)].ast), (yyvsp[(5) - (6)].ast)); }
    break;

  case 95:
/* Line 1792 of yacc.c  */
#line 322 "zend_language_parser.y"
    { RESET_DOC_COMMENT(); }
    break;

  case 96:
/* Line 1792 of yacc.c  */
#line 324 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_NAMESPACE, NULL, (yyvsp[(4) - (5)].ast)); }
    break;

  case 97:
/* Line 1792 of yacc.c  */
#line 325 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); }
    break;

  case 98:
/* Line 1792 of yacc.c  */
#line 326 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(3) - (4)].ast); (yyval.ast)->attr = (yyvsp[(2) - (4)].num); }
    break;

  case 99:
/* Line 1792 of yacc.c  */
#line 327 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); (yyval.ast)->attr = ZEND_SYMBOL_CLASS; }
    break;

  case 100:
/* Line 1792 of yacc.c  */
#line 328 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(3) - (4)].ast); (yyval.ast)->attr = (yyvsp[(2) - (4)].num); }
    break;

  case 101:
/* Line 1792 of yacc.c  */
#line 329 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); }
    break;

  case 102:
/* Line 1792 of yacc.c  */
#line 333 "zend_language_parser.y"
    { (yyval.num) = ZEND_SYMBOL_FUNCTION; }
    break;

  case 103:
/* Line 1792 of yacc.c  */
#line 334 "zend_language_parser.y"
    { (yyval.num) = ZEND_SYMBOL_CONST; }
    break;

  case 104:
/* Line 1792 of yacc.c  */
#line 339 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_GROUP_USE, (yyvsp[(1) - (6)].ast), (yyvsp[(4) - (6)].ast)); }
    break;

  case 105:
/* Line 1792 of yacc.c  */
#line 341 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_GROUP_USE, (yyvsp[(2) - (7)].ast), (yyvsp[(5) - (7)].ast)); }
    break;

  case 106:
/* Line 1792 of yacc.c  */
#line 346 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_GROUP_USE, (yyvsp[(1) - (6)].ast), (yyvsp[(4) - (6)].ast));}
    break;

  case 107:
/* Line 1792 of yacc.c  */
#line 348 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_GROUP_USE, (yyvsp[(2) - (7)].ast), (yyvsp[(5) - (7)].ast)); }
    break;

  case 110:
/* Line 1792 of yacc.c  */
#line 358 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 111:
/* Line 1792 of yacc.c  */
#line 360 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[(1) - (1)].ast)); }
    break;

  case 112:
/* Line 1792 of yacc.c  */
#line 365 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 113:
/* Line 1792 of yacc.c  */
#line 367 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[(1) - (1)].ast)); }
    break;

  case 114:
/* Line 1792 of yacc.c  */
#line 372 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 115:
/* Line 1792 of yacc.c  */
#line 374 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_USE, (yyvsp[(1) - (1)].ast)); }
    break;

  case 116:
/* Line 1792 of yacc.c  */
#line 378 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); (yyval.ast)->attr = ZEND_SYMBOL_CLASS; }
    break;

  case 117:
/* Line 1792 of yacc.c  */
#line 379 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (2)].ast); (yyval.ast)->attr = (yyvsp[(1) - (2)].num); }
    break;

  case 118:
/* Line 1792 of yacc.c  */
#line 384 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[(1) - (1)].ast), NULL); }
    break;

  case 119:
/* Line 1792 of yacc.c  */
#line 386 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_USE_ELEM, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 120:
/* Line 1792 of yacc.c  */
#line 390 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 121:
/* Line 1792 of yacc.c  */
#line 391 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (2)].ast); }
    break;

  case 122:
/* Line 1792 of yacc.c  */
#line 395 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 123:
/* Line 1792 of yacc.c  */
#line 396 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CONST_DECL, (yyvsp[(1) - (1)].ast)); }
    break;

  case 124:
/* Line 1792 of yacc.c  */
#line 401 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (2)].ast), (yyvsp[(2) - (2)].ast)); }
    break;

  case 125:
/* Line 1792 of yacc.c  */
#line 403 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }
    break;

  case 126:
/* Line 1792 of yacc.c  */
#line 408 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 127:
/* Line 1792 of yacc.c  */
#line 409 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 128:
/* Line 1792 of yacc.c  */
#line 410 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 129:
/* Line 1792 of yacc.c  */
#line 411 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 130:
/* Line 1792 of yacc.c  */
#line 412 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 131:
/* Line 1792 of yacc.c  */
#line 414 "zend_language_parser.y"
    { (yyval.ast) = NULL; zend_error_noreturn(E_COMPILE_ERROR,
			      "__HALT_COMPILER() can only be used from the outermost scope"); }
    break;

  case 132:
/* Line 1792 of yacc.c  */
#line 420 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); }
    break;

  case 133:
/* Line 1792 of yacc.c  */
#line 421 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 134:
/* Line 1792 of yacc.c  */
#line 422 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 135:
/* Line 1792 of yacc.c  */
#line 424 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_WHILE, (yyvsp[(3) - (5)].ast), (yyvsp[(5) - (5)].ast)); }
    break;

  case 136:
/* Line 1792 of yacc.c  */
#line 426 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_DO_WHILE, (yyvsp[(2) - (7)].ast), (yyvsp[(5) - (7)].ast)); }
    break;

  case 137:
/* Line 1792 of yacc.c  */
#line 428 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_FOR, (yyvsp[(3) - (9)].ast), (yyvsp[(5) - (9)].ast), (yyvsp[(7) - (9)].ast), (yyvsp[(9) - (9)].ast)); }
    break;

  case 138:
/* Line 1792 of yacc.c  */
#line 430 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_SWITCH, (yyvsp[(3) - (5)].ast), (yyvsp[(5) - (5)].ast)); }
    break;

  case 139:
/* Line 1792 of yacc.c  */
#line 431 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_BREAK, (yyvsp[(2) - (3)].ast)); }
    break;

  case 140:
/* Line 1792 of yacc.c  */
#line 432 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_CONTINUE, (yyvsp[(2) - (3)].ast)); }
    break;

  case 141:
/* Line 1792 of yacc.c  */
#line 433 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_RETURN, (yyvsp[(2) - (3)].ast)); }
    break;

  case 142:
/* Line 1792 of yacc.c  */
#line 434 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); }
    break;

  case 143:
/* Line 1792 of yacc.c  */
#line 435 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); }
    break;

  case 144:
/* Line 1792 of yacc.c  */
#line 436 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); }
    break;

  case 145:
/* Line 1792 of yacc.c  */
#line 437 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_ECHO, (yyvsp[(1) - (1)].ast)); }
    break;

  case 146:
/* Line 1792 of yacc.c  */
#line 438 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (2)].ast); }
    break;

  case 147:
/* Line 1792 of yacc.c  */
#line 439 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(3) - (5)].ast); }
    break;

  case 148:
/* Line 1792 of yacc.c  */
#line 441 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_FOREACH, (yyvsp[(3) - (7)].ast), (yyvsp[(5) - (7)].ast), NULL, (yyvsp[(7) - (7)].ast)); }
    break;

  case 149:
/* Line 1792 of yacc.c  */
#line 444 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_FOREACH, (yyvsp[(3) - (9)].ast), (yyvsp[(7) - (9)].ast), (yyvsp[(5) - (9)].ast), (yyvsp[(9) - (9)].ast)); }
    break;

  case 150:
/* Line 1792 of yacc.c  */
#line 446 "zend_language_parser.y"
    { zend_handle_encoding_declaration((yyvsp[(3) - (4)].ast)); }
    break;

  case 151:
/* Line 1792 of yacc.c  */
#line 448 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_DECLARE, (yyvsp[(3) - (6)].ast), (yyvsp[(6) - (6)].ast)); }
    break;

  case 152:
/* Line 1792 of yacc.c  */
#line 449 "zend_language_parser.y"
    { (yyval.ast) = NULL; }
    break;

  case 153:
/* Line 1792 of yacc.c  */
#line 451 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_TRY, (yyvsp[(3) - (6)].ast), (yyvsp[(5) - (6)].ast), (yyvsp[(6) - (6)].ast)); }
    break;

  case 154:
/* Line 1792 of yacc.c  */
#line 452 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_THROW, (yyvsp[(2) - (3)].ast)); }
    break;

  case 155:
/* Line 1792 of yacc.c  */
#line 453 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_GOTO, (yyvsp[(2) - (3)].ast)); }
    break;

  case 156:
/* Line 1792 of yacc.c  */
#line 454 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_LABEL, (yyvsp[(1) - (2)].ast)); }
    break;

  case 157:
/* Line 1792 of yacc.c  */
#line 459 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_CATCH_LIST); }
    break;

  case 158:
/* Line 1792 of yacc.c  */
#line 461 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (9)].ast), zend_ast_create(ZEND_AST_CATCH, (yyvsp[(4) - (9)].ast), (yyvsp[(5) - (9)].ast), (yyvsp[(8) - (9)].ast))); }
    break;

  case 159:
/* Line 1792 of yacc.c  */
#line 465 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_NAME_LIST, (yyvsp[(1) - (1)].ast)); }
    break;

  case 160:
/* Line 1792 of yacc.c  */
#line 466 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 161:
/* Line 1792 of yacc.c  */
#line 470 "zend_language_parser.y"
    { (yyval.ast) = NULL; }
    break;

  case 162:
/* Line 1792 of yacc.c  */
#line 471 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(3) - (4)].ast); }
    break;

  case 163:
/* Line 1792 of yacc.c  */
#line 475 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[(1) - (1)].ast)); }
    break;

  case 164:
/* Line 1792 of yacc.c  */
#line 476 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 165:
/* Line 1792 of yacc.c  */
#line 480 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_UNSET, (yyvsp[(1) - (1)].ast)); }
    break;

  case 166:
/* Line 1792 of yacc.c  */
#line 486 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_FUNC_DECL, (yyvsp[(2) - (13)].num) | (yyvsp[(13) - (13)].num), (yyvsp[(1) - (13)].num), (yyvsp[(4) - (13)].str),
		      zend_ast_get_str((yyvsp[(3) - (13)].ast)), (yyvsp[(6) - (13)].ast), NULL, (yyvsp[(11) - (13)].ast), (yyvsp[(8) - (13)].ast)); CG(extra_fn_flags) = (yyvsp[(9) - (13)].num); }
    break;

  case 167:
/* Line 1792 of yacc.c  */
#line 491 "zend_language_parser.y"
    { (yyval.num) = 0; }
    break;

  case 168:
/* Line 1792 of yacc.c  */
#line 492 "zend_language_parser.y"
    { (yyval.num) = ZEND_PARAM_REF; }
    break;

  case 169:
/* Line 1792 of yacc.c  */
#line 496 "zend_language_parser.y"
    { (yyval.num) = 0; }
    break;

  case 170:
/* Line 1792 of yacc.c  */
#line 497 "zend_language_parser.y"
    { (yyval.num) = ZEND_PARAM_VARIADIC; }
    break;

  case 171:
/* Line 1792 of yacc.c  */
#line 501 "zend_language_parser.y"
    { (yyval.num) = CG(zend_lineno); }
    break;

  case 172:
/* Line 1792 of yacc.c  */
#line 503 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, (yyvsp[(1) - (10)].num), (yyvsp[(3) - (10)].num), (yyvsp[(7) - (10)].str), zend_ast_get_str((yyvsp[(4) - (10)].ast)), (yyvsp[(5) - (10)].ast), (yyvsp[(6) - (10)].ast), (yyvsp[(9) - (10)].ast), NULL); }
    break;

  case 173:
/* Line 1792 of yacc.c  */
#line 504 "zend_language_parser.y"
    { (yyval.num) = CG(zend_lineno); }
    break;

  case 174:
/* Line 1792 of yacc.c  */
#line 506 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, 0, (yyvsp[(2) - (9)].num), (yyvsp[(6) - (9)].str), zend_ast_get_str((yyvsp[(3) - (9)].ast)), (yyvsp[(4) - (9)].ast), (yyvsp[(5) - (9)].ast), (yyvsp[(8) - (9)].ast), NULL); }
    break;

  case 175:
/* Line 1792 of yacc.c  */
#line 510 "zend_language_parser.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 176:
/* Line 1792 of yacc.c  */
#line 511 "zend_language_parser.y"
    { (yyval.num) = zend_add_class_modifier((yyvsp[(1) - (2)].num), (yyvsp[(2) - (2)].num)); }
    break;

  case 177:
/* Line 1792 of yacc.c  */
#line 515 "zend_language_parser.y"
    { (yyval.num) = ZEND_ACC_EXPLICIT_ABSTRACT_CLASS; }
    break;

  case 178:
/* Line 1792 of yacc.c  */
#line 516 "zend_language_parser.y"
    { (yyval.num) = ZEND_ACC_FINAL; }
    break;

  case 179:
/* Line 1792 of yacc.c  */
#line 520 "zend_language_parser.y"
    { (yyval.num) = CG(zend_lineno); }
    break;

  case 180:
/* Line 1792 of yacc.c  */
#line 522 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_TRAIT, (yyvsp[(2) - (7)].num), (yyvsp[(4) - (7)].str), zend_ast_get_str((yyvsp[(3) - (7)].ast)), NULL, NULL, (yyvsp[(6) - (7)].ast), NULL); }
    break;

  case 181:
/* Line 1792 of yacc.c  */
#line 526 "zend_language_parser.y"
    { (yyval.num) = CG(zend_lineno); }
    break;

  case 182:
/* Line 1792 of yacc.c  */
#line 528 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLASS, ZEND_ACC_INTERFACE, (yyvsp[(2) - (8)].num), (yyvsp[(5) - (8)].str), zend_ast_get_str((yyvsp[(3) - (8)].ast)), NULL, (yyvsp[(4) - (8)].ast), (yyvsp[(7) - (8)].ast), NULL); }
    break;

  case 183:
/* Line 1792 of yacc.c  */
#line 532 "zend_language_parser.y"
    { (yyval.ast) = NULL; }
    break;

  case 184:
/* Line 1792 of yacc.c  */
#line 533 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (2)].ast); }
    break;

  case 185:
/* Line 1792 of yacc.c  */
#line 537 "zend_language_parser.y"
    { (yyval.ast) = NULL; }
    break;

  case 186:
/* Line 1792 of yacc.c  */
#line 538 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (2)].ast); }
    break;

  case 187:
/* Line 1792 of yacc.c  */
#line 542 "zend_language_parser.y"
    { (yyval.ast) = NULL; }
    break;

  case 188:
/* Line 1792 of yacc.c  */
#line 543 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (2)].ast); }
    break;

  case 189:
/* Line 1792 of yacc.c  */
#line 547 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 190:
/* Line 1792 of yacc.c  */
#line 548 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_REF, (yyvsp[(2) - (2)].ast)); }
    break;

  case 191:
/* Line 1792 of yacc.c  */
#line 549 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(3) - (4)].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_LIST; }
    break;

  case 192:
/* Line 1792 of yacc.c  */
#line 550 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; }
    break;

  case 193:
/* Line 1792 of yacc.c  */
#line 554 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 194:
/* Line 1792 of yacc.c  */
#line 555 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (4)].ast); }
    break;

  case 195:
/* Line 1792 of yacc.c  */
#line 559 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 196:
/* Line 1792 of yacc.c  */
#line 560 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (4)].ast); }
    break;

  case 197:
/* Line 1792 of yacc.c  */
#line 564 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 198:
/* Line 1792 of yacc.c  */
#line 565 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (4)].ast); }
    break;

  case 199:
/* Line 1792 of yacc.c  */
#line 569 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); }
    break;

  case 200:
/* Line 1792 of yacc.c  */
#line 570 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(3) - (4)].ast); }
    break;

  case 201:
/* Line 1792 of yacc.c  */
#line 571 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (4)].ast); }
    break;

  case 202:
/* Line 1792 of yacc.c  */
#line 572 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(3) - (5)].ast); }
    break;

  case 203:
/* Line 1792 of yacc.c  */
#line 576 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_SWITCH_LIST); }
    break;

  case 204:
/* Line 1792 of yacc.c  */
#line 578 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (5)].ast), zend_ast_create(ZEND_AST_SWITCH_CASE, (yyvsp[(3) - (5)].ast), (yyvsp[(5) - (5)].ast))); }
    break;

  case 205:
/* Line 1792 of yacc.c  */
#line 580 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (4)].ast), zend_ast_create(ZEND_AST_SWITCH_CASE, NULL, (yyvsp[(4) - (4)].ast))); }
    break;

  case 208:
/* Line 1792 of yacc.c  */
#line 590 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 209:
/* Line 1792 of yacc.c  */
#line 591 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (4)].ast); }
    break;

  case 210:
/* Line 1792 of yacc.c  */
#line 597 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_IF,
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[(3) - (5)].ast), (yyvsp[(5) - (5)].ast))); }
    break;

  case 211:
/* Line 1792 of yacc.c  */
#line 600 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (6)].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[(4) - (6)].ast), (yyvsp[(6) - (6)].ast))); }
    break;

  case 212:
/* Line 1792 of yacc.c  */
#line 605 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 213:
/* Line 1792 of yacc.c  */
#line 607 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (3)].ast), zend_ast_create(ZEND_AST_IF_ELEM, NULL, (yyvsp[(3) - (3)].ast))); }
    break;

  case 214:
/* Line 1792 of yacc.c  */
#line 612 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_IF,
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[(3) - (6)].ast), (yyvsp[(6) - (6)].ast))); }
    break;

  case 215:
/* Line 1792 of yacc.c  */
#line 615 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (7)].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, (yyvsp[(4) - (7)].ast), (yyvsp[(7) - (7)].ast))); }
    break;

  case 216:
/* Line 1792 of yacc.c  */
#line 620 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (3)].ast); }
    break;

  case 217:
/* Line 1792 of yacc.c  */
#line 622 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (6)].ast),
			      zend_ast_create(ZEND_AST_IF_ELEM, NULL, (yyvsp[(4) - (6)].ast))); }
    break;

  case 218:
/* Line 1792 of yacc.c  */
#line 627 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 219:
/* Line 1792 of yacc.c  */
#line 628 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_PARAM_LIST); }
    break;

  case 220:
/* Line 1792 of yacc.c  */
#line 634 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_PARAM_LIST, (yyvsp[(1) - (1)].ast)); }
    break;

  case 221:
/* Line 1792 of yacc.c  */
#line 636 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 222:
/* Line 1792 of yacc.c  */
#line 641 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_PARAM, (yyvsp[(2) - (4)].num) | (yyvsp[(3) - (4)].num), (yyvsp[(1) - (4)].ast), (yyvsp[(4) - (4)].ast), NULL); }
    break;

  case 223:
/* Line 1792 of yacc.c  */
#line 643 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_PARAM, (yyvsp[(2) - (6)].num) | (yyvsp[(3) - (6)].num), (yyvsp[(1) - (6)].ast), (yyvsp[(4) - (6)].ast), (yyvsp[(6) - (6)].ast)); }
    break;

  case 224:
/* Line 1792 of yacc.c  */
#line 648 "zend_language_parser.y"
    { (yyval.ast) = NULL; }
    break;

  case 225:
/* Line 1792 of yacc.c  */
#line 649 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 226:
/* Line 1792 of yacc.c  */
#line 653 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 227:
/* Line 1792 of yacc.c  */
#line 654 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (2)].ast); (yyval.ast)->attr |= ZEND_TYPE_NULLABLE; }
    break;

  case 228:
/* Line 1792 of yacc.c  */
#line 658 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_ARRAY); }
    break;

  case 229:
/* Line 1792 of yacc.c  */
#line 659 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TYPE, IS_CALLABLE); }
    break;

  case 230:
/* Line 1792 of yacc.c  */
#line 660 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 231:
/* Line 1792 of yacc.c  */
#line 664 "zend_language_parser.y"
    { (yyval.ast) = NULL; }
    break;

  case 232:
/* Line 1792 of yacc.c  */
#line 665 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (2)].ast); }
    break;

  case 233:
/* Line 1792 of yacc.c  */
#line 669 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_ARG_LIST); }
    break;

  case 234:
/* Line 1792 of yacc.c  */
#line 670 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); }
    break;

  case 235:
/* Line 1792 of yacc.c  */
#line 675 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ARG_LIST, (yyvsp[(1) - (1)].ast)); }
    break;

  case 236:
/* Line 1792 of yacc.c  */
#line 677 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 237:
/* Line 1792 of yacc.c  */
#line 681 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 238:
/* Line 1792 of yacc.c  */
#line 682 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_UNPACK, (yyvsp[(2) - (2)].ast)); }
    break;

  case 239:
/* Line 1792 of yacc.c  */
#line 686 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 240:
/* Line 1792 of yacc.c  */
#line 687 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[(1) - (1)].ast)); }
    break;

  case 241:
/* Line 1792 of yacc.c  */
#line 692 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_GLOBAL, zend_ast_create(ZEND_AST_VAR, (yyvsp[(1) - (1)].ast))); }
    break;

  case 242:
/* Line 1792 of yacc.c  */
#line 697 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 243:
/* Line 1792 of yacc.c  */
#line 698 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[(1) - (1)].ast)); }
    break;

  case 244:
/* Line 1792 of yacc.c  */
#line 702 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC, (yyvsp[(1) - (1)].ast), NULL); }
    break;

  case 245:
/* Line 1792 of yacc.c  */
#line 703 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 246:
/* Line 1792 of yacc.c  */
#line 709 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (2)].ast), (yyvsp[(2) - (2)].ast)); }
    break;

  case 247:
/* Line 1792 of yacc.c  */
#line 711 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_STMT_LIST); }
    break;

  case 248:
/* Line 1792 of yacc.c  */
#line 717 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); (yyval.ast)->attr = (yyvsp[(1) - (3)].num); }
    break;

  case 249:
/* Line 1792 of yacc.c  */
#line 719 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(3) - (4)].ast); (yyval.ast)->attr = (yyvsp[(1) - (4)].num); }
    break;

  case 250:
/* Line 1792 of yacc.c  */
#line 721 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_USE_TRAIT, (yyvsp[(2) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 251:
/* Line 1792 of yacc.c  */
#line 724 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_METHOD, (yyvsp[(3) - (12)].num) | (yyvsp[(1) - (12)].num) | (yyvsp[(12) - (12)].num), (yyvsp[(2) - (12)].num), (yyvsp[(5) - (12)].str),
				  zend_ast_get_str((yyvsp[(4) - (12)].ast)), (yyvsp[(7) - (12)].ast), NULL, (yyvsp[(11) - (12)].ast), (yyvsp[(9) - (12)].ast)); CG(extra_fn_flags) = (yyvsp[(10) - (12)].num); }
    break;

  case 252:
/* Line 1792 of yacc.c  */
#line 729 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_NAME_LIST, (yyvsp[(1) - (1)].ast)); }
    break;

  case 253:
/* Line 1792 of yacc.c  */
#line 730 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 254:
/* Line 1792 of yacc.c  */
#line 734 "zend_language_parser.y"
    { (yyval.ast) = NULL; }
    break;

  case 255:
/* Line 1792 of yacc.c  */
#line 735 "zend_language_parser.y"
    { (yyval.ast) = NULL; }
    break;

  case 256:
/* Line 1792 of yacc.c  */
#line 736 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); }
    break;

  case 257:
/* Line 1792 of yacc.c  */
#line 741 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_TRAIT_ADAPTATIONS, (yyvsp[(1) - (1)].ast)); }
    break;

  case 258:
/* Line 1792 of yacc.c  */
#line 743 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (2)].ast), (yyvsp[(2) - (2)].ast)); }
    break;

  case 259:
/* Line 1792 of yacc.c  */
#line 747 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (2)].ast); }
    break;

  case 260:
/* Line 1792 of yacc.c  */
#line 748 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (2)].ast); }
    break;

  case 261:
/* Line 1792 of yacc.c  */
#line 753 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_TRAIT_PRECEDENCE, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 262:
/* Line 1792 of yacc.c  */
#line 758 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TRAIT_ALIAS, 0, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 263:
/* Line 1792 of yacc.c  */
#line 760 "zend_language_parser.y"
    { zval zv; zend_lex_tstring(&zv); (yyval.ast) = zend_ast_create_ex(ZEND_AST_TRAIT_ALIAS, 0, (yyvsp[(1) - (3)].ast), zend_ast_create_zval(&zv)); }
    break;

  case 264:
/* Line 1792 of yacc.c  */
#line 762 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TRAIT_ALIAS, (yyvsp[(3) - (4)].num), (yyvsp[(1) - (4)].ast), (yyvsp[(4) - (4)].ast)); }
    break;

  case 265:
/* Line 1792 of yacc.c  */
#line 764 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_TRAIT_ALIAS, (yyvsp[(3) - (3)].num), (yyvsp[(1) - (3)].ast), NULL); }
    break;

  case 266:
/* Line 1792 of yacc.c  */
#line 769 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_REFERENCE, NULL, (yyvsp[(1) - (1)].ast)); }
    break;

  case 267:
/* Line 1792 of yacc.c  */
#line 770 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 268:
/* Line 1792 of yacc.c  */
#line 775 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_REFERENCE, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 269:
/* Line 1792 of yacc.c  */
#line 779 "zend_language_parser.y"
    { (yyval.ast) = NULL; }
    break;

  case 270:
/* Line 1792 of yacc.c  */
#line 780 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); }
    break;

  case 271:
/* Line 1792 of yacc.c  */
#line 784 "zend_language_parser.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 272:
/* Line 1792 of yacc.c  */
#line 785 "zend_language_parser.y"
    { (yyval.num) = ZEND_ACC_PUBLIC; }
    break;

  case 273:
/* Line 1792 of yacc.c  */
#line 789 "zend_language_parser.y"
    { (yyval.num) = ZEND_ACC_PUBLIC; }
    break;

  case 274:
/* Line 1792 of yacc.c  */
#line 791 "zend_language_parser.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); if (!((yyval.num) & ZEND_ACC_PPP_MASK)) { (yyval.num) |= ZEND_ACC_PUBLIC; } }
    break;

  case 275:
/* Line 1792 of yacc.c  */
#line 795 "zend_language_parser.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 276:
/* Line 1792 of yacc.c  */
#line 797 "zend_language_parser.y"
    { (yyval.num) = zend_add_member_modifier((yyvsp[(1) - (2)].num), (yyvsp[(2) - (2)].num)); }
    break;

  case 277:
/* Line 1792 of yacc.c  */
#line 801 "zend_language_parser.y"
    { (yyval.num) = ZEND_ACC_PUBLIC; }
    break;

  case 278:
/* Line 1792 of yacc.c  */
#line 802 "zend_language_parser.y"
    { (yyval.num) = ZEND_ACC_PROTECTED; }
    break;

  case 279:
/* Line 1792 of yacc.c  */
#line 803 "zend_language_parser.y"
    { (yyval.num) = ZEND_ACC_PRIVATE; }
    break;

  case 280:
/* Line 1792 of yacc.c  */
#line 804 "zend_language_parser.y"
    { (yyval.num) = ZEND_ACC_STATIC; }
    break;

  case 281:
/* Line 1792 of yacc.c  */
#line 805 "zend_language_parser.y"
    { (yyval.num) = ZEND_ACC_ABSTRACT; }
    break;

  case 282:
/* Line 1792 of yacc.c  */
#line 806 "zend_language_parser.y"
    { (yyval.num) = ZEND_ACC_FINAL; }
    break;

  case 283:
/* Line 1792 of yacc.c  */
#line 810 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 284:
/* Line 1792 of yacc.c  */
#line 811 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_PROP_DECL, (yyvsp[(1) - (1)].ast)); }
    break;

  case 285:
/* Line 1792 of yacc.c  */
#line 816 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[(1) - (2)].ast), NULL, ((yyvsp[(2) - (2)].str) ? zend_ast_create_zval_from_str((yyvsp[(2) - (2)].str)) : NULL)); }
    break;

  case 286:
/* Line 1792 of yacc.c  */
#line 818 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP_ELEM, (yyvsp[(1) - (4)].ast), (yyvsp[(3) - (4)].ast), ((yyvsp[(4) - (4)].str) ? zend_ast_create_zval_from_str((yyvsp[(4) - (4)].str)) : NULL)); }
    break;

  case 287:
/* Line 1792 of yacc.c  */
#line 822 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 288:
/* Line 1792 of yacc.c  */
#line 823 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CLASS_CONST_DECL, (yyvsp[(1) - (1)].ast)); }
    break;

  case 289:
/* Line 1792 of yacc.c  */
#line 827 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, (yyvsp[(1) - (4)].ast), (yyvsp[(3) - (4)].ast), ((yyvsp[(4) - (4)].str) ? zend_ast_create_zval_from_str((yyvsp[(4) - (4)].str)) : NULL)); }
    break;

  case 290:
/* Line 1792 of yacc.c  */
#line 831 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_CONST_ELEM, (yyvsp[(1) - (4)].ast), (yyvsp[(3) - (4)].ast), ((yyvsp[(4) - (4)].str) ? zend_ast_create_zval_from_str((yyvsp[(4) - (4)].str)) : NULL)); }
    break;

  case 291:
/* Line 1792 of yacc.c  */
#line 835 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 292:
/* Line 1792 of yacc.c  */
#line 836 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_STMT_LIST, (yyvsp[(1) - (1)].ast)); }
    break;

  case 293:
/* Line 1792 of yacc.c  */
#line 839 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_ECHO, (yyvsp[(1) - (1)].ast)); }
    break;

  case 294:
/* Line 1792 of yacc.c  */
#line 843 "zend_language_parser.y"
    { (yyval.ast) = NULL; }
    break;

  case 295:
/* Line 1792 of yacc.c  */
#line 844 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 296:
/* Line 1792 of yacc.c  */
#line 848 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 297:
/* Line 1792 of yacc.c  */
#line 849 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_EXPR_LIST, (yyvsp[(1) - (1)].ast)); }
    break;

  case 298:
/* Line 1792 of yacc.c  */
#line 853 "zend_language_parser.y"
    { (yyval.num) = CG(zend_lineno); }
    break;

  case 299:
/* Line 1792 of yacc.c  */
#line 854 "zend_language_parser.y"
    {
			zend_ast *decl = zend_ast_create_decl(
				ZEND_AST_CLASS, ZEND_ACC_ANON_CLASS, (yyvsp[(2) - (9)].num), (yyvsp[(6) - (9)].str), NULL,
				(yyvsp[(4) - (9)].ast), (yyvsp[(5) - (9)].ast), (yyvsp[(8) - (9)].ast), NULL);
			(yyval.ast) = zend_ast_create(ZEND_AST_NEW, decl, (yyvsp[(3) - (9)].ast));
		}
    break;

  case 300:
/* Line 1792 of yacc.c  */
#line 864 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_NEW, (yyvsp[(2) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 301:
/* Line 1792 of yacc.c  */
#line 866 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (2)].ast); }
    break;

  case 302:
/* Line 1792 of yacc.c  */
#line 871 "zend_language_parser.y"
    { (yyvsp[(3) - (6)].ast)->attr = ZEND_ARRAY_SYNTAX_LIST; (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[(3) - (6)].ast), (yyvsp[(6) - (6)].ast)); }
    break;

  case 303:
/* Line 1792 of yacc.c  */
#line 873 "zend_language_parser.y"
    { (yyvsp[(2) - (5)].ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[(2) - (5)].ast), (yyvsp[(5) - (5)].ast)); }
    break;

  case 304:
/* Line 1792 of yacc.c  */
#line 875 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 305:
/* Line 1792 of yacc.c  */
#line 877 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_ASSIGN_REF, (yyvsp[(1) - (4)].ast), (yyvsp[(4) - (4)].ast)); }
    break;

  case 306:
/* Line 1792 of yacc.c  */
#line 878 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_CLONE, (yyvsp[(2) - (2)].ast)); }
    break;

  case 307:
/* Line 1792 of yacc.c  */
#line 880 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_assign_op(ZEND_ASSIGN_ADD, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 308:
/* Line 1792 of yacc.c  */
#line 882 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_assign_op(ZEND_ASSIGN_SUB, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 309:
/* Line 1792 of yacc.c  */
#line 884 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_assign_op(ZEND_ASSIGN_MUL, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 310:
/* Line 1792 of yacc.c  */
#line 886 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_assign_op(ZEND_ASSIGN_POW, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 311:
/* Line 1792 of yacc.c  */
#line 888 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_assign_op(ZEND_ASSIGN_DIV, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 312:
/* Line 1792 of yacc.c  */
#line 890 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_assign_op(ZEND_ASSIGN_CONCAT, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 313:
/* Line 1792 of yacc.c  */
#line 892 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_assign_op(ZEND_ASSIGN_MOD, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 314:
/* Line 1792 of yacc.c  */
#line 894 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_assign_op(ZEND_ASSIGN_BW_AND, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 315:
/* Line 1792 of yacc.c  */
#line 896 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_assign_op(ZEND_ASSIGN_BW_OR, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 316:
/* Line 1792 of yacc.c  */
#line 898 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_assign_op(ZEND_ASSIGN_BW_XOR, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 317:
/* Line 1792 of yacc.c  */
#line 900 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_assign_op(ZEND_ASSIGN_SL, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 318:
/* Line 1792 of yacc.c  */
#line 902 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_assign_op(ZEND_ASSIGN_SR, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 319:
/* Line 1792 of yacc.c  */
#line 903 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_POST_INC, (yyvsp[(1) - (2)].ast)); }
    break;

  case 320:
/* Line 1792 of yacc.c  */
#line 904 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_PRE_INC, (yyvsp[(2) - (2)].ast)); }
    break;

  case 321:
/* Line 1792 of yacc.c  */
#line 905 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_POST_DEC, (yyvsp[(1) - (2)].ast)); }
    break;

  case 322:
/* Line 1792 of yacc.c  */
#line 906 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_PRE_DEC, (yyvsp[(2) - (2)].ast)); }
    break;

  case 323:
/* Line 1792 of yacc.c  */
#line 908 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_OR, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 324:
/* Line 1792 of yacc.c  */
#line 910 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 325:
/* Line 1792 of yacc.c  */
#line 912 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_OR, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 326:
/* Line 1792 of yacc.c  */
#line 914 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 327:
/* Line 1792 of yacc.c  */
#line 916 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_binary_op(ZEND_BOOL_XOR, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 328:
/* Line 1792 of yacc.c  */
#line 917 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_OR, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 329:
/* Line 1792 of yacc.c  */
#line 918 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_AND, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 330:
/* Line 1792 of yacc.c  */
#line 919 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_binary_op(ZEND_BW_XOR, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 331:
/* Line 1792 of yacc.c  */
#line 920 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_binary_op(ZEND_CONCAT, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 332:
/* Line 1792 of yacc.c  */
#line 921 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_binary_op(ZEND_ADD, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 333:
/* Line 1792 of yacc.c  */
#line 922 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_binary_op(ZEND_SUB, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 334:
/* Line 1792 of yacc.c  */
#line 923 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_binary_op(ZEND_MUL, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 335:
/* Line 1792 of yacc.c  */
#line 924 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_binary_op(ZEND_POW, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 336:
/* Line 1792 of yacc.c  */
#line 925 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_binary_op(ZEND_DIV, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 337:
/* Line 1792 of yacc.c  */
#line 926 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_binary_op(ZEND_MOD, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 338:
/* Line 1792 of yacc.c  */
#line 927 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_binary_op(ZEND_SL, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 339:
/* Line 1792 of yacc.c  */
#line 928 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_binary_op(ZEND_SR, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 340:
/* Line 1792 of yacc.c  */
#line 929 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_UNARY_PLUS, (yyvsp[(2) - (2)].ast)); }
    break;

  case 341:
/* Line 1792 of yacc.c  */
#line 930 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_UNARY_MINUS, (yyvsp[(2) - (2)].ast)); }
    break;

  case 342:
/* Line 1792 of yacc.c  */
#line 931 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_UNARY_OP, ZEND_BOOL_NOT, (yyvsp[(2) - (2)].ast)); }
    break;

  case 343:
/* Line 1792 of yacc.c  */
#line 932 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_UNARY_OP, ZEND_BW_NOT, (yyvsp[(2) - (2)].ast)); }
    break;

  case 344:
/* Line 1792 of yacc.c  */
#line 934 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_IDENTICAL, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 345:
/* Line 1792 of yacc.c  */
#line 936 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_NOT_IDENTICAL, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 346:
/* Line 1792 of yacc.c  */
#line 938 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_EQUAL, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 347:
/* Line 1792 of yacc.c  */
#line 940 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_NOT_EQUAL, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 348:
/* Line 1792 of yacc.c  */
#line 942 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_SMALLER, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 349:
/* Line 1792 of yacc.c  */
#line 944 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_binary_op(ZEND_IS_SMALLER_OR_EQUAL, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 350:
/* Line 1792 of yacc.c  */
#line 946 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_GREATER, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 351:
/* Line 1792 of yacc.c  */
#line 948 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_GREATER_EQUAL, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 352:
/* Line 1792 of yacc.c  */
#line 950 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_binary_op(ZEND_SPACESHIP, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 353:
/* Line 1792 of yacc.c  */
#line 952 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_INSTANCEOF, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 354:
/* Line 1792 of yacc.c  */
#line 953 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); }
    break;

  case 355:
/* Line 1792 of yacc.c  */
#line 954 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 356:
/* Line 1792 of yacc.c  */
#line 956 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_CONDITIONAL, (yyvsp[(1) - (5)].ast), (yyvsp[(3) - (5)].ast), (yyvsp[(5) - (5)].ast)); }
    break;

  case 357:
/* Line 1792 of yacc.c  */
#line 958 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_CONDITIONAL, (yyvsp[(1) - (4)].ast), NULL, (yyvsp[(4) - (4)].ast)); }
    break;

  case 358:
/* Line 1792 of yacc.c  */
#line 960 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_COALESCE, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 359:
/* Line 1792 of yacc.c  */
#line 961 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 360:
/* Line 1792 of yacc.c  */
#line 962 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_cast(IS_LONG, (yyvsp[(2) - (2)].ast)); }
    break;

  case 361:
/* Line 1792 of yacc.c  */
#line 963 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_cast(IS_DOUBLE, (yyvsp[(2) - (2)].ast)); }
    break;

  case 362:
/* Line 1792 of yacc.c  */
#line 964 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_cast(IS_STRING, (yyvsp[(2) - (2)].ast)); }
    break;

  case 363:
/* Line 1792 of yacc.c  */
#line 965 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_cast(IS_ARRAY, (yyvsp[(2) - (2)].ast)); }
    break;

  case 364:
/* Line 1792 of yacc.c  */
#line 966 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_cast(IS_OBJECT, (yyvsp[(2) - (2)].ast)); }
    break;

  case 365:
/* Line 1792 of yacc.c  */
#line 967 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_cast(_IS_BOOL, (yyvsp[(2) - (2)].ast)); }
    break;

  case 366:
/* Line 1792 of yacc.c  */
#line 968 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_cast(IS_NULL, (yyvsp[(2) - (2)].ast)); }
    break;

  case 367:
/* Line 1792 of yacc.c  */
#line 969 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_EXIT, (yyvsp[(2) - (2)].ast)); }
    break;

  case 368:
/* Line 1792 of yacc.c  */
#line 970 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_SILENCE, (yyvsp[(2) - (2)].ast)); }
    break;

  case 369:
/* Line 1792 of yacc.c  */
#line 971 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 370:
/* Line 1792 of yacc.c  */
#line 972 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_SHELL_EXEC, (yyvsp[(2) - (3)].ast)); }
    break;

  case 371:
/* Line 1792 of yacc.c  */
#line 973 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_PRINT, (yyvsp[(2) - (2)].ast)); }
    break;

  case 372:
/* Line 1792 of yacc.c  */
#line 974 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, NULL, NULL); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 373:
/* Line 1792 of yacc.c  */
#line 975 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, (yyvsp[(2) - (2)].ast), NULL); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 374:
/* Line 1792 of yacc.c  */
#line 976 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD, (yyvsp[(4) - (4)].ast), (yyvsp[(2) - (4)].ast)); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 375:
/* Line 1792 of yacc.c  */
#line 977 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_YIELD_FROM, (yyvsp[(2) - (2)].ast)); CG(extra_fn_flags) |= ZEND_ACC_GENERATOR; }
    break;

  case 376:
/* Line 1792 of yacc.c  */
#line 980 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLOSURE, (yyvsp[(2) - (13)].num) | (yyvsp[(13) - (13)].num), (yyvsp[(1) - (13)].num), (yyvsp[(3) - (13)].str),
				  zend_string_init("{closure}", sizeof("{closure}") - 1, 0),
			      (yyvsp[(5) - (13)].ast), (yyvsp[(7) - (13)].ast), (yyvsp[(11) - (13)].ast), (yyvsp[(8) - (13)].ast)); CG(extra_fn_flags) = (yyvsp[(9) - (13)].num); }
    break;

  case 377:
/* Line 1792 of yacc.c  */
#line 985 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_decl(ZEND_AST_CLOSURE, (yyvsp[(3) - (14)].num) | (yyvsp[(14) - (14)].num) | ZEND_ACC_STATIC, (yyvsp[(2) - (14)].num), (yyvsp[(4) - (14)].str),
			      zend_string_init("{closure}", sizeof("{closure}") - 1, 0),
			      (yyvsp[(6) - (14)].ast), (yyvsp[(8) - (14)].ast), (yyvsp[(12) - (14)].ast), (yyvsp[(9) - (14)].ast)); CG(extra_fn_flags) = (yyvsp[(10) - (14)].num); }
    break;

  case 378:
/* Line 1792 of yacc.c  */
#line 991 "zend_language_parser.y"
    { (yyval.num) = CG(zend_lineno); }
    break;

  case 379:
/* Line 1792 of yacc.c  */
#line 995 "zend_language_parser.y"
    { (yyval.str) = CG(doc_comment); CG(doc_comment) = NULL; }
    break;

  case 380:
/* Line 1792 of yacc.c  */
#line 999 "zend_language_parser.y"
    { (yyval.num) = CG(extra_fn_flags); CG(extra_fn_flags) = 0; }
    break;

  case 381:
/* Line 1792 of yacc.c  */
#line 1003 "zend_language_parser.y"
    { (yyval.num) = 0; }
    break;

  case 382:
/* Line 1792 of yacc.c  */
#line 1004 "zend_language_parser.y"
    { (yyval.num) = ZEND_ACC_RETURN_REFERENCE; }
    break;

  case 383:
/* Line 1792 of yacc.c  */
#line 1008 "zend_language_parser.y"
    { (yyval.ast) = NULL; }
    break;

  case 384:
/* Line 1792 of yacc.c  */
#line 1009 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(3) - (4)].ast); }
    break;

  case 385:
/* Line 1792 of yacc.c  */
#line 1013 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 386:
/* Line 1792 of yacc.c  */
#line 1014 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_CLOSURE_USES, (yyvsp[(1) - (1)].ast)); }
    break;

  case 387:
/* Line 1792 of yacc.c  */
#line 1018 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 388:
/* Line 1792 of yacc.c  */
#line 1019 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (2)].ast); (yyval.ast)->attr = 1; }
    break;

  case 389:
/* Line 1792 of yacc.c  */
#line 1024 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_CALL, (yyvsp[(1) - (2)].ast), (yyvsp[(2) - (2)].ast)); }
    break;

  case 390:
/* Line 1792 of yacc.c  */
#line 1026 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_CALL, (yyvsp[(1) - (4)].ast), (yyvsp[(3) - (4)].ast), (yyvsp[(4) - (4)].ast)); }
    break;

  case 391:
/* Line 1792 of yacc.c  */
#line 1028 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_CALL, (yyvsp[(1) - (4)].ast), (yyvsp[(3) - (4)].ast), (yyvsp[(4) - (4)].ast)); }
    break;

  case 392:
/* Line 1792 of yacc.c  */
#line 1030 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_CALL, (yyvsp[(1) - (2)].ast), (yyvsp[(2) - (2)].ast)); }
    break;

  case 393:
/* Line 1792 of yacc.c  */
#line 1035 "zend_language_parser.y"
    { zval zv; ZVAL_INTERNED_STR(&zv, ZSTR_KNOWN(ZEND_STR_STATIC));
			  (yyval.ast) = zend_ast_create_zval_ex(&zv, ZEND_NAME_NOT_FQ); }
    break;

  case 394:
/* Line 1792 of yacc.c  */
#line 1037 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 395:
/* Line 1792 of yacc.c  */
#line 1041 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 396:
/* Line 1792 of yacc.c  */
#line 1042 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 397:
/* Line 1792 of yacc.c  */
#line 1046 "zend_language_parser.y"
    { (yyval.ast) = NULL; }
    break;

  case 398:
/* Line 1792 of yacc.c  */
#line 1047 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); }
    break;

  case 399:
/* Line 1792 of yacc.c  */
#line 1052 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_zval_from_str(ZSTR_EMPTY_ALLOC()); }
    break;

  case 400:
/* Line 1792 of yacc.c  */
#line 1053 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 401:
/* Line 1792 of yacc.c  */
#line 1054 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 402:
/* Line 1792 of yacc.c  */
#line 1059 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(0, ZEND_AST_ARG_LIST); }
    break;

  case 403:
/* Line 1792 of yacc.c  */
#line 1060 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 404:
/* Line 1792 of yacc.c  */
#line 1065 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(3) - (4)].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_LONG; }
    break;

  case 405:
/* Line 1792 of yacc.c  */
#line 1066 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); (yyval.ast)->attr = ZEND_ARRAY_SYNTAX_SHORT; }
    break;

  case 406:
/* Line 1792 of yacc.c  */
#line 1067 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 407:
/* Line 1792 of yacc.c  */
#line 1071 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 408:
/* Line 1792 of yacc.c  */
#line 1072 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 409:
/* Line 1792 of yacc.c  */
#line 1073 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_LINE); }
    break;

  case 410:
/* Line 1792 of yacc.c  */
#line 1074 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_FILE); }
    break;

  case 411:
/* Line 1792 of yacc.c  */
#line 1075 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_DIR); }
    break;

  case 412:
/* Line 1792 of yacc.c  */
#line 1076 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_TRAIT_C); }
    break;

  case 413:
/* Line 1792 of yacc.c  */
#line 1077 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_METHOD_C); }
    break;

  case 414:
/* Line 1792 of yacc.c  */
#line 1078 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_FUNC_C); }
    break;

  case 415:
/* Line 1792 of yacc.c  */
#line 1079 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_NS_C); }
    break;

  case 416:
/* Line 1792 of yacc.c  */
#line 1080 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_MAGIC_CONST, T_CLASS_C); }
    break;

  case 417:
/* Line 1792 of yacc.c  */
#line 1081 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); }
    break;

  case 418:
/* Line 1792 of yacc.c  */
#line 1083 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_zval_from_str(ZSTR_EMPTY_ALLOC()); }
    break;

  case 419:
/* Line 1792 of yacc.c  */
#line 1084 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); }
    break;

  case 420:
/* Line 1792 of yacc.c  */
#line 1085 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); }
    break;

  case 421:
/* Line 1792 of yacc.c  */
#line 1086 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 422:
/* Line 1792 of yacc.c  */
#line 1087 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 423:
/* Line 1792 of yacc.c  */
#line 1091 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_CONST, (yyvsp[(1) - (1)].ast)); }
    break;

  case 424:
/* Line 1792 of yacc.c  */
#line 1093 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_CLASS_CONST, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 425:
/* Line 1792 of yacc.c  */
#line 1095 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_CLASS_CONST, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 426:
/* Line 1792 of yacc.c  */
#line 1099 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 427:
/* Line 1792 of yacc.c  */
#line 1100 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 428:
/* Line 1792 of yacc.c  */
#line 1104 "zend_language_parser.y"
    { (yyval.ast) = NULL; }
    break;

  case 429:
/* Line 1792 of yacc.c  */
#line 1105 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 430:
/* Line 1792 of yacc.c  */
#line 1109 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 431:
/* Line 1792 of yacc.c  */
#line 1113 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 432:
/* Line 1792 of yacc.c  */
#line 1114 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); }
    break;

  case 433:
/* Line 1792 of yacc.c  */
#line 1115 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 434:
/* Line 1792 of yacc.c  */
#line 1119 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 435:
/* Line 1792 of yacc.c  */
#line 1120 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); }
    break;

  case 436:
/* Line 1792 of yacc.c  */
#line 1121 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 437:
/* Line 1792 of yacc.c  */
#line 1126 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[(1) - (1)].ast)); }
    break;

  case 438:
/* Line 1792 of yacc.c  */
#line 1128 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[(1) - (4)].ast), (yyvsp[(3) - (4)].ast)); }
    break;

  case 439:
/* Line 1792 of yacc.c  */
#line 1130 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[(1) - (4)].ast), (yyvsp[(3) - (4)].ast)); }
    break;

  case 440:
/* Line 1792 of yacc.c  */
#line 1132 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[(1) - (4)].ast), (yyvsp[(3) - (4)].ast)); }
    break;

  case 441:
/* Line 1792 of yacc.c  */
#line 1134 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_METHOD_CALL, (yyvsp[(1) - (4)].ast), (yyvsp[(3) - (4)].ast), (yyvsp[(4) - (4)].ast)); }
    break;

  case 442:
/* Line 1792 of yacc.c  */
#line 1135 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 443:
/* Line 1792 of yacc.c  */
#line 1140 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 444:
/* Line 1792 of yacc.c  */
#line 1142 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 445:
/* Line 1792 of yacc.c  */
#line 1144 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 446:
/* Line 1792 of yacc.c  */
#line 1148 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 447:
/* Line 1792 of yacc.c  */
#line 1149 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(3) - (4)].ast); }
    break;

  case 448:
/* Line 1792 of yacc.c  */
#line 1150 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[(2) - (2)].ast)); }
    break;

  case 449:
/* Line 1792 of yacc.c  */
#line 1155 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 450:
/* Line 1792 of yacc.c  */
#line 1157 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 451:
/* Line 1792 of yacc.c  */
#line 1162 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[(1) - (1)].ast)); }
    break;

  case 452:
/* Line 1792 of yacc.c  */
#line 1164 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[(1) - (4)].ast), (yyvsp[(3) - (4)].ast)); }
    break;

  case 453:
/* Line 1792 of yacc.c  */
#line 1166 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_DIM, (yyvsp[(1) - (4)].ast), (yyvsp[(3) - (4)].ast)); }
    break;

  case 454:
/* Line 1792 of yacc.c  */
#line 1168 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 455:
/* Line 1792 of yacc.c  */
#line 1170 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 456:
/* Line 1792 of yacc.c  */
#line 1172 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_STATIC_PROP, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 457:
/* Line 1792 of yacc.c  */
#line 1176 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 458:
/* Line 1792 of yacc.c  */
#line 1177 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); }
    break;

  case 459:
/* Line 1792 of yacc.c  */
#line 1178 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[(1) - (1)].ast)); }
    break;

  case 460:
/* Line 1792 of yacc.c  */
#line 1182 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 461:
/* Line 1792 of yacc.c  */
#line 1183 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); }
    break;

  case 462:
/* Line 1792 of yacc.c  */
#line 1184 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[(1) - (1)].ast)); }
    break;

  case 463:
/* Line 1792 of yacc.c  */
#line 1189 "zend_language_parser.y"
    { /* allow single trailing comma */ (yyval.ast) = zend_ast_list_rtrim((yyvsp[(1) - (1)].ast)); }
    break;

  case 464:
/* Line 1792 of yacc.c  */
#line 1193 "zend_language_parser.y"
    { (yyval.ast) = NULL; }
    break;

  case 465:
/* Line 1792 of yacc.c  */
#line 1194 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 466:
/* Line 1792 of yacc.c  */
#line 1199 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 467:
/* Line 1792 of yacc.c  */
#line 1201 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ARRAY, (yyvsp[(1) - (1)].ast)); }
    break;

  case 468:
/* Line 1792 of yacc.c  */
#line 1206 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[(3) - (3)].ast), (yyvsp[(1) - (3)].ast)); }
    break;

  case 469:
/* Line 1792 of yacc.c  */
#line 1208 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[(1) - (1)].ast), NULL); }
    break;

  case 470:
/* Line 1792 of yacc.c  */
#line 1210 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_ARRAY_ELEM, 1, (yyvsp[(4) - (4)].ast), (yyvsp[(1) - (4)].ast)); }
    break;

  case 471:
/* Line 1792 of yacc.c  */
#line 1212 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_ARRAY_ELEM, 1, (yyvsp[(2) - (2)].ast), NULL); }
    break;

  case 472:
/* Line 1792 of yacc.c  */
#line 1214 "zend_language_parser.y"
    { (yyvsp[(5) - (6)].ast)->attr = ZEND_ARRAY_SYNTAX_LIST;
			  (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[(5) - (6)].ast), (yyvsp[(1) - (6)].ast)); }
    break;

  case 473:
/* Line 1792 of yacc.c  */
#line 1217 "zend_language_parser.y"
    { (yyvsp[(3) - (4)].ast)->attr = ZEND_ARRAY_SYNTAX_LIST;
			  (yyval.ast) = zend_ast_create(ZEND_AST_ARRAY_ELEM, (yyvsp[(3) - (4)].ast), NULL); }
    break;

  case 474:
/* Line 1792 of yacc.c  */
#line 1223 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (2)].ast), (yyvsp[(2) - (2)].ast)); }
    break;

  case 475:
/* Line 1792 of yacc.c  */
#line 1225 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_list_add((yyvsp[(1) - (2)].ast), (yyvsp[(2) - (2)].ast)); }
    break;

  case 476:
/* Line 1792 of yacc.c  */
#line 1227 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(1, ZEND_AST_ENCAPS_LIST, (yyvsp[(1) - (1)].ast)); }
    break;

  case 477:
/* Line 1792 of yacc.c  */
#line 1229 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_list(2, ZEND_AST_ENCAPS_LIST, (yyvsp[(1) - (2)].ast), (yyvsp[(2) - (2)].ast)); }
    break;

  case 478:
/* Line 1792 of yacc.c  */
#line 1234 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[(1) - (1)].ast)); }
    break;

  case 479:
/* Line 1792 of yacc.c  */
#line 1236 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_DIM,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[(1) - (4)].ast)), (yyvsp[(3) - (4)].ast)); }
    break;

  case 480:
/* Line 1792 of yacc.c  */
#line 1239 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_PROP,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[(1) - (3)].ast)), (yyvsp[(3) - (3)].ast)); }
    break;

  case 481:
/* Line 1792 of yacc.c  */
#line 1242 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[(2) - (3)].ast)); }
    break;

  case 482:
/* Line 1792 of yacc.c  */
#line 1244 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[(2) - (3)].ast)); }
    break;

  case 483:
/* Line 1792 of yacc.c  */
#line 1246 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_DIM,
			      zend_ast_create(ZEND_AST_VAR, (yyvsp[(2) - (6)].ast)), (yyvsp[(4) - (6)].ast)); }
    break;

  case 484:
/* Line 1792 of yacc.c  */
#line 1248 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(2) - (3)].ast); }
    break;

  case 485:
/* Line 1792 of yacc.c  */
#line 1252 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 486:
/* Line 1792 of yacc.c  */
#line 1253 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 487:
/* Line 1792 of yacc.c  */
#line 1254 "zend_language_parser.y"
    { (yyval.ast) = zend_negate_num_string((yyvsp[(2) - (2)].ast)); }
    break;

  case 488:
/* Line 1792 of yacc.c  */
#line 1255 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_VAR, (yyvsp[(1) - (1)].ast)); }
    break;

  case 489:
/* Line 1792 of yacc.c  */
#line 1260 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(3) - (4)].ast); }
    break;

  case 490:
/* Line 1792 of yacc.c  */
#line 1261 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_EMPTY, (yyvsp[(3) - (4)].ast)); }
    break;

  case 491:
/* Line 1792 of yacc.c  */
#line 1263 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_INCLUDE, (yyvsp[(2) - (2)].ast)); }
    break;

  case 492:
/* Line 1792 of yacc.c  */
#line 1265 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_INCLUDE_ONCE, (yyvsp[(2) - (2)].ast)); }
    break;

  case 493:
/* Line 1792 of yacc.c  */
#line 1267 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_EVAL, (yyvsp[(3) - (4)].ast)); }
    break;

  case 494:
/* Line 1792 of yacc.c  */
#line 1269 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_REQUIRE, (yyvsp[(2) - (2)].ast)); }
    break;

  case 495:
/* Line 1792 of yacc.c  */
#line 1271 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create_ex(ZEND_AST_INCLUDE_OR_EVAL, ZEND_REQUIRE_ONCE, (yyvsp[(2) - (2)].ast)); }
    break;

  case 496:
/* Line 1792 of yacc.c  */
#line 1275 "zend_language_parser.y"
    { (yyval.ast) = (yyvsp[(1) - (1)].ast); }
    break;

  case 497:
/* Line 1792 of yacc.c  */
#line 1277 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_AND, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast)); }
    break;

  case 498:
/* Line 1792 of yacc.c  */
#line 1281 "zend_language_parser.y"
    { (yyval.ast) = zend_ast_create(ZEND_AST_ISSET, (yyvsp[(1) - (1)].ast)); }
    break;


/* Line 1792 of yacc.c  */
#line 7181 "zend_language_parser.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
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
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
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
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
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
      if (!yypact_value_is_default (yyn))
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
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


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

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
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


/* Line 2055 of yacc.c  */
#line 1284 "zend_language_parser.y"


/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T zend_yytnamerr(char *yyres, const char *yystr)
{
	/* CG(parse_error) states:
	 * 0 => yyres = NULL, yystr is the unexpected token
	 * 1 => yyres = NULL, yystr is one of the expected tokens
	 * 2 => yyres != NULL, yystr is the unexpected token
	 * 3 => yyres != NULL, yystr is one of the expected tokens
	 */
	if (yyres && CG(parse_error) < 2) {
		CG(parse_error) = 2;
	}

	if (CG(parse_error) % 2 == 0) {
		/* The unexpected token */
		char buffer[120];
		const unsigned char *end, *str, *tok1 = NULL, *tok2 = NULL;
		unsigned int len = 0, toklen = 0, yystr_len;

		CG(parse_error)++;

		if (LANG_SCNG(yy_text)[0] == 0 &&
			LANG_SCNG(yy_leng) == 1 &&
			memcmp(yystr, "\"end of file\"", sizeof("\"end of file\"") - 1) == 0) {
			if (yyres) {
				yystpcpy(yyres, "end of file");
			}
			return sizeof("end of file")-1;
		}

		str = LANG_SCNG(yy_text);
		end = memchr(str, '\n', LANG_SCNG(yy_leng));
		yystr_len = (unsigned int)yystrlen(yystr);

		if ((tok1 = memchr(yystr, '(', yystr_len)) != NULL
			&& (tok2 = zend_memrchr(yystr, ')', yystr_len)) != NULL) {
			toklen = (tok2 - tok1) + 1;
		} else {
			tok1 = tok2 = NULL;
			toklen = 0;
		}

		if (end == NULL) {
			len = LANG_SCNG(yy_leng) > 30 ? 30 : LANG_SCNG(yy_leng);
		} else {
			len = (end - str) > 30 ? 30 : (end - str);
		}
		if (yyres) {
			if (toklen) {
				snprintf(buffer, sizeof(buffer), "'%.*s' %.*s", len, str, toklen, tok1);
			} else {
				snprintf(buffer, sizeof(buffer), "'%.*s'", len, str);
			}
			yystpcpy(yyres, buffer);
		}
		return len + (toklen ? toklen + 1 : 0) + 2;
	}

	/* One of the expected tokens */
	if (!yyres) {
		return yystrlen(yystr) - (*yystr == '"' ? 2 : 0);
	}

	if (*yystr == '"') {
		YYSIZE_T yyn = 0;
		const char *yyp = yystr;

		for (; *++yyp != '"'; ++yyn) {
			yyres[yyn] = *yyp;
		}
		yyres[yyn] = '\0';
		return yyn;
	}
	yystpcpy(yyres, yystr);
	return strlen(yystr);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
