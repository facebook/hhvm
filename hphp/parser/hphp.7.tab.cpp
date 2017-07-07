// @generated
/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         Compiler7parse
#define yylex           Compiler7lex
#define yyerror         Compiler7error
#define yydebug         Compiler7debug
#define yynerrs         Compiler7nerrs


/* Copy the first part of user declarations.  */
#line 1 "hphp.y" /* yacc.c:339  */


/* By default this grammar is set up to be used by HPHP's compile parser.
 * However, it can be used to make parsers for different purposes by
 * making a Parser implementation with the same interface as
 * HPHP::Compiler::Parser in a header file specified by
 * PARSER_DEFINITIONS_HEADER, and specifying an alternate namespace with
 * HPHP_PARSER_NS.
 */

// macros for bison
#define YYSTYPE HPHP::HPHP_PARSER_NS::Token
#define YYSTYPE_IS_TRIVIAL false
#define YYLTYPE HPHP::Location
#define YYLTYPE_IS_TRIVIAL true
#define YYERROR_VERBOSE
#define YYINITDEPTH 500
#define YYLEX_PARAM _p

#ifdef PARSER_DEFINITIONS_HEADER
#include PARSER_DEFINITIONS_HEADER
#else
#include "hphp/compiler/parser/parser.h"
#endif

#include <folly/Conv.h>
#include <folly/String.h>

#include "hphp/util/logger.h"

#define line0 r.line0
#define char0 r.char0
#define line1 r.line1
#define char1 r.char1

#ifdef yyerror
#undef yyerror
#endif
#define yyerror(loc,p,msg) p->parseFatal(loc,msg)

#ifdef YYLLOC_DEFAULT
# undef YYLLOC_DEFAULT
#endif
#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#define YYLLOC_DEFAULT(Current, Rhs, N)                                 \
  do                                                                    \
    if (N) {                                                            \
      (Current).first(YYRHSLOC (Rhs, 1));                               \
      (Current).last (YYRHSLOC (Rhs, N));                               \
    } else {                                                            \
      (Current).line0 = (Current).line1 = YYRHSLOC (Rhs, 0).line1;\
      (Current).char0 = (Current).char1 = YYRHSLOC (Rhs, 0).char1;\
    }                                                                   \
  while (0);                                                            \
  _p->setRuleLocation(&Current);

#define YYCOPY(To, From, Count)                  \
  do {                                           \
    YYSIZE_T yyi;                                \
    for (yyi = 0; yyi < (Count); yyi++) {        \
      (To)[yyi] = (From)[yyi];                   \
    }                                            \
    if (From != From ## a) {                     \
      YYSTACK_FREE (From);                       \
    }                                            \
  }                                              \
  while (0)

#define YYCOPY_RESET(To, From, Count)           \
  do                                            \
    {                                           \
      YYSIZE_T yyi;                             \
      for (yyi = 0; yyi < (Count); yyi++) {     \
        (To)[yyi] = (From)[yyi];                \
        (From)[yyi].reset();                    \
      }                                         \
      if (From != From ## a) {                  \
        YYSTACK_FREE (From);                    \
      }                                         \
    }                                           \
  while (0)

#define YYTOKEN_RESET(From, Count)              \
  do                                            \
    {                                           \
      YYSIZE_T yyi;                             \
      for (yyi = 0; yyi < (Count); yyi++) {     \
        (From)[yyi].reset();                    \
      }                                         \
      if (From != From ## a) {                  \
        YYSTACK_FREE (From);                    \
      }                                         \
    }                                           \
  while (0)

# define YYSTACK_RELOCATE_RESET(Stack_alloc, Stack)                     \
  do                                                                    \
    {                                                                   \
      YYSIZE_T yynewbytes;                                              \
      YYCOPY_RESET (&yyptr->Stack_alloc, Stack, yysize);                \
      Stack = &yyptr->Stack_alloc;                                      \
      yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
      yyptr += yynewbytes / sizeof (*yyptr);                            \
    }                                                                   \
  while (0)

#define YYSTACK_CLEANUP                         \
  YYTOKEN_RESET (yyvs, yystacksize);            \
  if (yyvs != yyvsa) {                          \
    YYSTACK_FREE (yyvs);                        \
  }                                             \
  if (yyls != yylsa) {                          \
    YYSTACK_FREE (yyls);                        \
  }                                             \


// macros for rules
#define BEXP(...) _p->onBinaryOpExp(__VA_ARGS__);
#define UEXP(...) _p->onUnaryOpExp(__VA_ARGS__);

using namespace HPHP::HPHP_PARSER_NS;

typedef HPHP::ClosureType ClosureType;

///////////////////////////////////////////////////////////////////////////////
// helpers

static void scalar_num(Parser *_p, Token &out, const char *num) {
  Token t;
  t.setText(num);
  _p->onScalar(out, T_LNUMBER, t);
}

static void scalar_num(Parser *_p, Token &out, int num) {
  Token t;
  t.setText(folly::to<std::string>(num));
  _p->onScalar(out, T_LNUMBER, t);
}

static void scalar_null(Parser *_p, Token &out) {
  Token tnull; tnull.setText("null");
  _p->onConstantValue(out, tnull);
}

static void scalar_file(Parser *_p, Token &out) {
  Token file; file.setText("__FILE__");
  _p->onScalar(out, T_FILE, file);
}

static void scalar_line(Parser *_p, Token &out) {
  Token line; line.setText("__LINE__");
  _p->onScalar(out, T_LINE, line);
}

///////////////////////////////////////////////////////////////////////////////

static void constant_ae(Parser *_p, Token &out, Token &value) {
  const std::string& valueStr = value.text();
  if (valueStr.size() < 3 || valueStr.size() > 5 ||
      (strcasecmp("true", valueStr.c_str()) != 0 &&
       strcasecmp("false", valueStr.c_str()) != 0 &&
       strcasecmp("null", valueStr.c_str()) != 0 &&
       strcasecmp("inf", valueStr.c_str()) != 0 &&
       strcasecmp("nan", valueStr.c_str()) != 0)) {
    HPHP_PARSER_ERROR("User-defined constants are not allowed in user "
                      "attribute expressions", _p);
  }
  _p->onConstantValue(out, value);
}

///////////////////////////////////////////////////////////////////////////////

/**
 * XHP functions: They are defined here, so different parsers don't have to
 * handle XHP rules at all.
 */

static void xhp_tag(Parser *_p, Token &out, Token &label, Token &body) {
  if (!body.text().empty() && body.text() != label.text()) {
    HPHP_PARSER_ERROR("XHP: mismatched tag: '%s' not the same as '%s'",
                      _p, body.text().c_str(), label.text().c_str());
  }

  label.xhpLabel();
  Token name; _p->onName(name, label, Parser::StringName);
  _p->onNewObject(out, name, body);
}

static void xhp_attribute(Parser *_p, Token &out, Token &type, Token &label,
                          Token &def, Token &req) {
  /**
   * The bool, int, float, and string typenames are not given any special
   * treatment by the parser and are treated the same as regular class names
   * (which initially gets marked as type code 5). However, XHP wants to use
   * different type codes for bool, int, float, and string, so we need to fix
   * up the type code here to make XHP happy.
   */
  if (type.num() == 5) {
    auto* str = type.text().c_str();
    if (_p->scanner().isHHSyntaxEnabled()) {
      switch (type.text().size()) {
        case 6:
          if (!strcasecmp(str, "HH\\int")) {
            type.reset(); type.setNum(3);
          }
          break;
        case 7:
          if (!strcasecmp(str, "HH\\bool")) {
            type.reset(); type.setNum(2);
          }
          break;
        case 8:
          if (!strcasecmp(str, "HH\\float")) {
            type.reset(); type.setNum(8);
          } else if (!strcasecmp(str, "HH\\mixed")) {
            type.reset(); type.setNum(6);
          }
          break;
        case 9:
          if (!strcasecmp(str, "HH\\string")) {
            type.reset(); type.setNum(1);
          }
          break;
        default:
          break;
      }
    } else {
      switch (type.text().size()) {
        case 3:
          if (!strcasecmp(str, "int")) {
            type.reset(); type.setNum(3);
          }
          break;
        case 4:
          if (!strcasecmp(str, "bool")) {
            type.reset(); type.setNum(2);
          } else if (!strcasecmp(str, "real")) {
            type.reset(); type.setNum(8);
          }
          break;
        case 5:
          if (!strcasecmp(str, "float")) {
            type.reset(); type.setNum(8);
          } else if (!strcasecmp(str, "mixed")) {
            type.reset(); type.setNum(6);
          }
          break;
        case 6:
          if (!strcasecmp(str, "string")) {
            type.reset(); type.setNum(1);
          } else if (!strcasecmp(str, "double")) {
            type.reset(); type.setNum(8);
          }
          break;
        case 7:
          if (!strcasecmp(str, "integer")) {
            type.reset(); type.setNum(3);
          } else if (!strcasecmp(str, "boolean")) {
            type.reset(); type.setNum(2);
          }
          break;
        default:
          break;
      }
    }
  }

  Token num;  scalar_num(_p, num, type.num());
  Token arr1; _p->onArrayPair(arr1, 0, 0, num, 0);

  Token arr2;
  switch (type.num()) {
    case 5: /* class */ {
      Token cls; _p->onScalar(cls, T_CONSTANT_ENCAPSED_STRING, type);
      _p->onArrayPair(arr2, &arr1, 0, cls, 0);
      break;
    }
    case 7: /* enum */ {
      Token arr;   _p->onArray(arr, type);
      _p->onArrayPair(arr2, &arr1, 0, arr, 0);
      break;
    }
    default: {
      Token tnull; scalar_null(_p, tnull);
      _p->onArrayPair(arr2, &arr1, 0, tnull, 0);
      break;
    }
  }

  Token arr3; _p->onArrayPair(arr3, &arr2, 0, def, 0);
  Token arr4; _p->onArrayPair(arr4, &arr3, 0, req, 0);
  _p->onArray(out, arr4);
  out.setText(label);
}

static void xhp_attribute_list(Parser *_p, Token &out, Token *list,
                               Token &decl) {
  if (decl.num() == 0) {
    decl.xhpLabel();
    if (list) {
      out = *list;
      out.setText(list->text() + ":" + decl.text()); // avoiding vector<string>
    } else {
      out.setText(decl);
    }
  } else {
    Token name; _p->onScalar(name, T_CONSTANT_ENCAPSED_STRING, decl);
    _p->onArrayPair(out, list, &name, decl, 0);
    if (list) {
      out.setText(list->text());
    } else {
      out.setText("");
    }
  }
}

static void xhp_attribute_stmt(Parser *_p, Token &out, Token &attributes) {
  Token modifiers;
  Token fname; fname.setText("__xhpAttributeDeclaration");
  {
    Token m;
    Token m1; m1.setNum(T_PROTECTED); _p->onMemberModifier(m, NULL, m1);
    Token m2; m2.setNum(T_STATIC);    _p->onMemberModifier(modifiers, &m, m2);
  }
  _p->pushFuncLocation();
  _p->onMethodStart(fname, modifiers);

  std::vector<std::string> classes;
  folly::split(':', attributes.text(), classes, true);
  Token arrAttributes; _p->onArray(arrAttributes, attributes);

  Token dummy;

  Token stmts0;
  {
    _p->onStatementListStart(stmts0);
  }
  Token stmts1;
  {
    // static $_ = -1;
    Token one;     scalar_num(_p, one, "1");
    Token mone;    UEXP(mone, one, '-', 1);
    Token var;     var.set(T_VARIABLE, "_");
    Token decl;    _p->onStaticVariable(decl, 0, var, &mone);
    Token sdecl;   _p->onStatic(sdecl, decl);
    _p->addStatement(stmts1, stmts0, sdecl);
  }
  Token stmts2;
  {
    // if ($_ === -1) {
    //   $_ = array_merge(parent::__xhpAttributeDeclaration(),
    //                    attributes);
    // }
    Token parent;  parent.set(T_STRING, "parent");
    Token cls;     _p->onName(cls, parent, Parser::StringName);
    Token fname2;   fname2.setText("__xhpAttributeDeclaration");
    Token param1;  _p->onCall(param1, 0, fname2, dummy, &cls);
    Token params1; _p->onCallParam(params1, NULL, param1, false, false);

    for (unsigned int i = 0; i < classes.size(); i++) {
      Token parent2;  parent2.set(T_STRING, classes[i]);
      Token cls2;     _p->onName(cls2, parent2, Parser::StringName);
      Token fname3;   fname3.setText("__xhpAttributeDeclaration");
      Token param;   _p->onCall(param, 0, fname3, dummy, &cls2);

      Token params; _p->onCallParam(params, &params1, param, false, false);
      params1 = params;
    }

    Token params2; _p->onCallParam(params2, &params1, arrAttributes,
                                   false, false);

    Token name;    name.set(T_STRING, "array_merge");
    Token call;    _p->onCall(call, 0, name, params2, NULL);
    Token tvar;    tvar.set(T_VARIABLE, "_");
    Token var;     _p->onSimpleVariable(var, tvar);
    Token assign;  _p->onAssign(assign, var, call, 0);
    Token exp;     _p->onExpStatement(exp, assign);
    Token block;   _p->onBlock(block, exp);

    Token tvar2;   tvar2.set(T_VARIABLE, "_");
    Token var2;    _p->onSimpleVariable(var2, tvar2);
    Token one;     scalar_num(_p, one, "1");
    Token mone;    UEXP(mone, one, '-', 1);
    Token cond;    BEXP(cond, var2, mone, T_IS_IDENTICAL);
    Token dummy1, dummy2;
    Token sif;     _p->onIf(sif, cond, block, dummy1, dummy2);
    _p->addStatement(stmts2, stmts1, sif);
  }
  Token stmts3;
  {
    // return $_;
    Token tvar;    tvar.set(T_VARIABLE, "_");
    Token var;     _p->onSimpleVariable(var, tvar);
    Token ret;     _p->onReturn(ret, &var);
    _p->addStatement(stmts3, stmts2, ret);
  }
  Token stmt;
  {
    _p->finishStatement(stmt, stmts3);
    stmt = 1;
  }
  {
    Token params, ret, ref; ref = 0;
    _p->onMethod(out, modifiers, ret, ref, fname, params, stmt, nullptr, false);
  }
}

static void xhp_collect_attributes(Parser *_p, Token &out, Token &stmts) {
  Token *attr = _p->xhpGetAttributes();
  if (attr) {
    Token stmt;
    xhp_attribute_stmt(_p, stmt, *attr);
    _p->onClassStatement(out, stmts, stmt);
  } else {
    out = stmts;
  }
}

static void xhp_category_stmt(Parser *_p, Token &out, Token &categories) {
  Token fname;     fname.setText("__xhpCategoryDeclaration");
  Token m1;        m1.setNum(T_PROTECTED);
  Token modifiers; _p->onMemberModifier(modifiers, 0, m1);
  _p->pushFuncLocation();
  _p->onMethodStart(fname, modifiers);

  Token stmts0;
  {
    _p->onStatementListStart(stmts0);
  }
  Token stmts1;
  {
    // static $_ = categories;
    Token arr;     _p->onArray(arr, categories);
    Token var;     var.set(T_VARIABLE, "_");
    Token decl;    _p->onStaticVariable(decl, 0, var, &arr);
    Token sdecl;   _p->onStatic(sdecl, decl);
    _p->addStatement(stmts1, stmts0, sdecl);
  }
  Token stmts2;
  {
    // return $_;
    Token tvar;    tvar.set(T_VARIABLE, "_");
    Token var;     _p->onSimpleVariable(var, tvar);
    Token ret;     _p->onReturn(ret, &var);
    _p->addStatement(stmts2, stmts1, ret);
  }
  Token stmt;
  {
    _p->finishStatement(stmt, stmts2);
    stmt = 1;
  }
  {
    Token params, ret, ref; ref = 0;
    _p->onMethod(out, modifiers, ret, ref, fname, params, stmt, nullptr, false);
  }
}

static void xhp_children_decl_tag(Parser *_p, Token &arr, Token &tag) {
  Token num;  scalar_num(_p, num, tag.num());
  Token arr1; _p->onArrayPair(arr1, &arr, 0, num, 0);

  Token name;
  if (tag.num() == 3 || tag.num() == 4) {
    _p->onScalar(name, T_CONSTANT_ENCAPSED_STRING, tag);
  } else if (tag.num() >= 0) {
    scalar_null(_p, name);
  } else {
    HPHP_PARSER_ERROR("XHP: unknown children declaration", _p);
  }
  Token arr2; _p->onArrayPair(arr2, &arr1, 0, name, 0);
  arr = arr2;
}

static void xhp_children_decl(Parser *_p, Token &out, Token &op1, int op,
                              Token *op2) {
  Token num; scalar_num(_p, num, op);
  Token arr; _p->onArrayPair(arr, 0, 0, num, 0);

  if (op2) {
    Token arr1; _p->onArrayPair(arr1, &arr,  0, op1,  0);
    Token arr2; _p->onArrayPair(arr2, &arr1, 0, *op2, 0);
    _p->onArray(out, arr2);
  } else {
    xhp_children_decl_tag(_p, arr, op1);
    _p->onArray(out, arr);
  }
}

static void xhp_children_paren(Parser *_p, Token &out, Token exp, int op) {
  Token num;  scalar_num(_p, num, op);
  Token arr1; _p->onArrayPair(arr1, 0, 0, num, 0);

  Token num5; scalar_num(_p, num5, 5);
  Token arr2; _p->onArrayPair(arr2, &arr1, 0, num5, 0);

  Token arr3; _p->onArrayPair(arr3, &arr2, 0, exp, 0);
  _p->onArray(out, arr3);
}

static void xhp_children_stmt(Parser *_p, Token &out, Token &children) {
  Token fname;     fname.setText("__xhpChildrenDeclaration");
  Token m1;        m1.setNum(T_PROTECTED);
  Token modifiers; _p->onMemberModifier(modifiers, 0, m1);
  _p->pushFuncLocation();
  _p->onMethodStart(fname, modifiers);

  Token stmts0;
  {
    _p->onStatementListStart(stmts0);
  }
  Token stmts1;
  {
    // static $_ = children;
    Token arr;
    if (children.num() == 2) {
      arr = children;
    } else if (children.num() >= 0) {
      scalar_num(_p, arr, children.num());
    } else {
      HPHP_PARSER_ERROR("XHP: XHP unknown children declaration", _p);
    }
    Token var;     var.set(T_VARIABLE, "_");
    Token decl;    _p->onStaticVariable(decl, 0, var, &arr);
    Token sdecl;   _p->onStatic(sdecl, decl);
    _p->addStatement(stmts1, stmts0, sdecl);
  }
  Token stmts2;
  {
    // return $_;
    Token tvar;    tvar.set(T_VARIABLE, "_");
    Token var;     _p->onSimpleVariable(var, tvar);
    Token ret;     _p->onReturn(ret, &var);
    _p->addStatement(stmts2, stmts1, ret);
  }
  Token stmt;
  {
    _p->finishStatement(stmt, stmts2);
    stmt = 1;
  }
  {
    Token params, ret, ref; ref = 0;
    _p->onMethod(out, modifiers, ret, ref, fname, params, stmt, nullptr, false);
  }
}

static void only_in_hh_syntax(Parser *_p) {
  if (!_p->scanner().isHHSyntaxEnabled()) {
    HPHP_PARSER_ERROR(
      "Syntax only allowed in Hack files (<?hh) or with -v "
        "Eval.EnableHipHopSyntax=true",
      _p);
  }
}

static void validate_hh_variadic_variant(Parser* _p,
                                         Token& userAttrs, Token& typehint,
                                         Token* mod) {
  if (!userAttrs.text().empty() || !typehint.text().empty() ||
     (mod && !mod->text().empty())) {
    HPHP_PARSER_ERROR("Variadic '...' should be followed by a '$variable'", _p);
  }
  only_in_hh_syntax(_p);
}

// Shapes may not have leading integers in key names, considered as a
// parse time error.  This is because at runtime they are currently
// hphp arrays, which will treat leading integer keys as numbers.
static void validate_shape_keyname(Token& tok, Parser* _p) {
  if (tok.text().empty()) {
    HPHP_PARSER_ERROR("Shape key names may not be empty", _p);
  }
  if (isdigit(tok.text()[0])) {
    HPHP_PARSER_ERROR("Shape key names may not start with integers", _p);
  }
}

///////////////////////////////////////////////////////////////////////////////

static int yylex(YYSTYPE* token, HPHP::Location* loc, Parser* _p) {
  return _p->scan(token, loc);
}

#line 656 "hphp.7.tab.cpp" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
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
   by #include "hphp.7.tab.hpp".  */
#ifndef YY_YY_HPHP_Y_INCLUDED
# define YY_YY_HPHP_Y_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int Compiler7debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    T_INCLUDE = 258,
    T_INCLUDE_ONCE = 259,
    T_EVAL = 260,
    T_REQUIRE = 261,
    T_REQUIRE_ONCE = 262,
    T_LAMBDA_ARROW = 263,
    T_LOGICAL_OR = 264,
    T_LOGICAL_XOR = 265,
    T_LOGICAL_AND = 266,
    T_PRINT = 267,
    T_PLUS_EQUAL = 268,
    T_MINUS_EQUAL = 269,
    T_MUL_EQUAL = 270,
    T_DIV_EQUAL = 271,
    T_CONCAT_EQUAL = 272,
    T_MOD_EQUAL = 273,
    T_AND_EQUAL = 274,
    T_OR_EQUAL = 275,
    T_XOR_EQUAL = 276,
    T_SL_EQUAL = 277,
    T_SR_EQUAL = 278,
    T_POW_EQUAL = 279,
    T_AWAIT = 280,
    T_YIELD = 281,
    T_YIELD_FROM = 282,
    T_PIPE = 283,
    T_COALESCE = 284,
    T_BOOLEAN_OR = 285,
    T_BOOLEAN_AND = 286,
    T_IS_EQUAL = 287,
    T_IS_NOT_EQUAL = 288,
    T_IS_IDENTICAL = 289,
    T_IS_NOT_IDENTICAL = 290,
    T_IS_SMALLER_OR_EQUAL = 291,
    T_IS_GREATER_OR_EQUAL = 292,
    T_SPACESHIP = 293,
    T_SL = 294,
    T_SR = 295,
    T_INSTANCEOF = 296,
    T_INC = 297,
    T_DEC = 298,
    T_INT_CAST = 299,
    T_DOUBLE_CAST = 300,
    T_STRING_CAST = 301,
    T_ARRAY_CAST = 302,
    T_OBJECT_CAST = 303,
    T_BOOL_CAST = 304,
    T_UNSET_CAST = 305,
    T_POW = 306,
    T_NEW = 307,
    T_CLONE = 308,
    T_EXIT = 309,
    T_IF = 310,
    T_ELSEIF = 311,
    T_ELSE = 312,
    T_ENDIF = 313,
    T_LNUMBER = 314,
    T_DNUMBER = 315,
    T_ONUMBER = 316,
    T_STRING = 317,
    T_STRING_VARNAME = 318,
    T_VARIABLE = 319,
    T_PIPE_VAR = 320,
    T_NUM_STRING = 321,
    T_INLINE_HTML = 322,
    T_HASHBANG = 323,
    T_CHARACTER = 324,
    T_BAD_CHARACTER = 325,
    T_ENCAPSED_AND_WHITESPACE = 326,
    T_CONSTANT_ENCAPSED_STRING = 327,
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
    T_SUPER = 339,
    T_SWITCH = 340,
    T_ENDSWITCH = 341,
    T_CASE = 342,
    T_DEFAULT = 343,
    T_BREAK = 344,
    T_GOTO = 345,
    T_CONTINUE = 346,
    T_FUNCTION = 347,
    T_CONST = 348,
    T_RETURN = 349,
    T_TRY = 350,
    T_CATCH = 351,
    T_THROW = 352,
    T_USE = 353,
    T_GLOBAL = 354,
    T_STATIC = 355,
    T_ABSTRACT = 356,
    T_FINAL = 357,
    T_PRIVATE = 358,
    T_PROTECTED = 359,
    T_PUBLIC = 360,
    T_VAR = 361,
    T_UNSET = 362,
    T_ISSET = 363,
    T_EMPTY = 364,
    T_HALT_COMPILER = 365,
    T_CLASS = 366,
    T_INTERFACE = 367,
    T_EXTENDS = 368,
    T_IMPLEMENTS = 369,
    T_OBJECT_OPERATOR = 370,
    T_NULLSAFE_OBJECT_OPERATOR = 371,
    T_DOUBLE_ARROW = 372,
    T_LIST = 373,
    T_ARRAY = 374,
    T_DICT = 375,
    T_VEC = 376,
    T_VARRAY = 377,
    T_DARRAY = 378,
    T_KEYSET = 379,
    T_CALLABLE = 380,
    T_CLASS_C = 381,
    T_METHOD_C = 382,
    T_FUNC_C = 383,
    T_LINE = 384,
    T_FILE = 385,
    T_COMMENT = 386,
    T_DOC_COMMENT = 387,
    T_OPEN_TAG = 388,
    T_OPEN_TAG_WITH_ECHO = 389,
    T_CLOSE_TAG = 390,
    T_WHITESPACE = 391,
    T_START_HEREDOC = 392,
    T_END_HEREDOC = 393,
    T_DOLLAR_OPEN_CURLY_BRACES = 394,
    T_CURLY_OPEN = 395,
    T_DOUBLE_COLON = 396,
    T_NAMESPACE = 397,
    T_NS_C = 398,
    T_DIR = 399,
    T_NS_SEPARATOR = 400,
    T_XHP_LABEL = 401,
    T_XHP_TEXT = 402,
    T_XHP_ATTRIBUTE = 403,
    T_XHP_CATEGORY = 404,
    T_XHP_CATEGORY_LABEL = 405,
    T_XHP_CHILDREN = 406,
    T_ENUM = 407,
    T_XHP_REQUIRED = 408,
    T_TRAIT = 409,
    T_ELLIPSIS = 410,
    T_INSTEADOF = 411,
    T_TRAIT_C = 412,
    T_HH_ERROR = 413,
    T_FINALLY = 414,
    T_XHP_TAG_LT = 415,
    T_XHP_TAG_GT = 416,
    T_TYPELIST_LT = 417,
    T_TYPELIST_GT = 418,
    T_UNRESOLVED_LT = 419,
    T_COLLECTION = 420,
    T_SHAPE = 421,
    T_TYPE = 422,
    T_UNRESOLVED_TYPE = 423,
    T_NEWTYPE = 424,
    T_UNRESOLVED_NEWTYPE = 425,
    T_COMPILER_HALT_OFFSET = 426,
    T_ASYNC = 427,
    T_LAMBDA_OP = 428,
    T_LAMBDA_CP = 429,
    T_UNRESOLVED_OP = 430,
    T_WHERE = 431
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int line0;
  int char0;
  int line1;
  int char1;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int Compiler7parse (HPHP::HPHP_PARSER_NS::Parser *_p);

#endif /* !YY_YY_HPHP_Y_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 897 "hphp.7.tab.cpp" /* yacc.c:358  */

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
#else
typedef signed char yytype_int8;
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
# elif ! defined YYSIZE_T
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

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
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
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
struct yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (struct yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

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
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   18358

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  206
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  301
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1076
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1980

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   431

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    56,   204,     2,   201,    55,    38,   205,
     196,   197,    53,    50,     9,    51,    52,    54,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    32,   198,
      43,    14,    45,    31,    68,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    70,     2,   203,    37,     2,   202,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   199,    36,   200,    58,     2,     2,     2,
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
       5,     6,     7,     8,    10,    11,    12,    13,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    33,    34,    35,    39,    40,    41,
      42,    44,    46,    47,    48,    49,    57,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    69,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   751,   751,   751,   760,   762,   765,   766,   767,   768,
     769,   770,   771,   774,   776,   776,   778,   778,   780,   783,
     788,   793,   796,   799,   803,   807,   811,   816,   817,   818,
     819,   820,   821,   822,   823,   824,   825,   826,   827,   828,
     832,   833,   834,   835,   836,   837,   838,   839,   840,   841,
     842,   843,   844,   845,   846,   847,   848,   849,   850,   851,
     852,   853,   854,   855,   856,   857,   858,   859,   860,   861,
     862,   863,   864,   865,   866,   867,   868,   869,   870,   871,
     872,   873,   874,   875,   876,   877,   878,   879,   880,   881,
     882,   883,   884,   885,   886,   887,   888,   889,   890,   891,
     892,   893,   894,   898,   902,   903,   907,   908,   913,   915,
     920,   925,   926,   927,   929,   934,   936,   941,   946,   948,
     950,   955,   956,   960,   961,   963,   967,   974,   981,   985,
     991,   993,   996,   997,   998,   999,  1002,  1003,  1007,  1012,
    1012,  1018,  1018,  1025,  1024,  1030,  1030,  1035,  1036,  1037,
    1038,  1039,  1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,
    1048,  1049,  1053,  1051,  1060,  1058,  1065,  1075,  1069,  1079,
    1077,  1081,  1082,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
    1093,  1094,  1095,  1096,  1097,  1105,  1105,  1110,  1116,  1120,
    1120,  1128,  1129,  1133,  1134,  1138,  1144,  1142,  1157,  1154,
    1170,  1167,  1184,  1183,  1192,  1190,  1202,  1201,  1220,  1218,
    1237,  1236,  1245,  1243,  1254,  1254,  1261,  1260,  1272,  1270,
    1283,  1284,  1288,  1291,  1294,  1295,  1296,  1299,  1300,  1303,
    1305,  1308,  1309,  1312,  1313,  1316,  1317,  1321,  1322,  1327,
    1328,  1331,  1332,  1333,  1337,  1338,  1342,  1343,  1347,  1348,
    1352,  1353,  1358,  1359,  1365,  1366,  1367,  1368,  1371,  1374,
    1376,  1379,  1380,  1384,  1386,  1389,  1392,  1395,  1396,  1399,
    1400,  1404,  1410,  1416,  1423,  1425,  1430,  1435,  1441,  1445,
    1449,  1453,  1458,  1463,  1468,  1473,  1479,  1488,  1493,  1498,
    1504,  1506,  1510,  1514,  1519,  1523,  1526,  1529,  1533,  1537,
    1541,  1545,  1550,  1558,  1560,  1563,  1564,  1565,  1566,  1568,
    1570,  1575,  1576,  1579,  1580,  1581,  1585,  1586,  1588,  1589,
    1593,  1595,  1598,  1602,  1608,  1610,  1613,  1613,  1617,  1616,
    1620,  1622,  1625,  1628,  1626,  1643,  1639,  1654,  1656,  1658,
    1660,  1662,  1664,  1666,  1670,  1671,  1672,  1675,  1681,  1685,
    1691,  1694,  1699,  1701,  1706,  1711,  1715,  1716,  1720,  1721,
    1723,  1725,  1731,  1732,  1734,  1738,  1739,  1744,  1748,  1749,
    1753,  1754,  1758,  1760,  1766,  1771,  1772,  1774,  1778,  1779,
    1780,  1781,  1785,  1786,  1787,  1788,  1789,  1790,  1792,  1797,
    1800,  1801,  1805,  1806,  1810,  1811,  1814,  1815,  1818,  1819,
    1822,  1823,  1827,  1828,  1829,  1830,  1831,  1832,  1833,  1837,
    1838,  1841,  1842,  1843,  1846,  1848,  1850,  1851,  1854,  1856,
    1860,  1862,  1866,  1870,  1874,  1879,  1880,  1882,  1883,  1884,
    1885,  1888,  1892,  1893,  1897,  1898,  1902,  1903,  1904,  1905,
    1909,  1913,  1918,  1922,  1926,  1930,  1934,  1939,  1940,  1941,
    1942,  1943,  1947,  1949,  1950,  1951,  1954,  1955,  1956,  1957,
    1958,  1959,  1960,  1961,  1962,  1963,  1964,  1965,  1966,  1967,
    1968,  1969,  1970,  1971,  1972,  1973,  1974,  1975,  1976,  1977,
    1978,  1979,  1980,  1981,  1982,  1983,  1984,  1985,  1986,  1987,
    1988,  1989,  1990,  1991,  1992,  1993,  1994,  1995,  1996,  1997,
    1999,  2000,  2002,  2003,  2005,  2006,  2007,  2008,  2009,  2010,
    2011,  2012,  2013,  2014,  2015,  2016,  2017,  2018,  2019,  2020,
    2021,  2022,  2023,  2024,  2025,  2026,  2027,  2028,  2029,  2033,
    2037,  2042,  2041,  2056,  2054,  2072,  2071,  2090,  2089,  2108,
    2107,  2125,  2125,  2140,  2140,  2158,  2159,  2160,  2165,  2167,
    2171,  2175,  2181,  2185,  2191,  2193,  2197,  2199,  2203,  2207,
    2208,  2212,  2214,  2218,  2220,  2224,  2226,  2230,  2233,  2238,
    2240,  2244,  2247,  2252,  2256,  2260,  2264,  2268,  2272,  2276,
    2280,  2284,  2288,  2292,  2296,  2300,  2304,  2308,  2312,  2314,
    2318,  2320,  2324,  2326,  2330,  2337,  2344,  2346,  2351,  2352,
    2353,  2354,  2355,  2356,  2357,  2358,  2359,  2361,  2362,  2366,
    2367,  2368,  2369,  2373,  2379,  2388,  2401,  2402,  2405,  2408,
    2411,  2412,  2415,  2419,  2422,  2425,  2432,  2433,  2437,  2438,
    2440,  2445,  2446,  2447,  2448,  2449,  2450,  2451,  2452,  2453,
    2454,  2455,  2456,  2457,  2458,  2459,  2460,  2461,  2462,  2463,
    2464,  2465,  2466,  2467,  2468,  2469,  2470,  2471,  2472,  2473,
    2474,  2475,  2476,  2477,  2478,  2479,  2480,  2481,  2482,  2483,
    2484,  2485,  2486,  2487,  2488,  2489,  2490,  2491,  2492,  2493,
    2494,  2495,  2496,  2497,  2498,  2499,  2500,  2501,  2502,  2503,
    2504,  2505,  2506,  2507,  2508,  2509,  2510,  2511,  2512,  2513,
    2514,  2515,  2516,  2517,  2518,  2519,  2520,  2521,  2522,  2523,
    2524,  2525,  2529,  2534,  2535,  2539,  2540,  2541,  2542,  2544,
    2548,  2549,  2560,  2561,  2563,  2565,  2577,  2578,  2579,  2583,
    2584,  2585,  2589,  2590,  2591,  2594,  2596,  2600,  2601,  2602,
    2603,  2605,  2606,  2607,  2608,  2609,  2610,  2611,  2612,  2613,
    2614,  2617,  2622,  2623,  2624,  2626,  2627,  2629,  2630,  2631,
    2632,  2633,  2634,  2635,  2636,  2637,  2639,  2641,  2643,  2645,
    2647,  2648,  2649,  2650,  2651,  2652,  2653,  2654,  2655,  2656,
    2657,  2658,  2659,  2660,  2661,  2662,  2663,  2665,  2667,  2669,
    2671,  2672,  2675,  2676,  2680,  2684,  2686,  2690,  2691,  2695,
    2701,  2704,  2708,  2709,  2710,  2711,  2712,  2713,  2714,  2719,
    2721,  2725,  2726,  2729,  2730,  2734,  2737,  2739,  2741,  2745,
    2746,  2747,  2748,  2751,  2755,  2756,  2757,  2758,  2762,  2764,
    2771,  2772,  2773,  2774,  2779,  2780,  2781,  2782,  2784,  2785,
    2787,  2788,  2789,  2790,  2791,  2795,  2797,  2801,  2803,  2806,
    2809,  2811,  2813,  2816,  2818,  2822,  2824,  2827,  2830,  2836,
    2838,  2841,  2842,  2847,  2850,  2854,  2854,  2859,  2862,  2863,
    2867,  2868,  2872,  2873,  2874,  2878,  2883,  2888,  2889,  2893,
    2898,  2903,  2904,  2908,  2909,  2914,  2916,  2921,  2932,  2946,
    2958,  2973,  2974,  2975,  2976,  2977,  2978,  2979,  2989,  2998,
    3000,  3002,  3006,  3007,  3008,  3009,  3010,  3026,  3027,  3029,
    3031,  3038,  3039,  3040,  3041,  3042,  3043,  3044,  3045,  3047,
    3052,  3056,  3057,  3061,  3064,  3071,  3075,  3084,  3091,  3099,
    3101,  3102,  3106,  3107,  3108,  3110,  3115,  3116,  3127,  3128,
    3129,  3130,  3141,  3144,  3147,  3148,  3149,  3150,  3161,  3165,
    3166,  3167,  3169,  3170,  3171,  3175,  3177,  3180,  3182,  3183,
    3184,  3185,  3188,  3190,  3191,  3195,  3197,  3200,  3202,  3203,
    3204,  3208,  3210,  3213,  3216,  3218,  3220,  3224,  3225,  3227,
    3228,  3234,  3235,  3237,  3247,  3249,  3251,  3254,  3255,  3256,
    3260,  3261,  3262,  3263,  3264,  3265,  3266,  3267,  3268,  3269,
    3270,  3274,  3275,  3279,  3281,  3289,  3291,  3295,  3299,  3304,
    3308,  3316,  3317,  3321,  3322,  3328,  3329,  3338,  3339,  3347,
    3350,  3354,  3357,  3362,  3367,  3369,  3370,  3371,  3374,  3376,
    3382,  3383,  3387,  3388,  3392,  3393,  3397,  3398,  3401,  3406,
    3407,  3411,  3414,  3416,  3420,  3426,  3427,  3428,  3432,  3436,
    3444,  3449,  3461,  3463,  3467,  3470,  3472,  3477,  3482,  3488,
    3491,  3496,  3501,  3503,  3510,  3512,  3515,  3516,  3519,  3522,
    3523,  3528,  3530,  3534,  3540,  3550,  3551
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_INCLUDE", "T_INCLUDE_ONCE", "T_EVAL",
  "T_REQUIRE", "T_REQUIRE_ONCE", "T_LAMBDA_ARROW", "','", "T_LOGICAL_OR",
  "T_LOGICAL_XOR", "T_LOGICAL_AND", "T_PRINT", "'='", "T_PLUS_EQUAL",
  "T_MINUS_EQUAL", "T_MUL_EQUAL", "T_DIV_EQUAL", "T_CONCAT_EQUAL",
  "T_MOD_EQUAL", "T_AND_EQUAL", "T_OR_EQUAL", "T_XOR_EQUAL", "T_SL_EQUAL",
  "T_SR_EQUAL", "T_POW_EQUAL", "T_AWAIT", "T_YIELD", "T_YIELD_FROM",
  "T_PIPE", "'?'", "':'", "\"??\"", "T_BOOLEAN_OR", "T_BOOLEAN_AND", "'|'",
  "'^'", "'&'", "T_IS_EQUAL", "T_IS_NOT_EQUAL", "T_IS_IDENTICAL",
  "T_IS_NOT_IDENTICAL", "'<'", "T_IS_SMALLER_OR_EQUAL", "'>'",
  "T_IS_GREATER_OR_EQUAL", "T_SPACESHIP", "T_SL", "T_SR", "'+'", "'-'",
  "'.'", "'*'", "'/'", "'%'", "'!'", "T_INSTANCEOF", "'~'", "T_INC",
  "T_DEC", "T_INT_CAST", "T_DOUBLE_CAST", "T_STRING_CAST", "T_ARRAY_CAST",
  "T_OBJECT_CAST", "T_BOOL_CAST", "T_UNSET_CAST", "'@'", "T_POW", "'['",
  "T_NEW", "T_CLONE", "T_EXIT", "T_IF", "T_ELSEIF", "T_ELSE", "T_ENDIF",
  "T_LNUMBER", "T_DNUMBER", "T_ONUMBER", "T_STRING", "T_STRING_VARNAME",
  "T_VARIABLE", "T_PIPE_VAR", "T_NUM_STRING", "T_INLINE_HTML",
  "T_HASHBANG", "T_CHARACTER", "T_BAD_CHARACTER",
  "T_ENCAPSED_AND_WHITESPACE", "T_CONSTANT_ENCAPSED_STRING", "T_ECHO",
  "T_DO", "T_WHILE", "T_ENDWHILE", "T_FOR", "T_ENDFOR", "T_FOREACH",
  "T_ENDFOREACH", "T_DECLARE", "T_ENDDECLARE", "T_AS", "T_SUPER",
  "T_SWITCH", "T_ENDSWITCH", "T_CASE", "T_DEFAULT", "T_BREAK", "T_GOTO",
  "T_CONTINUE", "T_FUNCTION", "T_CONST", "T_RETURN", "T_TRY", "T_CATCH",
  "T_THROW", "T_USE", "T_GLOBAL", "T_STATIC", "T_ABSTRACT", "T_FINAL",
  "T_PRIVATE", "T_PROTECTED", "T_PUBLIC", "T_VAR", "T_UNSET", "T_ISSET",
  "T_EMPTY", "T_HALT_COMPILER", "T_CLASS", "T_INTERFACE", "T_EXTENDS",
  "T_IMPLEMENTS", "T_OBJECT_OPERATOR", "T_NULLSAFE_OBJECT_OPERATOR",
  "T_DOUBLE_ARROW", "T_LIST", "T_ARRAY", "T_DICT", "T_VEC", "T_VARRAY",
  "T_DARRAY", "T_KEYSET", "T_CALLABLE", "T_CLASS_C", "T_METHOD_C",
  "T_FUNC_C", "T_LINE", "T_FILE", "T_COMMENT", "T_DOC_COMMENT",
  "T_OPEN_TAG", "T_OPEN_TAG_WITH_ECHO", "T_CLOSE_TAG", "T_WHITESPACE",
  "T_START_HEREDOC", "T_END_HEREDOC", "T_DOLLAR_OPEN_CURLY_BRACES",
  "T_CURLY_OPEN", "T_DOUBLE_COLON", "T_NAMESPACE", "T_NS_C", "T_DIR",
  "T_NS_SEPARATOR", "T_XHP_LABEL", "T_XHP_TEXT", "T_XHP_ATTRIBUTE",
  "T_XHP_CATEGORY", "T_XHP_CATEGORY_LABEL", "T_XHP_CHILDREN", "T_ENUM",
  "T_XHP_REQUIRED", "T_TRAIT", "\"...\"", "T_INSTEADOF", "T_TRAIT_C",
  "T_HH_ERROR", "T_FINALLY", "T_XHP_TAG_LT", "T_XHP_TAG_GT",
  "T_TYPELIST_LT", "T_TYPELIST_GT", "T_UNRESOLVED_LT", "T_COLLECTION",
  "T_SHAPE", "T_TYPE", "T_UNRESOLVED_TYPE", "T_NEWTYPE",
  "T_UNRESOLVED_NEWTYPE", "T_COMPILER_HALT_OFFSET", "T_ASYNC",
  "T_LAMBDA_OP", "T_LAMBDA_CP", "T_UNRESOLVED_OP", "T_WHERE", "'('", "')'",
  "';'", "'{'", "'}'", "'$'", "'`'", "']'", "'\"'", "'\\''", "$accept",
  "start", "$@1", "top_statement_list", "top_statement", "$@2", "$@3",
  "ident_no_semireserved", "ident_for_class_const", "ident",
  "group_use_prefix", "non_empty_use_declarations", "use_declarations",
  "use_declaration", "non_empty_mixed_use_declarations",
  "mixed_use_declarations", "mixed_use_declaration", "namespace_name",
  "namespace_string", "namespace_string_typeargs",
  "class_namespace_string_typeargs", "constant_declaration",
  "inner_statement_list", "inner_statement", "statement", "$@4", "$@5",
  "$@6", "$@7", "$@8", "$@9", "$@10", "$@11", "try_statement_list", "$@12",
  "additional_catches", "finally_statement_list", "$@13",
  "optional_finally", "is_reference", "function_loc",
  "function_declaration_statement", "$@14", "$@15", "$@16",
  "enum_declaration_statement", "$@17", "$@18",
  "class_declaration_statement", "$@19", "$@20", "$@21", "$@22",
  "class_expression", "$@23", "trait_declaration_statement", "$@24",
  "$@25", "class_decl_name", "interface_decl_name", "trait_decl_name",
  "class_entry_type", "extends_from", "implements_list",
  "interface_extends_list", "interface_list", "trait_list",
  "foreach_optional_arg", "foreach_variable", "for_statement",
  "foreach_statement", "while_statement", "declare_statement",
  "declare_list", "switch_case_list", "case_list", "case_separator",
  "elseif_list", "new_elseif_list", "else_single", "new_else_single",
  "method_parameter_list", "non_empty_method_parameter_list",
  "parameter_list", "non_empty_parameter_list",
  "function_call_parameter_list", "non_empty_fcall_parameter_list",
  "global_var_list", "global_var", "static_var_list",
  "enum_statement_list", "enum_statement", "enum_constant_declaration",
  "class_statement_list", "class_statement", "$@26", "$@27", "$@28",
  "$@29", "trait_rules", "trait_precedence_rule", "trait_alias_rule",
  "trait_alias_rule_method", "xhp_attribute_stmt", "xhp_attribute_decl",
  "xhp_nullable_attribute_decl_type", "xhp_attribute_decl_type",
  "non_empty_xhp_attribute_enum", "xhp_attribute_enum",
  "xhp_attribute_default", "xhp_attribute_is_required",
  "xhp_category_stmt", "xhp_category_decl", "xhp_children_stmt",
  "xhp_children_paren_expr", "xhp_children_decl_expr",
  "xhp_children_decl_tag", "function_body", "method_body",
  "variable_modifiers", "method_modifiers", "non_empty_member_modifiers",
  "member_modifier", "parameter_modifiers", "parameter_modifier",
  "class_variable_declaration", "class_constant_declaration",
  "class_abstract_constant_declaration", "class_type_constant_declaration",
  "class_type_constant", "expr_with_parens", "parenthesis_expr",
  "expr_list", "for_expr", "yield_expr", "yield_assign_expr",
  "yield_list_assign_expr", "yield_from_expr", "yield_from_assign_expr",
  "await_expr", "await_assign_expr", "await_list_assign_expr", "expr",
  "expr_no_variable", "lambda_use_vars", "closure_expression", "$@30",
  "$@31", "lambda_expression", "$@32", "$@33", "$@34", "$@35", "$@36",
  "lambda_body", "shape_keyname", "non_empty_shape_pair_list",
  "non_empty_static_shape_pair_list", "shape_pair_list",
  "static_shape_pair_list", "shape_literal", "array_literal",
  "dict_pair_list", "non_empty_dict_pair_list", "static_dict_pair_list",
  "non_empty_static_dict_pair_list", "static_dict_pair_list_ae",
  "non_empty_static_dict_pair_list_ae", "dict_literal",
  "static_dict_literal", "static_dict_literal_ae", "vec_literal",
  "static_vec_literal", "static_vec_literal_ae", "keyset_literal",
  "static_keyset_literal", "static_keyset_literal_ae", "varray_literal",
  "static_varray_literal", "static_varray_literal_ae", "darray_literal",
  "static_darray_literal", "static_darray_literal_ae", "vec_ks_expr_list",
  "static_vec_ks_expr_list", "static_vec_ks_expr_list_ae",
  "collection_literal", "static_collection_literal", "dim_expr",
  "dim_expr_base", "lexical_var_list", "xhp_tag", "xhp_tag_body",
  "xhp_opt_end_label", "xhp_attributes", "xhp_children",
  "xhp_attribute_name", "xhp_attribute_value", "xhp_child", "xhp_label_ws",
  "xhp_bareword", "simple_function_call", "fully_qualified_class_name",
  "static_class_name_base", "static_class_name_no_calls",
  "static_class_name", "class_name_reference", "exit_expr",
  "backticks_expr", "ctor_arguments", "common_scalar", "static_expr",
  "static_expr_list", "static_class_class_constant",
  "static_class_constant", "scalar", "static_array_pair_list",
  "possible_comma", "hh_possible_comma",
  "non_empty_static_array_pair_list", "common_scalar_ae",
  "static_numeric_scalar_ae", "static_string_expr_ae", "static_scalar_ae",
  "static_scalar_ae_list", "static_array_pair_list_ae",
  "non_empty_static_array_pair_list_ae", "non_empty_static_scalar_list_ae",
  "static_shape_pair_list_ae", "non_empty_static_shape_pair_list_ae",
  "static_scalar_list_ae", "attribute_static_scalar_list",
  "non_empty_user_attribute_list", "user_attribute_list", "$@37",
  "non_empty_user_attributes", "optional_user_attributes",
  "object_operator", "object_property_name_no_variables",
  "object_property_name", "object_method_name_no_variables",
  "object_method_name", "array_access", "dimmable_variable_access",
  "dimmable_variable_no_calls_access", "object_property_access_on_expr",
  "object_property_access_on_expr_no_variables", "variable",
  "dimmable_variable", "callable_variable",
  "lambda_or_closure_with_parens", "lambda_or_closure",
  "object_method_call", "class_method_call", "variable_no_objects",
  "reference_variable", "compound_variable", "dim_offset",
  "variable_no_calls", "dimmable_variable_no_calls", "assignment_list",
  "array_pair_list", "non_empty_array_pair_list", "collection_init",
  "non_empty_collection_init", "static_collection_init",
  "non_empty_static_collection_init", "encaps_list", "encaps_var",
  "encaps_var_offset", "internal_functions", "variable_list",
  "class_constant", "hh_opt_constraint", "hh_type_alias_statement",
  "hh_name_with_type", "hh_constname_with_type", "hh_name_with_typevar",
  "hh_name_no_semireserved_with_typevar", "hh_typeargs_opt",
  "hh_non_empty_type_list", "hh_type_list", "hh_func_type_list",
  "opt_type_constraint_where_clause", "non_empty_constraint_list",
  "hh_generalised_constraint", "opt_return_type", "hh_constraint",
  "hh_typevar_list", "hh_non_empty_constraint_list",
  "hh_non_empty_typevar_list", "hh_typevar_variance",
  "hh_shape_member_type", "hh_non_empty_shape_member_list",
  "hh_shape_member_list", "hh_shape_type", "hh_access_type_start",
  "hh_access_type", "array_typelist", "hh_type", "hh_type_opt", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,    44,
     264,   265,   266,   267,    61,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,    63,    58,   284,   285,   286,   124,    94,    38,   287,
     288,   289,   290,    60,   291,    62,   292,   293,   294,   295,
      43,    45,    46,    42,    47,    37,    33,   296,   126,   297,
     298,   299,   300,   301,   302,   303,   304,   305,    64,   306,
      91,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   389,   390,   391,   392,   393,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,   405,
     406,   407,   408,   409,   410,   411,   412,   413,   414,   415,
     416,   417,   418,   419,   420,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,    40,    41,    59,   123,
     125,    36,    96,    93,    34,    39
};
# endif

#define YYPACT_NINF -1605

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1605)))

#define YYTABLE_NINF -1060

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1060)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1605,   147, -1605, -1605,  5990, 13907, 13907,   -30, 13907, 13907,
   13907, 13907,  5230, 13907, -1605, 13907, 13907, 13907, 13907, 16928,
   16928, 13907, 13907, 13907, 13907, 13907, 13907, 13907, 13907, 11877,
   17642, 13907,    -7,    10, -1605, -1605, -1605,   124, -1605,   235,
   -1605, -1605, -1605,   290, 13907, -1605,    10,   211,   216,   280,
   -1605,    10, 12080, 14479, 12283, -1605, 14937, 10862,   239, 13907,
   16218,    56,    85,   423,    65, -1605, -1605, -1605,   352,   363,
     372,   383, -1605, 14479,   402,   416,   546,   548,   561,   600,
     603, -1605, -1605, -1605, -1605, -1605, 13907,   399,  4074, -1605,
   -1605, 14479, -1605, -1605, -1605, -1605, 14479, -1605, 14479, -1605,
     468,   433, 14479, 14479, -1605,   212, -1605, -1605, 12486, -1605,
   -1605,   326,   563,   566,   566, -1605,   643,   524,   467,   504,
   -1605,    92, -1605,   666, -1605, -1605, -1605, -1605,  2668,   640,
   -1605, -1605,   536,   544,   554,   558,   576,   580,   582,   583,
    5157, -1605, -1605, -1605, -1605,   126,   650,   653,   674,   714,
     717, -1605,   718,   719, -1605,    64,   599, -1605,   639,    11,
   -1605,  1212,   143, -1605, -1605,  3000,   155,   605,   182, -1605,
     156,   164,   608,   186, -1605, -1605,   732, -1605,   646, -1605,
   -1605,   612,   654, -1605, 13907, -1605,   666,   640, 18062,  4747,
   18062, 13907, 18062, 18062, 15467, 15467,   619, 17097, 18062,   771,
   14479,   754,   754,   115,   754, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605,    43, 13907,   647, -1605, -1605,   664,
     642,   256,   662,   256,   754,   754,   754,   754,   754,   754,
     754,   754, 16928, 17145,   629,   851,   646, -1605, 13907,   647,
   -1605,   705, -1605,   715,   680, -1605,   157, -1605, -1605, -1605,
     256,   155, -1605, 12689, -1605, -1605, 13907,  9441,   869,    99,
   18062, 10456, -1605, 13907, 13907, 14479, -1605, -1605, 12064,   681,
   -1605, 12673, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605,  3794, -1605,  3794, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605,    96,    90,   654, -1605, -1605, -1605, -1605,   687,
    2595,    98, -1605, -1605,   724,   864, -1605,   731, 15656, -1605,
     694,   697, 13891, -1605,    59, 16129,  5435,  5435, 14327, 14479,
   14327,   699,   891,   703, -1605,    91, -1605, 16516,   102, -1605,
     893,   106,   775, -1605,   778, -1605, 16928, 13907, 13907,   712,
     737, -1605, -1605, 16619, 11877, 13907, 13907, 13907, 13907, 13907,
     107,   283,   274, -1605, 14110, 16928,   483, -1605, 14479, -1605,
     242,   524, -1605, -1605, -1605, -1605, 17742,   908,   823, -1605,
   -1605, -1605,    71, 13907,   734,   741, 18062,   745,  1210,   747,
    6193, 13907, -1605,   441,   727,   574,   441,   531,   464, -1605,
   14479,  3794,   749, 11065, 14937, -1605, -1605,  4347, -1605, -1605,
   -1605, -1605, -1605,   666, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, 13907, 13907, 13907, 13907, 12892, 13907, 13907,
   13907, 13907, 13907, 13907, 13907, 13907, 13907, 13907, 13907, 13907,
   13907, 13907, 13907, 13907, 13907, 13907, 13907, 13907, 13907, 13907,
   13907, 17842, 13907, -1605, 13907, 13907, 13907, 14281, 14479, 14479,
   14479, 14479, 14479,  2668,   835,   594, 10659, 13907, 13907, 13907,
   13907, 13907, 13907, 13907, 13907, 13907, 13907, 13907, 13907, -1605,
   -1605, -1605, -1605,  2525, 13907, 13907, -1605, 11065, 11065, 13907,
   13907, 16619,   751,   666, 13095, 16177, -1605, 13907, -1605,   752,
     942,   796,   757,   760, 14433,   256, 13298, -1605, 13501, -1605,
     680,   763,   766,  1840, -1605,    77, 11065, -1605,  2769, -1605,
   -1605, 16225, -1605, -1605, 11268, -1605, 13907, -1605,   870,  9644,
     958,   770, 14094,   963,    86,    68, -1605, -1605, -1605,   797,
   -1605, -1605, -1605,  3794, -1605,  3030,   784,   974,  4552, 14479,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,   798,
   -1605, -1605,   801,   799,   804,   806, 14479,   808,   381,   388,
     809, 16294, 14327, -1605, -1605, 14479, 14479, 13907,   256,    56,
   -1605,  4552,   927, -1605, -1605, -1605,   256,   121,   128,   816,
     817,  2405,   180,   821,   822,   525,   889,   826,   256,   131,
     827, 17201,   825,  1016,  1020,   829,   831,   833,   834, -1605,
    4706, 14479, -1605, -1605,   969,  3043,    16, -1605, -1605, -1605,
     524, -1605, -1605, -1605,  1010,   910,   865,   249,   887, 13907,
     912,  1042,   856, -1605,   896, -1605,   165, -1605,  3794,  3794,
    1043,   869,    71, -1605,   871,  1053, -1605,  3794,    94, -1605,
     472,   144, -1605, -1605, -1605, -1605, -1605, -1605, -1605,   750,
    3427, -1605, -1605, -1605, -1605,  1054,   884, -1605, 16928, 13907,
     873,  1058, 18062,  1057, -1605, -1605,   940,  4807, 12268, 18245,
   15467, 14762, 13907, 18014, 14938, 11856, 13276, 13681, 12868, 15283,
   17823, 17823, 17823, 17823,  1936,  1936,  1936,  1936,  1936,   936,
     936,   772,   772,   772,   115,   115,   115, -1605,   754, 18062,
     875,   876, 17249,   868,  1064,    -6, 13907,    51,   647,   142,
   -1605, -1605, -1605,  1067,   823, -1605,   666, 16722, -1605, -1605,
   -1605, 15467, 15467, 15467, 15467, 15467, 15467, 15467, 15467, 15467,
   15467, 15467, 15467, 15467, -1605, 13907,   207, -1605,   166, -1605,
     647,   217,   882,  3704,   892,   894,   883,  4090,   132,   897,
   -1605, 18062, 16475, -1605, 14479, -1605,    94,    74, 16928, 18062,
   16928, 17305,   940,    94,   256,   168, -1605,   165,   930,   899,
   13907, -1605,   170, -1605, -1605, -1605,  9238,   311, -1605, -1605,
   18062, 18062,    10, -1605, -1605, -1605, 13907,   996, 16372,  4552,
   14479,  9847,   902,   903, -1605,  1095,  4906,   970, -1605,   945,
   -1605,  1098,   911,  3191,  3794,  4552,  4552,  4552,  4552,  4552,
     913,  1059,  1062,  1063,  1068,  1069,   917,  4552,    20, -1605,
   -1605, -1605, -1605, -1605, -1605,    -2, -1605, 18156, -1605, -1605,
      22, -1605,  6396,  5077,   928, 14327, -1605, 14327, -1605,   462,
   -1605, 14479, 14479, -1605, 14327, 14327, -1605,  1119,   934, -1605,
   -1605, -1605,  4426, -1605, 18156,  1122, 16928,   939, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605,   959,  1131, 14479,
    5077,   943, 16619, 16825,  1132, -1605, 13907, -1605, 13907, -1605,
   13907, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,   944,
   -1605, 13907, -1605, -1605,  5584, -1605,  3794,  5077,   951, -1605,
   -1605, -1605, -1605,  1143,   961, 13907, 17742, -1605, -1605, 14281,
     964, -1605,  3794, -1605,   966,  6599,  1136,    83, -1605, -1605,
      80,  2525, -1605,  2769, -1605,  3794, -1605, -1605,   256, 18062,
   -1605, 11471, -1605,  4552,    69,   967,  5077,   910, -1605, -1605,
   14938, 13907, -1605, -1605, 13907, -1605, 13907, -1605,  4543,   973,
   11065,   889,  1139,   910,  3794,  1159,   940, 14479, 17842,   256,
    4983,   978, -1605, -1605,   153,   979, -1605, -1605,  1162,  1506,
    1506, 16475, -1605, -1605, -1605,  1125,   982,  1109,  1111,  1113,
    1115,  1123,    63,   998,   407, -1605, -1605, -1605, -1605, -1605,
    1036, -1605, -1605, -1605, -1605,  1190,  1003,   752,   256,   256,
   13704,   910,  2769, -1605, -1605,  5084,   620,    10, 10456, -1605,
    6802,  1004,  7005,  1006, 16372, 16928,  1005,  1072,   256, 18156,
    1195, -1605, -1605, -1605, -1605,   456, -1605,   288,  3794,  1028,
    1075,  1052,  3794, 14479,  3351, -1605, -1605, -1605,  1206, -1605,
    1019,  1054,   630,   630,  1149,  1149,  4358,  1017,  1213,  4552,
    4552,  4552,  4552,  4552,  4552, 17742,  2546, 15808,  4552,  4552,
    4552,  4552, 16253,  4552,  4552,  4552,  4552,  4552,  4552,  4552,
    4552,  4552,  4552,  4552,  4552,  4552,  4552,  4552,  4552,  4552,
    4552,  4552,  4552,  4552,  4552,  4552, 14479, -1605, -1605,  1138,
   -1605, -1605,  1037,  1039, -1605, -1605, -1605, 16294, -1605,  1021,
   -1605,  4552,   256, -1605, -1605,   154, -1605,   559,  1231, -1605,
   -1605,   133,  1046,   256, 11674, 18062, 17353, -1605,  2444, -1605,
    5787,   823,  1231, -1605,    12,   226, -1605, 18062,  1107,  1049,
   -1605,  1048,  1136, -1605,  3794,   869,  3794,    72,  1233,  1165,
     171, -1605,   647,   172, -1605, -1605, 16928, 13907, 18062, 18156,
    1060,    69, -1605,  1050,    69,  1055, 14938, 18062, 17409,  1065,
   11065,  1071,  1051,  3794,  1061,  1077,  3794,   910, -1605,   680,
     219, 11065, 13907, -1605, -1605, -1605, -1605, -1605, -1605,  1120,
    1056,  1253,  1172, 16475, 16475, 16475, 16475, 16475, 16475,  1108,
   -1605, 17742,   266, 16475, -1605, -1605, -1605, 16928, 18062,  1074,
   -1605,    10,  1232,  1189, 10456, -1605, -1605, -1605,  1079, 13907,
    1072,   256, 16619, 16372,  1082,  4552,  7208,   586,  1083, 13907,
      70,   339, -1605,  1101, -1605,  3794, 14479, -1605,  1148, -1605,
   -1605,  3470,  1254,  1090,  4552, -1605,  4552, -1605,  1091,  1092,
    1287, 17457,  1094, 18156,  1290,  1100,  1102,  1105,  1168,  1292,
    1112, -1605, -1605, -1605, 17512,  1110,  1302, 18201, 18289, 11045,
    4552, 18110, 13074, 13479, 14279, 15107, 16453, 16700, 16700, 16700,
   16700,  1843,  1843,  1843,  1843,  1843,  1066,  1066,   630,   630,
     630,  1149,  1149,  1149,  1149, -1605,  1117, -1605,  1124,  1126,
   -1605, -1605, 18156, 14479,  3794,  3794, -1605,   559,  5077,  1679,
   -1605, 16619, -1605, -1605, 15467, 13907,  1118, -1605,  1116,  1713,
   -1605,   100, 13907, -1605, -1605, -1605, 13907, -1605, 13907, -1605,
     869, -1605, -1605,   105,  1303,  1237, 13907, -1605,  1127,   256,
   18062,  1136,  1128, -1605,  1129,    69, 13907, 11065,  1130, -1605,
   -1605,   823, -1605, -1605,  1140,  1142,  1133, -1605,  1151, 16475,
   -1605, 16475, -1605, -1605,  1152,  1147,  1312,  1203,  1163, -1605,
    1337,  1166,  1167,  1174, -1605,  1216,  1158,  1356, -1605, -1605,
     256, -1605,  1336, -1605,  1177, -1605, -1605,  1181,  1191,   136,
   -1605, -1605, 18156,  1188,  1192, -1605,  4307, -1605, -1605, -1605,
   -1605, -1605, -1605,  1256,  3794, -1605,  3794, -1605, 18156, 17560,
   -1605, -1605,  4552, -1605,  4552, -1605,  4552, -1605, -1605, -1605,
   -1605,  4552, 17742, -1605, -1605,  4552, -1605,  4552, -1605, 11451,
    4552,  1194,  7411, -1605, -1605,   559, -1605, -1605, -1605, -1605,
     577, 15113,  5077,  1282, -1605,  4137,  1226,  4535, -1605, -1605,
   -1605,   835,   672,   111,   112,  1199,   823,   594,   137, 18062,
   -1605, -1605, -1605,  1234,  5441,  5516, 18062, -1605,    79,  1388,
    1322, 13907, -1605, 18062, 11065,  1291,  1136,  2040,  1136,  1215,
   18062,  1218, -1605,  2055,  1219,  2075, -1605, -1605,    69, -1605,
   -1605,  1273, -1605, -1605, 16475, -1605, 16475, -1605, 16475, -1605,
   -1605, -1605, -1605, 16475, -1605, 17742, -1605,  2211, -1605,  9238,
   -1605, -1605, -1605, -1605, 10050, -1605, -1605, -1605,  9238,  3794,
   -1605,  1220,  4552, 17615, 18156, 18156, 18156,  1283, 18156, 17663,
   11451, -1605, -1605,   559,  5077,  5077, 14479, -1605,  1406, 15960,
      88, -1605, 15113,   823, 15550, -1605,  1240, -1605,   113,  1223,
     114, -1605, 15466, -1605, -1605, -1605,   116, -1605, -1605,  3711,
   -1605,  1225, -1605,  1341,   666, -1605, 15290, -1605, 15290, -1605,
   -1605,  1411,   835, -1605, 14585, -1605, -1605, -1605, -1605,  1412,
    1344, 13907, -1605, 18062,  1239,  1238,  1136,   538, -1605,  1291,
    1136, -1605, -1605, -1605, -1605,  2246,  1243, 16475,  1305, -1605,
   -1605, -1605,  1306, -1605,  9238, 10253, 10050, -1605, -1605, -1605,
    9238, -1605, -1605, 18156,  4552,  4552,  4552,  7614,  1230,  1246,
   -1605,  4552, -1605,  5077, -1605, -1605, -1605, -1605, -1605,  3794,
    1211,  4137, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605,   151, -1605,  1226, -1605, -1605, -1605,
   -1605, -1605,   123,   627, -1605,  1432,   117, 15656,  1341,  1433,
   -1605,  3794,   666, -1605, -1605,  1250,  1438, 13907, -1605, 18062,
   -1605,   336,  1257, -1605, -1605, -1605,  1136,   538, 14761, -1605,
    1136, -1605, 16475, 16475, -1605, -1605, -1605, -1605,  7817, 18156,
   18156, 18156, -1605, -1605, -1605, 18156, -1605,  3724,  1444,  1446,
    1258, -1605, -1605,  4552, 15466, 15466,  1389, -1605,  3711,  3711,
     628, -1605, -1605, -1605,  4552,  1378, -1605,  1281,  1267,   118,
    4552, -1605, 14479, -1605,  4552, 18062,  1382, -1605,  1457, -1605,
    8020,  1268, -1605, -1605,   538, -1605, -1605,  8223,  1270,  1355,
   -1605,  1369,  1313, -1605, -1605,  1370,  3794,  1293,  1211, -1605,
   -1605, 18156, -1605, -1605,  1304, -1605,  1441, -1605, -1605, -1605,
   -1605, 18156,  1464,   525, -1605, -1605, 18156,  1285, 18156, -1605,
     364,  1286,  8426, -1605, -1605, -1605,  1288, -1605,  1289,  1309,
   14479,   594,  1308, -1605, -1605, -1605,  4552,  1319,   101, -1605,
    1401, -1605, -1605, -1605,  8629, -1605,  5077,   928, -1605,  1320,
   14479,   726, -1605, 18156, -1605,  1297,  1479,   606,   101, -1605,
   -1605,  1419, -1605,  5077,  1310, -1605,  1136,   104, -1605, -1605,
   -1605, -1605,  3794, -1605,  1314,  1315,   119, -1605,  1311,   606,
     139,  1136,  1316, -1605,  3794,   556,  3794,    87,  1489,  1424,
    1311, -1605,  1500, -1605,   522, -1605, -1605, -1605,   161,  1503,
    1436, 13907, -1605,   556,  8832,  3794, -1605,  3794, -1605,  9035,
     261,  1507,  1437, 13907, -1605, 18062, -1605, -1605, -1605, -1605,
   -1605,  1508,  1442, 13907, -1605, 18062, 13907, -1605, 18062, 18062
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,     0,   436,     0,   865,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   956,
     944,     0,   731,     0,   737,   738,   739,    27,   803,   932,
     933,   160,   161,   740,     0,   141,     0,     0,     0,     0,
      28,     0,     0,     0,     0,   195,     0,     0,     0,     0,
       0,     0,   405,   406,   407,   404,   403,   402,     0,     0,
       0,     0,   224,     0,     0,     0,    35,    36,    38,    39,
      37,   744,   746,   747,   741,   742,     0,     0,     0,   748,
     743,     0,   714,    30,    31,    32,    34,    33,     0,   745,
       0,     0,     0,     0,   749,   408,   543,    29,     0,   159,
     131,     0,   732,     0,     0,     4,   121,   123,   802,     0,
     713,     0,     6,   194,     7,     9,     8,    10,     0,     0,
     400,   449,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   447,   921,   922,   525,   519,   520,   521,   522,   523,
     524,   430,   528,     0,   429,   892,   715,   722,     0,   805,
     518,   399,   895,   896,   907,   448,     0,     0,   451,   450,
     893,   894,   891,   928,   931,   508,   804,    11,   405,   406,
     407,     0,     0,    34,     0,   121,   194,     0,   996,   448,
     997,     0,   999,  1000,   527,   444,     0,   437,   442,     0,
       0,   490,   491,   492,   493,    27,   932,   740,   717,    35,
      36,    38,    39,    37,     0,     0,  1020,   914,   715,     0,
     716,   469,     0,   471,   509,   510,   511,   512,   513,   514,
     515,   517,     0,   960,     0,   812,   727,   214,     0,  1020,
     427,   726,   720,     0,   736,   716,   939,   940,   946,   938,
     728,     0,   428,     0,   730,   516,     0,     0,     0,     0,
     433,     0,   139,   435,     0,     0,   145,   147,     0,     0,
     149,     0,    73,    74,    79,    80,    65,    66,    57,    77,
      88,    89,     0,    60,     0,    64,    72,    70,    91,    83,
      82,    55,    78,    98,    99,    56,    94,    53,    95,    54,
      96,    52,   100,    87,    92,    97,    84,    85,    59,    86,
      90,    51,    81,    67,   101,    75,    68,    58,    45,    46,
      47,    48,    49,    50,    69,   103,   102,   105,    62,    43,
      44,    71,  1067,  1068,    63,  1072,    42,    61,    93,     0,
       0,   121,   104,  1011,  1066,     0,  1069,     0,     0,   151,
       0,     0,     0,   185,     0,     0,     0,     0,     0,     0,
       0,     0,   814,     0,   109,   111,   313,     0,     0,   312,
     318,     0,     0,   225,     0,   228,     0,     0,     0,     0,
    1017,   210,   222,   952,   956,   562,   589,   589,   562,   589,
       0,   981,     0,   751,     0,     0,     0,   979,     0,    16,
       0,   125,   202,   216,   223,   619,   555,     0,  1005,   535,
     537,   539,   869,   436,   449,     0,     0,   447,   448,   450,
       0,     0,   935,   733,     0,   734,     0,     0,     0,   184,
       0,     0,   127,   304,     0,    26,   193,     0,   221,   206,
     220,   405,   408,   194,   401,   174,   175,   176,   177,   178,
     180,   181,   183,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   944,     0,   173,   937,   937,   966,     0,     0,     0,
       0,     0,     0,     0,     0,   398,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   468,
     470,   870,   871,     0,   937,     0,   883,   304,   304,   937,
       0,   952,     0,   194,     0,     0,   153,     0,   867,   862,
     812,     0,   449,   447,     0,   964,     0,   560,   811,   955,
     736,   449,   447,   448,   127,     0,   304,   426,     0,   885,
     729,     0,   131,   264,     0,   542,     0,   156,     0,     0,
     434,     0,     0,     0,     0,     0,   148,   172,   150,  1067,
    1068,  1064,  1065,     0,  1071,  1057,     0,     0,     0,     0,
      76,    41,    63,    40,  1012,   179,   182,   152,   131,     0,
     169,   171,     0,     0,     0,     0,     0,     0,   111,   112,
       0,     0,   813,   110,    18,     0,   106,     0,   314,     0,
     154,     0,     0,   155,   226,   227,  1001,     0,     0,   449,
     447,   448,   451,   450,     0,  1047,   234,     0,   953,     0,
       0,     0,     0,   812,   812,     0,     0,     0,     0,   157,
       0,     0,   750,   980,   803,     0,     0,   978,   808,   977,
     124,     5,    13,    14,     0,   232,     0,     0,   548,     0,
       0,   812,     0,   724,     0,   723,   718,   549,     0,     0,
       0,     0,   869,   131,     0,   814,   868,  1076,   425,   439,
     504,   901,   920,   136,   130,   132,   133,   134,   135,   399,
       0,   526,   806,   807,   122,   812,     0,  1021,     0,     0,
       0,   814,   305,     0,   531,   196,   230,     0,   474,   476,
     475,   487,     0,     0,   507,   472,   473,   477,   479,   478,
     496,   497,   494,   495,   498,   499,   500,   501,   502,   488,
     489,   481,   482,   480,   483,   484,   486,   503,   485,   936,
       0,     0,   970,     0,   812,  1004,     0,  1003,  1020,   898,
     212,   204,   218,     0,  1005,   208,   194,     0,   440,   443,
     445,   453,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   873,     0,   872,   875,   897,   879,
    1020,   876,     0,     0,     0,     0,     0,     0,     0,     0,
     998,   438,   860,   864,   811,   866,     0,   719,     0,   959,
       0,   958,   230,     0,   719,   943,   942,   928,   931,     0,
       0,   872,   875,   941,   876,   431,   266,   268,   131,   546,
     545,   432,     0,   131,   248,   140,   435,     0,     0,     0,
       0,     0,   260,   260,   146,   812,     0,     0,  1056,     0,
    1053,   812,     0,  1027,     0,     0,     0,     0,     0,   810,
       0,    35,    36,    38,    39,    37,     0,     0,   753,   757,
     758,   759,   760,   761,   763,     0,   752,   129,   801,   762,
    1020,  1070,     0,     0,     0,     0,    21,     0,    22,   112,
      19,     0,   107,    20,     0,     0,   118,   814,     0,   116,
     108,   113,     0,   311,   319,   316,     0,     0,   990,   995,
     992,   991,   994,   993,    12,  1045,  1046,     0,   812,     0,
       0,     0,   952,   949,     0,   559,     0,   573,   811,   561,
     811,   588,   576,   582,   585,   579,   989,   988,   987,     0,
     983,     0,   984,   986,     0,     5,     0,     0,     0,   613,
     614,   622,   621,     0,   447,     0,   811,   554,   558,     0,
       0,  1006,     0,   536,     0,     0,  1034,   869,   290,  1075,
       0,     0,   884,     0,   934,   811,  1023,  1019,   306,   307,
     712,   813,   303,     0,   869,     0,     0,   232,   533,   198,
     506,     0,   596,   597,     0,   594,   811,   965,     0,     0,
     304,   234,     0,   232,     0,     0,   230,     0,   944,   454,
       0,     0,   881,   882,   899,   900,   929,   930,     0,     0,
       0,   848,   819,   820,   821,   828,     0,    35,    36,    38,
      39,    37,     0,     0,   834,   840,   841,   842,   843,   844,
       0,   832,   830,   831,   854,   812,     0,   862,   963,   962,
       0,   232,     0,   886,   735,     0,   270,     0,     0,   137,
       0,     0,     0,     0,     0,     0,     0,   240,   241,   252,
       0,   131,   250,   166,   260,     0,   260,     0,   811,     0,
       0,     0,     0,     0,   811,  1055,  1058,  1026,   812,  1025,
       0,   812,   784,   785,   782,   783,   818,     0,   812,   810,
     566,   591,   591,   566,   591,   557,     0,     0,   972,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1061,   186,     0,
     189,   170,     0,     0,   114,   119,   120,   813,   117,     0,
     315,     0,  1002,   158,  1018,  1047,  1038,  1042,   233,   235,
     325,     0,     0,   950,     0,   564,     0,   982,     0,    17,
       0,  1005,   231,   325,     0,     0,   719,   551,     0,   725,
    1007,     0,  1034,   540,     0,     0,  1076,     0,   295,   293,
     875,   887,  1020,   875,   888,  1022,     0,     0,   308,   128,
       0,   869,   229,     0,   869,     0,   505,   969,   968,     0,
     304,     0,     0,     0,     0,     0,     0,   232,   200,   736,
     874,   304,     0,   824,   825,   826,   827,   835,   836,   852,
       0,   812,     0,   848,   570,   593,   593,   570,   593,     0,
     823,   856,     0,   811,   859,   861,   863,     0,   957,     0,
     874,     0,     0,     0,     0,   267,   547,   142,     0,   435,
     240,   242,   952,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   254,     0,  1062,     0,     0,  1048,     0,  1054,
    1052,   811,     0,     0,     0,   755,   811,   809,     0,     0,
     812,     0,     0,   798,   812,     0,     0,     0,     0,   812,
       0,   764,   799,   800,   976,     0,   812,   767,   769,   768,
       0,     0,   765,   766,   770,   772,   771,   788,   789,   786,
     787,   790,   791,   792,   793,   794,   779,   780,   774,   775,
     773,   776,   777,   778,   781,  1060,     0,   131,     0,     0,
     115,    23,   317,     0,     0,     0,  1039,  1044,     0,   399,
     954,   952,   441,   446,   452,     0,     0,    15,     0,   399,
     625,     0,     0,   627,   620,   623,     0,   618,     0,  1009,
       0,  1035,   544,     0,   296,     0,     0,   291,     0,   310,
     309,  1034,     0,   325,     0,   869,     0,   304,     0,   926,
     325,  1005,   325,  1008,     0,     0,     0,   455,     0,     0,
     838,   811,   847,   829,     0,     0,   812,     0,     0,   846,
     812,     0,     0,     0,   822,     0,     0,   812,   833,   853,
     961,   325,     0,   131,     0,   263,   249,     0,     0,     0,
     239,   162,   253,     0,     0,   256,     0,   261,   262,   131,
     255,  1063,  1049,     0,     0,  1024,     0,  1074,   817,   816,
     754,   574,   811,   565,     0,   577,   811,   590,   583,   586,
     580,     0,   811,   556,   756,     0,   595,   811,   971,   796,
       0,     0,     0,    24,    25,  1041,  1036,  1037,  1040,   236,
       0,     0,     0,   406,   397,     0,     0,     0,   211,   324,
     326,     0,   396,     0,     0,     0,  1005,   399,     0,   563,
     985,   321,   217,   616,     0,     0,   550,   538,     0,   299,
     289,     0,   292,   298,   304,   530,  1034,   399,  1034,     0,
     967,     0,   925,   399,     0,   399,  1010,   325,   869,   923,
     851,   850,   837,   575,   811,   569,     0,   578,   811,   592,
     584,   587,   581,     0,   839,   811,   855,   399,   131,   269,
     138,   143,   164,   243,     0,   251,   257,   131,   259,     0,
    1050,     0,     0,     0,   568,   797,   553,     0,   975,   974,
     795,   131,   190,  1043,     0,     0,     0,  1013,     0,     0,
       0,   237,     0,  1005,     0,   362,   358,   364,   714,    34,
       0,   352,     0,   357,   361,   374,     0,   372,   377,     0,
     376,     0,   375,     0,   194,   328,     0,   330,     0,   331,
     332,     0,     0,   951,     0,   617,   615,   626,   624,   300,
       0,     0,   287,   297,     0,     0,  1034,     0,   207,   530,
    1034,   927,   213,   321,   219,   399,     0,     0,     0,   572,
     845,   858,     0,   215,   265,     0,     0,   131,   246,   163,
     258,  1051,  1073,   815,     0,     0,     0,     0,     0,     0,
     424,     0,  1014,     0,   342,   346,   421,   422,   356,     0,
       0,     0,   337,   675,   676,   674,   677,   678,   695,   697,
     696,   666,   638,   636,   637,   656,   671,   672,   632,   643,
     644,   646,   645,   665,   649,   647,   648,   650,   651,   652,
     653,   654,   655,   657,   658,   659,   660,   661,   662,   664,
     663,   633,   634,   635,   639,   640,   642,   680,   681,   685,
     686,   687,   688,   689,   690,   673,   692,   682,   683,   684,
     667,   668,   669,   670,   693,   694,   698,   700,   699,   701,
     702,   679,   704,   703,   706,   708,   707,   641,   711,   709,
     710,   705,   691,   631,   369,   628,     0,   338,   390,   391,
     389,   382,     0,   383,   339,   416,     0,     0,     0,     0,
     420,     0,   194,   203,   320,     0,     0,     0,   288,   302,
     924,     0,     0,   392,   131,   197,  1034,     0,     0,   209,
    1034,   849,     0,     0,   131,   244,   144,   165,     0,   567,
     552,   973,   188,   340,   341,   419,   238,     0,   812,   812,
       0,   365,   353,     0,     0,     0,   371,   373,     0,     0,
     378,   385,   386,   384,     0,     0,   327,  1015,     0,     0,
       0,   423,     0,   322,     0,   301,     0,   611,   814,   131,
       0,     0,   199,   205,     0,   571,   857,     0,     0,   167,
     343,   121,     0,   344,   345,     0,   811,     0,   811,   367,
     363,   368,   629,   630,     0,   354,   387,   388,   380,   381,
     379,   417,   414,  1047,   333,   329,   418,     0,   323,   612,
     813,     0,     0,   393,   131,   201,     0,   247,     0,   192,
       0,   399,     0,   359,   366,   370,     0,     0,   869,   335,
       0,   609,   529,   532,     0,   245,     0,     0,   168,   350,
       0,   398,   360,   415,  1016,     0,   814,   410,   869,   610,
     534,     0,   191,     0,     0,   349,  1034,   869,   274,   413,
     412,   411,  1076,   409,     0,     0,     0,   348,  1028,   410,
       0,  1034,     0,   347,     0,     0,  1076,     0,   279,   277,
    1028,   131,   814,  1030,     0,   394,   131,   334,     0,   280,
       0,     0,   275,     0,     0,   813,  1029,     0,  1033,     0,
       0,   283,   273,     0,   276,   282,   336,   187,  1031,  1032,
     395,   284,     0,     0,   271,   281,     0,   272,   286,   285
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1605, -1605, -1605,  -590, -1605, -1605, -1605,   482,     0,   -41,
     409, -1605,  -274,  -526, -1605, -1605,   389,   529,  1775, -1605,
    2077, -1605,  -492, -1605,    28, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605,  -370, -1605, -1605,  -161,
     140,    24, -1605, -1605, -1605, -1605, -1605, -1605,    27, -1605,
   -1605, -1605, -1605, -1605, -1605,    31, -1605, -1605,  1038,  1041,
    1045,   -84,  -704,  -884,   542,   601,  -377,   287,  -962, -1605,
     -96, -1605, -1605, -1605, -1605,  -745,   122, -1605, -1605, -1605,
   -1605,  -367, -1605,  -608, -1605,  -473, -1605, -1605,   933, -1605,
     -80, -1605, -1605, -1074, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605,  -115, -1605,   -27, -1605, -1605, -1605,
   -1605, -1605,  -197, -1605,    73, -1040, -1605, -1604,  -402, -1605,
    -159,   120,  -120,  -376, -1605,  -206, -1605, -1605, -1605,    84,
     -17,    -3,    50,  -741,   -70, -1605, -1605,    30, -1605,   -11,
   -1605, -1605,    -5,   -46,   -63, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605,  -629,  -880, -1605, -1605, -1605, -1605,
   -1605,   632,  1170, -1605,   466, -1605,   333, -1605, -1605, -1605,
   -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1605, -1605, -1605,    55,  -502,  -632, -1605, -1605, -1605, -1605,
   -1605,   397, -1605, -1605, -1605, -1605, -1605, -1605, -1605, -1605,
   -1033,  -349,  2974,    35, -1605,   198,  -407, -1605, -1605,  -504,
    3813,  3715, -1605,  -372, -1605, -1605,   473,  -129,  -648, -1605,
   -1605,   553,   342,   -10, -1605,   343, -1605, -1605, -1605, -1605,
   -1605,   528, -1605, -1605, -1605,   125,  -906,  -160,  -435,  -434,
   -1605,   607,  -116, -1605, -1605,    40,    42,   682, -1605, -1605,
     252,   -28, -1605,  -338,    25,  -369,   163,   160, -1605, -1605,
    -507,  1183, -1605, -1605, -1605, -1605, -1605,   636,   507, -1605,
   -1605, -1605,  -337,  -683, -1605,  1137, -1323, -1605,   -69,  -198,
     -67,   730, -1605,  -366, -1605,  -385, -1109, -1279,  -288,   127,
   -1605,   431,   503, -1605, -1605, -1605, -1605,   454, -1605,  1861,
   -1136
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   115,   935,   651,   185,  1567,   748,
     361,   362,   363,   364,   887,   888,   889,   117,   118,   119,
     120,   121,   420,   684,   685,   559,   261,  1635,   565,  1544,
    1636,  1879,   874,   354,   588,  1839,  1131,  1327,  1898,   437,
     186,   686,   975,  1195,  1386,   125,   654,   992,   687,   706,
     996,   626,   991,   240,   540,   688,   655,   993,   439,   381,
     403,   128,   977,   938,   911,  1148,  1570,  1254,  1057,  1786,
    1639,   825,  1063,   564,   834,  1065,  1429,   817,  1046,  1049,
    1243,  1905,  1906,   674,   675,   700,   701,   368,   369,   371,
    1604,  1764,  1765,  1339,  1479,  1593,  1758,  1888,  1908,  1797,
    1843,  1844,  1845,  1580,  1581,  1582,  1583,  1799,  1800,  1806,
    1855,  1586,  1587,  1591,  1751,  1752,  1753,  1775,  1947,  1480,
    1481,   187,   130,  1922,  1923,  1756,  1483,  1484,  1485,  1486,
     131,   254,   560,   561,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,  1616,   142,   974,  1194,   143,   671,
     672,   673,   258,   412,   555,   660,   661,  1289,   662,  1290,
     144,   145,   632,   633,  1279,  1280,  1395,  1396,   146,   859,
    1025,   147,   860,  1026,   148,   861,  1027,   149,   862,  1028,
     150,   863,  1029,   635,  1282,  1398,   151,   864,   152,   153,
    1828,   154,   656,  1606,   657,  1164,   943,  1357,  1354,  1744,
    1745,   155,   156,   157,   243,   158,   244,   255,   424,   547,
     159,  1283,  1284,   868,   869,   160,  1087,   966,   603,  1088,
    1032,  1217,  1033,  1399,  1400,  1220,  1221,  1035,  1406,  1407,
    1036,   793,   530,   199,   200,   689,   677,   513,  1180,  1181,
     779,   780,   962,   162,   246,   163,   164,   189,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   740,   250,   251,
     629,   234,   235,   743,   744,  1295,  1296,   396,   397,   929,
     175,   617,   176,   670,   177,   345,  1766,  1818,   382,   432,
     695,   696,  1080,  1935,  1942,  1943,  1175,  1336,   907,  1337,
     908,   909,   840,   841,   842,   346,   347,   871,   574,  1569,
     960
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     188,   190,   494,   192,   193,   194,   195,   197,   198,   444,
     201,   202,   203,   204,   788,   343,   224,   225,   226,   227,
     228,   229,   230,   231,   233,   522,   252,   958,   124,   404,
     257,   126,   122,   407,   408,   127,   802,   666,   415,   260,
    1363,   544,   953,   262,   784,   785,   351,   268,   266,   271,
     516,  1176,   352,   972,   355,   249,   342,   663,  1468,   440,
     816,   934,   417,  1360,   954,   242,  1168,   444,   665,   667,
     247,   995,   248,   809,   737,   886,   890,   493,   777,   778,
     419,   260,   593,   595,   597,  1053,   600,   350,  1067,  1349,
     548,   414,  1250,  1193,   259,   830,   872,  1653,  1041,   -41,
     832,   434,  1427,   416,   -41,   -76,   539,   -40,   556,  1204,
     -76,   609,   -40,   812,   813,   612,   556,    14,  1177,    14,
    1596,  1598,  -355,  1661,   129,  1746,  1815,  1815,  1653,   161,
     896,    14,  1808,   531,  -902,   549,   390,   556,   417,   366,
     913,   913,   913,  1498,   123,   913,   913,     3,  1568,    14,
     511,   512,    14,  1229,  1493,  1364,   419,  1239,  1097,  1809,
     206,    40,  1609,  1178,   514,  1803,   191,   414,   370,   533,
    1949,  -716,   481,  1832,   589,  -105,   807,  1937,  1350,   416,
   -1020,   955,  1126,  1804,   482,   374,   525,   419,  1499,   253,
    -105,  1351,   542,   605,  -608,   375,  -598,  1098,   532,  1960,
     393,   431,  1805,   431,   905,   906,   256,  -915,   511,   512,
     416,  1352,  -910,  -905,  -909,  1288,   933,   219,   219, -1020,
    1230,   541,  1938,  -908,  -724,   514,  -903,  -945,   511,   512,
    1875,   641,  -104,   416,  -904,   519,  -906,   590,  -948,  1138,
    -947,  -889,  -890,  -541,  1961,  -717,  1365,  -104,   551,  1656,
    -605,   551,  -605,  1610,  1179,   606,   519,   367,   260,   562,
    -913,  1950,  1505,  -902,  -294,   405,  -294,   833,  1428,   443,
    -917,   222,   222,  1759,   422,  1760,  -813,   573,   111,  1500,
    -813,   495,   707,   831,  1468,   553,  1654,  1655,   -41,   558,
     435,  1420,  1207,   515,   -76,   409,   -40,   557,  -278,  1507,
     610,  -813,  -725,   940,   613,   639,  1513,   584,  1515,  1597,
    1599,  -355,  1662,  1939,  1747,  1816,  1865,  1933,   897,  1257,
    1810,  1261,  -914,  1385,  -723,   898,  1050,   523,   914,  1008,
    1340,  1052,   620,  1543,  1603,  1962,  -811,  1537,   990,  -912,
    -916,  -910,  -905,  -909,  1971,  1160,  -718,  1408,   342,  -919,
     623,  1405,  -908,   640,   515,  -903,  -945,   391,  1135,  1136,
    -606,   619,   789,  -904,   520,  -906,  1190,  -948,   705,  -947,
    -889,  -890,   260,   416,  1826,   444,   518,   902,   518,   233,
     631,   260,   260,   631,   260,   520,  1047,  1048,  -877,   645,
     511,   512,  1355,   343,  1259,  1260,  1292,  1617,  -880,  1619,
    -878,   795,  1890,  -877,   410,  1151,   430,   263,   197,   206,
      40,   411,   264,  -880,   941,  -878,   690,   511,   512,  1827,
    1031,   404,   753,   754,   440,  1356,   758,   618,   702,   942,
     219,   642,   394,   395,   342,  1972,   634,   634,   353,   634,
     652,   653,   636,  1625,   638,  1259,  1260,  1891,   708,   709,
     710,   711,   713,   714,   715,   716,   717,   718,   719,   720,
     721,   722,   723,   724,   725,   726,   727,   728,   729,   730,
     731,   732,   733,   734,   735,   736,   265,   738,  1348,   739,
     739,   742,   391,   605,   222,   760,   116,   747,  1262,   392,
     881,   761,   762,   763,   764,   765,   766,   767,   768,   769,
     770,   771,   772,   773,   919,   921,   249,  1772,  1417,   739,
     783,  1777,   702,   702,   739,   787,   242,  1201,   961,   761,
     963,   247,   791,   248,   391,   421,   759,   111,  1183,  1184,
     494,   799,   947,   801,   342,   269,  1957,   676,   341,  1430,
     129,   702,  -607,   819,   372,   430,  1362,   391,   376,   820,
     989,   821,   882,   373,   647,   380,   393,   394,   395,   377,
     123,  1258,  1259,  1260,   881,   219,   391,  -121,   378,  1256,
     806,  -121,  1557,   647,   219,   905,   906,   666,   402,   379,
     380,   219,  1001,  1372,   380,   380,  1374,   824,  -121,   365,
    1285,  1209,  1287,   219,  1401,   997,  1403,   663,   383,   394,
     395,  1132,   892,  1133,   664,   493,   511,   512,   665,   667,
     380,   886,   384,   944,   391,   987,   385,   400,   386,   222,
     401,   647,   394,   395,  1334,  1335,   430, -1020,   222,   406,
     622,   387,  -719,   405,   756,   222,   961,   963,   979,  1031,
     648,   394,   395,  1042,   963,   741,   391,   222,   431,   391,
     749,   217,   217,   423,   416,  1632,   426,   391,  1811,  1858,
     544,  1334,  1335, -1020,   647,  1958, -1020,  1831,  -917,   693,
     388,  1834,  1127,   389,   782,   429,   781,  1812,  1859,   786,
    1813,  1860,   529,  1122,  1123,  1124,   165,  1043,   430,   394,
     395,  1424,  1259,  1260,   969,  1241,  1242,   749,  1514,  1125,
     433,   221,   223,   282,   436,  1387,  1069,   980,   808,  1564,
    1565,   814,  1075,   441,   179,   180,    65,    66,    67,   219,
    -599,   394,   395,  -600,   394,   395,   666,  1378,  1919,  1920,
    1921,  1497,   394,   395,   445,   692,  1773,  1774,  1388,   116,
     284,   988,   446,   116,  -601,  1419,   663,   563,   425,   427,
     428,    55,   447,   205,  1945,  1946,   448,   665,   667,   441,
     179,   180,    65,    66,    67,   592,   594,  1509,  1856,  1857,
    1000,  1852,  1853,   222,   449,    50,  1078,  1081,   450,  1146,
     451,   452,  1034,  -398,  -602,   442,  1930,  -603,   484,   485,
     418,   441,   179,   180,    65,    66,    67,   676,   486,   487,
    1948,   517,  -604,  1601,  -911,  1045,  -717,  1928,   521,   495,
     569,   209,   210,   211,   212,   213,   570,   526,   398,  1051,
     528,   260,  1940,   482,   534,   478,   479,   480,   431,   481,
     583,   442,   537,   182,  1488,  1462,    91,   335,  -915,    93,
      94,   482,    95,   183,    97,   441,   179,   180,    65,    66,
      67,  1031,  1031,  1031,  1031,  1031,  1031,   339,   518,  1062,
     538,  1031,   666,   442,   217,  -715,   418,   107,   340,   441,
      63,    64,    65,    66,    67,   545,   546,   554,   578,   567,
      72,   488,   663,   575, -1059,   365,   365,   598,   599,   598,
    1657,   579,   585,   665,   667,   586,   219,   418,   601,   643,
     602,   604,   116,   649,  1511,   614,  1234,   611,   615,   624,
    1626,  1155,   694,  1156,   535,   821,   341,   442,   625,   380,
     543,  1539,   668,   490,  1915,   669,  1158,   650,  1208,   691,
     643,   678,   649,   643,   649,   649,   129,  1548,   679,   165,
    1167,   442,   680,   165,   682,  -126,    55,   704,   792,   747,
     222,   794,  1273,   642,   796,   219,   123,   797,   124,  1277,
     803,   126,   122,   804,   822,   127,  1188,   556,   826,   583,
     380,   751,   380,   380,   380,   380,  1196,   829,   573,  1197,
     843,  1198,  1907,   844,  1368,   702,   475,   476,   477,   478,
     479,   480,   129,   481,   873,   776,   219,   876,   219,   217,
     875,  1219,  1907,   877,   878,   482,   880,   883,   217,   222,
     895,  1929,   123,   899,   900,   217,   583,  1031,   903,  1031,
     904,   910,   912,   249,   915,   918,   219,   217,   917,   920,
     811,  1614,   922,   242,   923,  1238,   924,   925,   247,   931,
     248,   116,   936,   937,  1244,   939,  1634,  -740,   945,   608,
     222,   946,   222,   948,   129,  1640,   949,   952,   616,   161,
     621,   870,   957,   965,   956,   628,   967,   971,   985,  1647,
     970,   973,   976,   986,   123,   129,  1245,   646,   982,   983,
     222,   994,   676,   666,  1342,  1002,  1006,   891,   694,  1004,
    -721,  1005,  1392,   978,   219,   123,  1044,  1293,  1054,   676,
    1064,  1066,   165,   663,  1068,  1073,  1072,  1074,  1076,  1089,
     219,   219,  1169,  1095,   665,   667,  1119,  1120,  1121,  1122,
    1123,  1124,   928,   930,   781,   879,   814,  1130,  1137,  1090,
     598,   598,  1091,  1092,  1139,  1125,  1141,  1143,  1093,  1094,
    1145,  1144,  1150,  1343,   664,  1788,  1154,  1157,   222,  1344,
    1163,  1443,  1031,   217,  1031,  1447,  1031,  1165,  1166,  1172,
    1453,  1031,  1170,  1191,   222,   222,   666,  1458,  1174,  1200,
     129,  1203,   129,  1206,  1211,  -918,  1212,  1222,  1223,  1224,
    1871,  1225,  1370,  1226,   124,  1227,   663,   126,   122,   380,
     123,   127,   123,  1228,  1231,   702,  1232,   665,   667,  1233,
    1235,  1252,  1247,   628,  1249,   814,   702,  1344,  1253,  1255,
    1264,  1265,  1266,  1219,  1397,  1271,  1272,  1397,  1125,  1331,
    1275,  1326,  1276,  1409,   524,   497,   498,   499,   500,   501,
     502,   503,   504,   505,   506,   507,   508,  1328,  1412,  1329,
    1338,   165,  1341,  1358,   260,   990,  1359,  1366,  1367,  1373,
    1380,  1375,   219,   219,  1426,  1031,  1389,  1371,  1918,  1390,
    1382,  1377,  1391,  1015,  1413,  1404,  1414,  1525,  1379,   509,
     510,  1529,  1415,  1411,  1024,  1383,  1037,  1416,  1536,  1421,
     129,  1425,  1830,  1431,  1434,   161,  1436,  1437,  1440,    34,
      35,    36,  1837,   664,  1956,  1441,  1442,  1445,   116,  1446,
     123,  1452,   207,  1448,  1451,  1449,   222,   222,  1450,  1454,
    1456,  1457,  1060,   116,  1461,  1491,   676,  1501,  1490,   676,
    1502,  1524,  1463,  1504,  1464,  1506,  1508,  1512,  1602,  1518,
     217,   441,    63,    64,    65,    66,    67,  1872,  1516,  1526,
    1489,  1517,    72,   488,   511,   512,  1528,  1494,  1519,  1522,
    1523,  1495,  1533,  1496,   116,  1534,    81,    82,    83,    84,
      85,  1503,   444,  1134,   694,  1535,  1527,   214,  1538,  1530,
    1531,  1510,   702,    89,    90,  1540,   129,  1532,  1541,  1520,
     968,  1521,  1894,   489,   219,   490,  1545,    99,  1542,   217,
    1546,  1147,  1549,  1561,  1572,  1585,   123,  1600,   491,  1605,
     492,   104,  1611,   442,   598,  1612,   598,   681,  1615,  1627,
    1031,  1031,  1620,   598,   598,  1621,   116,  1642,  1623,  1645,
    1651,  1659,  1660,  1754,  1755,  1761,  1767,  1768,  1793,   664,
     217,   583,   217,  1757,  1771,   219,  1770,   116,   222,   999,
    1780,  1782,  1783,   776,  1794,   811,  1814,  1820,  1823,  1954,
     219,   219,  1824,  1846,  1959,  1848,  1829,  1854,  1850,  1482,
     217,  1862,  1863,  1864,  1487,  1869,  1870,  1874,  1877,  1482,
    1878,  -351,  1881,  1880,  1487,  1883,  1885,  1809,  1886,   380,
    1038,  1889,  1039,  1892,  1909,  1896,  1895,  1897,  1917,   222,
    1902,  1216,  1216,  1024,  1916,  1913,  1613,  1650,   165,   702,
     676,  1904,  1925,  1951,   222,   222,  1934,  1952,  1927,  1955,
    1058,  1931,  1932,   165,  1628,  1941,  1629,  1963,  1630,  1964,
    1974,  1973,  1976,  1631,   811,  1977,  1330,  1912,   217,   750,
     116,   755,   116,  1202,   116,   752,  1926,  1418,  1162,   219,
    1787,  1924,   893,  1778,   217,   217,  1802,  1658,  1547,  1807,
    1592,  1966,  1819,  1936,   165,  1268,  1776,  1573,   637,  1286,
    1402,  1353,  1278,  1218,  1393,  1236,  1394,   630,  1182,  1652,
    1968,   703,  1638,  1079,  1953,  1887,  1333,  1270,  1142,   583,
    1325,     0,   129,     0,  1213,  1214,  1215,   205,     0,     0,
       0,     0,  1563,   222,   628,  1153,     0,     0,     0,     0,
       0,  1822,   123,     0,     0,     0,  1769,   495,   870,    50,
       0,     0,     0,     0,     0,     0,   165,  1781,     0,     0,
       0,  1594,     0,     0,     0,     0,     0,  1482,     0,     0,
       0,     0,  1487,  1482,     0,  1482,     0,   165,  1487,     0,
    1487,     0,   116,   676,     0,   209,   210,   211,   212,   213,
     664,     0,     0,     0,     0,     0,     0,  1482,     0,   129,
       0,     0,  1487,  1785,  1638,     0,   598,     0,   129,  1847,
    1849,     0,     0,    93,    94,     0,    95,   183,    97,   123,
       0,     0,     0,     0,     0,  1470,   217,   217,   123,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   107,     0,     0,     0,  1024,  1024,  1024,  1024,  1024,
    1024,     0,     0,     0,     0,  1024,  1817,     0,     0,  1470,
       0,     0,  1900,     0,     0,     0,   116,    14,     0,     0,
     165,     0,   165,   664,   165,     0,  1058,  1251,   116,     0,
       0,     0,  1762,     0,     0,  1482,     0,     0,  1433,     0,
    1487,     0,     0,  1867,   129,     0,     0,   342,     0,     0,
     129,    14,  1825,     0,     0,     0,     0,   129,     0,     0,
       0,     0,  1835,  1836,   123,     0,     0,     0,     0,     0,
     123,   444,     0,     0,     0,     0,     0,   123,     0,     0,
       0,  1471,     0,     0,   216,   216,  1472,     0,   441,  1473,
     180,    65,    66,    67,  1474,   239,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1465,     0,     0,   217,     0,
       0,     0,     0,     0,     0,  1471,     0,     0,     0,     0,
    1472,   239,   441,  1473,   180,    65,    66,    67,  1474,     0,
       0,     0,   165,     0,     0,     0,  1475,  1476,     0,  1477,
       0,     0,     0,     0,   524,   497,   498,   499,   500,   501,
     502,   503,   504,   505,   506,   507,   508,     0,  1369,   217,
     442,  1024,     0,  1024,     0,     0,     0,     0,     0,  1478,
    1475,  1476,     0,  1477,   217,   217, -1060, -1060, -1060, -1060,
   -1060,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,   509,
     510,     0,     0,     0,   442,     0,     0,     0,   129,     0,
       0,     0,  1125,  1492,     0,     0,     0,   348,     0,  1410,
       0,     0,     0,     0,     0,     0,   165,     0,   123,     0,
       0,     0,     0,     0,   628,  1058,     0,     0,   165,     0,
       0,     0,     0,     0,   116,     0,  1965,     0,     0,     0,
     129,     0,     0,   341,     0,     0,     0,   129,  1975,  1590,
       0,     0,     0,     0,     0,     0,     0,     0,  1978,     0,
     123,  1979,     0,   217,   511,   512,     0,   123,     0, -1060,
   -1060, -1060, -1060, -1060,   473,   474,   475,   476,   477,   478,
     479,   480,   129,   481,     0,     0,     0,     0,     0,     0,
       0,  1901,     0,     0,     0,   482,  1024,   216,  1024,     0,
    1024,     0,   123,   676,   129,  1024,     0,     0,     0,     0,
       0,   116,     0,   628,     0,     0,   116,     0,     0,     0,
     116,     0,     0,   676,   123,     0,     0,   805,     0,     0,
       0,     0,   676,     0,     0,     0,  1470,     0,   380,     0,
       0,   583,     0,     0,   341,     0,     0,   239,     0,   239,
       0,  1470,     0,     0,  1743,     0,     0,     0,     0,     0,
       0,  1750,     0,     0,   129,     0,     0,     0,   341,   129,
     341,  1470,     0,     0,     0,     0,   341,     0,    14,     0,
       0,     0,     0,     0,   123,     0,     0,     0,     0,   123,
       0,     0,     0,    14,     0,     0,     0,     0,     0,  1024,
       0,     0,     0,     0,     0,   239,   116,   116,   116,     0,
       0,     0,   116,    14,     0,     0,     0,     0,     0,   116,
       0,     0,     0,   344,     0,     0,     0,     0,     0,     0,
       0,     0,   216,   571,   165,   572,     0,     0,     0,     0,
       0,   216,  1471,     0,     0,     0,     0,  1472,   216,   441,
    1473,   180,    65,    66,    67,  1474,     0,  1471,     0,     0,
     216,     0,  1472,     0,   441,  1473,   180,    65,    66,    67,
    1474,   216,     0,     0,     0,     0,     0,  1471,     0,     0,
       0,     0,  1472,     0,   441,  1473,   180,    65,    66,    67,
    1474,   577,     0,     0,     0,     0,   239,  1475,  1476,   239,
    1477,     0,     0,     0,     0,     0,     0,  1470,     0,     0,
       0,   165,  1475,  1476,     0,  1477,   165,     0,     0,     0,
     165,   442,     0,     0,     0,     0,     0,     0,     0,   583,
    1618,     0,  1475,  1476,     0,  1477,   442,     0,     0,     0,
       0,     0,  1470,     0,     0,  1622,   239,     0,     0,    14,
     341,     0,     0,     0,  1024,  1024,   442,     0,     0,     0,
     116,     0,     0,     0,     0,  1624,     0,     0,     0,  1841,
       0,     0,     0,     0,     0,     0,  1743,  1743,     0,     0,
    1750,  1750,   697,     0,    14,   348,   216,     0,     0,     0,
       0,     0,     0,     0,   380,     0,     0,     0,     0,     0,
       0,     0,   116,     0,     0,     0,   165,   165,   165,   116,
       0,     0,   165,  1471,     0,     0,     0,     0,  1472,   165,
     441,  1473,   180,    65,    66,    67,  1474,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   239,     0,
     239,     0,     0,   858,   116,     0,     0,     0,  1471,   344,
       0,   344,  1899,  1472,     0,   441,  1473,   180,    65,    66,
      67,  1474,     0,     0,     0,     0,   116,     0,  1475,  1476,
       0,  1477,  1914,     0,     0,     0,   858,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   442,     0,     0,     0,     0,     0,     0,     0,
       0,  1633,     0,  1475,  1476,     0,  1477,   344,     0,   524,
     497,   498,   499,   500,   501,   502,   503,   504,   505,   506,
     507,   508,     0,     0,   835,     0,   116,   442,     0,     0,
       0,   116,     0,   239,   239,     0,  1779,     0,     0,     0,
       0,     0,   239,     0,   453,   454,   455,     0,     0,     0,
       0,     0,     0,     0,   509,   510,     0,     0,     0,     0,
     165,     0,     0,   216,   456,   457,     0,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
       0,   481,     0,     0,     0,     0,     0,     0,   344,     0,
       0,   344,   165,   482,     0,     0,     0,     0,     0,   165,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   950,
     951,     0,   216,     0,     0,     0,     0,     0,   959,   511,
     512,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   165,     0,  1099,  1100,  1101,     0,
       0,     0,     0,     0,     0,     0,     0,   239,     0,     0,
       0,     0,     0,   216,     0,   216,   165,  1102,     0,     0,
    1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,
    1123,  1124,   901,   216,   858,     0,   205,     0,   206,    40,
       0,   239,     0,     0,     0,  1125,     0,     0,   239,   239,
     858,   858,   858,   858,   858,     0,   282,     0,    50,     0,
       0,     0,   858,     0,     0,     0,   165,     0,     0,     0,
       0,   165,     0,     0,     0,     0,     0,  1346,   239,     0,
     344,     0,   839,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   284,   209,   210,   211,   212,   213,     0,
       0,   216,     0,     0,     0,     0,   205,     0,     0,     0,
       0,     0,     0,     0,     0,   239,     0,   216,   216,     0,
     774,     0,    93,    94,     0,    95,   183,    97,    50,     0,
       0,     0,     0,     0,   697,   697,   576,     0,     0,     0,
       0,   239,   239,     0,     0,     0,     0,     0,     0,     0,
     107,   216,     0,     0,   775,     0,   111,   239,     0,     0,
       0,     0,     0,   569,   209,   210,   211,   212,   213,   570,
     239,     0,     0,  1291,     0,   344,   344,     0,   858,   205,
       0,   239,     0,     0,   344,     0,   182,     0,     0,    91,
     335,     0,    93,    94,     0,    95,   183,    97,     0,   239,
       0,    50,     0,   239,     0,     0,     0,     0,     0,     0,
     339,     0,     0,     0,     0,     0,   239,     0,     0,     0,
     107,   340,     0,     0,     0,     0,     0,  1161,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   209,   210,   211,
     212,   213,     0,  1171,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1185,     0,     0,   216,
     216,     0,     0,   438,     0,    93,    94,     0,    95,   183,
      97,     0,     0,   239,     0,     0,     0,   239,     0,   239,
     205,     0,   206,    40,     0,  1205,     0,     0,     0,     0,
       0,     0,     0,   107,   858,   858,   858,   858,   858,   858,
     216,     0,    50,   858,   858,   858,   858,   858,   858,   858,
     858,   858,   858,   858,   858,   858,   858,   858,   858,   858,
     858,   858,   858,   858,   858,   858,   858,   858,   858,   858,
     858,     0,     0,     0,     0,     0,     0,     0,   209,   210,
     211,   212,   213,  1071,     0,     0,   858,     0,     0,     0,
     344,   344,     0,     0,     0,     0,     0,     0,     0,  1263,
       0,     0,     0,  1267,   774,     0,    93,    94,     0,    95,
     183,    97,     0,     0,     0,     0,     0,     0,     0,   239,
       0,   239,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   216,     0,     0,   107,     0,     0,     0,   810,     0,
     111,     0,     0,     0,     0,     0,     0,     0,   239,     0,
       0,   239,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   218,   218,     0,     0,     0,   239,   239,
     239,   239,   239,   239,   241,     0,   216,     0,   239,     0,
       0,     0,   216,   344,   496,   497,   498,   499,   500,   501,
     502,   503,   504,   505,   506,   507,   508,   216,   216,   344,
     858,     0,     0,     0,     0,  1361,     0,   959,     0,     0,
     239,     0,   344,     0,     0,     0,   239,     0,     0,   858,
       0,   858,     0,   453,   454,   455,     0,     0,     0,   509,
     510,   836,     0,     0,  1381,     0,     0,  1384,     0,     0,
       0,   344,     0,   456,   457,   858,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,     0,     0,     0,     0,     0,     0,     0,     0,   239,
     239,   205,   482,   239,     0,     0,   216,     0,     0,     0,
       0,   837,     0,     0,     0,     0,  1432,     0,     0,     0,
       0,     0,  1185,    50,   511,   512,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   344,     0,     0,     0,   344,
       0,   839,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   239,     0,   239,     0,     0,   209,
     210,   211,   212,   213,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   182,     0,     0,    91,  1466,  1467,    93,    94,     0,
      95,   183,    97,     0,   838,     0,   218,     0,     0,   239,
       0,   239,     0,     0,     0,     0,     0,   858,     0,   858,
       0,   858,   282,     0,     0,   107,   858,   216,     0,     0,
     858,     0,   858,     0,     0,   858,     0,     0,     0,     0,
       0,     0,     0,   932,     0,     0,   239,   239,     0,     0,
     239,   344,     0,   344,     0,     0,     0,   239,     0,   284,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   205,     0,     0,     0,     0,     0,     0,     0,
     344,     0,     0,   344,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,  1550,     0,  1551,     0,   239,
       0,   239,     0,   239,     0,     0,     0,     0,   239,     0,
     216,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   239,     0,     0,   858,     0,   569,
     209,   210,   211,   212,   213,   570,     0,     0,     0,   239,
     239,   218,   344,  1595,     0,     0,     0,   239,   344,   239,
     218,     0,   182,     0,     0,    91,   335,   218,    93,    94,
       0,    95,   183,    97,     0,  1077,     0,     0,     0,   218,
       0,   239,     0,   239,     0,     0,   339,     0,     0,   239,
     218,     0,   836,     0,     0,     0,   107,   340,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   239,     0,     0,     0,     0,     0,     0,     0,
    1641,   344,   344,     0,     0,     0,     0,     0,     0,   858,
     858,   858,     0,     0,     0,     0,   858,     0,   239,     0,
       0,     0,   205,     0,   239,     0,   239,   453,   454,   455,
       0,     0,   837,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,   241,     0,   456,   457,     0,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,     0,   481,     0,     0,     0,     0,     0,
     209,   210,   211,   212,   213,   218,   482,     0,     0,     0,
       0,   282,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   344,   182,   344,     0,    91,     0,     0,    93,    94,
    1798,    95,   183,    97,     0,  1269,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   239,     0,   284,     0,
       0,     0,     0,     0,     0,     0,   107,     0,   344,     0,
       0,   205,   865,   239,     0,     0,     0,   239,   239,   344,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   239,    50,     0,     0,     0,     0,   858,     0,
       0,     0,     0,     0,     0,   865,     0,     0,     0,   858,
       0,     0,     0,     0,     0,   858,     0,     0,     0,   858,
       0,     0,     0,     0,     0,     0,     0,     0,   569,   209,
     210,   211,   212,   213,   570,     0,     0,     0,     0,     0,
       0,   239,  1821,     0,     0,     0,   344,   964,     0,     0,
       0,   182,     0,     0,    91,   335,     0,    93,    94,     0,
      95,   183,    97,     0,  1435,     0,     0,     0,     0,   344,
       0,     0,     0,     0,     0,   339,     0,     0,     0,     0,
       0,   858,     0,     0,     0,   107,   340,     0,     0,     0,
       0,   239,   218,   344,     0,   344,     0,     0,     0,     0,
       0,   344,     0,     0,     0,     0,     0,     0,   239,     0,
       0,     0,     0,     0,     0,     0,     0,   239,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1882,     0,   239,
       0,   239,     0,     0,   453,   454,   455,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     239,   218,   239,     0,   456,   457,   344,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
       0,   481,     0,     0,     0,     0,  1030,     0,     0,     0,
       0,     0,   218,   482,   218,     0,     0,     0,     0,     0,
       0,     0,     0,   959,     0,     0,     0,     0,     0,     0,
       0,     0,   205,     0,     0,  1944,     0,   959,     0,     0,
       0,     0,   218,   865,     0,   205,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,  1944,     0,  1969,   865,
     865,   865,   865,   865,     0,   282,     0,    50,     0,     0,
       0,   865,   220,   220,     0,     0,     0,     0,   344,     0,
       0,     0,     0,   245,     0,     0,     0,  1129,     0,     0,
     209,   210,   211,   212,   213,   344,     0,     0,     0,     0,
       0,     0,   284,   209,   210,   211,   212,   213,     0,     0,
     218,     0,     0,     0,  1842,   205,  1748,     0,    93,    94,
    1749,    95,   183,    97,  1149,   182,   218,   218,    91,     0,
       0,    93,    94,     0,    95,   183,    97,    50,     0,     0,
       0,     0,     0,     0,  1003,     0,   107,  1589,     0,     0,
       0,  1149,     0,     0,     0,     0,     0,     0,     0,   107,
     218,     0,     0,   344,  1840,     0,     0,     0,     0,     0,
       0,     0,   569,   209,   210,   211,   212,   213,   570,     0,
       0,     0,     0,     0,     0,     0,     0,   865,     0,     0,
    1192,     0,     0,     0,     0,   182,     0,     0,    91,   335,
       0,    93,    94,     0,    95,   183,    97,     0,     0,     0,
       0,     0,   241,     0,     0,     0,     0,     0,     0,   339,
       0,     0,     0,     0,     0,  1030,     0,     0,     0,   107,
     340,     0,     0,     0,     0,     0,     0,     0,     0,   344,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   344,     0,   344,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   218,   218,
       0,     0,   344,     0,   344,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   220,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   865,   865,   865,   865,   865,   865,   218,
       0,     0,   865,   865,   865,   865,   865,   865,   865,   865,
     865,   865,   865,   865,   865,   865,   865,   865,   865,   865,
     865,   865,   865,   865,   865,   865,   865,   865,   865,   865,
     453,   454,   455,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   865,     0,     0,     0,     0,
     456,   457,     0,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,     0,   481,     0,     0,
       0,     0,     0,     0,     0,   205,     0,     0,     0,   482,
     218,     0,     0,     0,     0,     0,     0,     0,  1574,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
     220,     0,     0,     0,     0,     0,     0,     0,     0,   220,
       0,     0,     0,     0,     0,     0,   220,  1030,  1030,  1030,
    1030,  1030,  1030,     0,     0,   218,     0,  1030,   220,     0,
       0,   218,     0,   209,   210,   211,   212,   213,   205,   245,
       0,     0,     0,     0,     0,     0,   218,   218,     0,   865,
       0,     0,     0,     0,     0,     0,     0,     0,   398,     0,
      50,    93,    94,     0,    95,   183,    97,     0,   865,     0,
     865,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1575,     0,     0,     0,     0,     0,     0,   107,
       0,     0,     0,   399,   865,  1576,   209,   210,   211,   212,
     213,  1577,     0,     0,     0,     0,     0,     0,     0,     0,
    1007,     0,     0,   867,   245,     0,     0,     0,   182,     0,
       0,    91,  1578,     0,    93,    94,     0,    95,  1579,    97,
       0,     0,  1469,     0,     0,   218,     0,   453,   454,   455,
       0,     0,     0,     0,     0,     0,   894,     0,     0,     0,
       0,     0,   107,     0,   220,     0,     0,   456,   457,  1427,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,  1030,   481,  1030,     0,     0,  1099,  1100,
    1101,     0,     0,     0,     0,     0,   482,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1102,
       0,   866,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,
    1121,  1122,  1123,  1124,     0,     0,   865,     0,   865,     0,
     865,     0,     0,     0,   866,   865,   218,  1125,   205,   865,
       0,   865,     0,     0,   865,     0,   453,   454,   455,     0,
       0,     0,     0,     0,     0,     0,  1571,     0,     0,  1584,
      50,     0,     0,     0,     0,     0,   456,   457,     0,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,     0,     0,   209,   210,   211,   212,
     213,     0,     0,     0,  1274,   482,     0,     0,  1030,     0,
    1030,     0,  1030,     0,     0,  1428,     0,  1030,     0,   218,
       0,   220,     0,     0,    93,    94,     0,    95,   183,    97,
       0,     0,     0,     0,     0,     0,   865,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1648,  1649,
       0,     0,   107,   704,  1059,     0,     0,     0,  1584,     0,
       0,     0,     0,   453,   454,   455,     0,     0,     0,     0,
    1082,  1083,  1084,  1085,  1086,     0,     0,     0,     0,     0,
     220,     0,  1096,   456,   457,     0,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,  1030,   845,   846,     0,     0,     0,     0,   847,     0,
     848,   220,   482,   220,     0,     0,   205,     0,   865,   865,
     865,     0,   849,     0,     0,   865,  1140,  1796,     0,     0,
      34,    35,    36,   205,     0,  1584,     0,     0,    50,     0,
       0,   220,   866,   207,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,   866,   866,
     866,   866,   866,  1588,     0,     0,     0,     0,     0,     0,
     866,     0,     0,     0,   209,   210,   211,   212,   213,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1189,     0,
     850,   851,   852,   853,   854,   855,     0,    81,    82,    83,
      84,    85,    93,    94,     0,    95,   183,    97,   214,   220,
       0,     0,     0,   182,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   183,    97,   220,   220,     0,    99,     0,
     107,  1589,     0,     0,     0,     0,     0,   856,     0,     0,
       0,     0,   104,  1199,     0,     0,     0,   107,   857,     0,
       0,     0,     0,     0,     0,     0,  1030,  1030,     0,   245,
       0,   524,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   508,     0,     0,     0,   865,     0,     0,
       0,     0,     0,     0,     0,     0,   866,   205,   865,   926,
       0,   927,     0,     0,   865,     0,     0,     0,   865,     0,
       0,     0,     0,     0,  1086,  1281,   509,   510,  1281,    50,
       0,   245,     0,  1294,  1297,  1298,  1299,  1301,  1302,  1303,
    1304,  1305,  1306,  1307,  1308,  1309,  1310,  1311,  1312,  1313,
    1314,  1315,  1316,  1317,  1318,  1319,  1320,  1321,  1322,  1323,
    1324,     0,     0,     0,     0,   209,   210,   211,   212,   213,
       0,     0,     0,     0,     0,     0,  1332,     0,     0,     0,
     865,     0,     0,     0,     0,     0,     0,   220,   220,     0,
    1911,     0,     0,    93,    94,     0,    95,   183,    97,     0,
       0,   511,   512,     0,     0,     0,     0,  1571,   205,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   107,   866,   866,   866,   866,   866,   866,   245,     0,
      50,   866,   866,   866,   866,   866,   866,   866,   866,   866,
     866,   866,   866,   866,   866,   866,   866,   866,   866,   866,
     866,   866,   866,   866,   866,   866,   866,   866,   866,     0,
       0,     0,     0,     0,     0,     0,   209,   210,   211,   212,
     213,     0,     0,     0,   866,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1422,     0,     0,     0,    93,    94,     0,    95,   183,    97,
       0,     0,     0,     0,     0,     0,     0,   205,     0,  1438,
       0,  1439,     0,   453,   454,   455,     0,  1070,     0,   220,
       0,     0,   107,   978,     0,     0,     0,     0,     0,    50,
       0,     0,     0,   456,   457,  1459,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,     0,     0,     0,   245,   209,   210,   211,   212,   213,
     220,     0,   482,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   220,   220,   182,   866,     0,
      91,     0,     0,    93,    94,     0,    95,   183,    97,     0,
       0,     0,     0,     0,     0,     0,     0,   866,     0,   866,
       0,     0,     0,     0,   453,   454,   455,     0,     0,     0,
       0,   107,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   866,   456,   457,     0,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
       0,   481,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   482,   220,     0,     0,  1553,   205,  1554,
       0,  1555,     0,     0,     0,     0,  1556,   453,   454,   455,
    1558,     0,  1559,     0,     0,  1560,     0,     0,     0,     0,
      50,     0,     0,  1210,     0,     0,     0,   456,   457,     0,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,     0,   481,     0,   209,   210,   211,   212,
     213,     0,     0,     0,     0,     0,   482,     0,     0,     0,
       0,     0,     0,     5,     6,     7,     8,     9,   182,     0,
       0,    91,    92,    10,    93,    94,     0,    95,   183,    97,
       0,     0,     0,     0,     0,   866,     0,   866,     0,   866,
       0,     0,     0,     0,   866,   245,     0,  1643,   866,     0,
     866,     0,   107,   866,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,  1240,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,   196,     0,
       0,    55,     0,     0,     0,     0,     0,     0,   245,   178,
     179,   180,    65,    66,    67,   483,     0,    69,    70,  1789,
    1790,  1791,     0,     0,     0,   866,  1795,   181,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   182,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   183,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   184,     0,     0,     0,
       0,   111,   112,     0,   113,   114,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   453,   454,   455,     0,     0,     0,   866,   866,   866,
       0,     0,     0,     0,   866,     0,     0,     0,     0,     0,
       0,   456,   457,  1801,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,     0,   481,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     482,     0,     0,     0,     0,     0,   205,     0,  1851,     0,
       0,     0,     0,     0,     0,     0,   453,   454,   455,  1861,
       0,     0,     0,     0,     0,  1866,     0,     0,    50,  1868,
       0,     0,     0,     0,     0,     0,   456,   457,     0,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,   209,   210,   211,   212,   213,     0,
       0,     0,     0,     0,     0,   482,     0,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,   359,
       0,  1903,    93,    94,     0,    95,   183,    97,     0,     0,
       0,    11,    12,    13,     0,     0,   866,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   866,     0,     0,
     107,     0,    14,   866,    15,    16,     0,   866,     0,     0,
      17,  1607,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,  1884,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,    56,    57,    58,   866,
      59,    60,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,    71,    72,    73,  1608,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,    88,    89,    90,    91,    92,
       0,    93,    94,     0,    95,    96,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,     0,   103,     0,   104,   105,   106,     0,     0,   107,
     108,     0,   109,   110,  1159,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,    56,
      57,    58,     0,    59,    60,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,    88,    89,
      90,    91,    92,     0,    93,    94,     0,    95,    96,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,   103,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,  1347,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,    56,    57,    58,     0,    59,    60,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,    88,    89,    90,    91,    92,     0,    93,    94,     0,
      95,    96,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,   103,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   182,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   183,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   108,
       0,   109,   110,   683,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   182,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   183,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,  1128,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     182,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     183,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   108,     0,   109,   110,  1173,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   182,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   183,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,  1246,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
    1248,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   182,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   183,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,  1423,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,     0,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,   182,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   183,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   108,     0,   109,   110,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   182,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   183,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   108,     0,   109,
     110,  1562,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,     0,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   182,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   183,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     108,     0,   109,   110,  1792,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,  1838,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   182,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   183,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   182,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   183,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
    1873,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
    1876,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   182,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   183,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   108,
       0,   109,   110,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   182,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   183,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,  1893,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     182,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     183,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   108,     0,   109,   110,  1910,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   182,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   183,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,  1967,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   182,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   183,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,  1970,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,     0,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,   182,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   183,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   108,     0,   109,   110,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,   552,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,    61,
      62,   179,   180,    65,    66,    67,     0,    68,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   182,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   183,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   108,     0,   109,
     110,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,   823,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,    61,    62,   179,   180,    65,    66,    67,     0,
      68,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   182,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   183,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     108,     0,   109,   110,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,  1061,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,    61,    62,   179,   180,    65,
      66,    67,     0,    68,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   182,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   183,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,  1637,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,    61,    62,
     179,   180,    65,    66,    67,     0,    68,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   182,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   183,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,  1784,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,    61,    62,   179,   180,    65,    66,    67,     0,    68,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   182,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   183,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   108,
       0,   109,   110,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,   179,   180,    65,    66,
      67,     0,    68,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   182,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   183,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,   413,    13,     0,
       0,     0,     0,     0,     0,     0,     0,   757,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,   178,   179,
     180,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   181,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     182,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     183,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   108,     0,     0,     0,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
       0,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,   178,   179,   180,    65,    66,    67,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   181,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   182,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   183,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   184,     0,
     349,     0,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,  1102,     0,    10,  1103,
    1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,
    1124,     0,     0,   698,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1125,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,   178,   179,   180,    65,    66,    67,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   181,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   182,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   183,    97,     0,   699,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   184,     0,     0,     0,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,   178,   179,   180,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   181,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   182,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   183,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   184,     0,     0,   818,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,  1103,  1104,  1105,  1106,  1107,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,  1120,  1121,  1122,  1123,  1124,     0,     0,  1186,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1125,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
     178,   179,   180,    65,    66,    67,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   181,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   182,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   183,    97,     0,  1187,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   184,     0,     0,
       0,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,   413,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,   178,   179,   180,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   181,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   182,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   183,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     108,     0,     0,     0,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,     0,   232,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   482,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,   178,   179,   180,    65,
      66,    67,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   181,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   182,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   183,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   184,   453,   454,   455,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   456,   457,     0,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
       0,   481,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,   482,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,   178,
     179,   180,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   181,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   182,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   183,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,   566,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   184,     0,   267,   454,
     455,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,   456,   457,
       0,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,   482,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,   178,   179,   180,    65,    66,    67,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     181,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   182,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   183,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   184,
       0,   270,     0,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   413,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,   178,   179,   180,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   181,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   182,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   183,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,   453,   454,   455,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   456,   457,     0,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,   482,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,   178,   179,
     180,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   181,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     182,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     183,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,   568,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   184,   550,     0,     0,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,   712,   481,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   482,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,   178,   179,   180,    65,    66,    67,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   181,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   182,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   183,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   184,     0,
       0,     0,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,  1104,
    1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,
       0,     0,     0,   757,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1125,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,   178,   179,   180,    65,    66,    67,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   181,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   182,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   183,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   184,     0,     0,     0,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,     0,     0,   798,     0,     0,     0,
       0,     0,     0,     0,     0,   482,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,   178,   179,   180,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   181,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   182,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   183,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   184,     0,     0,     0,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,  1105,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
    1120,  1121,  1122,  1123,  1124,     0,     0,     0,     0,   800,
       0,     0,     0,     0,     0,     0,     0,     0,  1125,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
     178,   179,   180,    65,    66,    67,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   181,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   182,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   183,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   184,     0,     0,
       0,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,     0,   481,     0,
       0,     0,  1237,     0,     0,     0,     0,     0,     0,     0,
     482,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,   178,   179,   180,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   181,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   182,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   183,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     184,   453,   454,   455,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   456,   457,     0,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,     0,   481,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
     482,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,   178,   179,   180,    65,
      66,    67,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   181,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   182,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   183,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,   587,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   184,   453,   454,   455,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,   827,     0,    10,   456,   457,     0,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
       0,   481,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,   482,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,   644,    39,    40,     0,   828,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,   178,
     179,   180,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   181,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   182,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   183,    97,     0,   272,   273,    99,   274,   275,   100,
       0,   276,   277,   278,   279,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   184,     0,     0,   280,
     281,   111,   112,     0,   113,   114,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
    1120,  1121,  1122,  1123,  1124,     0,     0,     0,   283,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1125,     0,
       0,     0,   285,   286,   287,   288,   289,   290,   291,     0,
       0,     0,   205,     0,   206,    40,     0,     0,     0,     0,
       0,     0,     0,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,    50,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   205,   326,
       0,   745,   328,   329,   330,     0,     0,     0,   331,   580,
     209,   210,   211,   212,   213,   581,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,   272,   273,     0,   274,
     275,     0,   582,   276,   277,   278,   279,     0,    93,    94,
       0,    95,   183,    97,   336,     0,   337,     0,     0,   338,
       0,   280,   281,     0,     0,     0,   209,   210,   211,   212,
     213,     0,     0,     0,     0,     0,   107,     0,     0,     0,
     746,     0,   111,     0,     0,     0,     0,     0,     0,     0,
     283,   596,     0,     0,    93,    94,     0,    95,   183,    97,
       0,     0,     0,     0,   285,   286,   287,   288,   289,   290,
     291,     0,     0,     0,   205,     0,   206,    40,     0,     0,
       0,     0,   107,     0,     0,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,    50,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     205,   326,     0,   327,   328,   329,   330,     0,     0,     0,
     331,   580,   209,   210,   211,   212,   213,   581,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,   272,   273,
       0,   274,   275,     0,   582,   276,   277,   278,   279,     0,
      93,    94,     0,    95,   183,    97,   336,     0,   337,     0,
       0,   338,     0,   280,   281,     0,   282,     0,   209,   210,
     211,   212,   213,     0,     0,     0,     0,     0,   107,     0,
       0,     0,   746,     0,   111,     0,     0,     0,     0,     0,
       0,     0,   283,     0,     0,     0,    93,    94,     0,    95,
     183,    97,     0,   284,     0,     0,   285,   286,   287,   288,
     289,   290,   291,     0,     0,     0,   205,     0,     0,     0,
       0,     0,     0,     0,   107,     0,     0,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,    50,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,     0,   326,     0,     0,   328,   329,   330,     0,
       0,     0,   331,   332,   209,   210,   211,   212,   213,   333,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   334,     0,     0,    91,
     335,     0,    93,    94,     0,    95,   183,    97,   336,     0,
     337,     0,     0,   338,   272,   273,     0,   274,   275,     0,
     339,   276,   277,   278,   279,     0,     0,     0,     0,     0,
     107,   340,     0,     0,     0,  1763,     0,     0,     0,   280,
     281,     0,   282,   457,     0,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,   283,   481,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   284,
       0,   482,   285,   286,   287,   288,   289,   290,   291,     0,
       0,     0,   205,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,    50,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,     0,   326,
       0,     0,   328,   329,   330,     0,     0,     0,   331,   332,
     209,   210,   211,   212,   213,   333,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   334,     0,     0,    91,   335,     0,    93,    94,
       0,    95,   183,    97,   336,     0,   337,     0,     0,   338,
     272,   273,     0,   274,   275,     0,   339,   276,   277,   278,
     279,     0,     0,     0,     0,     0,   107,   340,     0,     0,
       0,  1833,     0,     0,     0,   280,   281,     0,   282,     0,
       0,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,   283,   481,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   284,     0,   482,   285,   286,
     287,   288,   289,   290,   291,     0,     0,     0,   205,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
      50,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,     0,   326,     0,   327,   328,   329,
     330,     0,     0,     0,   331,   332,   209,   210,   211,   212,
     213,   333,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   334,     0,
       0,    91,   335,     0,    93,    94,     0,    95,   183,    97,
     336,     0,   337,     0,     0,   338,   272,   273,     0,   274,
     275,     0,   339,   276,   277,   278,   279,     0,     0,     0,
       0,     0,   107,   340,     0,     0,     0,     0,     0,     0,
       0,   280,   281,     0,   282,  1107,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,  1123,  1124,     0,     0,     0,     0,     0,     0,     0,
     283,     0,     0,     0,     0,     0,  1125,     0,     0,     0,
       0,   284,     0,     0,   285,   286,   287,   288,   289,   290,
     291,     0,     0,     0,   205,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,    50,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
       0,   326,     0,     0,   328,   329,   330,     0,     0,     0,
     331,   332,   209,   210,   211,   212,   213,   333,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   334,     0,     0,    91,   335,     0,
      93,    94,     0,    95,   183,    97,   336,     0,   337,     0,
       0,   338,     0,   272,   273,     0,   274,   275,   339,  1566,
     276,   277,   278,   279,     0,     0,     0,     0,   107,   340,
       0,     0,     0,     0,     0,     0,     0,     0,   280,   281,
       0,   282,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,     0,     0,     0,     0,     0,     0,   283,     0,     0,
       0,     0,   482,     0,     0,     0,     0,     0,   284,     0,
       0,   285,   286,   287,   288,   289,   290,   291,     0,     0,
       0,   205,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,    50,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,     0,   326,     0,
       0,   328,   329,   330,     0,     0,     0,   331,   332,   209,
     210,   211,   212,   213,   333,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   334,     0,     0,    91,   335,     0,    93,    94,     0,
      95,   183,    97,   336,     0,   337,     0,     0,   338,  1663,
    1664,  1665,  1666,  1667,     0,   339,  1668,  1669,  1670,  1671,
       0,     0,     0,     0,     0,   107,   340,     0,     0,     0,
       0,     0,     0,  1672,  1673,  1674,     0,   456,   457,     0,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,  1675,   481,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   482,  1676,  1677,  1678,
    1679,  1680,  1681,  1682,     0,     0,     0,   205,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1683,  1684,
    1685,  1686,  1687,  1688,  1689,  1690,  1691,  1692,  1693,    50,
    1694,  1695,  1696,  1697,  1698,  1699,  1700,  1701,  1702,  1703,
    1704,  1705,  1706,  1707,  1708,  1709,  1710,  1711,  1712,  1713,
    1714,  1715,  1716,  1717,  1718,  1719,  1720,  1721,  1722,  1723,
       0,     0,     0,  1724,  1725,   209,   210,   211,   212,   213,
       0,  1726,  1727,  1728,  1729,  1730,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1731,  1732,  1733,
       0,   205,     0,    93,    94,     0,    95,   183,    97,  1734,
       0,  1735,  1736,     0,  1737,     0,     0,     0,     0,     0,
       0,  1738,  1739,    50,  1740,     0,  1741,  1742,     0,   272,
     273,   107,   274,   275,     0,     0,   276,   277,   278,   279,
       0,     0,     0,     0,     0,  1575,     0,     0,     0,     0,
       0,     0,     0,     0,   280,   281,     0,     0,  1576,   209,
     210,   211,   212,   213,  1577,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   182,     0,   283,    91,    92,     0,    93,    94,     0,
      95,  1579,    97,     0,     0,     0,     0,   285,   286,   287,
     288,   289,   290,   291,     0,     0,     0,   205,     0,     0,
       0,     0,     0,     0,     0,   107,     0,     0,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,    50,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,     0,   326,     0,   327,   328,   329,   330,
       0,     0,     0,   331,   580,   209,   210,   211,   212,   213,
     581,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   272,   273,     0,   274,   275,     0,   582,   276,   277,
     278,   279,     0,    93,    94,     0,    95,   183,    97,   336,
       0,   337,     0,     0,   338,     0,   280,   281,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   107,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   283,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   285,
     286,   287,   288,   289,   290,   291,     0,     0,     0,   205,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,    50,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,     0,   326,     0,  1292,   328,
     329,   330,     0,     0,     0,   331,   580,   209,   210,   211,
     212,   213,   581,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   272,   273,     0,   274,   275,     0,   582,
     276,   277,   278,   279,     0,    93,    94,     0,    95,   183,
      97,   336,     0,   337,     0,     0,   338,     0,   280,   281,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   107,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   283,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   285,   286,   287,   288,   289,   290,   291,     0,     0,
       0,   205,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,    50,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,     0,   326,     0,
       0,   328,   329,   330,     0,     0,     0,   331,   580,   209,
     210,   211,   212,   213,   581,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   582,     0,     0,     0,     0,     0,    93,    94,     0,
      95,   183,    97,   336,     0,   337,     0,     0,   338,   453,
     454,   455,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   107,     0,     0,     0,   456,
     457,     0,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,     0,   481,   453,   454,   455,
       0,     0,     0,     0,     0,     0,     0,     0,   482,     0,
       0,     0,     0,     0,     0,     0,     0,   456,   457,     0,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,     0,   481,   453,   454,   455,     0,     0,
       0,     0,     0,     0,     0,     0,   482,     0,     0,     0,
       0,     0,     0,     0,     0,   456,   457,     0,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,     0,   481,     0,     0,  1300,     0,     0,     0,     0,
       0,     0,     0,     0,   482,     0,     0,     0,     0,   205,
       0,     0,     0,   845,   846,     0,     0,     0,     0,   847,
       0,   848,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,   849,     0,     0,     0,   591,     0,   356,
     357,    34,    35,    36,   205,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   207,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,   209,   210,   211,
     212,   213,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   790,   205,     0,     0,     0,   358,
       0,     0,   359,     0,     0,    93,    94,     0,    95,   183,
      97,   850,   851,   852,   853,   854,   855,    50,    81,    82,
      83,    84,    85,     0,   360,   884,   885,     0,     0,   214,
    1055,     0,     0,   107,   182,    89,    90,    91,    92,     0,
      93,    94,   815,    95,   183,    97,     0,     0,     0,    99,
       0,     0,     0,   209,   210,   211,   212,   213,   856,     0,
       0,     0,    29,   104,     0,     0,     0,     0,   107,   857,
      34,    35,    36,   205,     0,   206,    40,     0,   596,     0,
       0,    93,    94,   207,    95,   183,    97,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   107,
       0,   208,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1056,
      75,   209,   210,   211,   212,   213,     0,    81,    82,    83,
      84,    85,  1125,     0,     0,  1009,  1010,     0,   214,     0,
       0,     0,     0,   182,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   183,    97,  1011,     0,     0,    99,     0,
       0,     0,     0,  1012,  1013,  1014,   205,     0,     0,     0,
       0,     0,   104,     0,     0,     0,  1015,   107,   215,     0,
       0,     0,     0,   111,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    29,     0,     0,     0,
       0,     0,     0,     0,    34,    35,    36,   205,     0,   206,
      40,     0,     0,     0,     0,     0,     0,   207,     0,     0,
       0,     0,     0,  1016,  1017,  1018,  1019,  1020,  1021,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1022,     0,     0,     0,   208,   182,     0,     0,    91,
      92,     0,    93,    94,     0,    95,   183,    97,     0,     0,
       0,     0,     0,     0,    75,   209,   210,   211,   212,   213,
    1023,    81,    82,    83,    84,    85,     0,     0,     0,     0,
     107,     0,   214,     0,     0,     0,     0,   182,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   183,    97,    29,
       0,     0,    99,     0,     0,     0,     0,    34,    35,    36,
     205,     0,   206,    40,     0,     0,   104,     0,     0,     0,
     207,   107,   215,     0,     0,   607,     0,   111,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   208, -1060,
   -1060, -1060, -1060,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,  1122,  1123,  1124,   627,    75,   209,   210,
     211,   212,   213,     0,    81,    82,    83,    84,    85,  1125,
       0,     0,     0,     0,     0,   214,     0,     0,     0,     0,
     182,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     183,    97,    29,   998,     0,    99,     0,     0,     0,     0,
      34,    35,    36,   205,     0,   206,    40,     0,     0,   104,
       0,     0,     0,   207,   107,   215,     0,     0,     0,     0,
     111,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   208,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,   209,   210,   211,   212,   213,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,   214,     0,
       0,     0,     0,   182,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   183,    97,    29,     0,     0,    99,     0,
       0,     0,     0,    34,    35,    36,   205,     0,   206,    40,
       0,     0,   104,     0,     0,     0,   207,   107,   215,     0,
       0,     0,     0,   111,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   208,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1152,    75,   209,   210,   211,   212,   213,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,   214,     0,     0,     0,     0,   182,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   183,    97,    29,     0,
       0,    99,     0,     0,     0,     0,    34,    35,    36,   205,
       0,   206,    40,     0,     0,   104,     0,     0,     0,   207,
     107,   215,     0,     0,     0,     0,   111,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   208,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    75,   209,   210,   211,
     212,   213,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,   214,     0,     0,     0,     0,   182,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   183,
      97,     0,     0,     0,    99,     0,     0,   453,   454,   455,
       0,     0,     0,     0,     0,     0,     0,     0,   104,     0,
       0,     0,     0,   107,   215,     0,     0,   456,   457,   111,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,     0,   481,   453,   454,   455,     0,     0,
       0,     0,     0,     0,     0,     0,   482,     0,     0,     0,
       0,     0,     0,     0,     0,   456,   457,     0,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,     0,   481,     0,     0,     0,     0,     0,     0,     0,
       0,   453,   454,   455,   482,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   456,   457,   527,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,     0,   481,   453,
     454,   455,     0,     0,     0,     0,     0,     0,     0,     0,
     482,     0,     0,     0,     0,     0,     0,     0,     0,   456,
     457,   536,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,     0,   481,     0,     0,     0,
       0,     0,     0,     0,     0,   453,   454,   455,   482,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   456,   457,   916,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,     0,   481,   453,   454,   455,     0,     0,     0,     0,
       0,     0,     0,     0,   482,     0,     0,     0,     0,     0,
       0,     0,     0,   456,   457,   984,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,     0,     0,     0,     0,     0,     0,     0,     0,   453,
     454,   455,   482,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   456,
     457,  1040,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,     0,   481,  1099,  1100,  1101,
       0,     0,     0,     0,     0,     0,     0,     0,   482,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1102,  1345,
       0,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,  1123,  1124,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1099,  1100,  1101,     0,  1125,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1102,     0,  1376,  1103,  1104,  1105,  1106,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,     0,     0,
    1099,  1100,  1101,     0,     0,     0,     0,     0,     0,     0,
       0,  1125,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1102,     0,  1444,  1103,  1104,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,  1122,  1123,  1124,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1099,  1100,  1101,     0,  1125,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1102,     0,  1455,  1103,
    1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,
    1124,     0,     0,  1099,  1100,  1101,     0,     0,     0,     0,
       0,     0,     0,     0,  1125,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1102,     0,  1552,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,     0,
      34,    35,    36,   205,     0,   206,    40,     0,     0,     0,
       0,     0,  1125,   207,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,  1644,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   236,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   237,     0,     0,     0,     0,     0,     0,     0,
       0,   209,   210,   211,   212,   213,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,   214,  1646,
       0,     0,     0,   182,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   183,    97,     0,     0,     0,    99,     0,
      34,    35,    36,   205,     0,   206,    40,     0,     0,     0,
       0,     0,   104,   658,     0,     0,     0,   107,   238,     0,
       0,     0,     0,   111,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   208, -1060, -1060, -1060, -1060,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,   209,   210,   211,   212,   213,     0,    81,    82,    83,
      84,    85,   482,     0,     0,     0,     0,     0,   214,     0,
       0,     0,     0,   182,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   183,    97,     0,     0,     0,    99,     0,
      34,    35,    36,   205,     0,   206,    40,     0,     0,     0,
       0,     0,   104,   207,     0,     0,     0,   107,   659,     0,
       0,     0,     0,   111,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   236,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   209,   210,   211,   212,   213,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,   214,     0,
       0,     0,     0,   182,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   183,    97,     0,     0,     0,    99,     0,
       0,     0,     0,     0,   453,   454,   455,     0,     0,     0,
       0,     0,   104,     0,     0,     0,     0,   107,   238,     0,
       0,     0,     0,   111,   456,   457,   981,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
       0,   481,   453,   454,   455,     0,     0,     0,     0,     0,
       0,     0,     0,   482,     0,     0,     0,     0,     0,     0,
       0,     0,   456,   457,     0,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,     0,   481,
    1099,  1100,  1101,     0,     0,     0,     0,     0,     0,     0,
       0,   482,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1102,  1460,     0,  1103,  1104,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,  1122,  1123,  1124,  1099,  1100,  1101,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1125,
       0,     0,     0,     0,     0,     0,     0,  1102,     0,     0,
    1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,
    1123,  1124,  1100,  1101,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1125,     0,     0,     0,     0,
       0,     0,  1102,     0,     0,  1103,  1104,  1105,  1106,  1107,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,  1120,  1121,  1122,  1123,  1124,   455,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1125,     0,     0,     0,     0,   456,   457,     0,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,  1101,   481,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   482,     0,     0,     0,     0,     0,
    1102,     0,     0,  1103,  1104,  1105,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
    1120,  1121,  1122,  1123,  1124,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1125
};

static const yytype_int16 yycheck[] =
{
       5,     6,   161,     8,     9,    10,    11,    12,    13,   129,
      15,    16,    17,    18,   521,    56,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   186,    31,   675,     4,    98,
      33,     4,     4,   102,   103,     4,   540,   406,   108,    44,
    1176,   239,   671,    46,   517,   518,    57,    52,    51,    54,
     166,   957,    57,   701,    59,    30,    56,   406,  1337,   128,
     552,   651,   108,  1172,   672,    30,   946,   187,   406,   406,
      30,   754,    30,   546,   481,   601,   602,   161,   513,   513,
     108,    86,   356,   357,   358,   826,   360,    57,   833,  1163,
     250,   108,  1054,   977,    44,     9,   588,     9,   802,     9,
      32,     9,    32,   108,    14,     9,   235,     9,     9,   993,
      14,     9,    14,   548,   548,     9,     9,    48,    38,    48,
       9,     9,     9,     9,     4,     9,     9,     9,     9,     4,
       9,    48,     9,    90,    70,   251,    86,     9,   184,    83,
       9,     9,     9,    38,     4,     9,     9,     0,  1471,    48,
     134,   135,    48,    90,    54,    83,   184,  1041,   160,    36,
      83,    84,    83,    83,    70,    14,   196,   184,    83,   215,
      83,   160,    57,  1777,   115,   181,   545,    38,   166,   184,
     160,   673,   160,    32,    69,   120,   191,   215,    83,   196,
     196,   179,   238,   102,    70,   130,    70,   199,   215,    38,
     157,   181,    51,   181,    50,    51,   196,   196,   134,   135,
     215,   199,    70,    70,    70,  1095,   200,    19,    20,   199,
     157,   238,    83,    70,   160,    70,    70,    70,   134,   135,
    1834,   391,   181,   238,    70,    70,    70,   178,    70,   887,
      70,    70,    70,     8,    83,   160,   174,   196,   253,  1572,
      70,   256,    70,   174,   174,   164,    70,   201,   263,   264,
     196,   174,  1371,   199,   193,   165,   197,   199,   198,   129,
     196,    19,    20,  1596,   111,  1598,   193,   181,   201,   174,
     197,   161,   443,   197,  1563,   257,   198,   199,   198,   261,
     198,  1253,   996,   199,   198,    83,   198,   198,   197,  1373,
     198,   197,   160,    54,   198,   198,  1380,   348,  1382,   198,
     198,   198,   198,   174,   198,   198,   198,   198,   197,  1064,
     197,  1066,   196,  1207,   160,   197,   818,   187,   197,   197,
     197,   823,   378,   197,   197,   174,   182,  1411,   196,   196,
     196,   199,   199,   199,    83,   935,   160,    81,   348,   196,
     378,  1231,   199,    70,   199,   199,   199,    83,   884,   885,
      70,   378,   523,   199,   199,   199,   974,   199,   437,   199,
     199,   199,   377,   378,    38,   495,   196,   197,   196,   384,
     385,   386,   387,   388,   389,   199,    75,    76,   181,   394,
     134,   135,   166,   434,   106,   107,   130,  1506,   181,  1508,
     181,   530,    38,   196,   192,   912,   164,   196,   413,    83,
      84,   199,   196,   196,   165,   196,   421,   134,   135,    83,
     792,   490,   491,   492,   493,   199,   496,   377,   433,   180,
     232,   157,   158,   159,   434,   174,   386,   387,   199,   389,
     198,   199,   387,  1517,   389,   106,   107,    83,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   196,   482,  1161,   484,
     485,   486,    83,   102,   232,   496,     4,   487,   200,    90,
     102,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   508,   633,   634,   481,  1616,  1249,   514,
     515,  1620,   517,   518,   519,   520,   481,   990,   678,   524,
     680,   481,   527,   481,    83,   199,   496,   201,   963,   963,
     689,   536,   661,   538,   534,    53,    14,   412,    56,   200,
     420,   546,    70,   554,   121,   164,  1175,    83,   196,   554,
     748,   556,   164,   130,    90,    73,   157,   158,   159,   196,
     420,   105,   106,   107,   102,   367,    83,   160,   196,  1061,
     545,   164,  1452,    90,   376,    50,    51,   946,    96,   196,
      98,   383,   780,  1191,   102,   103,  1194,   559,   181,    60,
    1092,   998,  1094,   395,  1226,   756,  1228,   946,   196,   158,
     159,   875,   607,   877,   406,   689,   134,   135,   946,   946,
     128,  1137,   196,   659,    83,   744,    70,    88,    70,   367,
      91,    90,   158,   159,   102,   103,   164,   160,   376,   196,
     378,    70,   160,   165,   494,   383,   796,   797,   707,  1011,
     157,   158,   159,   803,   804,   485,    83,   395,   181,    83,
     487,    19,    20,    90,   659,  1535,    90,    83,    31,    31,
     858,   102,   103,   196,    90,  1944,   199,  1776,   196,   205,
      70,  1780,   870,    70,   514,    32,   513,    50,    50,   519,
      53,    53,   200,    53,    54,    55,     4,   803,   164,   158,
     159,   105,   106,   107,   699,    75,    76,   534,  1381,    69,
     196,    19,    20,    31,    38,  1209,   835,   712,   545,   132,
     133,   548,   841,   119,   120,   121,   122,   123,   124,   521,
      70,   158,   159,    70,   158,   159,  1095,  1200,   122,   123,
     124,  1360,   158,   159,   198,   204,   198,   199,  1211,   257,
      68,   746,   198,   261,    70,  1252,  1095,   265,   112,   113,
     114,   111,   198,    81,   198,   199,   198,  1095,  1095,   119,
     120,   121,   122,   123,   124,   356,   357,  1375,  1808,  1809,
     775,  1804,  1805,   521,   198,   103,   843,   844,   198,   908,
     198,   198,   792,   111,    70,   191,  1922,    70,    70,    70,
     108,   119,   120,   121,   122,   123,   124,   672,   199,   160,
    1936,   196,    70,  1486,   196,   810,   160,  1916,   196,   689,
     138,   139,   140,   141,   142,   143,   144,   198,   164,   822,
      49,   826,  1931,    69,   160,    53,    54,    55,   181,    57,
     348,   191,   203,   161,  1341,  1327,   164,   165,   196,   167,
     168,    69,   170,   171,   172,   119,   120,   121,   122,   123,
     124,  1223,  1224,  1225,  1226,  1227,  1228,   185,   196,   831,
       9,  1233,  1231,   191,   232,   160,   184,   195,   196,   119,
     120,   121,   122,   123,   124,   160,   196,     8,    14,   198,
     130,   131,  1231,   196,   160,   356,   357,   358,   359,   360,
    1573,   160,   198,  1231,  1231,   198,   698,   215,   199,   392,
       9,   198,   420,   396,  1377,   130,  1035,    14,   130,   197,
    1518,   916,   430,   918,   232,   920,   434,   191,   181,   437,
     238,  1413,    14,   173,   198,   102,   931,   398,   997,   202,
     423,   197,   425,   426,   427,   428,   816,  1429,   197,   257,
     945,   191,   197,   261,   197,   196,   111,   196,   196,   949,
     698,     9,  1081,   157,   197,   757,   816,   197,   934,  1088,
     197,   934,   934,   197,    94,   934,   971,     9,   198,   487,
     488,   489,   490,   491,   492,   493,   981,    14,   181,   984,
     196,   986,  1888,     9,  1182,   990,    50,    51,    52,    53,
      54,    55,   872,    57,   196,   513,   798,   198,   800,   367,
     199,  1011,  1908,   199,   198,    69,   198,   198,   376,   757,
      83,  1917,   872,   197,   197,   383,   534,  1389,   197,  1391,
     198,   132,   196,   998,   197,     9,   828,   395,   203,     9,
     548,  1504,   203,   998,   203,  1040,   203,   203,   998,    70,
     998,   559,    32,   133,  1047,   180,  1538,   160,   136,   367,
     798,     9,   800,   197,   934,  1547,   160,    14,   376,   934,
     378,   579,     9,     9,   193,   383,   182,     9,   200,  1561,
     197,    14,   132,     9,   934,   955,  1048,   395,   203,   203,
     828,    14,   957,  1452,  1154,   203,   203,   605,   606,   197,
     160,   197,  1221,   196,   896,   955,   197,  1097,   102,   974,
     198,   198,   420,  1452,     9,   160,   136,     9,   197,   196,
     912,   913,   949,   196,  1452,  1452,    50,    51,    52,    53,
      54,    55,   640,   641,   961,   596,   963,   199,     9,    70,
     601,   602,    70,    70,   200,    69,    14,   198,    70,    70,
       9,   182,   199,  1154,   946,  1637,    14,   203,   896,  1154,
     199,  1280,  1524,   521,  1526,  1284,  1528,    14,   197,   193,
    1289,  1533,   198,   196,   912,   913,  1535,  1296,    32,   196,
    1050,    32,  1052,    14,   196,   196,    14,    52,   196,    70,
    1828,    70,  1187,    70,  1160,    70,  1535,  1160,  1160,   707,
    1050,  1160,  1052,    70,   196,  1200,   160,  1535,  1535,     9,
     197,   196,   198,   521,   198,  1042,  1211,  1212,   136,    14,
     182,   136,   160,  1223,  1224,     9,   197,  1227,    69,   198,
     203,    83,     9,  1233,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,   200,  1241,   200,
       9,   559,   196,   136,  1249,   196,   198,    14,    83,   199,
     199,   196,  1054,  1055,  1259,  1627,   136,   197,  1906,   203,
     199,   196,     9,    91,    32,   157,    77,  1396,   197,    59,
      60,  1400,  1244,   199,   792,   198,   794,   198,  1407,   197,
    1160,   198,  1774,   182,   136,  1160,    32,   197,   197,    78,
      79,    80,  1784,  1095,  1942,   203,     9,   203,   816,     9,
    1160,     9,    91,   203,   136,   203,  1054,  1055,   203,   197,
     200,     9,   830,   831,   197,   199,  1191,    14,   200,  1194,
      83,     9,   198,   196,   198,   197,   197,   197,  1487,   196,
     698,   119,   120,   121,   122,   123,   124,  1829,   198,   136,
    1345,   199,   130,   131,   134,   135,     9,  1352,   197,   197,
     203,  1356,   136,  1358,   872,   197,   145,   146,   147,   148,
     149,  1366,  1482,   881,   882,     9,   203,   156,    32,   203,
     203,  1376,  1377,   162,   163,   198,  1256,   203,   197,  1389,
     698,  1391,  1874,   171,  1186,   173,   198,   176,   197,   757,
     198,   909,   136,   199,   112,   169,  1256,   198,   186,   165,
     188,   190,    14,   191,   875,    83,   877,   197,   117,   136,
    1782,  1783,   197,   884,   885,   197,   934,   197,   199,   136,
      14,   181,   199,   198,    83,    14,    14,    83,   198,  1231,
     798,   949,   800,  1594,   196,  1237,   197,   955,  1186,   757,
     197,   136,   136,   961,   198,   963,    14,    14,   198,  1941,
    1252,  1253,    14,     9,  1946,     9,   199,    68,   200,  1339,
     828,    83,   181,   196,  1339,    83,     9,   199,   198,  1349,
     115,   102,   102,   160,  1349,   182,   172,    36,    14,   997,
     798,   196,   800,   197,    83,   196,   198,   178,     9,  1237,
     182,  1009,  1010,  1011,   197,   175,  1501,  1566,   816,  1504,
    1375,   182,    83,    14,  1252,  1253,   195,    83,   198,     9,
     828,   197,   197,   831,  1524,   199,  1526,    14,  1528,    83,
      83,    14,    14,  1533,  1042,    83,  1137,  1897,   896,   488,
    1048,   493,  1050,   991,  1052,   490,  1913,  1250,   937,  1341,
    1636,  1908,   609,  1623,   912,   913,  1661,  1574,  1426,  1746,
    1477,  1953,  1758,  1929,   872,  1073,  1619,  1473,   388,  1093,
    1227,  1164,  1089,  1010,  1222,  1037,  1223,   384,   961,  1569,
    1955,   434,  1544,   843,  1940,  1863,  1145,  1074,   896,  1097,
    1126,    -1,  1462,    -1,    78,    79,    80,    81,    -1,    -1,
      -1,    -1,  1465,  1341,   912,   913,    -1,    -1,    -1,    -1,
      -1,  1762,  1462,    -1,    -1,    -1,  1611,  1487,  1126,   103,
      -1,    -1,    -1,    -1,    -1,    -1,   934,  1627,    -1,    -1,
      -1,  1481,    -1,    -1,    -1,    -1,    -1,  1507,    -1,    -1,
      -1,    -1,  1507,  1513,    -1,  1515,    -1,   955,  1513,    -1,
    1515,    -1,  1160,  1518,    -1,   139,   140,   141,   142,   143,
    1452,    -1,    -1,    -1,    -1,    -1,    -1,  1537,    -1,  1539,
      -1,    -1,  1537,  1635,  1636,    -1,  1137,    -1,  1548,  1798,
    1799,    -1,    -1,   167,   168,    -1,   170,   171,   172,  1539,
      -1,    -1,    -1,    -1,    -1,     6,  1054,  1055,  1548,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   195,    -1,    -1,    -1,  1223,  1224,  1225,  1226,  1227,
    1228,    -1,    -1,    -1,    -1,  1233,  1757,    -1,    -1,     6,
      -1,    -1,  1881,    -1,    -1,    -1,  1244,    48,    -1,    -1,
    1048,    -1,  1050,  1535,  1052,    -1,  1054,  1055,  1256,    -1,
      -1,    -1,  1602,    -1,    -1,  1625,    -1,    -1,  1266,    -1,
    1625,    -1,    -1,  1822,  1634,    -1,    -1,  1757,    -1,    -1,
    1640,    48,  1767,    -1,    -1,    -1,    -1,  1647,    -1,    -1,
      -1,    -1,  1782,  1783,  1634,    -1,    -1,    -1,    -1,    -1,
    1640,  1901,    -1,    -1,    -1,    -1,    -1,  1647,    -1,    -1,
      -1,   112,    -1,    -1,    19,    20,   117,    -1,   119,   120,
     121,   122,   123,   124,   125,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1333,    -1,    -1,  1186,    -1,
      -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,
     117,    56,   119,   120,   121,   122,   123,   124,   125,    -1,
      -1,    -1,  1160,    -1,    -1,    -1,   167,   168,    -1,   170,
      -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,  1186,  1237,
     191,  1389,    -1,  1391,    -1,    -1,    -1,    -1,    -1,   200,
     167,   168,    -1,   170,  1252,  1253,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    59,
      60,    -1,    -1,    -1,   191,    -1,    -1,    -1,  1788,    -1,
      -1,    -1,    69,   200,    -1,    -1,    -1,    56,    -1,  1237,
      -1,    -1,    -1,    -1,    -1,    -1,  1244,    -1,  1788,    -1,
      -1,    -1,    -1,    -1,  1252,  1253,    -1,    -1,  1256,    -1,
      -1,    -1,    -1,    -1,  1462,    -1,  1951,    -1,    -1,    -1,
    1830,    -1,    -1,  1471,    -1,    -1,    -1,  1837,  1963,  1477,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1973,    -1,
    1830,  1976,    -1,  1341,   134,   135,    -1,  1837,    -1,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,  1872,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1881,    -1,    -1,    -1,    69,  1524,   232,  1526,    -1,
    1528,    -1,  1872,  1888,  1894,  1533,    -1,    -1,    -1,    -1,
      -1,  1539,    -1,  1341,    -1,    -1,  1544,    -1,    -1,    -1,
    1548,    -1,    -1,  1908,  1894,    -1,    -1,   197,    -1,    -1,
      -1,    -1,  1917,    -1,    -1,    -1,     6,    -1,  1566,    -1,
      -1,  1569,    -1,    -1,  1572,    -1,    -1,   282,    -1,   284,
      -1,     6,    -1,    -1,  1582,    -1,    -1,    -1,    -1,    -1,
      -1,  1589,    -1,    -1,  1954,    -1,    -1,    -1,  1596,  1959,
    1598,     6,    -1,    -1,    -1,    -1,  1604,    -1,    48,    -1,
      -1,    -1,    -1,    -1,  1954,    -1,    -1,    -1,    -1,  1959,
      -1,    -1,    -1,    48,    -1,    -1,    -1,    -1,    -1,  1627,
      -1,    -1,    -1,    -1,    -1,   340,  1634,  1635,  1636,    -1,
      -1,    -1,  1640,    48,    -1,    -1,    -1,    -1,    -1,  1647,
      -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   367,   282,  1462,   284,    -1,    -1,    -1,    -1,
      -1,   376,   112,    -1,    -1,    -1,    -1,   117,   383,   119,
     120,   121,   122,   123,   124,   125,    -1,   112,    -1,    -1,
     395,    -1,   117,    -1,   119,   120,   121,   122,   123,   124,
     125,   406,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,
      -1,    -1,   117,    -1,   119,   120,   121,   122,   123,   124,
     125,   340,    -1,    -1,    -1,    -1,   431,   167,   168,   434,
     170,    -1,    -1,    -1,    -1,    -1,    -1,     6,    -1,    -1,
      -1,  1539,   167,   168,    -1,   170,  1544,    -1,    -1,    -1,
    1548,   191,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1757,
     200,    -1,   167,   168,    -1,   170,   191,    -1,    -1,    -1,
      -1,    -1,     6,    -1,    -1,   200,   481,    -1,    -1,    48,
    1778,    -1,    -1,    -1,  1782,  1783,   191,    -1,    -1,    -1,
    1788,    -1,    -1,    -1,    -1,   200,    -1,    -1,    -1,  1797,
      -1,    -1,    -1,    -1,    -1,    -1,  1804,  1805,    -1,    -1,
    1808,  1809,   431,    -1,    48,   434,   521,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1822,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1830,    -1,    -1,    -1,  1634,  1635,  1636,  1837,
      -1,    -1,  1640,   112,    -1,    -1,    -1,    -1,   117,  1647,
     119,   120,   121,   122,   123,   124,   125,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   573,    -1,
     575,    -1,    -1,   578,  1872,    -1,    -1,    -1,   112,   282,
      -1,   284,  1880,   117,    -1,   119,   120,   121,   122,   123,
     124,   125,    -1,    -1,    -1,    -1,  1894,    -1,   167,   168,
      -1,   170,  1900,    -1,    -1,    -1,   611,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   191,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   200,    -1,   167,   168,    -1,   170,   340,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,   573,    -1,  1954,   191,    -1,    -1,
      -1,  1959,    -1,   668,   669,    -1,   200,    -1,    -1,    -1,
      -1,    -1,   677,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    59,    60,    -1,    -1,    -1,    -1,
    1788,    -1,    -1,   698,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,   431,    -1,
      -1,   434,  1830,    69,    -1,    -1,    -1,    -1,    -1,  1837,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   668,
     669,    -1,   757,    -1,    -1,    -1,    -1,    -1,   677,   134,
     135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1872,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   792,    -1,    -1,
      -1,    -1,    -1,   798,    -1,   800,  1894,    31,    -1,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,   197,   828,   829,    -1,    81,    -1,    83,    84,
      -1,   836,    -1,    -1,    -1,    69,    -1,    -1,   843,   844,
     845,   846,   847,   848,   849,    -1,    31,    -1,   103,    -1,
      -1,    -1,   857,    -1,    -1,    -1,  1954,    -1,    -1,    -1,
      -1,  1959,    -1,    -1,    -1,    -1,    -1,   203,   873,    -1,
     573,    -1,   575,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,   139,   140,   141,   142,   143,    -1,
      -1,   896,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   910,    -1,   912,   913,    -1,
     165,    -1,   167,   168,    -1,   170,   171,   172,   103,    -1,
      -1,    -1,    -1,    -1,   843,   844,   111,    -1,    -1,    -1,
      -1,   936,   937,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     195,   946,    -1,    -1,   199,    -1,   201,   952,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
     965,    -1,    -1,   197,    -1,   668,   669,    -1,   973,    81,
      -1,   976,    -1,    -1,   677,    -1,   161,    -1,    -1,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,   994,
      -1,   103,    -1,   998,    -1,    -1,    -1,    -1,    -1,    -1,
     185,    -1,    -1,    -1,    -1,    -1,  1011,    -1,    -1,    -1,
     195,   196,    -1,    -1,    -1,    -1,    -1,   936,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,   952,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   965,    -1,    -1,  1054,
    1055,    -1,    -1,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,  1068,    -1,    -1,    -1,  1072,    -1,  1074,
      81,    -1,    83,    84,    -1,   994,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   195,  1089,  1090,  1091,  1092,  1093,  1094,
    1095,    -1,   103,  1098,  1099,  1100,  1101,  1102,  1103,  1104,
    1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,
    1125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   836,    -1,    -1,  1141,    -1,    -1,    -1,
     843,   844,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1068,
      -1,    -1,    -1,  1072,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1174,
      -1,  1176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1186,    -1,    -1,   195,    -1,    -1,    -1,   199,    -1,
     201,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1203,    -1,
      -1,  1206,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    19,    20,    -1,    -1,    -1,  1223,  1224,
    1225,  1226,  1227,  1228,    30,    -1,  1231,    -1,  1233,    -1,
      -1,    -1,  1237,   936,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,  1252,  1253,   952,
    1255,    -1,    -1,    -1,    -1,  1174,    -1,  1176,    -1,    -1,
    1265,    -1,   965,    -1,    -1,    -1,  1271,    -1,    -1,  1274,
      -1,  1276,    -1,    10,    11,    12,    -1,    -1,    -1,    59,
      60,    31,    -1,    -1,  1203,    -1,    -1,  1206,    -1,    -1,
      -1,   994,    -1,    30,    31,  1300,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1334,
    1335,    81,    69,  1338,    -1,    -1,  1341,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,  1265,    -1,    -1,    -1,
      -1,    -1,  1271,   103,   134,   135,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1068,    -1,    -1,    -1,  1072,
      -1,  1074,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1389,    -1,  1391,    -1,    -1,   139,
     140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   161,    -1,    -1,   164,  1334,  1335,   167,   168,    -1,
     170,   171,   172,    -1,   174,    -1,   232,    -1,    -1,  1434,
      -1,  1436,    -1,    -1,    -1,    -1,    -1,  1442,    -1,  1444,
      -1,  1446,    31,    -1,    -1,   195,  1451,  1452,    -1,    -1,
    1455,    -1,  1457,    -1,    -1,  1460,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   200,    -1,    -1,  1471,  1472,    -1,    -1,
    1475,  1174,    -1,  1176,    -1,    -1,    -1,  1482,    -1,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1203,    -1,    -1,  1206,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,  1434,    -1,  1436,    -1,  1524,
      -1,  1526,    -1,  1528,    -1,    -1,    -1,    -1,  1533,    -1,
    1535,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1549,    -1,    -1,  1552,    -1,   138,
     139,   140,   141,   142,   143,   144,    -1,    -1,    -1,  1564,
    1565,   367,  1265,  1482,    -1,    -1,    -1,  1572,  1271,  1574,
     376,    -1,   161,    -1,    -1,   164,   165,   383,   167,   168,
      -1,   170,   171,   172,    -1,   174,    -1,    -1,    -1,   395,
      -1,  1596,    -1,  1598,    -1,    -1,   185,    -1,    -1,  1604,
     406,    -1,    31,    -1,    -1,    -1,   195,   196,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1627,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1549,  1334,  1335,    -1,    -1,    -1,    -1,    -1,    -1,  1644,
    1645,  1646,    -1,    -1,    -1,    -1,  1651,    -1,  1653,    -1,
      -1,    -1,    81,    -1,  1659,    -1,  1661,    10,    11,    12,
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,   481,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   521,    69,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1434,   161,  1436,    -1,   164,    -1,    -1,   167,   168,
    1659,   170,   171,   172,    -1,   174,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1761,    -1,    68,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,  1471,    -1,
      -1,    81,   578,  1778,    -1,    -1,    -1,  1782,  1783,  1482,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1797,   103,    -1,    -1,    -1,    -1,  1803,    -1,
      -1,    -1,    -1,    -1,    -1,   611,    -1,    -1,    -1,  1814,
      -1,    -1,    -1,    -1,    -1,  1820,    -1,    -1,    -1,  1824,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,  1846,  1761,    -1,    -1,    -1,  1549,   200,    -1,    -1,
      -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,   174,    -1,    -1,    -1,    -1,  1572,
      -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,
      -1,  1886,    -1,    -1,    -1,   195,   196,    -1,    -1,    -1,
      -1,  1896,   698,  1596,    -1,  1598,    -1,    -1,    -1,    -1,
      -1,  1604,    -1,    -1,    -1,    -1,    -1,    -1,  1913,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1922,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1846,    -1,  1934,
      -1,  1936,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1955,   757,  1957,    -1,    30,    31,  1659,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,   792,    -1,    -1,    -1,
      -1,    -1,   798,    69,   800,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1922,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,  1934,    -1,  1936,    -1,    -1,
      -1,    -1,   828,   829,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,  1955,    -1,  1957,   845,
     846,   847,   848,   849,    -1,    31,    -1,   103,    -1,    -1,
      -1,   857,    19,    20,    -1,    -1,    -1,    -1,  1761,    -1,
      -1,    -1,    -1,    30,    -1,    -1,    -1,   873,    -1,    -1,
     139,   140,   141,   142,   143,  1778,    -1,    -1,    -1,    -1,
      -1,    -1,    68,   139,   140,   141,   142,   143,    -1,    -1,
     896,    -1,    -1,    -1,  1797,    81,   165,    -1,   167,   168,
     169,   170,   171,   172,   910,   161,   912,   913,   164,    -1,
      -1,   167,   168,    -1,   170,   171,   172,   103,    -1,    -1,
      -1,    -1,    -1,    -1,   200,    -1,   195,   196,    -1,    -1,
      -1,   937,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,
     946,    -1,    -1,  1846,   200,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,   142,   143,   144,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   973,    -1,    -1,
     976,    -1,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
      -1,    -1,   998,    -1,    -1,    -1,    -1,    -1,    -1,   185,
      -1,    -1,    -1,    -1,    -1,  1011,    -1,    -1,    -1,   195,
     196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1922,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1934,    -1,  1936,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1054,  1055,
      -1,    -1,  1955,    -1,  1957,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   232,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1089,  1090,  1091,  1092,  1093,  1094,  1095,
      -1,    -1,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1141,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    69,
    1186,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
     367,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   376,
      -1,    -1,    -1,    -1,    -1,    -1,   383,  1223,  1224,  1225,
    1226,  1227,  1228,    -1,    -1,  1231,    -1,  1233,   395,    -1,
      -1,  1237,    -1,   139,   140,   141,   142,   143,    81,   406,
      -1,    -1,    -1,    -1,    -1,    -1,  1252,  1253,    -1,  1255,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   164,    -1,
     103,   167,   168,    -1,   170,   171,   172,    -1,  1274,    -1,
    1276,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   125,    -1,    -1,    -1,    -1,    -1,    -1,   195,
      -1,    -1,    -1,   199,  1300,   138,   139,   140,   141,   142,
     143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     200,    -1,    -1,   578,   481,    -1,    -1,    -1,   161,    -1,
      -1,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,  1338,    -1,    -1,  1341,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,   611,    -1,    -1,    -1,
      -1,    -1,   195,    -1,   521,    -1,    -1,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,  1389,    57,  1391,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,   578,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,  1442,    -1,  1444,    -1,
    1446,    -1,    -1,    -1,   611,  1451,  1452,    69,    81,  1455,
      -1,  1457,    -1,    -1,  1460,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1472,    -1,    -1,  1475,
     103,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,   139,   140,   141,   142,
     143,    -1,    -1,    -1,   136,    69,    -1,    -1,  1524,    -1,
    1526,    -1,  1528,    -1,    -1,   198,    -1,  1533,    -1,  1535,
      -1,   698,    -1,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,    -1,    -1,    -1,  1552,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1564,  1565,
      -1,    -1,   195,   196,   829,    -1,    -1,    -1,  1574,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
     845,   846,   847,   848,   849,    -1,    -1,    -1,    -1,    -1,
     757,    -1,   857,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,  1627,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,   798,    69,   800,    -1,    -1,    81,    -1,  1644,  1645,
    1646,    -1,    70,    -1,    -1,  1651,   200,  1653,    -1,    -1,
      78,    79,    80,    81,    -1,  1661,    -1,    -1,   103,    -1,
      -1,   828,   829,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,   845,   846,
     847,   848,   849,   128,    -1,    -1,    -1,    -1,    -1,    -1,
     857,    -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   973,    -1,
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,   167,   168,    -1,   170,   171,   172,   156,   896,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,   912,   913,    -1,   176,    -1,
     195,   196,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,
      -1,    -1,   190,   200,    -1,    -1,    -1,   195,   196,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1782,  1783,    -1,   946,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,  1803,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   973,    81,  1814,    83,
      -1,    85,    -1,    -1,  1820,    -1,    -1,    -1,  1824,    -1,
      -1,    -1,    -1,    -1,  1089,  1090,    59,    60,  1093,   103,
      -1,   998,    -1,  1098,  1099,  1100,  1101,  1102,  1103,  1104,
    1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,
    1125,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,    -1,    -1,  1141,    -1,    -1,    -1,
    1886,    -1,    -1,    -1,    -1,    -1,    -1,  1054,  1055,    -1,
    1896,    -1,    -1,   167,   168,    -1,   170,   171,   172,    -1,
      -1,   134,   135,    -1,    -1,    -1,    -1,  1913,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   195,  1089,  1090,  1091,  1092,  1093,  1094,  1095,    -1,
     103,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,    -1,    -1,    -1,  1141,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1255,    -1,    -1,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,  1274,
      -1,  1276,    -1,    10,    11,    12,    -1,    91,    -1,  1186,
      -1,    -1,   195,   196,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    30,    31,  1300,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,  1231,   139,   140,   141,   142,   143,
    1237,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1252,  1253,   161,  1255,    -1,
     164,    -1,    -1,   167,   168,    -1,   170,   171,   172,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1274,    -1,  1276,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,   195,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1300,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,  1341,    -1,    -1,  1442,    81,  1444,
      -1,  1446,    -1,    -1,    -1,    -1,  1451,    10,    11,    12,
    1455,    -1,  1457,    -1,    -1,  1460,    -1,    -1,    -1,    -1,
     103,    -1,    -1,   200,    -1,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,   139,   140,   141,   142,
     143,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,     7,   161,    -1,
      -1,   164,   165,    13,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,    -1,    -1,  1442,    -1,  1444,    -1,  1446,
      -1,    -1,    -1,    -1,  1451,  1452,    -1,  1552,  1455,    -1,
    1457,    -1,   195,  1460,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,   200,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,   108,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,  1535,   119,
     120,   121,   122,   123,   124,   198,    -1,   127,   128,  1644,
    1645,  1646,    -1,    -1,    -1,  1552,  1651,   137,   138,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,   179,
      -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,
     190,   191,   192,    -1,    -1,   195,   196,    -1,    -1,    -1,
      -1,   201,   202,    -1,   204,   205,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,  1644,  1645,  1646,
      -1,    -1,    -1,    -1,  1651,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,  1660,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    81,    -1,  1803,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,  1814,
      -1,    -1,    -1,    -1,    -1,  1820,    -1,    -1,   103,  1824,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,   139,   140,   141,   142,   143,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,   164,
      -1,  1886,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,  1803,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1814,    -1,    -1,
     195,    -1,    48,  1820,    50,    51,    -1,  1824,    -1,    -1,
      56,   200,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    -1,
      -1,  1848,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,
      96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,    -1,   108,   109,   110,   111,   112,   113,   114,  1886,
     116,   117,   118,   119,   120,   121,   122,   123,   124,    -1,
     126,   127,   128,   129,   130,   131,   200,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,    -1,   145,
     146,   147,   148,   149,    -1,    -1,    -1,   153,    -1,    -1,
     156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,   173,    -1,    -1,
     176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,
     186,    -1,   188,    -1,   190,   191,   192,    -1,    -1,   195,
     196,    -1,   198,   199,   200,   201,   202,    -1,   204,   205,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,
      93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,
     103,   104,    -1,    -1,    -1,   108,   109,   110,   111,   112,
     113,   114,    -1,   116,   117,   118,   119,   120,   121,   122,
     123,   124,    -1,   126,   127,   128,   129,   130,   131,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,
     153,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
     173,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,
      -1,    -1,   185,   186,    -1,   188,    -1,   190,   191,   192,
      -1,    -1,   195,   196,    -1,   198,   199,   200,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    -1,    96,    -1,    98,    -1,
     100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,
     110,   111,   112,   113,   114,    -1,   116,   117,   118,   119,
     120,   121,   122,   123,   124,    -1,   126,   127,   128,   129,
     130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,   153,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,   173,    -1,    -1,   176,    -1,    -1,   179,
      -1,    -1,    -1,    -1,    -1,   185,   186,    -1,   188,    -1,
     190,   191,   192,    -1,    -1,   195,   196,    -1,   198,   199,
      -1,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,
      -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,
      -1,   108,   109,   110,   111,    -1,   113,   114,    -1,   116,
      -1,   118,   119,   120,   121,   122,   123,   124,    -1,   126,
     127,   128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,   153,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,   173,    -1,    -1,   176,
      -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,
      -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,
      -1,   198,   199,   200,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,
      94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,
     104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,
     114,    -1,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,    -1,   126,   127,   128,    -1,   130,   131,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,   153,
      -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,   173,
      -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,
      -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,
      -1,   195,   196,    -1,   198,   199,   200,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,
      91,    92,    93,    94,    -1,    96,    -1,    98,    -1,   100,
      -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,    -1,   113,   114,    -1,   116,    -1,   118,   119,   120,
     121,   122,   123,   124,    -1,   126,   127,   128,    -1,   130,
     131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,   153,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,   173,    -1,    -1,   176,    -1,    -1,   179,    -1,
      -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,
     191,   192,    -1,    -1,   195,   196,    -1,   198,   199,   200,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    87,
      -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,
      98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,
     108,   109,   110,   111,    -1,   113,   114,    -1,   116,    -1,
     118,   119,   120,   121,   122,   123,   124,    -1,   126,   127,
     128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,   153,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,   173,    -1,    -1,   176,    -1,
      -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,
      -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,
     198,   199,   200,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,    94,
      95,    96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,   114,
      -1,   116,    -1,   118,   119,   120,   121,   122,   123,   124,
      -1,   126,   127,   128,    -1,   130,   131,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,   153,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,   173,    -1,
      -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,
     185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,
     195,   196,    -1,   198,   199,    -1,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,
      92,    93,    94,    -1,    96,    -1,    98,    -1,   100,   101,
      -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,   111,
      -1,   113,   114,    -1,   116,    -1,   118,   119,   120,   121,
     122,   123,   124,    -1,   126,   127,   128,    -1,   130,   131,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,   153,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,   173,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,
      -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,
     192,    -1,    -1,   195,   196,    -1,   198,   199,    -1,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    87,    -1,
      -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,    98,
      -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,
     109,   110,   111,    -1,   113,   114,    -1,   116,    -1,   118,
     119,   120,   121,   122,   123,   124,    -1,   126,   127,   128,
      -1,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,    -1,    -1,    -1,   153,    -1,    -1,   156,    -1,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,   173,    -1,    -1,   176,    -1,    -1,
     179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,
      -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,   198,
     199,   200,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,
      96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,    -1,   108,   109,   110,   111,    -1,   113,   114,    -1,
     116,    -1,   118,   119,   120,   121,   122,   123,   124,    -1,
     126,   127,   128,    -1,   130,   131,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,    -1,   145,
     146,   147,   148,   149,    -1,    -1,    -1,   153,    -1,    -1,
     156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,   173,    -1,    -1,
     176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,
      -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,
     196,    -1,   198,   199,   200,   201,   202,    -1,   204,   205,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,
      93,    94,    -1,    96,    -1,    98,    99,   100,    -1,    -1,
     103,   104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,
     113,   114,    -1,   116,    -1,   118,   119,   120,   121,   122,
     123,   124,    -1,   126,   127,   128,    -1,   130,   131,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,
     153,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
     173,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,
      -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,
      -1,    -1,   195,   196,    -1,   198,   199,    -1,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    -1,    96,    -1,    98,    -1,
     100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,
     110,   111,    -1,   113,   114,    -1,   116,    -1,   118,   119,
     120,   121,   122,   123,   124,    -1,   126,   127,   128,    -1,
     130,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,   153,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,   173,    -1,    -1,   176,    -1,    -1,   179,
      -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,
     190,   191,   192,    -1,    -1,   195,   196,    -1,   198,   199,
     200,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,
      97,    98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,
      -1,   108,   109,   110,   111,    -1,   113,   114,    -1,   116,
      -1,   118,   119,   120,   121,   122,   123,   124,    -1,   126,
     127,   128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,   153,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,   173,    -1,    -1,   176,
      -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,
      -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,
      -1,   198,   199,    -1,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,
      94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,
     104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,
     114,    -1,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,    -1,   126,   127,   128,    -1,   130,   131,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,   153,
      -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,   173,
      -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,
      -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,
      -1,   195,   196,    -1,   198,   199,   200,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,
      91,    92,    93,    94,    -1,    96,    -1,    98,    -1,   100,
      -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,    -1,   113,   114,    -1,   116,    -1,   118,   119,   120,
     121,   122,   123,   124,    -1,   126,   127,   128,    -1,   130,
     131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,   153,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,   173,    -1,    -1,   176,    -1,    -1,   179,    -1,
      -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,
     191,   192,    -1,    -1,   195,   196,    -1,   198,   199,   200,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    87,
      -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,
      98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,
     108,   109,   110,   111,    -1,   113,   114,    -1,   116,    -1,
     118,   119,   120,   121,   122,   123,   124,    -1,   126,   127,
     128,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,   153,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,   173,    -1,    -1,   176,    -1,
      -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,
      -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,
     198,   199,   200,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,    94,
      -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,   114,
      -1,   116,    -1,   118,   119,   120,   121,   122,   123,   124,
      -1,   126,   127,   128,    -1,   130,   131,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,   153,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,   173,    -1,
      -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,
     185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,
     195,   196,    -1,   198,   199,   200,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,
      92,    93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,
      -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,   111,
      -1,   113,   114,    -1,   116,    -1,   118,   119,   120,   121,
     122,   123,   124,    -1,   126,   127,   128,    -1,   130,   131,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,   153,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,   173,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,
      -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,
     192,    -1,    -1,   195,   196,    -1,   198,   199,    -1,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    87,    -1,
      -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,    98,
      -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,
     109,   110,   111,    -1,   113,   114,    -1,   116,    -1,   118,
     119,   120,   121,   122,   123,   124,    -1,   126,   127,   128,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,    -1,    -1,    -1,   153,    -1,    -1,   156,    -1,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,
     179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,
      -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,   198,
     199,    -1,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,
      96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,
      -1,    -1,   108,   109,   110,   111,    -1,   113,   114,    -1,
     116,    -1,   118,   119,   120,   121,   122,   123,   124,    -1,
     126,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,    -1,   145,
     146,   147,   148,   149,    -1,    -1,    -1,   153,    -1,    -1,
     156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
     176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,
      -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,
     196,    -1,   198,   199,    -1,   201,   202,    -1,   204,   205,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,
      93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,
     103,   104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,
     113,   114,    -1,   116,    -1,   118,   119,   120,   121,   122,
     123,   124,    -1,   126,   127,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,
     153,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,
      -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,
      -1,    -1,   195,   196,    -1,   198,   199,    -1,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    -1,    96,    -1,    98,    -1,
     100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,
     110,   111,    -1,   113,   114,    -1,   116,    -1,   118,   119,
     120,   121,   122,   123,   124,    -1,   126,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,   153,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,   179,
      -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,
     190,   191,   192,    -1,    -1,   195,   196,    -1,   198,   199,
      -1,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      87,    -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,
      -1,    98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,
      -1,   108,   109,   110,   111,    -1,   113,   114,    -1,   116,
      -1,   118,   119,   120,   121,   122,   123,   124,    -1,   126,
     127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,   153,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,
      -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,
      -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,
      -1,   198,   199,    -1,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,
      94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,
     104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,
     114,    -1,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,    -1,   126,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,   153,
      -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    -1,
      -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,
      -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,
      -1,   195,   196,    -1,   198,   199,    -1,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    -1,    -1,   176,    -1,    -1,   179,    -1,
      -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,
     191,   192,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,
      -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,
      -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,
     198,    -1,    -1,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    31,    -1,    13,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,   174,
      -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,
     185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,
     195,   196,    -1,    -1,    -1,    -1,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,
     122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,
      -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,
     192,    -1,    -1,   195,   196,    -1,    -1,   199,    -1,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,   120,   121,   122,   123,   124,    -1,    -1,   127,   128,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,   174,    -1,   176,    -1,    -1,
     179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,
      -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,    -1,
      -1,    -1,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,   120,   121,   122,   123,   124,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,    -1,   145,
     146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,
     156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
     176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,
      -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,
     196,    -1,    -1,    -1,    -1,   201,   202,    -1,   204,   205,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,
     123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,
      -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,
      -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,
      -1,    -1,   195,   196,    10,    11,    12,    -1,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    69,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,   179,
      -1,    -1,   198,    -1,    -1,   185,    -1,    -1,    -1,    -1,
     190,   191,   192,    -1,    -1,   195,   196,    -1,   198,    11,
      12,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    69,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,   120,   121,   122,   123,   124,    -1,    -1,
     127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,
      -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,
      -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,
      -1,   198,    -1,    -1,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,
     124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,
      -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    -1,
      -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,
      -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,
      -1,   195,   196,    10,    11,    12,    -1,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    69,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    -1,    -1,   176,    -1,    -1,   179,    -1,
      -1,   198,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,
     191,   192,    -1,    -1,   195,   196,   197,    -1,    -1,    -1,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    32,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,
      -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,
      -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,
      -1,    -1,    -1,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,
     185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,
     195,   196,    -1,    -1,    -1,    -1,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,
     122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,
      -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,
     192,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,   120,   121,   122,   123,   124,    -1,    -1,   127,   128,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,
     179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,
      -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,    -1,
      -1,    -1,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,   120,   121,   122,   123,   124,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,    -1,   145,
     146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,
     156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
     176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,
      -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,
     196,    10,    11,    12,    -1,   201,   202,    -1,   204,   205,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      69,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,
     123,   124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,
      -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,   198,
      -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,
      -1,    -1,   195,   196,    10,    11,    12,    -1,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    27,    -1,    13,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    69,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,   102,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,     3,     4,   176,     6,     7,   179,
      -1,    10,    11,    12,    13,   185,    -1,    -1,    -1,    -1,
     190,   191,   192,    -1,    -1,   195,   196,    -1,    -1,    28,
      29,   201,   202,    -1,   204,   205,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    81,   128,
      -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,
       7,    -1,   161,    10,    11,    12,    13,    -1,   167,   168,
      -1,   170,   171,   172,   173,    -1,   175,    -1,    -1,   178,
      -1,    28,    29,    -1,    -1,    -1,   139,   140,   141,   142,
     143,    -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,    -1,
     199,    -1,   201,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      57,   164,    -1,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    83,    84,    -1,    -1,
      -1,    -1,   195,    -1,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      81,   128,    -1,   130,   131,   132,   133,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,    -1,   161,    10,    11,    12,    13,    -1,
     167,   168,    -1,   170,   171,   172,   173,    -1,   175,    -1,
      -1,   178,    -1,    28,    29,    -1,    31,    -1,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,   195,    -1,
      -1,    -1,   199,    -1,   201,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    57,    -1,    -1,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    68,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   195,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,    -1,    -1,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,   144,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,   173,    -1,
     175,    -1,    -1,   178,     3,     4,    -1,     6,     7,    -1,
     185,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
     195,   196,    -1,    -1,    -1,   200,    -1,    -1,    -1,    28,
      29,    -1,    31,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    57,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    69,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
      -1,    -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,   173,    -1,   175,    -1,    -1,   178,
       3,     4,    -1,     6,     7,    -1,   185,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,
      -1,   200,    -1,    -1,    -1,    28,    29,    -1,    31,    -1,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    57,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    -1,    69,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,    -1,   130,   131,   132,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,    -1,
      -1,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
     173,    -1,   175,    -1,    -1,   178,     3,     4,    -1,     6,
       7,    -1,   185,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,   195,   196,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    29,    -1,    31,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    68,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,    -1,    -1,   131,   132,   133,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,   173,    -1,   175,    -1,
      -1,   178,    -1,     3,     4,    -1,     6,     7,   185,   186,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,   195,   196,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    29,
      -1,    31,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,    -1,
      -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,   173,    -1,   175,    -1,    -1,   178,     3,
       4,     5,     6,     7,    -1,   185,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    29,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    57,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,   162,   163,
      -1,    81,    -1,   167,   168,    -1,   170,   171,   172,   173,
      -1,   175,   176,    -1,   178,    -1,    -1,    -1,    -1,    -1,
      -1,   185,   186,   103,   188,    -1,   190,   191,    -1,     3,
       4,   195,     6,     7,    -1,    -1,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   125,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    29,    -1,    -1,   138,   139,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   161,    -1,    57,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,   128,    -1,   130,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,   161,    10,    11,
      12,    13,    -1,   167,   168,    -1,   170,   171,   172,   173,
      -1,   175,    -1,    -1,   178,    -1,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   195,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,    -1,   130,   131,
     132,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,   161,
      10,    11,    12,    13,    -1,   167,   168,    -1,   170,   171,
     172,   173,    -1,   175,    -1,    -1,   178,    -1,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   195,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,    -1,
      -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   161,    -1,    -1,    -1,    -1,    -1,   167,   168,    -1,
     170,   171,   172,   173,    -1,   175,    -1,    -1,   178,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    70,    -1,    -1,    -1,   198,    -1,   111,
     112,    78,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   197,    81,    -1,    -1,    -1,   161,
      -1,    -1,   164,    -1,    -1,   167,   168,    -1,   170,   171,
     172,   138,   139,   140,   141,   142,   143,   103,   145,   146,
     147,   148,   149,    -1,   186,   111,   112,    -1,    -1,   156,
      38,    -1,    -1,   195,   161,   162,   163,   164,   165,    -1,
     167,   168,   197,   170,   171,   172,    -1,    -1,    -1,   176,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   185,    -1,
      -1,    -1,    70,   190,    -1,    -1,    -1,    -1,   195,   196,
      78,    79,    80,    81,    -1,    83,    84,    -1,   164,    -1,
      -1,   167,   168,    91,   170,   171,   172,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,
      -1,   119,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,   137,
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    69,    -1,    -1,    50,    51,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    70,    -1,    -1,   176,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,   190,    -1,    -1,    -1,    91,   195,   196,    -1,
      -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   156,    -1,    -1,    -1,   119,   161,    -1,    -1,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
     185,   145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,
     195,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    70,
      -1,    -1,   176,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,   190,    -1,    -1,    -1,
      91,   195,   196,    -1,    -1,   199,    -1,   201,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    69,
      -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    70,    71,    -1,   176,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,   190,
      -1,    -1,    -1,    91,   195,   196,    -1,    -1,    -1,    -1,
     201,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    70,    -1,    -1,   176,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,   190,    -1,    -1,    -1,    91,   195,   196,    -1,
      -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    70,    -1,
      -1,   176,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,   190,    -1,    -1,    -1,    91,
     195,   196,    -1,    -1,    -1,    -1,   201,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,   176,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   190,    -1,
      -1,    -1,    -1,   195,   196,    -1,    -1,    30,    31,   201,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,   136,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,   136,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,   136,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,   136,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,   136,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,   136,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    -1,   136,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    -1,   136,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,   136,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,   136,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,
      -1,    -1,    69,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,   136,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,
      -1,    -1,   190,    91,    -1,    -1,    -1,   195,   196,    -1,
      -1,    -1,    -1,   201,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    69,    -1,    -1,    -1,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,
      -1,    -1,   190,    91,    -1,    -1,    -1,   195,   196,    -1,
      -1,    -1,    -1,   201,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,   190,    -1,    -1,    -1,    -1,   195,   196,    -1,
      -1,    -1,    -1,   201,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    32,    -1,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    12,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   207,   208,     0,   209,     3,     4,     5,     6,     7,
      13,    27,    28,    29,    48,    50,    51,    56,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    70,
      71,    72,    73,    74,    78,    79,    80,    81,    82,    83,
      84,    86,    87,    91,    92,    93,    94,    96,    98,   100,
     103,   104,   108,   109,   110,   111,   112,   113,   114,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   126,   127,
     128,   129,   130,   131,   137,   138,   139,   140,   141,   142,
     143,   145,   146,   147,   148,   149,   153,   156,   161,   162,
     163,   164,   165,   167,   168,   170,   171,   172,   173,   176,
     179,   185,   186,   188,   190,   191,   192,   195,   196,   198,
     199,   201,   202,   204,   205,   210,   213,   223,   224,   225,
     226,   227,   230,   246,   247,   251,   254,   261,   267,   327,
     328,   336,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   351,   354,   366,   367,   374,   377,   380,   383,
     386,   392,   394,   395,   397,   407,   408,   409,   411,   416,
     421,   441,   449,   451,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   476,   478,   480,   119,   120,
     121,   137,   161,   171,   196,   213,   246,   327,   348,   453,
     348,   196,   348,   348,   348,   348,   108,   348,   348,   439,
     440,   348,   348,   348,   348,    81,    83,    91,   119,   139,
     140,   141,   142,   143,   156,   196,   224,   367,   408,   411,
     416,   453,   456,   453,   348,   348,   348,   348,   348,   348,
     348,   348,    38,   348,   467,   468,   119,   130,   196,   224,
     259,   408,   409,   410,   412,   416,   450,   451,   452,   460,
     464,   465,   348,   196,   337,   413,   196,   337,   358,   338,
     348,   232,   337,   196,   196,   196,   337,   198,   348,   213,
     198,   348,     3,     4,     6,     7,    10,    11,    12,    13,
      28,    29,    31,    57,    68,    71,    72,    73,    74,    75,
      76,    77,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   128,   130,   131,   132,
     133,   137,   138,   144,   161,   165,   173,   175,   178,   185,
     196,   213,   214,   215,   226,   481,   501,   502,   505,   198,
     343,   345,   348,   199,   239,   348,   111,   112,   161,   164,
     186,   216,   217,   218,   219,   223,    83,   201,   293,   294,
      83,   295,   121,   130,   120,   130,   196,   196,   196,   196,
     213,   265,   484,   196,   196,    70,    70,    70,    70,    70,
     338,    83,    90,   157,   158,   159,   473,   474,   164,   199,
     223,   223,   213,   266,   484,   165,   196,   484,   484,    83,
     192,   199,   359,    28,   336,   340,   348,   349,   453,   457,
     228,   199,   462,    90,   414,   473,    90,   473,   473,    32,
     164,   181,   485,   196,     9,   198,    38,   245,   165,   264,
     484,   119,   191,   246,   328,   198,   198,   198,   198,   198,
     198,   198,   198,    10,    11,    12,    30,    31,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    57,    69,   198,    70,    70,   199,   160,   131,   171,
     173,   186,   188,   267,   326,   327,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    59,
      60,   134,   135,   443,    70,   199,   448,   196,   196,    70,
     199,   196,   245,   246,    14,   348,   198,   136,    49,   213,
     438,    90,   336,   349,   160,   453,   136,   203,     9,   423,
     260,   336,   349,   453,   485,   160,   196,   415,   443,   448,
     197,   348,    32,   230,     8,   360,     9,   198,   230,   231,
     338,   339,   348,   213,   279,   234,   198,   198,   198,   138,
     144,   505,   505,   181,   504,   196,   111,   505,    14,   160,
     138,   144,   161,   213,   215,   198,   198,   198,   240,   115,
     178,   198,   216,   218,   216,   218,   164,   218,   223,   223,
     218,   199,     9,   424,   198,   102,   164,   199,   453,     9,
     198,    14,     9,   198,   130,   130,   453,   477,   338,   336,
     349,   453,   456,   457,   197,   181,   257,   137,   453,   466,
     467,   348,   368,   369,   338,   389,   389,   368,   389,   198,
      70,   443,   157,   474,    82,   348,   453,    90,   157,   474,
     223,   212,   198,   199,   252,   262,   398,   400,    91,   196,
     361,   362,   364,   407,   411,   459,   461,   478,    14,   102,
     479,   355,   356,   357,   289,   290,   441,   442,   197,   197,
     197,   197,   197,   200,   229,   230,   247,   254,   261,   441,
     348,   202,   204,   205,   213,   486,   487,   505,    38,   174,
     291,   292,   348,   481,   196,   484,   255,   245,   348,   348,
     348,   348,    32,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   412,   348,   348,
     463,   463,   348,   469,   470,   130,   199,   214,   215,   462,
     265,   213,   266,   484,   484,   264,   246,    38,   340,   343,
     345,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   165,   199,   213,   444,   445,   446,
     447,   462,   463,   348,   291,   291,   463,   348,   466,   245,
     197,   348,   196,   437,     9,   423,   197,   197,    38,   348,
      38,   348,   415,   197,   197,   197,   460,   461,   462,   291,
     199,   213,   444,   445,   462,   197,   228,   283,   199,   345,
     348,   348,    94,    32,   230,   277,   198,    27,   102,    14,
       9,   197,    32,   199,   280,   505,    31,    91,   174,   226,
     498,   499,   500,   196,     9,    50,    51,    56,    58,    70,
     138,   139,   140,   141,   142,   143,   185,   196,   224,   375,
     378,   381,   384,   387,   393,   408,   416,   417,   419,   420,
     213,   503,   228,   196,   238,   199,   198,   199,   198,   223,
     198,   102,   164,   198,   111,   112,   219,   220,   221,   222,
     219,   213,   348,   294,   417,    83,     9,   197,   197,   197,
     197,   197,   197,   197,   198,    50,    51,   494,   496,   497,
     132,   270,   196,     9,   197,   197,   136,   203,     9,   423,
       9,   423,   203,   203,   203,   203,    83,    85,   213,   475,
     213,    70,   200,   200,   209,   211,    32,   133,   269,   180,
      54,   165,   180,   402,   349,   136,     9,   423,   197,   160,
     505,   505,    14,   360,   289,   228,   193,     9,   424,   505,
     506,   443,   448,   443,   200,     9,   423,   182,   453,   348,
     197,     9,   424,    14,   352,   248,   132,   268,   196,   484,
     348,    32,   203,   203,   136,   200,     9,   423,   348,   485,
     196,   258,   253,   263,    14,   479,   256,   245,    71,   453,
     348,   485,   203,   200,   197,   197,   203,   200,   197,    50,
      51,    70,    78,    79,    80,    91,   138,   139,   140,   141,
     142,   143,   156,   185,   213,   376,   379,   382,   385,   388,
     408,   419,   426,   428,   429,   433,   436,   213,   453,   453,
     136,   268,   443,   448,   197,   348,   284,    75,    76,   285,
     228,   337,   228,   339,   102,    38,   137,   274,   453,   417,
     213,    32,   230,   278,   198,   281,   198,   281,     9,   423,
      91,   226,   136,   160,     9,   423,   197,   174,   486,   487,
     488,   486,   417,   417,   417,   417,   417,   422,   425,   196,
      70,    70,    70,    70,    70,   196,   417,   160,   199,    10,
      11,    12,    31,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    69,   160,   485,   200,   408,
     199,   242,   218,   218,   213,   219,   219,     9,   424,   200,
     200,    14,   453,   198,   182,     9,   423,   213,   271,   408,
     199,   466,   137,   453,    14,   348,   348,   203,   348,   200,
     209,   505,   271,   199,   401,    14,   197,   348,   361,   462,
     198,   505,   193,   200,    32,   492,   442,    38,    83,   174,
     444,   445,   447,   444,   445,   505,    38,   174,   348,   417,
     289,   196,   408,   269,   353,   249,   348,   348,   348,   200,
     196,   291,   270,    32,   269,   505,    14,   268,   484,   412,
     200,   196,    14,    78,    79,    80,   213,   427,   427,   429,
     431,   432,    52,   196,    70,    70,    70,    70,    70,    90,
     157,   196,   160,     9,   423,   197,   437,    38,   348,   269,
     200,    75,    76,   286,   337,   230,   200,   198,    95,   198,
     274,   453,   196,   136,   273,    14,   228,   281,   105,   106,
     107,   281,   200,   505,   182,   136,   160,   505,   213,   174,
     498,     9,   197,   423,   136,   203,     9,   423,   422,   370,
     371,   417,   390,   417,   418,   390,   370,   390,   361,   363,
     365,   197,   130,   214,   417,   471,   472,   417,   417,   417,
      32,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   503,    83,   243,   200,   200,
     222,   198,   417,   497,   102,   103,   493,   495,     9,   299,
     197,   196,   340,   345,   348,   136,   203,   200,   479,   299,
     166,   179,   199,   397,   404,   166,   199,   403,   136,   198,
     492,   505,   360,   506,    83,   174,    14,    83,   485,   453,
     348,   197,   289,   199,   289,   196,   136,   196,   291,   197,
     199,   505,   199,   198,   505,   269,   250,   415,   291,   136,
     203,     9,   423,   428,   431,   372,   373,   429,   391,   429,
     430,   391,   372,   391,   157,   361,   434,   435,    81,   429,
     453,   199,   337,    32,    77,   230,   198,   339,   273,   466,
     274,   197,   417,   101,   105,   198,   348,    32,   198,   282,
     200,   182,   505,   213,   136,   174,    32,   197,   417,   417,
     197,   203,     9,   423,   136,   203,     9,   423,   203,   203,
     203,   136,     9,   423,   197,   136,   200,     9,   423,   417,
      32,   197,   228,   198,   198,   213,   505,   505,   493,   408,
       6,   112,   117,   120,   125,   167,   168,   170,   200,   300,
     325,   326,   327,   332,   333,   334,   335,   441,   466,   348,
     200,   199,   200,    54,   348,   348,   348,   360,    38,    83,
     174,    14,    83,   348,   196,   492,   197,   299,   197,   289,
     348,   291,   197,   299,   479,   299,   198,   199,   196,   197,
     429,   429,   197,   203,     9,   423,   136,   203,     9,   423,
     203,   203,   203,   136,   197,     9,   423,   299,    32,   228,
     198,   197,   197,   197,   235,   198,   198,   282,   228,   136,
     505,   505,   136,   417,   417,   417,   417,   361,   417,   417,
     417,   199,   200,   495,   132,   133,   186,   214,   482,   505,
     272,   408,   112,   335,    31,   125,   138,   144,   165,   171,
     309,   310,   311,   312,   408,   169,   317,   318,   128,   196,
     213,   319,   320,   301,   246,   505,     9,   198,     9,   198,
     198,   479,   326,   197,   296,   165,   399,   200,   200,    83,
     174,    14,    83,   348,   291,   117,   350,   492,   200,   492,
     197,   197,   200,   199,   200,   299,   289,   136,   429,   429,
     429,   429,   361,   200,   228,   233,   236,    32,   230,   276,
     228,   505,   197,   417,   136,   136,   136,   228,   408,   408,
     484,    14,   214,     9,   198,   199,   482,   479,   312,   181,
     199,     9,   198,     3,     4,     5,     6,     7,    10,    11,
      12,    13,    27,    28,    29,    57,    71,    72,    73,    74,
      75,    76,    77,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   137,   138,   145,   146,   147,   148,
     149,   161,   162,   163,   173,   175,   176,   178,   185,   186,
     188,   190,   191,   213,   405,   406,     9,   198,   165,   169,
     213,   320,   321,   322,   198,    83,   331,   245,   302,   482,
     482,    14,   246,   200,   297,   298,   482,    14,    83,   348,
     197,   196,   492,   198,   199,   323,   350,   492,   296,   200,
     197,   429,   136,   136,    32,   230,   275,   276,   228,   417,
     417,   417,   200,   198,   198,   417,   408,   305,   505,   313,
     314,   416,   310,    14,    32,    51,   315,   318,     9,    36,
     197,    31,    50,    53,    14,     9,   198,   215,   483,   331,
      14,   505,   245,   198,    14,   348,    38,    83,   396,   199,
     228,   492,   323,   200,   492,   429,   429,   228,    99,   241,
     200,   213,   226,   306,   307,   308,     9,   423,     9,   423,
     200,   417,   406,   406,    68,   316,   321,   321,    31,    50,
      53,   417,    83,   181,   196,   198,   417,   484,   417,    83,
       9,   424,   228,   200,   199,   323,    97,   198,   115,   237,
     160,   102,   505,   182,   416,   172,    14,   494,   303,   196,
      38,    83,   197,   200,   228,   198,   196,   178,   244,   213,
     326,   327,   182,   417,   182,   287,   288,   442,   304,    83,
     200,   408,   242,   175,   213,   198,   197,     9,   424,   122,
     123,   124,   329,   330,   287,    83,   272,   198,   492,   442,
     506,   197,   197,   198,   195,   489,   329,    38,    83,   174,
     492,   199,   490,   491,   505,   198,   199,   324,   506,    83,
     174,    14,    83,   489,   228,     9,   424,    14,   493,   228,
      38,    83,   174,    14,    83,   348,   324,   200,   491,   505,
     200,    83,   174,    14,    83,   348,    14,    83,   348,   348
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   206,   208,   207,   209,   209,   210,   210,   210,   210,
     210,   210,   210,   210,   211,   210,   212,   210,   210,   210,
     210,   210,   210,   210,   210,   210,   210,   213,   213,   213,
     213,   213,   213,   213,   213,   213,   213,   213,   213,   213,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   215,   215,   216,   216,   217,   217,
     218,   219,   219,   219,   219,   220,   220,   221,   222,   222,
     222,   223,   223,   224,   224,   224,   225,   226,   227,   227,
     228,   228,   229,   229,   229,   229,   230,   230,   230,   231,
     230,   232,   230,   233,   230,   234,   230,   230,   230,   230,
     230,   230,   230,   230,   230,   230,   230,   230,   230,   230,
     230,   230,   235,   230,   236,   230,   230,   237,   230,   238,
     230,   230,   230,   230,   230,   230,   230,   230,   230,   230,
     230,   230,   230,   230,   230,   240,   239,   241,   241,   243,
     242,   244,   244,   245,   245,   246,   248,   247,   249,   247,
     250,   247,   252,   251,   253,   251,   255,   254,   256,   254,
     257,   254,   258,   254,   260,   259,   262,   261,   263,   261,
     264,   264,   265,   266,   267,   267,   267,   267,   267,   268,
     268,   269,   269,   270,   270,   271,   271,   272,   272,   273,
     273,   274,   274,   274,   275,   275,   276,   276,   277,   277,
     278,   278,   279,   279,   280,   280,   280,   280,   281,   281,
     281,   282,   282,   283,   283,   284,   284,   285,   285,   286,
     286,   287,   287,   287,   287,   287,   287,   287,   287,   288,
     288,   288,   288,   288,   288,   288,   288,   289,   289,   289,
     289,   289,   289,   289,   289,   290,   290,   290,   290,   290,
     290,   290,   290,   291,   291,   292,   292,   292,   292,   292,
     292,   293,   293,   294,   294,   294,   295,   295,   295,   295,
     296,   296,   297,   298,   299,   299,   301,   300,   302,   300,
     300,   300,   300,   303,   300,   304,   300,   300,   300,   300,
     300,   300,   300,   300,   305,   305,   305,   306,   307,   307,
     308,   308,   309,   309,   310,   310,   311,   311,   312,   312,
     312,   312,   312,   312,   312,   313,   313,   314,   315,   315,
     316,   316,   317,   317,   318,   319,   319,   319,   320,   320,
     320,   320,   321,   321,   321,   321,   321,   321,   321,   322,
     322,   322,   323,   323,   324,   324,   325,   325,   326,   326,
     327,   327,   328,   328,   328,   328,   328,   328,   328,   329,
     329,   330,   330,   330,   331,   331,   331,   331,   332,   332,
     333,   333,   334,   334,   335,   336,   336,   336,   336,   336,
     336,   337,   338,   338,   339,   339,   340,   340,   340,   340,
     341,   342,   343,   344,   345,   346,   347,   348,   348,   348,
     348,   348,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   350,
     350,   352,   351,   353,   351,   355,   354,   356,   354,   357,
     354,   358,   354,   359,   354,   360,   360,   360,   361,   361,
     362,   362,   363,   363,   364,   364,   365,   365,   366,   367,
     367,   368,   368,   369,   369,   370,   370,   371,   371,   372,
     372,   373,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   389,   389,
     390,   390,   391,   391,   392,   393,   394,   394,   395,   395,
     395,   395,   395,   395,   395,   395,   395,   395,   395,   396,
     396,   396,   396,   397,   398,   398,   399,   399,   400,   400,
     401,   401,   402,   403,   403,   404,   404,   404,   405,   405,
     405,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   407,   408,   408,   409,   409,   409,   409,   409,
     410,   410,   411,   411,   411,   411,   412,   412,   412,   413,
     413,   413,   414,   414,   414,   415,   415,   416,   416,   416,
     416,   416,   416,   416,   416,   416,   416,   416,   416,   416,
     416,   416,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   418,   418,   419,
     420,   420,   421,   421,   421,   421,   421,   421,   421,   422,
     422,   423,   423,   424,   424,   425,   425,   425,   425,   426,
     426,   426,   426,   426,   427,   427,   427,   427,   428,   428,
     429,   429,   429,   429,   429,   429,   429,   429,   429,   429,
     429,   429,   429,   429,   429,   430,   430,   431,   431,   432,
     432,   432,   432,   433,   433,   434,   434,   435,   435,   436,
     436,   437,   437,   438,   438,   440,   439,   441,   442,   442,
     443,   443,   444,   444,   444,   445,   445,   446,   446,   447,
     447,   448,   448,   449,   449,   450,   450,   451,   451,   452,
     452,   453,   453,   453,   453,   453,   453,   453,   453,   453,
     453,   453,   454,   454,   454,   454,   454,   454,   454,   454,
     454,   455,   455,   455,   455,   455,   455,   455,   455,   455,
     456,   457,   457,   458,   458,   459,   459,   459,   460,   461,
     461,   461,   462,   462,   462,   462,   463,   463,   464,   464,
     464,   464,   464,   464,   465,   465,   465,   465,   465,   466,
     466,   466,   466,   466,   466,   467,   467,   468,   468,   468,
     468,   468,   468,   468,   468,   469,   469,   470,   470,   470,
     470,   471,   471,   472,   472,   472,   472,   473,   473,   473,
     473,   474,   474,   474,   474,   474,   474,   475,   475,   475,
     476,   476,   476,   476,   476,   476,   476,   476,   476,   476,
     476,   477,   477,   478,   478,   479,   479,   480,   480,   480,
     480,   481,   481,   482,   482,   483,   483,   484,   484,   485,
     485,   486,   486,   487,   488,   488,   488,   488,   489,   489,
     490,   490,   491,   491,   492,   492,   493,   493,   494,   495,
     495,   496,   496,   496,   496,   497,   497,   497,   498,   498,
     498,   498,   499,   499,   500,   500,   500,   500,   501,   502,
     503,   503,   504,   504,   505,   505,   505,   505,   505,   505,
     505,   505,   505,   505,   505,   506,   506
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     1,     4,     3,     0,     6,     0,     5,     3,     4,
       4,     4,     4,     6,     7,     7,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     3,     3,     1,
       2,     1,     2,     3,     4,     3,     1,     2,     1,     2,
       2,     1,     3,     1,     3,     2,     2,     2,     5,     4,
       2,     0,     1,     1,     1,     1,     3,     5,     8,     0,
       4,     0,     6,     0,    10,     0,     4,     2,     3,     2,
       3,     2,     3,     3,     3,     3,     3,     3,     5,     1,
       1,     1,     0,     9,     0,    10,     5,     0,    13,     0,
       5,     3,     3,     2,     2,     2,     2,     2,     2,     3,
       2,     2,     3,     2,     2,     0,     4,     9,     0,     0,
       4,     2,     0,     1,     0,     1,     0,     9,     0,    10,
       0,    11,     0,     9,     0,    10,     0,     8,     0,     9,
       0,     7,     0,     8,     0,     8,     0,     7,     0,     8,
       1,     1,     1,     1,     1,     2,     3,     3,     2,     2,
       0,     2,     0,     2,     0,     1,     3,     1,     3,     2,
       0,     1,     2,     4,     1,     4,     1,     4,     1,     4,
       1,     4,     3,     5,     3,     4,     4,     5,     5,     4,
       0,     1,     1,     4,     0,     5,     0,     2,     0,     3,
       0,     7,     8,     6,     2,     5,     6,     4,     0,     4,
       5,     7,     6,     6,     7,     9,     8,     6,     7,     5,
       2,     4,     5,     3,     0,     3,     4,     6,     5,     5,
       6,     8,     7,     2,     0,     1,     2,     2,     3,     4,
       4,     3,     1,     1,     2,     4,     3,     5,     1,     3,
       2,     0,     2,     3,     2,     0,     0,     4,     0,     5,
       2,     2,     2,     0,    11,     0,    12,     3,     3,     3,
       4,     4,     3,     5,     2,     2,     0,     6,     5,     4,
       3,     1,     1,     3,     4,     1,     2,     1,     1,     5,
       6,     1,     1,     4,     1,     1,     3,     2,     2,     0,
       2,     0,     1,     3,     1,     1,     1,     1,     3,     4,
       4,     4,     1,     1,     2,     2,     2,     3,     3,     1,
       1,     1,     1,     3,     1,     3,     1,     1,     1,     0,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     1,     1,     1,     3,     5,     1,     3,     5,     4,
       3,     3,     3,     4,     3,     3,     3,     2,     2,     1,
       1,     3,     3,     1,     1,     0,     1,     2,     4,     3,
       3,     6,     2,     3,     2,     3,     6,     1,     1,     1,
       1,     1,     6,     3,     4,     6,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     5,     4,     3,     1,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     2,     1,     5,
       0,     0,    12,     0,    13,     0,     4,     0,     7,     0,
       5,     0,     3,     0,     6,     2,     2,     4,     1,     1,
       5,     3,     5,     3,     2,     0,     2,     0,     4,     4,
       3,     2,     0,     5,     3,     2,     0,     5,     3,     2,
       0,     5,     3,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     2,     0,
       2,     0,     2,     0,     4,     4,     4,     4,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     3,
       4,     1,     2,     4,     2,     6,     0,     1,     4,     0,
       2,     0,     1,     1,     3,     1,     3,     1,     1,     3,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     4,     1,     1,     1,     1,     1,     1,     3,
       1,     3,     1,     1,     1,     3,     1,     1,     1,     2,
       1,     0,     0,     1,     1,     3,     0,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     2,     1,     1,     4,     3,     4,     1,     1,     1,
       1,     1,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     5,     4,     3,     1,     3,
       3,     1,     1,     1,     1,     1,     3,     3,     3,     2,
       0,     1,     0,     1,     0,     5,     3,     3,     1,     1,
       1,     1,     3,     2,     1,     1,     1,     1,     1,     3,
       1,     1,     1,     3,     1,     2,     2,     4,     3,     4,
       1,     1,     1,     1,     1,     3,     1,     2,     0,     5,
       3,     3,     1,     3,     1,     2,     0,     5,     3,     2,
       0,     3,     0,     4,     2,     0,     3,     3,     1,     0,
       1,     1,     1,     1,     3,     1,     1,     1,     3,     1,
       1,     3,     3,     2,     4,     2,     4,     5,     5,     5,
       5,     1,     1,     1,     1,     1,     1,     3,     3,     4,
       4,     3,     1,     1,     1,     1,     3,     1,     4,     3,
       3,     1,     1,     1,     1,     1,     3,     3,     4,     4,
       3,     1,     1,     7,     9,     7,     6,     8,     1,     4,
       4,     1,     1,     1,     4,     2,     1,     0,     1,     1,
       1,     3,     3,     3,     0,     1,     1,     3,     3,     2,
       3,     6,     0,     1,     4,     2,     0,     5,     3,     3,
       1,     6,     4,     4,     2,     2,     0,     5,     3,     3,
       1,     2,     0,     5,     3,     3,     1,     2,     2,     1,
       2,     1,     4,     3,     3,     6,     3,     1,     1,     1,
       4,     4,     4,     4,     4,     4,     2,     2,     4,     2,
       2,     1,     3,     3,     3,     0,     2,     5,     6,     6,
       7,     1,     2,     1,     2,     1,     4,     1,     4,     3,
       0,     1,     3,     2,     3,     1,     1,     0,     0,     3,
       1,     3,     3,     2,     0,     2,     2,     2,     2,     1,
       2,     4,     2,     5,     3,     1,     1,     0,     3,     4,
       5,     6,     3,     1,     3,     2,     1,     0,     4,     1,
       3,     2,     4,     5,     2,     2,     1,     1,     1,     1,
       3,     2,     1,     8,     6,     1,     0
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


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
      yyerror (&yylloc, _p, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).line0   = YYRHSLOC (Rhs, 1).line0;        \
          (Current).char0 = YYRHSLOC (Rhs, 1).char0;      \
          (Current).line1    = YYRHSLOC (Rhs, N).line1;         \
          (Current).char1  = YYRHSLOC (Rhs, N).char1;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).line0   = (Current).line1   =              \
            YYRHSLOC (Rhs, 0).line1;                                \
          (Current).char0 = (Current).char1 =              \
            YYRHSLOC (Rhs, 0).char1;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  unsigned res = 0;
  int end_col = 0 != yylocp->char1 ? yylocp->char1 - 1 : 0;
  if (0 <= yylocp->line0)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->line0);
      if (0 <= yylocp->char0)
        res += YYFPRINTF (yyo, ".%d", yylocp->char0);
    }
  if (0 <= yylocp->line1)
    {
      if (yylocp->line0 < yylocp->line1)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->line1);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->char0 < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location, _p); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, HPHP::HPHP_PARSER_NS::Parser *_p)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  YYUSE (_p);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, HPHP::HPHP_PARSER_NS::Parser *_p)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, _p);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, HPHP::HPHP_PARSER_NS::Parser *_p)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , _p);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, _p); \
} while (0)

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
#ifndef YYINITDEPTH
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
static YYSIZE_T
yystrlen (const char *yystr)
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
static char *
yystpcpy (char *yydest, const char *yysrc)
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
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
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
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
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

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, HPHP::HPHP_PARSER_NS::Parser *_p)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (_p);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (HPHP::HPHP_PARSER_NS::Parser *_p)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

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

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  yylsp[0] = yylloc;
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
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yyls1, yysize * sizeof (*yylsp),
                    &yystacksize);

        yyls = yyls1;
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
        struct yyalloc *yyptr =
          (struct yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
      memset(yyptr, 0, YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE_RESET (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

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
      yychar = yylex (&yylval, &yylloc, _p);
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
  *++yylsp = yylloc;
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
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 751 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
#line 6887 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 754 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 6895 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 761 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 6901 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 762 "hphp.y" /* yacc.c:1646  */
    { }
#line 6907 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 765 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 6913 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 766 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6919 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 767 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6925 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 768 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6931 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 769 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6937 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 770 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 6943 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 771 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 6951 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 774 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 6958 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 776 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 6964 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 6970 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 778 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 6976 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 779 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 6982 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 780 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6990 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 784 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6999 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 789 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7008 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 794 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7015 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 797 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7022 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 800 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7030 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 804 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7038 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 808 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7046 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 811 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7053 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 816 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7059 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 817 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7065 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 818 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7071 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 819 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7077 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 820 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7083 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 821 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7089 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 822 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7095 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 823 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7101 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 824 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7107 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 825 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7113 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 826 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7119 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 827 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7125 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 828 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7131 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 907 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7137 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 909 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7143 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 914 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7149 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 915 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7156 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 921 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7162 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 925 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7168 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 926 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7174 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 928 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7180 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 930 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7186 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 935 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7192 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 936 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7199 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 942 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7205 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 946 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7212 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 948 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7219 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 950 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7226 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 955 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7232 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 957 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7238 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 960 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7244 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 962 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7250 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 963 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7256 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 968 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7265 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 975 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7274 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 983 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7281 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 986 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7288 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 992 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7294 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 993 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7300 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 996 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7306 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 997 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7312 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 998 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7318 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 999 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7324 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1002 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7330 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 1006 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7336 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1011 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7342 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1012 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7349 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1014 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7357 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1018 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7364 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1021 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7372 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1025 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7379 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1027 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7387 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1030 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7394 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1032 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7402 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1035 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7408 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1036 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7414 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1037 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7420 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1038 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7426 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1039 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7432 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1040 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7438 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1041 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7444 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1042 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7450 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1043 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7456 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1044 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7462 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1045 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7468 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1046 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 7474 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1047 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 7480 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1048 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 7486 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1049 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 7493 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1053 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7500 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1055 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 7508 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1060 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7515 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1062 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 7523 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1066 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 7531 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1075 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7537 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1076 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 7543 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1079 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7549 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1080 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 7555 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1081 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 7561 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1082 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 7570 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1086 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7576 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1087 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7582 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1088 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7588 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1089 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7594 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1090 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7600 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1091 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7606 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1092 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7612 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1093 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7618 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1094 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7624 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1095 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 7630 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1096 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7636 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1097 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 7646 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1105 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);}
#line 7652 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1106 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7658 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1115 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 7664 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1116 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7670 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1120 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7677 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1122 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7685 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1128 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7691 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1129 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7697 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1133 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 7703 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1134 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7709 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1138 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 7715 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1144 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7724 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1150 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7733 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1157 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7742 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1163 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7751 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1170 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7760 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1176 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7769 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1184 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 7776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1188 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 7782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1192 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 7789 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1196 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 7795 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 1202 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 7802 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 1205 "hphp.y" /* yacc.c:1646  */
    { Token stmts;
                                         if (_p->peekClass()) {
                                           xhp_collect_attributes(_p,stmts,(yyvsp[-1]));
                                         } else {
                                           stmts = (yyvsp[-1]);
                                         }
                                         _p->onClass((yyval),(yyvsp[-7]).num(),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),
                                                     stmts,0,nullptr);
                                         if (_p->peekClass()) {
                                           _p->xhpResetAttributes();
                                         }
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7820 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 208:
#line 1220 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 7827 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 1223 "hphp.y" /* yacc.c:1646  */
    { Token stmts;
                                         if (_p->peekClass()) {
                                           xhp_collect_attributes(_p,stmts,(yyvsp[-1]));
                                         } else {
                                           stmts = (yyvsp[-1]);
                                         }
                                         _p->onClass((yyval),(yyvsp[-7]).num(),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),
                                                     stmts,&(yyvsp[-8]),nullptr);
                                         if (_p->peekClass()) {
                                           _p->xhpResetAttributes();
                                         }
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7845 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 1237 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 7852 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1240 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7860 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1245 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 7867 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1248 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1254 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 7881 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1257 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 7887 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1261 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 7894 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1264 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1272 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 7912 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1275 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7923 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1283 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1284 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 7936 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1288 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7942 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1291 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7948 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1294 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 7954 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1295 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 7960 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1296 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 7968 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1299 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 7974 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1300 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 7980 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1304 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7986 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1305 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7992 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1308 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7998 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1309 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8004 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1312 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8010 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1313 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8016 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 1316 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 8022 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1318 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8028 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 1321 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 8034 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1323 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8040 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1327 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8046 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1328 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8052 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1331 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8058 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1332 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8064 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1333 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8070 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1337 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8076 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1339 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8082 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1342 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8088 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1344 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8094 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1347 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8100 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1349 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8106 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1352 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8112 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1354 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8118 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1358 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8124 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1360 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8131 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1365 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8137 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1366 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8143 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1367 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8149 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1368 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8155 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1373 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8161 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1375 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8167 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1376 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8173 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1379 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8179 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1380 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8185 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1385 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8191 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1386 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8197 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1391 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8203 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1392 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8209 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1395 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8215 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1396 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8221 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1399 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8227 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1400 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8233 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1408 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8240 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1414 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8247 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1420 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8255 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1424 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8261 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1428 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8268 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1433 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8275 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1438 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1441 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1447 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8296 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1451 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8303 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1456 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8310 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1461 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8317 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1466 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8324 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1471 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1477 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8338 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1483 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8345 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1491 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 8352 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1496 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 8359 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1501 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 8367 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1505 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8373 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1508 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 8380 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1512 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 8387 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1516 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 8395 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1519 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8401 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1524 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8408 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1527 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8415 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1531 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8422 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1535 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8429 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1539 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1543 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8443 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1548 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8450 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1553 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8457 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1559 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8463 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1560 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8469 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1563 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,false);}
#line 8475 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1564 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),true,false);}
#line 8481 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1565 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,true);}
#line 8487 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1567 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),false, false);}
#line 8493 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1569 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),false,true);}
#line 8499 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1571 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),true, false);}
#line 8505 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1575 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8511 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1576 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 8517 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1579 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8523 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1580 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8529 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1581 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 8535 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1585 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 8541 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1587 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 8547 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1588 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 8553 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1589 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 8559 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1594 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 8565 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1595 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8571 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1598 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 8578 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1603 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 8584 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1609 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 8590 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1610 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8596 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1613 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[0]));}
#line 8602 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1614 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 8609 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1617 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[-1]));}
#line 8615 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1618 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-3]));}
#line 8622 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1620 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 8629 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1623 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL, true);}
#line 8636 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1625 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8642 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1628 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 8650 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1635 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8659 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1643 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 8667 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1650 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8676 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1655 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 8682 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1657 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 8688 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1659 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 8694 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1661 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 8700 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1663 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 8706 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1664 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 8713 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1667 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 8719 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1670 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 8725 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1671 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 8731 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1672 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8737 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1678 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 8743 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1683 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 8750 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1686 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 8758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1693 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 8764 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1694 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 8771 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1699 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 8778 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1702 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 8784 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1709 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 8791 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1711 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8797 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1715 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8803 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1720 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8809 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 359:
#line 1722 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8815 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1724 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8821 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1725 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 8832 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1731 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 8838 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1733 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 8844 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1734 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 8850 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1738 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,0,(yyvsp[0]),0);}
#line 8856 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1740 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),0,(yyvsp[0]),0);}
#line 8862 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1745 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8868 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1748 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8874 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1749 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 8880 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1753 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 8886 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1754 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 8892 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1758 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 8899 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1761 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 8906 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1766 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 8913 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1771 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 8919 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1772 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 8926 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1774 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 8932 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1778 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 8938 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1779 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 8944 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1780 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 8950 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1781 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 8956 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1785 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8962 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 1786 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 8968 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1787 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 8974 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1788 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 8980 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1789 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 8986 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1791 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 8992 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1793 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 8998 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1797 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 9006 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1800 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 9012 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1801 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 9018 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 1805 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9024 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1806 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9030 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 394:
#line 1810 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9036 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1811 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9042 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1814 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9048 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1815 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9054 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1818 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9060 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1819 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9066 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1822 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9072 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1824 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9078 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1827 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9084 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1828 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9090 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1829 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9096 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1830 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9102 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1831 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9108 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9114 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9120 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9126 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1838 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9132 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1841 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9138 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1842 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9144 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1843 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9150 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1847 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9156 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1849 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9162 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1850 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9168 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1851 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9174 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1855 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9180 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1857 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9186 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1861 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9192 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1863 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9198 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1867 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9206 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1871 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9213 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1875 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9219 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1879 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9225 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1881 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9231 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1882 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9237 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1883 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9243 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1884 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9249 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1885 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9255 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1888 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9261 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1892 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9267 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1893 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9273 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 1897 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9279 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 1898 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9285 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 1902 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 9291 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 1903 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 9297 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 1904 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 9303 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 1905 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9309 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 1909 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9315 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 1914 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9321 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 1918 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 9327 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 1922 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9333 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 1926 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 9339 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 1930 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9345 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 1935 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9351 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 1939 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9357 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 1940 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9363 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 1941 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9369 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 1942 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9375 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 1943 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9381 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 1948 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 9387 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 1949 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 9393 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 1950 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 9399 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 1953 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 9405 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 1954 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 9411 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 1955 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 9417 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 1956 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 9423 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 1957 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 9429 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 1958 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 9435 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 1959 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 9441 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 1960 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 9447 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 1961 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 9453 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 1962 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 9459 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 9465 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 1964 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 9471 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 1965 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 9477 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 1966 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 9483 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 1967 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 9489 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 1968 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 9495 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 1969 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 9501 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 1970 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 9507 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 1971 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 9513 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 1972 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 9519 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 1973 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 9525 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 1974 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 9531 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 1975 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 9537 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 1976 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 9543 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 1977 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 9549 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 1978 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 9555 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 1979 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 9561 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 1980 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 9567 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 1981 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 9573 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 1982 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 9579 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 1983 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 9585 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 1984 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 9591 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 1985 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 9597 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 1986 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 9603 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 1987 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 9609 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 1988 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 9615 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 1989 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 9621 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 1990 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 9627 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 1991 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 9633 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 1992 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 9639 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 1993 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 9645 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 1994 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 9651 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 1995 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 9657 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 1996 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 9663 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 1997 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 9670 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 1999 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 9676 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 9683 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 2002 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 9689 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2004 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 9695 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2005 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9701 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2006 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 9707 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2007 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 9713 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2008 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 9719 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2009 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9725 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2010 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 9731 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2011 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 9737 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2012 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 9743 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2013 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 9749 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2014 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 9755 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2015 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 9761 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2016 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 9767 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 9773 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2018 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 9779 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2019 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9785 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2020 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9791 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2021 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9797 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2022 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9803 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2023 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9809 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2024 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9815 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2025 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9821 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2026 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9827 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2027 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 9833 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2028 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 9839 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2029 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9845 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2036 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 9851 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2037 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9857 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2042 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 9866 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2048 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9877 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2056 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 9886 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2062 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[-12]),
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9897 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2072 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),0,
                                                     NULL,NULL,NULL);}
#line 9910 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2080 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         (yyvsp[-3]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[-3]), nullptr, (yyvsp[-3]));
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[-3]),
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9924 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2090 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9934 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2098 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         (yyvsp[-6]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[-6]), nullptr, (yyvsp[-6]));
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[-6]),
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9948 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2108 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9958 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2114 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v; Token w; Token x;
                                         Token y;
                                         (yyvsp[-4]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[-4]), nullptr, (yyvsp[-4]));
                                         _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[-4]),
                                                            u,v,w,(yyvsp[-1]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);
                                         _p->onCall((yyval),1,(yyval),y,NULL);}
#line 9974 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2125 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),0,
                                                     NULL,NULL,NULL);}
#line 9987 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2133 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9999 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2140 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10009 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2148 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10021 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2158 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10027 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2159 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10033 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2161 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10039 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2165 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10046 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2167 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10052 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2174 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10058 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2177 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10064 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2184 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10070 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2187 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10076 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2192 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10082 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2193 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10088 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2198 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10094 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2199 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10100 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2203 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval), (yyvsp[-1]), T_ARRAY);}
#line 10106 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2207 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10112 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2208 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10118 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2213 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10124 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2214 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10130 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2219 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10136 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2220 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10142 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2225 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10148 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2226 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10154 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2232 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10160 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2234 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10166 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2239 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10172 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 570:
#line 2240 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10178 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2246 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10184 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 572:
#line 2248 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10190 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2252 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10196 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2256 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10202 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2260 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10208 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
#line 2264 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10214 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2268 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10220 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
#line 2272 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10226 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
#line 2276 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10232 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
#line 2280 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10238 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2284 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10244 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
#line 2288 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10250 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2292 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10256 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2296 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10262 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2300 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10268 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2304 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10274 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2308 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10280 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2313 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10286 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2314 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10292 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2319 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10298 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2320 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10304 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2325 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10310 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2326 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10316 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2331 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10324 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2338 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10332 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2345 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10338 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2347 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10344 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2351 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10350 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2352 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10356 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2353 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10362 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2354 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10368 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2355 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10374 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2356 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10380 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2357 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10386 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2358 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10392 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2359 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10399 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2361 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10405 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2362 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10411 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2366 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10417 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2367 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 10423 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2368 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 10429 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2369 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 10435 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2376 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 10441 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2379 "hphp.y" /* yacc.c:1646  */
    { Token t1; _p->onArray(t1,(yyvsp[-1]));
                                         Token t2; _p->onArray(t2,(yyvsp[0]));
                                         Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onCallParam((yyvsp[-1]),NULL,t1,0,0);
                                         _p->onCallParam((yyval), &(yyvsp[-1]),t2,0,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),file,0,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),line,0,0);
                                         (yyval).setText("");}
#line 10455 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2390 "hphp.y" /* yacc.c:1646  */
    { Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onArray((yyvsp[-2]),(yyvsp[-5]));
                                         _p->onArray((yyvsp[-1]),(yyvsp[-3]));
                                         _p->onCallParam((yyvsp[-4]),NULL,(yyvsp[-2]),0,0);
                                         _p->onCallParam((yyval), &(yyvsp[-4]),(yyvsp[-1]),0,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),file,0,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),line,0,0);
                                         (yyval).setText((yyvsp[0]).text());}
#line 10469 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2401 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 10475 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2402 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 10481 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2407 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10487 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2408 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10493 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2411 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-1]),0,(yyvsp[0]),0);}
#line 10499 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2412 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10505 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2415 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10512 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2419 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10520 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2422 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10526 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2425 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         if ((yyvsp[0]).htmlTrim()) {
                                           (yyvsp[0]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));
                                         }
                                       }
#line 10538 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2432 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10544 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2433 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10550 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2437 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10556 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2439 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 10562 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2441 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 10568 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2445 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10574 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2446 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10580 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2447 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10586 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2448 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10592 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2449 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10598 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2450 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10604 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2451 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10610 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2452 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10616 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2453 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10622 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2454 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10628 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2455 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10634 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2456 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10640 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2457 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10646 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2458 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10652 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10658 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2460 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10664 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2461 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10670 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2462 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10676 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2463 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10682 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2464 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10688 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2465 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10694 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2466 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10700 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 653:
#line 2467 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10706 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2468 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10712 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2469 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10718 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2470 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10724 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2471 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10730 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2472 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10736 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2473 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10742 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2474 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10748 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2475 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10754 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2476 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10760 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2477 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10766 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2478 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10772 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2479 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10778 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2480 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10784 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2481 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10790 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2482 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10796 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10802 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2484 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10808 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2485 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10814 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2486 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10820 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2487 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10826 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2488 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10832 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2489 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10838 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2490 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10844 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2491 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10850 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2492 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10856 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2493 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10862 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2494 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10868 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2495 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10874 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2496 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10880 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2497 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10886 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2498 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10892 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2499 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10898 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2500 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10904 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2501 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10910 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2502 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10916 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2503 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10922 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2504 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10928 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2505 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10934 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2506 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10940 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2507 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10946 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2508 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10952 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2509 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10958 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2510 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10964 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2511 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10970 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2512 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10976 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2513 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10982 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2514 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10988 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2515 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10994 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2516 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11000 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2517 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11006 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2518 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11012 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2519 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11018 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2520 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11024 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2521 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11030 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2522 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11036 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2523 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11042 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2524 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11048 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2525 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11054 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2530 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11060 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2534 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11066 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2535 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11072 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2539 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11078 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2540 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11084 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2541 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11090 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2542 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11097 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2544 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11104 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2548 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11110 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2557 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11116 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2560 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11122 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2561 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11129 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2563 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11136 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2573 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11142 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2577 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11148 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2578 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11154 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2579 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11160 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2583 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11166 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2584 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11172 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2585 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11178 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2589 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11184 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2590 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11190 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2591 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11196 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2595 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11202 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2596 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11208 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2600 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11214 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2601 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11220 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2602 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11226 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2603 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 11233 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2605 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 11239 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2606 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 11245 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2607 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 11251 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2608 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 11257 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2609 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 11263 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2610 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 11269 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2611 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 11275 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2612 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 11281 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2613 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 11287 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2616 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11293 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2618 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11299 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2622 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11305 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2623 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11311 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2625 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11317 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2626 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11323 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2628 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11329 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2629 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11335 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2630 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11341 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2631 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11347 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2632 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11353 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2633 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11359 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2634 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11365 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2635 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11371 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2636 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11377 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2638 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 11383 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2640 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 11389 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2642 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 11395 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2644 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 11401 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2646 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 11407 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2647 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 11413 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2648 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 11419 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2649 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 11425 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2650 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 11431 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2651 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 11437 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2652 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 11443 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2653 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 11449 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2654 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 11455 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2655 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 11461 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2656 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 11467 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2657 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 11473 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2658 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 11479 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2659 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 11485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2660 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 11491 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2661 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11497 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2662 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11503 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2664 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 11509 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2666 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 11515 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2668 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 11521 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2670 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 11527 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2671 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 11533 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2673 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 11540 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2675 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 11546 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2678 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 11553 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2682 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 11559 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2685 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 11565 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2686 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 11571 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2690 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 11577 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2691 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 11583 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2697 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 11589 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2703 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 11595 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2704 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11601 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2708 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11607 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2709 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11613 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2710 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11619 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2711 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11625 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2712 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 11631 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2713 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 11637 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2715 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 11644 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2720 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11650 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2721 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11656 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2725 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11662 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2726 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11668 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2729 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 11674 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2730 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11680 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2736 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11686 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2738 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11692 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2740 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11698 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2741 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11704 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2745 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11710 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2746 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11716 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2747 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11722 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2750 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11728 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2752 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11734 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2755 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 11740 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2756 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 11746 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2757 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 11752 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2758 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 11758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2762 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 11765 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2765 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 11773 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2772 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11779 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 832:
#line 2773 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11785 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2776 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 11793 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2779 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 11799 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2780 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11805 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2781 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11811 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2783 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11817 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2784 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11823 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2786 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11829 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2787 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11835 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2788 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11841 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2789 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11847 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2790 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11853 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2791 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11859 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2796 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 11865 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2797 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 11871 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2802 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11877 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2803 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11883 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2808 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11889 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2810 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11895 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2812 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11901 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2813 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11907 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2817 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11913 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2818 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11919 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2823 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11925 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2824 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 11931 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2829 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 11937 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2832 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 11943 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 2837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11949 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2838 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11955 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2841 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11961 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2842 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 11968 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2849 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 11974 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2851 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 11980 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2854 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 11986 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2856 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11992 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2859 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11998 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2862 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12004 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2863 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12010 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 2867 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 12016 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2868 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 12022 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2872 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12028 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2873 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 12034 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2874 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 12040 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2878 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12046 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 2883 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12052 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 877:
#line 2888 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12058 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 2889 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12064 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 2893 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12070 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 2898 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12076 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 2903 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12082 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 2904 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12088 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 2908 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12094 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 2910 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12100 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 2915 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12106 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
#line 2917 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12112 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 2923 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12126 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 2934 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12140 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 2949 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12154 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 2961 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12168 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 2973 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12174 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 2974 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12180 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 2975 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12186 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 2976 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12192 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 2977 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12198 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 2978 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12204 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 2980 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12218 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 2997 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12224 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 2999 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12230 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 3001 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12236 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3002 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12242 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3006 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12248 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12254 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3008 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12260 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3009 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12266 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3017 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12280 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3026 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12286 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3028 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12292 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3029 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12298 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3033 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12304 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3038 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12310 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3039 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12316 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3040 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12322 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3041 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12328 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3042 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12334 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3043 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12340 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3044 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12346 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3046 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12352 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3048 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12358 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3052 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12364 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3056 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12370 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3057 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12376 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3063 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12382 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3067 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12388 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3074 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 12394 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3083 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 12400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3087 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 12406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3091 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3100 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12418 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3101 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3102 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3106 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3107 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 12442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3108 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 12448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3110 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]) = 1; _p->onIndirectRef((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12454 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3115 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
#line 3116 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12466 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
#line 3127 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12472 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3128 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12478 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
#line 3129 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12484 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
#line 3132 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12498 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3143 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12504 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3144 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12510 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3148 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12516 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
#line 3149 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12522 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
#line 3152 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12536 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
#line 3161 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12542 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3165 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 12548 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3166 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 12554 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3168 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 12560 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3169 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 12566 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3170 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 12572 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3171 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 12578 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3176 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12584 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3177 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12590 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3181 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12596 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3182 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12602 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3183 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12608 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3184 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12614 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3187 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 12620 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3189 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 12626 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3190 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 12632 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3191 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 12638 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3196 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12644 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3197 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 12650 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3201 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12656 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3202 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12662 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3203 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12668 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3204 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12674 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3209 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12680 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3210 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 12686 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3215 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12692 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3217 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12698 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3219 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12704 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3220 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12710 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3224 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 12716 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3226 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 12722 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3227 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 12728 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3229 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 12735 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3234 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12741 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3236 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12747 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3238 "hphp.y" /* yacc.c:1646  */
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[-2]),
                                           !(yyvsp[-1]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[0])
                                         );
                                       }
#line 12761 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3248 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 12767 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3250 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 12773 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3251 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12779 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3254 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 12785 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3255 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 12791 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3256 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 12797 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3260 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 12803 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3261 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 12809 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3262 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12815 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3263 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12821 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3264 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12827 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 995:
#line 3265 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12833 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3266 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 12839 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3267 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 12845 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
#line 3268 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 12851 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3269 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 12857 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3270 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 12863 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3274 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12869 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3275 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3280 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 12881 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1004:
#line 3282 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 12887 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1007:
#line 3296 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 12895 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1008:
#line 3301 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 12903 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3305 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 12911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3310 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 12919 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3316 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12925 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3317 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 12931 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3321 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12937 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3322 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 12943 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3328 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 12949 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3332 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 12955 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3338 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 12961 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3342 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 12968 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3349 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12974 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3350 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12980 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3354 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 12988 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3357 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 12995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3363 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13001 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3368 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]); }
#line 13007 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3369 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13013 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3370 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1027:
#line 3371 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1034:
#line 3392 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1035:
#line 3393 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13037 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1038:
#line 3402 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13043 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1041:
#line 3413 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13049 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1042:
#line 3415 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13055 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3419 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13061 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3422 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13067 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3426 "hphp.y" /* yacc.c:1646  */
    {}
#line 13073 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3427 "hphp.y" /* yacc.c:1646  */
    {}
#line 13079 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3428 "hphp.y" /* yacc.c:1646  */
    {}
#line 13085 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3434 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13092 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3439 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13102 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3448 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13108 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3454 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13117 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3462 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 13123 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3463 "hphp.y" /* yacc.c:1646  */
    { }
#line 13129 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3469 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 13135 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1055:
#line 3471 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 13141 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1056:
#line 3472 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 13151 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1057:
#line 3477 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 13158 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1058:
#line 3483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("array"); }
#line 13165 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1059:
#line 3488 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13171 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1060:
#line 3493 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 13179 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1061:
#line 3497 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13185 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1062:
#line 3502 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 13191 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3504 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 13197 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3510 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 13204 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3512 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 13212 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3515 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13218 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3516 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13226 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3519 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13234 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1069:
#line 3522 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13240 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3525 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 13248 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3528 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13255 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3530 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 13264 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3536 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 13273 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3542 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 13283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3550 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1076:
#line 3551 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13295 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;


#line 13299 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, _p, YY_("syntax error"));
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
        yyerror (&yylloc, _p, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

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
                      yytoken, &yylval, &yylloc, _p);
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

  yyerror_range[1] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

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

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp, _p);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

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
  yyerror (&yylloc, _p, YY_("memory exhausted"));
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
                  yytoken, &yylval, &yylloc, _p);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp, _p);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
  YYSTACK_CLEANUP;
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 3554 "hphp.y" /* yacc.c:1906  */

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}
