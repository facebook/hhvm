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
#define YYLAST   18106

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  206
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  301
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1074
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1974

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
     769,   770,   771,   774,   776,   776,   778,   778,   780,   782,
     785,   788,   792,   796,   800,   805,   806,   807,   808,   809,
     810,   811,   812,   813,   814,   815,   816,   817,   821,   822,
     823,   824,   825,   826,   827,   828,   829,   830,   831,   832,
     833,   834,   835,   836,   837,   838,   839,   840,   841,   842,
     843,   844,   845,   846,   847,   848,   849,   850,   851,   852,
     853,   854,   855,   856,   857,   858,   859,   860,   861,   862,
     863,   864,   865,   866,   867,   868,   869,   870,   871,   872,
     873,   874,   875,   876,   877,   878,   879,   880,   881,   882,
     883,   887,   891,   892,   896,   897,   902,   904,   909,   914,
     915,   916,   918,   923,   925,   930,   935,   937,   939,   944,
     945,   949,   950,   952,   956,   963,   970,   974,   980,   982,
     985,   986,   987,   988,   991,   992,   996,  1001,  1001,  1007,
    1007,  1014,  1013,  1019,  1019,  1024,  1025,  1026,  1027,  1028,
    1029,  1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,  1038,
    1042,  1040,  1049,  1047,  1054,  1064,  1058,  1068,  1066,  1070,
    1071,  1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,
    1084,  1085,  1086,  1094,  1094,  1099,  1105,  1109,  1109,  1117,
    1118,  1122,  1123,  1127,  1133,  1131,  1146,  1143,  1159,  1156,
    1173,  1172,  1181,  1179,  1191,  1190,  1209,  1207,  1226,  1225,
    1234,  1232,  1243,  1243,  1250,  1249,  1261,  1259,  1272,  1273,
    1277,  1280,  1283,  1284,  1285,  1288,  1289,  1292,  1294,  1297,
    1298,  1301,  1302,  1305,  1306,  1310,  1311,  1316,  1317,  1320,
    1321,  1322,  1326,  1327,  1331,  1332,  1336,  1337,  1341,  1342,
    1347,  1348,  1354,  1355,  1356,  1357,  1360,  1363,  1365,  1368,
    1369,  1373,  1375,  1378,  1381,  1384,  1385,  1388,  1389,  1393,
    1399,  1405,  1412,  1414,  1419,  1424,  1430,  1434,  1438,  1442,
    1447,  1452,  1457,  1462,  1468,  1477,  1482,  1487,  1493,  1495,
    1499,  1503,  1508,  1512,  1515,  1518,  1522,  1526,  1530,  1534,
    1539,  1547,  1549,  1552,  1553,  1554,  1555,  1557,  1559,  1564,
    1565,  1568,  1569,  1570,  1574,  1575,  1577,  1578,  1582,  1584,
    1587,  1591,  1597,  1599,  1602,  1602,  1606,  1605,  1609,  1611,
    1614,  1617,  1615,  1632,  1628,  1643,  1645,  1647,  1649,  1651,
    1653,  1655,  1659,  1660,  1661,  1664,  1670,  1674,  1680,  1683,
    1688,  1690,  1695,  1700,  1704,  1705,  1709,  1710,  1712,  1714,
    1720,  1721,  1723,  1727,  1728,  1733,  1737,  1738,  1742,  1743,
    1747,  1749,  1755,  1760,  1761,  1763,  1767,  1768,  1769,  1770,
    1774,  1775,  1776,  1777,  1778,  1779,  1781,  1786,  1789,  1790,
    1794,  1795,  1799,  1800,  1803,  1804,  1807,  1808,  1811,  1812,
    1816,  1817,  1818,  1819,  1820,  1821,  1822,  1826,  1827,  1830,
    1831,  1832,  1835,  1837,  1839,  1840,  1843,  1845,  1849,  1851,
    1855,  1859,  1863,  1868,  1869,  1871,  1872,  1873,  1874,  1877,
    1881,  1882,  1886,  1887,  1891,  1892,  1893,  1894,  1898,  1902,
    1907,  1911,  1915,  1919,  1923,  1928,  1929,  1930,  1931,  1932,
    1936,  1938,  1939,  1940,  1943,  1944,  1945,  1946,  1947,  1948,
    1949,  1950,  1951,  1952,  1953,  1954,  1955,  1956,  1957,  1958,
    1959,  1960,  1961,  1962,  1963,  1964,  1965,  1966,  1967,  1968,
    1969,  1970,  1971,  1972,  1973,  1974,  1975,  1976,  1977,  1978,
    1979,  1980,  1981,  1982,  1983,  1984,  1985,  1986,  1988,  1989,
    1991,  1992,  1994,  1995,  1996,  1997,  1998,  1999,  2000,  2001,
    2002,  2003,  2004,  2005,  2006,  2007,  2008,  2009,  2010,  2011,
    2012,  2013,  2014,  2015,  2016,  2017,  2018,  2022,  2026,  2031,
    2030,  2045,  2043,  2061,  2060,  2079,  2078,  2097,  2096,  2114,
    2114,  2129,  2129,  2147,  2148,  2149,  2154,  2156,  2160,  2164,
    2170,  2174,  2180,  2182,  2186,  2188,  2192,  2196,  2197,  2201,
    2203,  2207,  2209,  2213,  2215,  2219,  2222,  2227,  2229,  2233,
    2236,  2241,  2245,  2249,  2253,  2257,  2261,  2265,  2269,  2273,
    2277,  2281,  2285,  2289,  2293,  2297,  2301,  2303,  2307,  2309,
    2313,  2315,  2319,  2326,  2333,  2335,  2340,  2341,  2342,  2343,
    2344,  2345,  2346,  2347,  2348,  2350,  2351,  2355,  2356,  2357,
    2358,  2362,  2368,  2377,  2390,  2391,  2394,  2397,  2400,  2401,
    2404,  2408,  2411,  2414,  2421,  2422,  2426,  2427,  2429,  2434,
    2435,  2436,  2437,  2438,  2439,  2440,  2441,  2442,  2443,  2444,
    2445,  2446,  2447,  2448,  2449,  2450,  2451,  2452,  2453,  2454,
    2455,  2456,  2457,  2458,  2459,  2460,  2461,  2462,  2463,  2464,
    2465,  2466,  2467,  2468,  2469,  2470,  2471,  2472,  2473,  2474,
    2475,  2476,  2477,  2478,  2479,  2480,  2481,  2482,  2483,  2484,
    2485,  2486,  2487,  2488,  2489,  2490,  2491,  2492,  2493,  2494,
    2495,  2496,  2497,  2498,  2499,  2500,  2501,  2502,  2503,  2504,
    2505,  2506,  2507,  2508,  2509,  2510,  2511,  2512,  2513,  2514,
    2518,  2523,  2524,  2528,  2529,  2530,  2531,  2533,  2537,  2538,
    2549,  2550,  2552,  2554,  2566,  2567,  2568,  2572,  2573,  2574,
    2578,  2579,  2580,  2583,  2585,  2589,  2590,  2591,  2592,  2594,
    2595,  2596,  2597,  2598,  2599,  2600,  2601,  2602,  2603,  2606,
    2611,  2612,  2613,  2615,  2616,  2618,  2619,  2620,  2621,  2622,
    2623,  2624,  2625,  2626,  2628,  2630,  2632,  2634,  2636,  2637,
    2638,  2639,  2640,  2641,  2642,  2643,  2644,  2645,  2646,  2647,
    2648,  2649,  2650,  2651,  2652,  2654,  2656,  2658,  2660,  2661,
    2664,  2665,  2669,  2673,  2675,  2679,  2680,  2684,  2690,  2693,
    2697,  2698,  2699,  2700,  2701,  2702,  2703,  2708,  2710,  2714,
    2715,  2718,  2719,  2723,  2726,  2728,  2730,  2734,  2735,  2736,
    2737,  2740,  2744,  2745,  2746,  2747,  2751,  2753,  2760,  2761,
    2762,  2763,  2768,  2769,  2770,  2771,  2773,  2774,  2776,  2777,
    2778,  2779,  2780,  2784,  2786,  2790,  2792,  2795,  2798,  2800,
    2802,  2805,  2807,  2811,  2813,  2816,  2819,  2825,  2827,  2830,
    2831,  2836,  2839,  2843,  2843,  2848,  2851,  2852,  2856,  2857,
    2861,  2862,  2863,  2867,  2872,  2877,  2878,  2882,  2887,  2892,
    2893,  2897,  2898,  2903,  2905,  2910,  2921,  2935,  2947,  2962,
    2963,  2964,  2965,  2966,  2967,  2968,  2978,  2987,  2989,  2991,
    2995,  2996,  2997,  2998,  2999,  3015,  3016,  3018,  3020,  3027,
    3028,  3029,  3030,  3031,  3032,  3033,  3034,  3036,  3041,  3045,
    3046,  3050,  3053,  3060,  3064,  3073,  3080,  3088,  3090,  3091,
    3095,  3096,  3097,  3099,  3104,  3105,  3116,  3117,  3118,  3119,
    3130,  3133,  3136,  3137,  3138,  3139,  3150,  3154,  3155,  3156,
    3158,  3159,  3160,  3164,  3166,  3169,  3171,  3172,  3173,  3174,
    3177,  3179,  3180,  3184,  3186,  3189,  3191,  3192,  3193,  3197,
    3199,  3202,  3205,  3207,  3209,  3213,  3214,  3216,  3217,  3223,
    3224,  3226,  3236,  3238,  3240,  3243,  3244,  3245,  3249,  3250,
    3251,  3252,  3253,  3254,  3255,  3256,  3257,  3258,  3259,  3263,
    3264,  3268,  3270,  3278,  3280,  3284,  3288,  3293,  3297,  3305,
    3306,  3310,  3311,  3317,  3318,  3327,  3328,  3336,  3339,  3343,
    3346,  3351,  3356,  3358,  3359,  3360,  3363,  3365,  3371,  3372,
    3376,  3377,  3381,  3382,  3386,  3387,  3390,  3395,  3396,  3400,
    3403,  3405,  3409,  3415,  3416,  3417,  3421,  3425,  3433,  3438,
    3450,  3452,  3456,  3459,  3461,  3466,  3471,  3477,  3480,  3485,
    3490,  3492,  3499,  3501,  3504,  3505,  3508,  3511,  3512,  3517,
    3519,  3523,  3529,  3539,  3540
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

#define YYPACT_NINF -1387

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1387)))

#define YYTABLE_NINF -1058

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1058)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1387,   180, -1387, -1387,  5189, 13106, 13106,     1, 13106, 13106,
   13106, 13106, 10873, 13106, -1387, 13106, 13106, 13106, 13106, 16612,
   16612, 13106, 13106, 13106, 13106, 13106, 13106, 13106, 13106, 11076,
   17325, 13106,    26,   206, -1387, -1387, -1387,   124, -1387,   404,
   -1387, -1387, -1387,   350, 13106, -1387,   206,   238,   243,   285,
   -1387,   206, 11279, 14560, 11482, -1387, 14136,  9858,    20, 13106,
    2125,    94,    31,   476,   281, -1387, -1387, -1387,   290,   313,
     369,   399, -1387, 14560,   411,   420,   564,   574,   585,   616,
     633, -1387, -1387, -1387, -1387, -1387, 13106,   632,  2225, -1387,
   -1387, 14560, -1387, -1387, -1387, -1387, 14560, -1387, 14560, -1387,
     494,   514, 14560, 14560, -1387,   244, -1387, -1387, 11685, -1387,
   -1387,   432,   617,   668,   668, -1387,   639,   382,     3,   520,
   -1387,    98, -1387,   683, -1387, -1387, -1387, -1387, 13678,   571,
   -1387, -1387,   528,   541,   543,   547,   552,   565,   600,   602,
    4308, -1387, -1387, -1387, -1387,   103,   713,   746,   754,   760,
     762, -1387,   767,   779, -1387,    80,   651, -1387,   691,   204,
   -1387,  1282,    53, -1387, -1387,  2024,    74,   659,   151, -1387,
      79,    90,   661,   185, -1387, -1387,   788, -1387,   699, -1387,
   -1387,   664,   703, -1387, 13106, -1387,   683,   571, 17745,  3076,
   17745, 13106, 17745, 17745, 17963, 17963,   670, 16131, 17745,   824,
   14560,   805,   805,   512,   805, -1387, -1387, -1387, -1387, -1387,
   -1387, -1387, -1387, -1387,    85, 13106,   697, -1387, -1387,   715,
     684,   491,   685,   491,   805,   805,   805,   805,   805,   805,
     805,   805, 16612, 16781,   679,   875,   699, -1387, 13106,   697,
   -1387,   725, -1387,   727,   692, -1387,   142, -1387, -1387, -1387,
     491,    74, -1387, 11888, -1387, -1387, 13106,  8640,   883,    99,
   17745,  9655, -1387, 13106, 13106, 14560, -1387, -1387, 15424,   695,
   -1387, 15472, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387,
   -1387, -1387,  3837, -1387,  3837, -1387, -1387, -1387, -1387, -1387,
   -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387,
   -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387,
   -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387,
   -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387,
   -1387, -1387,    88,    92,   703, -1387, -1387, -1387, -1387,   696,
    3233,    96, -1387, -1387,   743,   891, -1387,   747, 14855, -1387,
     711,   712, 15542, -1387,    49, 15590, 14207, 14207, 14560,   714,
     902,   718, -1387,    65, -1387, 16200,   100, -1387,   898,   102,
     791, -1387,   792, -1387, 16612, 13106, 13106,   726,   745, -1387,
   -1387, 16303, 11076, 13106, 13106, 13106, 13106, 13106,   104,   264,
     246, -1387, 13309, 16612,   647, -1387, 14560, -1387,   478,   382,
   -1387, -1387, -1387, -1387, 17425,   913,   826, -1387, -1387, -1387,
      51, 13106,   733,   737, 17745,   738,  1764,   740,  5392, 13106,
   -1387,   452,   739,   694,   452,   443,   589, -1387, 14560,  3837,
     752, 10061, 14136, -1387, -1387,  4806, -1387, -1387, -1387, -1387,
   -1387,   683, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387,
   -1387, 13106, 13106, 13106, 13106, 12091, 13106, 13106, 13106, 13106,
   13106, 13106, 13106, 13106, 13106, 13106, 13106, 13106, 13106, 13106,
   13106, 13106, 13106, 13106, 13106, 13106, 13106, 13106, 13106, 17525,
   13106, -1387, 13106, 13106, 13106, 13480, 14560, 14560, 14560, 14560,
   14560, 13678,   818,   924,  4458, 13106, 13106, 13106, 13106, 13106,
   13106, 13106, 13106, 13106, 13106, 13106, 13106, -1387, -1387, -1387,
   -1387,   671, 13106, 13106, -1387, 10061, 10061, 13106, 13106, 16303,
     755,   683, 12294, 15638, -1387, 13106, -1387,   757,   934,   793,
     764,   768, 13632,   491, 12497, -1387, 12700, -1387,   692,   771,
     773,  2912, -1387,    70, 10061, -1387,  2243, -1387, -1387, 15708,
   -1387, -1387, 10264, -1387, 13106, -1387,   870,  8843,   962,   775,
   13293,   961,    84,    62, -1387, -1387, -1387,   800, -1387, -1387,
   -1387,  3837, -1387,  2681,   786,   974, 16056, 14560, -1387, -1387,
   -1387, -1387, -1387, -1387, -1387, -1387, -1387,   789, -1387, -1387,
     785,   790,   797,   802,   263,  2318, 14383, -1387, -1387, 14560,
   14560, 13106,   491,    94, -1387, 16056,   904, -1387, -1387, -1387,
     491,   115,   128,   794,   804,  2992,    71,   806,   809,   612,
     857,   808,   491,   129,   811, 16829,   807,  1000,  1003,   812,
     813,   815,   819, -1387, 14031, 14560, -1387, -1387,   951,  2694,
      23, -1387, -1387, -1387,   382, -1387, -1387, -1387,   995,   896,
     851,   211,   872, 13106,   897,  1027,   840, -1387,   878, -1387,
     154, -1387,  3837,  3837,  1035,   883,    51, -1387,   858,  1041,
   -1387,  3837,    81, -1387,   464,   144, -1387, -1387, -1387, -1387,
   -1387, -1387, -1387,   698,  3782, -1387, -1387, -1387, -1387,  1043,
     871, -1387, 16612, 13106,   859,  1045, 17745,  1046, -1387, -1387,
     925,  4812, 11467, 17882, 17963, 18000, 13106, 17697, 18037, 11055,
   12475, 12880, 12067, 14313, 14666, 14666, 14666, 14666,  4973,  4973,
    4973,  4973,  4973,  1073,  1073,   630,   630,   630,   512,   512,
     512, -1387,   805, 17745,   861,   873, 16885,   882,  1066,     0,
   13106,   355,   697,   176, -1387, -1387, -1387,  1070,   826, -1387,
     683, 16406, -1387, -1387, -1387, 17963, 17963, 17963, 17963, 17963,
   17963, 17963, 17963, 17963, 17963, 17963, 17963, 17963, -1387, 13106,
     381, -1387,   155, -1387,   697,   409,   886,  4049,   893,   899,
     894,  4140,   131,   903, -1387, 17745, 16159, -1387, 14560, -1387,
      81,   419, 16612, 17745, 16612, 16933,   925,    81,   491,   161,
   -1387,   154,   938,   905, 13106, -1387,   168, -1387, -1387, -1387,
    8437,   638, -1387, -1387, 17745, 17745,   206, -1387, -1387, -1387,
   13106,   999, 15932, 16056, 14560,  9046,   907,   908, -1387,  1098,
   13855,   972, -1387,   950, -1387,  1109,   920,  3601,  3837, 16056,
   16056, 16056, 16056, 16056,   923,  1050,  1051,  1059,  1061,  1062,
     937, 16056,   226, -1387, -1387, -1387, -1387, -1387, -1387,   -17,
   -1387,  3447, -1387, -1387,    11, -1387,  5595, 13526,   939, 14383,
   -1387, 14383, -1387, 14560, 14560, 14383, 14383, 14560, -1387,  1130,
     940, -1387,   267, -1387, -1387,  4634, -1387,  3447,  1131, 16612,
     949, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387,
     966,  1135, 14560, 13526,   952, 16303, 16509,  1136, -1387, 13106,
   -1387, 13106, -1387, 13106, -1387, -1387, -1387, -1387, -1387, -1387,
   -1387, -1387,   953, -1387, 13106, -1387, -1387,  4735, -1387,  3837,
   13526,   954, -1387, -1387, -1387, -1387,  1141,   964, 13106, 17425,
   -1387, -1387, 13480,   960, -1387,  3837, -1387,   956,  5798,  1132,
      50, -1387, -1387,    89,   671, -1387,  2243, -1387,  3837, -1387,
   -1387,   491, 17745, -1387, 10467, -1387, 16056,    69,   967, 13526,
     896, -1387, -1387, 18037, 13106, -1387, -1387, 13106, -1387, 13106,
   -1387, 10857,   969, 10061,   857,  1137,   896,  3837,  1148,   925,
   14560, 17525,   491, 11263,   976, -1387, -1387,   150,   978, -1387,
   -1387,  1156,  3468,  3468, 16159, -1387, -1387, -1387,  1126,   983,
    1114,  1115,  1120,  1121,  1122,   271,   997,   423, -1387, -1387,
   -1387, -1387, -1387,  1036, -1387, -1387, -1387, -1387,  1188,  1005,
     757,   491,   491, 12903,   896,  2243, -1387, -1387, 11872,   648,
     206,  9655, -1387,  6001,  1006,  6204,  1007, 15932, 16612,  1011,
    1067,   491,  3447,  1194, -1387, -1387, -1387, -1387,   591, -1387,
     288,  3837,  1028,  1075,  1049,  3837, 14560, 15632, -1387, -1387,
   -1387,  1203, -1387,  1017,  1043,   597,   597,  1146,  1146, 17092,
    1016,  1212, 16056, 16056, 16056, 16056, 16056, 16056, 17425,  4682,
   15007, 16056, 16056, 16056, 16056, 15773, 16056, 16056, 16056, 16056,
   16056, 16056, 16056, 16056, 16056, 16056, 16056, 16056, 16056, 16056,
   16056, 16056, 16056, 16056, 16056, 16056, 16056, 16056, 16056, 14560,
   -1387, -1387,  1149, -1387, -1387,  1024,  1034, -1387, -1387, -1387,
     374,  2318, -1387,  1037, -1387, 16056,   491, -1387, -1387,   111,
   -1387,   626,  1227, -1387, -1387,   133,  1044,   491, 10670, 17745,
   16989, -1387,  3113, -1387,  4985,   826,  1227, -1387,    10,    12,
   -1387, 17745,  1103,  1047, -1387,  1054,  1132, -1387,  3837,   883,
    3837,    82,  1228,  1158,   175, -1387,   697,   184, -1387, -1387,
   16612, 13106, 17745,  3447,  1052,    69, -1387,  1048,    69,  1063,
   18037, 17745, 17037,  1065, 10061,  1068,  1064,  3837,  1076,  1069,
    3837,   896, -1387,   692,   460, 10061, 13106, -1387, -1387, -1387,
   -1387, -1387, -1387,  1133,  1077,  1235,  1171, 16159, 16159, 16159,
   16159, 16159, 16159,  1124, -1387, 17425,   307, 16159, -1387, -1387,
   -1387, 16612, 17745,  1079, -1387,   206,  1234,  1205,  9655, -1387,
   -1387, -1387,  1085, 13106,  1067,   491, 16303, 15932,  1088, 16056,
    6407,   674,  1089, 13106,    64,   302, -1387,  1107, -1387,  3837,
   14560, -1387,  1152, -1387, -1387,  3825,  1259,  1095, 16056, -1387,
   16056, -1387,  1096,  1092,  1287, 17140,  1104,  3447,  1290,  1111,
    1117,  1118,  1173,  1301,  1125, -1387, -1387, -1387, 17195,  1112,
    1302, 17838, 17926, 10041, 16056, 17793, 12273, 12678, 13478, 14137,
   14490, 16013, 16013, 16013, 16013,  1586,  1586,  1586,  1586,  1586,
    1250,  1250,   597,   597,   597,  1146,  1146,  1146,  1146, -1387,
    1129, -1387,  1119,  1134, -1387, -1387,  3447, 14560,  3837,  3837,
   -1387,   626, 13526,   946, -1387, 16303, -1387, -1387, 17963, 13106,
    1123, -1387,  1128,  1106, -1387,   112, 13106, -1387, -1387, -1387,
   13106, -1387, 13106, -1387,   883, -1387, -1387,   117,  1314,  1246,
   13106, -1387,  1140,   491, 17745,  1132,  1142, -1387,  1143,    69,
   13106, 10061,  1145, -1387, -1387,   826, -1387, -1387,  1139,  1159,
    1161, -1387,  1147, 16159, -1387, 16159, -1387, -1387,  1151,  1127,
    1329,  1224,  1164, -1387,  1352,  1166,  1168,  1169, -1387,  1226,
    1176,  1359, -1387, -1387,   491, -1387,  1342, -1387,  1177, -1387,
   -1387,  1179,  1180,   136, -1387, -1387,  3447,  1181,  1182, -1387,
   15376, -1387, -1387, -1387, -1387, -1387, -1387,  1245,  3837, -1387,
    3837, -1387,  3447, 17243, -1387, -1387, 16056, -1387, 16056, -1387,
   16056, -1387, -1387, -1387, -1387, 16056, 17425, -1387, -1387, 16056,
   -1387, 16056, -1387, 10447, 16056,  1184,  6610, -1387, -1387,   626,
   -1387, -1387, -1387, -1387,   601, 14312, 13526,  1272, -1387, 15836,
    1216,  4133, -1387, -1387, -1387,   818, 15698,   106,   107,  1190,
     826,   924,   138, 17745, -1387, -1387, -1387,  1221, 13090, 15328,
   17745, -1387,   174,  1375,  1307, 13106, -1387, 17745, 10061,  1274,
    1132,  1389,  1132,  1196, 17745,  1197, -1387,  1426,  1198,  1911,
   -1387, -1387,    69, -1387, -1387,  1260, -1387, -1387, 16159, -1387,
   16159, -1387, 16159, -1387, -1387, -1387, -1387, 16159, -1387, 17425,
   -1387,  2009, -1387,  8437, -1387, -1387, -1387, -1387,  9249, -1387,
   -1387, -1387,  8437,  3837, -1387,  1201, 16056, 17298,  3447,  3447,
    3447,  1263,  3447, 17346, 10447, -1387, -1387,   626, 13526, 13526,
   14560, -1387,  1393, 15159,    86, -1387, 14312,   826, 14749, -1387,
    1230, -1387,   113,  1210,   119, -1387, 14665, -1387, -1387, -1387,
     121, -1387, -1387,  4307, -1387,  1217, -1387,  1331,   683, -1387,
   14489, -1387, 14489, -1387, -1387,  1402,   818, -1387, 13784, -1387,
   -1387, -1387, -1387,  1403,  1335, 13106, -1387, 17745,  1222,  1225,
    1132,   558, -1387,  1274,  1132, -1387, -1387, -1387, -1387,  2049,
    1223, 16159,  1286, -1387, -1387, -1387,  1288, -1387,  8437,  9452,
    9249, -1387, -1387, -1387,  8437, -1387, -1387,  3447, 16056, 16056,
   16056,  6813,  1233,  1236, -1387, 16056, -1387, 13526, -1387, -1387,
   -1387, -1387, -1387,  3837,  1108, 15836, -1387, -1387, -1387, -1387,
   -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387,
   -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387,
   -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387,
   -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387,
   -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387,
   -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387,
   -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387,
   -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387,   378, -1387,
    1216, -1387, -1387, -1387, -1387, -1387,   110,   535, -1387,  1409,
     122, 14855,  1331,  1411, -1387,  3837,   683, -1387, -1387,  1237,
    1414, 13106, -1387, 17745, -1387,   358,  1239, -1387, -1387, -1387,
    1132,   558, 13960, -1387,  1132, -1387, 16159, 16159, -1387, -1387,
   -1387, -1387,  7016,  3447,  3447,  3447, -1387, -1387, -1387,  3447,
   -1387,  3091,  1424,  1427,  1240, -1387, -1387, 16056, 14665, 14665,
    1371, -1387,  4307,  4307,   596, -1387, -1387, -1387, 16056,  1358,
   -1387,  1261,  1247,   123, 16056, -1387, 14560, -1387, 16056, 17745,
    1363, -1387,  1440, -1387,  7219,  1251, -1387, -1387,   558, -1387,
   -1387,  7422,  1256,  1341, -1387,  1356,  1300, -1387, -1387,  1361,
    3837,  1279,  1108, -1387, -1387,  3447, -1387, -1387,  1293, -1387,
    1430, -1387, -1387, -1387, -1387,  3447,  1453,   612, -1387, -1387,
    3447,  1275,  3447, -1387,   445,  1280,  7625, -1387, -1387, -1387,
    1278, -1387,  1276,  1303, 14560,   924,  1296, -1387, -1387, -1387,
   16056,  1297,    73, -1387,  1399, -1387, -1387, -1387,  7828, -1387,
   13526,   939, -1387,  1310, 14560,   649, -1387,  3447, -1387,  1289,
    1480,   663,    73, -1387, -1387,  1408, -1387, 13526,  1294, -1387,
    1132,    77, -1387, -1387, -1387, -1387,  3837, -1387,  1299,  1305,
     126, -1387,  1308,   663,   118,  1132,  1295, -1387,  3837,   562,
    3837,   181,  1483,  1421,  1308, -1387,  1496, -1387,   510, -1387,
   -1387, -1387,   149,  1493,  1433, 13106, -1387,   562,  8031,  3837,
   -1387,  3837, -1387,  8234,   239,  1504,  1436, 13106, -1387, 17745,
   -1387, -1387, -1387, -1387, -1387,  1506,  1438, 13106, -1387, 17745,
   13106, -1387, 17745, 17745
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,     0,   434,     0,   863,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   954,
     942,     0,   729,     0,   735,   736,   737,    25,   801,   930,
     931,   158,   159,   738,     0,   139,     0,     0,     0,     0,
      26,     0,     0,     0,     0,   193,     0,     0,     0,     0,
       0,     0,   403,   404,   405,   402,   401,   400,     0,     0,
       0,     0,   222,     0,     0,     0,    33,    34,    36,    37,
      35,   742,   744,   745,   739,   740,     0,     0,     0,   746,
     741,     0,   712,    28,    29,    30,    32,    31,     0,   743,
       0,     0,     0,     0,   747,   406,   541,    27,     0,   157,
     129,     0,   730,     0,     0,     4,   119,   121,   800,     0,
     711,     0,     6,   192,     7,     9,     8,    10,     0,     0,
     398,   447,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   445,   919,   920,   523,   517,   518,   519,   520,   521,
     522,   428,   526,     0,   427,   890,   713,   720,     0,   803,
     516,   397,   893,   894,   905,   446,     0,     0,   449,   448,
     891,   892,   889,   926,   929,   506,   802,    11,   403,   404,
     405,     0,     0,    32,     0,   119,   192,     0,   994,   446,
     995,     0,   997,   998,   525,   442,     0,   435,   440,     0,
       0,   488,   489,   490,   491,    25,   930,   738,   715,    33,
      34,    36,    37,    35,     0,     0,  1018,   912,   713,     0,
     714,   467,     0,   469,   507,   508,   509,   510,   511,   512,
     513,   515,     0,   958,     0,   810,   725,   212,     0,  1018,
     425,   724,   718,     0,   734,   714,   937,   938,   944,   936,
     726,     0,   426,     0,   728,   514,     0,     0,     0,     0,
     431,     0,   137,   433,     0,     0,   143,   145,     0,     0,
     147,     0,    71,    72,    77,    78,    63,    64,    55,    75,
      86,    87,     0,    58,     0,    62,    70,    68,    89,    81,
      80,    53,    76,    96,    97,    54,    92,    51,    93,    52,
      94,    50,    98,    85,    90,    95,    82,    83,    57,    84,
      88,    49,    79,    65,    99,    73,    66,    56,    43,    44,
      45,    46,    47,    48,    67,   101,   100,   103,    60,    41,
      42,    69,  1065,  1066,    61,  1070,    40,    59,    91,     0,
       0,   119,   102,  1009,  1064,     0,  1067,     0,     0,   149,
       0,     0,     0,   183,     0,     0,     0,     0,     0,     0,
     812,     0,   107,   109,   311,     0,     0,   310,   316,     0,
       0,   223,     0,   226,     0,     0,     0,     0,  1015,   208,
     220,   950,   954,   560,   587,   587,   560,   587,     0,   979,
       0,   749,     0,     0,     0,   977,     0,    16,     0,   123,
     200,   214,   221,   617,   553,     0,  1003,   533,   535,   537,
     867,   434,   447,     0,     0,   445,   446,   448,     0,     0,
     933,   731,     0,   732,     0,     0,     0,   182,     0,     0,
     125,   302,     0,    24,   191,     0,   219,   204,   218,   403,
     406,   192,   399,   172,   173,   174,   175,   176,   178,   179,
     181,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   942,
       0,   171,   935,   935,   964,     0,     0,     0,     0,     0,
       0,     0,     0,   396,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   466,   468,   868,
     869,     0,   935,     0,   881,   302,   302,   935,     0,   950,
       0,   192,     0,     0,   151,     0,   865,   860,   810,     0,
     447,   445,     0,   962,     0,   558,   809,   953,   734,   447,
     445,   446,   125,     0,   302,   424,     0,   883,   727,     0,
     129,   262,     0,   540,     0,   154,     0,     0,   432,     0,
       0,     0,     0,     0,   146,   170,   148,  1065,  1066,  1062,
    1063,     0,  1069,  1055,     0,     0,     0,     0,    74,    39,
      61,    38,  1010,   177,   180,   150,   129,     0,   167,   169,
       0,     0,     0,     0,   110,     0,   811,   108,    18,     0,
     104,     0,   312,     0,   152,     0,     0,   153,   224,   225,
     999,     0,     0,   447,   445,   446,   449,   448,     0,  1045,
     232,     0,   951,     0,     0,     0,     0,   810,   810,     0,
       0,     0,     0,   155,     0,     0,   748,   978,   801,     0,
       0,   976,   806,   975,   122,     5,    13,    14,     0,   230,
       0,     0,   546,     0,     0,   810,     0,   722,     0,   721,
     716,   547,     0,     0,     0,     0,   867,   129,     0,   812,
     866,  1074,   423,   437,   502,   899,   918,   134,   128,   130,
     131,   132,   133,   397,     0,   524,   804,   805,   120,   810,
       0,  1019,     0,     0,     0,   812,   303,     0,   529,   194,
     228,     0,   472,   474,   473,   485,     0,     0,   505,   470,
     471,   475,   477,   476,   494,   495,   492,   493,   496,   497,
     498,   499,   500,   486,   487,   479,   480,   478,   481,   482,
     484,   501,   483,   934,     0,     0,   968,     0,   810,  1002,
       0,  1001,  1018,   896,   210,   202,   216,     0,  1003,   206,
     192,     0,   438,   441,   443,   451,   454,   455,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   871,     0,
     870,   873,   895,   877,  1018,   874,     0,     0,     0,     0,
       0,     0,     0,     0,   996,   436,   858,   862,   809,   864,
       0,   717,     0,   957,     0,   956,   228,     0,   717,   941,
     940,   926,   929,     0,     0,   870,   873,   939,   874,   429,
     264,   266,   129,   544,   543,   430,     0,   129,   246,   138,
     433,     0,     0,     0,     0,     0,   258,   258,   144,   810,
       0,     0,  1054,     0,  1051,   810,     0,  1025,     0,     0,
       0,     0,     0,   808,     0,    33,    34,    36,    37,    35,
       0,     0,   751,   755,   756,   757,   758,   759,   761,     0,
     750,   127,   799,   760,  1018,  1068,     0,     0,     0,     0,
      19,     0,    20,     0,   105,     0,     0,     0,   116,   812,
       0,   114,   109,   106,   111,     0,   309,   317,   314,     0,
       0,   988,   993,   990,   989,   992,   991,    12,  1043,  1044,
       0,   810,     0,     0,     0,   950,   947,     0,   557,     0,
     571,   809,   559,   809,   586,   574,   580,   583,   577,   987,
     986,   985,     0,   981,     0,   982,   984,     0,     5,     0,
       0,     0,   611,   612,   620,   619,     0,   445,     0,   809,
     552,   556,     0,     0,  1004,     0,   534,     0,     0,  1032,
     867,   288,  1073,     0,     0,   882,     0,   932,   809,  1021,
    1017,   304,   305,   710,   811,   301,     0,   867,     0,     0,
     230,   531,   196,   504,     0,   594,   595,     0,   592,   809,
     963,     0,     0,   302,   232,     0,   230,     0,     0,   228,
       0,   942,   452,     0,     0,   879,   880,   897,   898,   927,
     928,     0,     0,     0,   846,   817,   818,   819,   826,     0,
      33,    34,    36,    37,    35,     0,     0,   832,   838,   839,
     840,   841,   842,     0,   830,   828,   829,   852,   810,     0,
     860,   961,   960,     0,   230,     0,   884,   733,     0,   268,
       0,     0,   135,     0,     0,     0,     0,     0,     0,     0,
     238,   239,   250,     0,   129,   248,   164,   258,     0,   258,
       0,   809,     0,     0,     0,     0,     0,   809,  1053,  1056,
    1024,   810,  1023,     0,   810,   782,   783,   780,   781,   816,
       0,   810,   808,   564,   589,   589,   564,   589,   555,     0,
       0,   970,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1059,   184,     0,   187,   168,     0,     0,   112,   117,   118,
     110,   811,   115,     0,   313,     0,  1000,   156,  1016,  1045,
    1036,  1040,   231,   233,   323,     0,     0,   948,     0,   562,
       0,   980,     0,    17,     0,  1003,   229,   323,     0,     0,
     717,   549,     0,   723,  1005,     0,  1032,   538,     0,     0,
    1074,     0,   293,   291,   873,   885,  1018,   873,   886,  1020,
       0,     0,   306,   126,     0,   867,   227,     0,   867,     0,
     503,   967,   966,     0,   302,     0,     0,     0,     0,     0,
       0,   230,   198,   734,   872,   302,     0,   822,   823,   824,
     825,   833,   834,   850,     0,   810,     0,   846,   568,   591,
     591,   568,   591,     0,   821,   854,     0,   809,   857,   859,
     861,     0,   955,     0,   872,     0,     0,     0,     0,   265,
     545,   140,     0,   433,   238,   240,   950,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   252,     0,  1060,     0,
       0,  1046,     0,  1052,  1050,   809,     0,     0,     0,   753,
     809,   807,     0,     0,   810,     0,     0,   796,   810,     0,
       0,     0,     0,   810,     0,   762,   797,   798,   974,     0,
     810,   765,   767,   766,     0,     0,   763,   764,   768,   770,
     769,   786,   787,   784,   785,   788,   789,   790,   791,   792,
     777,   778,   772,   773,   771,   774,   775,   776,   779,  1058,
       0,   129,     0,     0,   113,    21,   315,     0,     0,     0,
    1037,  1042,     0,   397,   952,   950,   439,   444,   450,     0,
       0,    15,     0,   397,   623,     0,     0,   625,   618,   621,
       0,   616,     0,  1007,     0,  1033,   542,     0,   294,     0,
       0,   289,     0,   308,   307,  1032,     0,   323,     0,   867,
       0,   302,     0,   924,   323,  1003,   323,  1006,     0,     0,
       0,   453,     0,     0,   836,   809,   845,   827,     0,     0,
     810,     0,     0,   844,   810,     0,     0,     0,   820,     0,
       0,   810,   831,   851,   959,   323,     0,   129,     0,   261,
     247,     0,     0,     0,   237,   160,   251,     0,     0,   254,
       0,   259,   260,   129,   253,  1061,  1047,     0,     0,  1022,
       0,  1072,   815,   814,   752,   572,   809,   563,     0,   575,
     809,   588,   581,   584,   578,     0,   809,   554,   754,     0,
     593,   809,   969,   794,     0,     0,     0,    22,    23,  1039,
    1034,  1035,  1038,   234,     0,     0,     0,   404,   395,     0,
       0,     0,   209,   322,   324,     0,   394,     0,     0,     0,
    1003,   397,     0,   561,   983,   319,   215,   614,     0,     0,
     548,   536,     0,   297,   287,     0,   290,   296,   302,   528,
    1032,   397,  1032,     0,   965,     0,   923,   397,     0,   397,
    1008,   323,   867,   921,   849,   848,   835,   573,   809,   567,
       0,   576,   809,   590,   582,   585,   579,     0,   837,   809,
     853,   397,   129,   267,   136,   141,   162,   241,     0,   249,
     255,   129,   257,     0,  1048,     0,     0,     0,   566,   795,
     551,     0,   973,   972,   793,   129,   188,  1041,     0,     0,
       0,  1011,     0,     0,     0,   235,     0,  1003,     0,   360,
     356,   362,   712,    32,     0,   350,     0,   355,   359,   372,
       0,   370,   375,     0,   374,     0,   373,     0,   192,   326,
       0,   328,     0,   329,   330,     0,     0,   949,     0,   615,
     613,   624,   622,   298,     0,     0,   285,   295,     0,     0,
    1032,     0,   205,   528,  1032,   925,   211,   319,   217,   397,
       0,     0,     0,   570,   843,   856,     0,   213,   263,     0,
       0,   129,   244,   161,   256,  1049,  1071,   813,     0,     0,
       0,     0,     0,     0,   422,     0,  1012,     0,   340,   344,
     419,   420,   354,     0,     0,     0,   335,   673,   674,   672,
     675,   676,   693,   695,   694,   664,   636,   634,   635,   654,
     669,   670,   630,   641,   642,   644,   643,   663,   647,   645,
     646,   648,   649,   650,   651,   652,   653,   655,   656,   657,
     658,   659,   660,   662,   661,   631,   632,   633,   637,   638,
     640,   678,   679,   683,   684,   685,   686,   687,   688,   671,
     690,   680,   681,   682,   665,   666,   667,   668,   691,   692,
     696,   698,   697,   699,   700,   677,   702,   701,   704,   706,
     705,   639,   709,   707,   708,   703,   689,   629,   367,   626,
       0,   336,   388,   389,   387,   380,     0,   381,   337,   414,
       0,     0,     0,     0,   418,     0,   192,   201,   318,     0,
       0,     0,   286,   300,   922,     0,     0,   390,   129,   195,
    1032,     0,     0,   207,  1032,   847,     0,     0,   129,   242,
     142,   163,     0,   565,   550,   971,   186,   338,   339,   417,
     236,     0,   810,   810,     0,   363,   351,     0,     0,     0,
     369,   371,     0,     0,   376,   383,   384,   382,     0,     0,
     325,  1013,     0,     0,     0,   421,     0,   320,     0,   299,
       0,   609,   812,   129,     0,     0,   197,   203,     0,   569,
     855,     0,     0,   165,   341,   119,     0,   342,   343,     0,
     809,     0,   809,   365,   361,   366,   627,   628,     0,   352,
     385,   386,   378,   379,   377,   415,   412,  1045,   331,   327,
     416,     0,   321,   610,   811,     0,     0,   391,   129,   199,
       0,   245,     0,   190,     0,   397,     0,   357,   364,   368,
       0,     0,   867,   333,     0,   607,   527,   530,     0,   243,
       0,     0,   166,   348,     0,   396,   358,   413,  1014,     0,
     812,   408,   867,   608,   532,     0,   189,     0,     0,   347,
    1032,   867,   272,   411,   410,   409,  1074,   407,     0,     0,
       0,   346,  1026,   408,     0,  1032,     0,   345,     0,     0,
    1074,     0,   277,   275,  1026,   129,   812,  1028,     0,   392,
     129,   332,     0,   278,     0,     0,   273,     0,     0,   811,
    1027,     0,  1031,     0,     0,   281,   271,     0,   274,   280,
     334,   185,  1029,  1030,   393,   282,     0,     0,   269,   279,
       0,   270,   284,   283
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1387, -1387, -1387,  -572, -1387, -1387, -1387,   132,   -11,   -42,
     436, -1387,  -285,  -517, -1387, -1387,   392,   148,  1432, -1387,
    1518, -1387,  -460, -1387,    52, -1387, -1387, -1387, -1387, -1387,
   -1387, -1387, -1387, -1387, -1387, -1387,  -367, -1387, -1387,  -158,
      66,    -2, -1387, -1387, -1387, -1387, -1387, -1387,    27, -1387,
   -1387, -1387, -1387, -1387, -1387,    28, -1387, -1387,  1038,  1039,
    1040,  -110,  -707,  -886,   542,   604,  -380,   286,  -960, -1387,
     -99, -1387, -1387, -1387, -1387,  -741,   116, -1387, -1387, -1387,
   -1387,  -370, -1387,  -629, -1387,  -456, -1387, -1387,   932, -1387,
     -80, -1387, -1387, -1066, -1387, -1387, -1387, -1387, -1387, -1387,
   -1387, -1387, -1387, -1387,  -116, -1387,   -28, -1387, -1387, -1387,
   -1387, -1387,  -198, -1387,    83, -1007, -1387, -1386,  -403, -1387,
    -146,   114,  -120,  -368, -1387,  -194, -1387, -1387, -1387,    93,
     -32,    15,    48,  -740,   -75, -1387, -1387,    25, -1387,    -7,
   -1387, -1387,    -5,   -45,   -60, -1387, -1387, -1387, -1387, -1387,
   -1387, -1387, -1387, -1387,  -608,  -875, -1387, -1387, -1387, -1387,
   -1387,  2233,  1175, -1387,   477, -1387,   344, -1387, -1387, -1387,
   -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387,
   -1387, -1387, -1387,   245,  -417,  -516, -1387, -1387, -1387, -1387,
   -1387,   413, -1387, -1387, -1387, -1387, -1387, -1387, -1387, -1387,
    -990,  -363,  2426,     6, -1387,  1464,  -411, -1387, -1387,  -492,
    3321,  2484, -1387,  -582, -1387, -1387,   485,  1165,  -642, -1387,
   -1387,   569,   357,  -560, -1387,   359, -1387, -1387, -1387, -1387,
   -1387,   545, -1387, -1387, -1387,   310,  -916,  -117,  -434,  -426,
   -1387,   623,  -122, -1387, -1387,    35,    44,   521, -1387, -1387,
     198,   -25, -1387,  -361,    39,  -369,    57,  -314, -1387, -1387,
    -481,  1199, -1387, -1387, -1387, -1387, -1387,   732,   475, -1387,
   -1387, -1387,  -346,  -673, -1387,  1150, -1224, -1387,   -73,  -199,
      -3,   741, -1387,  -355, -1387,  -366, -1062, -1269,  -273,   127,
   -1387,   446,   525, -1387, -1387, -1387, -1387,   468, -1387,   662,
   -1128
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   115,   928,   645,   185,  1561,   742,
     359,   360,   361,   362,   879,   880,   881,   117,   118,   119,
     120,   121,   418,   678,   679,   557,   261,  1629,   563,  1538,
    1630,  1873,   868,   354,   586,  1833,  1124,  1321,  1892,   435,
     186,   680,   968,  1189,  1380,   125,   648,   985,   681,   700,
     989,   620,   984,   240,   538,   682,   649,   986,   437,   379,
     401,   128,   970,   931,   904,  1142,  1564,  1248,  1050,  1780,
    1633,   819,  1056,   562,   828,  1058,  1423,   811,  1039,  1042,
    1237,  1899,  1900,   668,   669,   694,   695,   366,   367,   369,
    1598,  1758,  1759,  1333,  1473,  1587,  1752,  1882,  1902,  1791,
    1837,  1838,  1839,  1574,  1575,  1576,  1577,  1793,  1794,  1800,
    1849,  1580,  1581,  1585,  1745,  1746,  1747,  1769,  1941,  1474,
    1475,   187,   130,  1916,  1917,  1750,  1477,  1478,  1479,  1480,
     131,   254,   558,   559,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,  1610,   142,   967,  1188,   143,   665,
     666,   667,   258,   410,   553,   654,   655,  1283,   656,  1284,
     144,   145,   626,   627,  1273,  1274,  1389,  1390,   146,   853,
    1018,   147,   854,  1019,   148,   855,  1020,   149,   856,  1021,
     150,   857,  1022,   629,  1276,  1392,   151,   858,   152,   153,
    1822,   154,   650,  1600,   651,  1158,   936,  1351,  1348,  1738,
    1739,   155,   156,   157,   243,   158,   244,   255,   422,   545,
     159,  1277,  1278,   862,   863,   160,  1080,   959,   597,  1081,
    1025,  1211,  1026,  1393,  1394,  1214,  1215,  1028,  1400,  1401,
    1029,   787,   528,   199,   200,   683,   671,   511,  1174,  1175,
     773,   774,   955,   162,   246,   163,   164,   189,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   734,   250,   251,
     623,   234,   235,   737,   738,  1289,  1290,   394,   395,   922,
     175,   611,   176,   664,   177,   345,  1760,  1812,   380,   430,
     689,   690,  1073,  1929,  1936,  1937,  1169,  1330,   900,  1331,
     901,   902,   834,   835,   836,   346,   347,   865,   572,  1563,
     953
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     188,   190,   124,   192,   193,   194,   195,   197,   198,   442,
     201,   202,   203,   204,   343,   492,   224,   225,   226,   227,
     228,   229,   230,   231,   233,   402,   252,   951,   520,   405,
     406,   126,   127,   413,  1170,   660,   242,   947,   782,   260,
     542,   657,  1357,   659,   514,   342,   796,   268,   257,   271,
     351,   491,   352,   965,   355,   438,   122,   946,   661,   778,
     779,   262,  1462,   415,  1162,   247,   266,   442,   731,   249,
     123,   591,   593,   927,   248,   988,   412,   771,   878,   883,
    1046,   260,   350,   417,  1187,   772,  1060,  1244,   803,  1034,
     810,  1343,   259,   824,   826,  1647,  1421,   -74,    14,    14,
    1198,   -39,   -74,   414,  1354,   -38,   -39,   432,   554,   603,
     -38,   606,   806,   554,   368,  1590,  1592,    14,   129,  1802,
     807,    14,  -353,  -903,   889,    14,   866,  1171,  1655,   547,
    1740,  1809,  1809,   546,   388,  1647,   116,   554,   906,   415,
     906,  -603,   906,  1090,   512,   906,  1803,   906,  1233,  -901,
    -900,   512,   412,   206,    40,  1492,  1931,   509,   510,   417,
    -902,   898,   899, -1018,   587,  1358,  1487,   599,   420,   735,
     531,  1119,  1172,  -596,   801,   529,  1344,   364,  1349,   414,
       3,  -103,  1091,   530,   429,   269,   523,  1954,   341,  1345,
     417,  -715,   429,   540,  -606,   441,  -103,   191,   776, -1018,
    1493,  1932, -1018,   780,  1024,   378,   539,   948,   363,  1346,
     414,  1350,  -943,  1282,  -907,   509,   510,   222,   222,   353,
    -906,  -603,   253,   926,   517,  -904,  1027,   588,   400,   600,
     378,  -946,  1955,   414,   378,   378,   398,  1132,  -945,   399,
    -722,  1562,   391,  -811,  -292,  -887,  -908,  -811,   549,  -910,
    -721,   549,  -903,   521,  -888,   517,  1359,  1603,   260,   560,
     378,   827,  1422,  1173,  1943,   933,  -292,   516,   895,   571,
    -276,   111,   635,   513,  -811,   493,  -911,   403,  -901,  -900,
     513,   825,  1201,   701,  1648,  1649,   -74,  1414,  1462,  -902,
     -39,  1494,  1933,  -809,   -38,   365,   433,   555,   604,  -912,
     607,  1501,   633,  1499,  1591,  1593,   582,  1804,  1507,   551,
    1509,  -353,   890,   556,   161,  1379,  1251,  1656,  1255,  1741,
    1810,  1859,  1965,  1956,  1927,   891,   907,   407,  1001,   389,
    1334,   614,   527,  1537,   634,  1597,  -723,   342,  1184,  1531,
    -914,  -943,  1650,  -907,   613,  -716,  -917,   516,  1604,  -906,
    1399,   617,  1043,   518,  -904,  1944,  1154,  1045,  1128,  1129,
    -946,  1223,   699,   783,  -714,   873,  1753,  -945,  1754,   599,
     260,   414,   983,   442,  -887,  -908,   934,   233,   625,   260,
     260,   625,   260,  -888,   518,  1826, -1018,   639,  1402,   116,
     343,   935,  1797,   116,  1253,  1254,  1820,   561,   509,   510,
    -913,   372,   256,   636,   392,   393,   197,   429,  1253,  1254,
    1798,   373,  -539,  1966,   684,   402,   747,   748,   438,   752,
    -604,   342,  1024,   612,  1145, -1018,   696,   874,  1224,  1799,
     222,   428,   628,   628,   263,   628,   408,  1286,  1611,   264,
    1613,  1821,  1869,   409,  1213,  1619,   702,   703,   704,   705,
     707,   708,   709,   710,   711,   712,   713,   714,   715,   716,
     717,   718,   719,   720,   721,   722,   723,   724,   725,   726,
     727,   728,   729,   730,   741,   732,   873,   733,   733,   736,
     581,   265,  1342,  1884,   123,   242,   374,   754,  1256,   755,
     756,   757,   758,   759,   760,   761,   762,   763,   764,   765,
     766,   767,  1424,  1411,   363,   363,   594,   733,   777,   375,
     696,   696,   733,   781,   247,   206,    40,   755,   249,   753,
     785,   342,  1177,   248,  1951,   165,   389,  1195,  1885,   793,
    1178,   795,   129,   641,  -605,   389,  -102,   492,   428,   696,
     221,   223,   743,   982,   644,   813,   428,   814,  1766,   815,
     116,  -102,  1771,   509,   510,   954,  1366,   956,   750,  1368,
     688,  1356,  -875,   222,   341,   376,  1805,   378,   775,   479,
     660,  1551,   222,   491,   616,   994,   657,  -875,   659,   222,
    1203,   480,   800,  -119,  1125,  1806,  1126,  -119,  1807,   743,
    -878,   222,   990,   661,  1250,   377,   885,   370,   509,   510,
     802,   392,   393,   808,  -119,  -878,   371,   381,   937,   818,
     392,   393,  1328,  1329,   878,  -915,   382,   581,   378,   745,
     378,   378,   378,   378,  -717,   509,   510,  1852,   972,   416,
     630,   419,   632,   111,   383,  1024,  1024,  1024,  1024,  1024,
    1024,  -876,   428,   770,   384,  1024,  1853,   686,   414,  1854,
    1115,  1116,  1117,   542,  1626,   385,  -876,  1213,  1391,   403,
    -915,  1391,   898,   899,   581,  1120,  1118,  1403,  1279,  1952,
    1281,   427,   389,   954,   956,  1036,   646,   647,   805,   641,
    1035,   956,    55,   476,   477,   478,   386,   479,   962,   116,
     439,   179,   180,    65,    66,    67,  1252,  1253,  1254,   480,
     389,   973,  1508,   387,  1395,   416,  1397,   421,  1825,   864,
     404,  1381,  1828,  1040,  1041,   389,   431,   222,   348,   660,
     670,   434,   390,  1235,  1236,   657,   443,   659,  1328,  1329,
     389,   884,   688,  1558,  1559,   981,   416,   641,  1372,   444,
    1503,   445,   661,   882,   882,   446,  1491,   392,   393,  1382,
     447,   389,   205,   533,   206,    40,  1767,  1768,   424,   541,
    1939,  1940,   440,   448,   993,  1413,   921,   923,   439,   179,
     180,    65,    66,    67,    50,   392,   393,   389,   165,  1418,
    1253,  1254,   165,  -597,   641,  1913,  1914,  1915,  1924,   391,
     392,   393,   590,   592,   687,  1850,  1851,   493,   449,  1038,
     450,  1024,  1942,  1024,   642,   392,   393,  1595,  1846,  1847,
     209,   210,   211,   212,   213,   260,  -598,   439,    63,    64,
      65,    66,    67,  1514,  -599,  1515,   392,   393,    72,   486,
    -600,  1044,  -601,   378,  1071,  1074,   768,   482,    93,    94,
     440,    95,   183,    97,   423,   425,   426,  1909,  1922,   483,
     484,   485,   392,   393,  1482,   515,   660,  -909,  -602,  -715,
     519,  1456,   657,  1934,   659,   637,   107,   396,   524,   643,
     769,   488,   111,   526,   480,   532,   123,  1055,   429,   661,
    -913,   516,   535,  1620,   536,  -713,   602,   543,   544,   440,
     222,   552,   573,   565,  1651,   610,   637,   615,   643,   637,
     643,   643,   622, -1057,  1149,   576,  1150,   577,   815,   583,
     584,   596,   605,   595,   640,  1505,   598,  1202,  1017,  1152,
    1030,   608,   609,   618,   129,   124,   619,   662,   663,    55,
     672,   741,   123,  1161,   673,   674,  1024,   676,  1024,   165,
    1024,   685,   116,   788,   569,  1024,   570,  1533,  -124,   222,
     636,   698,  1464,   786,   126,   127,  1053,   116,  1622,  1182,
    1623,   790,  1624,  1542,   816,   791,  1901,  1625,   797,  1190,
     798,   554,  1191,   820,  1192,   823,   670,  1362,   696,   122,
     129,   571,   837,   838,   869,   867,  1901,   888,   870,   903,
     222,   892,   222,   123,    14,  1923,   871,   242,   116,  1163,
     872,   893,   575,   896,   905,  1127,   688,   897,   908,   911,
     910,   775,   913,   808,   123,   915,   916,   882,   917,   882,
     222,   924,   918,   882,   882,  1130,   247,   929,  1232,   930,
     249,   932,  -738,   938,  1141,   248,   939,   941,   942,  1024,
     622,   129,  1608,   439,   179,   180,    65,    66,    67,   945,
     950,   949,   958,   960,   964,  1238,   963,   969,  1465,   116,
     966,  1775,   129,  1466,   975,   439,  1467,   180,    65,    66,
      67,  1468,  1628,  1336,   581,   979,   976,   660,   165,  1287,
     116,  1634,   978,   657,   987,   659,   770,   222,   805,   995,
     997,   691,   808,  1239,   348,  1641,   998,   999,  -719,   971,
     661,  1047,  1037,   222,   222,  1057,  1059,  1061,  1065,   123,
    1066,   123,  1464,  1469,  1470,   440,  1471,  1069,  1067,  1082,
    1083,  1084,   378,   473,   474,   475,   476,   477,   478,  1085,
     479,  1086,  1087,  1088,  1210,  1210,  1017,   440,  1123,  1131,
    1133,  1337,   480,  1338,  1139,  1135,  1472,  1137,  1138,  1166,
    1148,  1144,   124,  1157,    14,  1159,  1151,   129,  1164,   129,
     660,  1160,  1200,  1185,  1168,  1194,   657,   805,   659,  1197,
    1206,  1782,  1205,   116,  -916,   116,  1364,   116,  1216,  1217,
    1865,   126,   127,   661,  1218,  1219,    34,    35,    36,   696,
    1220,  1221,  1222,  1225,  1024,  1024,  1226,  1227,  1262,   207,
     696,  1338,  1229,  1247,  1241,  1243,   122,  1246,  1249,  1260,
    1258,  1259,  1265,   961,  1266,  1118,  1829,  1830,  1465,  1269,
     123,  1270,   581,  1466,  1322,   439,  1467,   180,    65,    66,
      67,  1468,  1320,   829,  1323,  1325,  1332,   161,   260,  1352,
    1335,  1361,  1360,   983,  1385,   222,   222,  1367,  1420,  1365,
    1406,   864,  1353,    81,    82,    83,    84,    85,  1912,  1369,
     670,  1371,  1008,  1374,   214,  1373,  1407,  1377,   129,  1383,
      89,    90,   992,  1469,  1470,  1376,  1471,   670,  1405,   882,
    1384,  1398,  1408,  1410,    99,  1415,   116,  1419,  1428,  1425,
    1409,  1430,  1431,  1434,  1950,  1435,  1436,   440,   104,  1440,
    1112,  1113,  1114,  1115,  1116,  1117,  1486,  1439,  1824,  1445,
    1446,  1451,  1450,  1031,  1442,  1032,   123,  1457,  1831,  1118,
    1443,  1444,  1448,  1484,   943,   944,  1455,  1485,  1495,  1496,
    1517,   165,  1458,   952,  1483,  1596,  1498,  1510,  1518,  1500,
    1502,  1488,  1506,  1051,  1513,  1489,   165,  1490,  1516,  1017,
    1017,  1017,  1017,  1017,  1017,  1497,   442,  1512,  1511,  1017,
    1520,  1522,  1527,  1866,   129,  1504,   696,  1521,  1529,  1524,
     116,  1525,  1526,  1528,  1532,  1534,  1535,  1536,   222,  1539,
    1540,  1543,   116,  1555,  1566,  1579,  1599,   165,  1594,  1605,
    1606,  1609,  1427,  1614,  1615,  1464,  1621,  1617,  1636,  1639,
     537,   439,    63,    64,    65,    66,    67,  1645,  1888,  1654,
    1136,  1653,    72,   486,  1749,  1748,  1755,  1761,  1762,  1764,
    1774,  1765,  1776,  1808,  1777,  1814,   622,  1147,  1818,   222,
    1751,  1787,  1464,  1840,  1788,  1817,  1842,    14,  1823,  1848,
    1844,  1856,  1857,  1858,   222,   222,  1863,  1476,   165,  1864,
    1868,   216,   216,   487,  1871,   488,  1872,  1476,  -349,  1459,
    1874,  1877,   239,  1875,   161,  1879,  1803,  1880,   489,   165,
     490,  1883,  1890,   440,    14,  1948,  1889,  1886,  1896,  1898,
    1953,  1891,  1903,   219,   219,  1907,  1910,  1644,   239,  1911,
    1607,  1919,  1921,   696,  1935,   670,  1925,  1945,   670,   691,
     691,  1465,  1926,  1928,  1946,  1949,  1466,  1957,   439,  1467,
     180,    65,    66,    67,  1468,  1017,  1958,  1017,  1967,  1968,
    1970,  1971,   123,  1324,  1906,   744,  1196,  1920,   746,   749,
    1412,  1781,  1918,   222,  1156,   886,  1541,  1772,  1465,  1796,
    1652,  1588,  1801,  1466,  1960,   439,  1467,   180,    65,    66,
      67,  1468,  1646,  1770,  1586,  1930,  1469,  1470,  1813,  1471,
    1567,   631,   165,  1280,   165,  1396,   165,  1272,  1051,  1245,
     129,  1347,  1212,  1387,   344,  1230,  1388,  1176,  1072,  1947,
     440,   624,   697,  1962,  1881,  1327,  1557,  1319,   116,  1612,
    1632,  1155,  1264,  1469,  1470,   493,  1471,   341,  1816,   123,
    1763,     0,     0,  1584,     0,     0,     0,  1165,   123,     0,
       0,     0,     0,     0,     0,  1476,     0,   440,     0,     0,
    1179,  1476,     0,  1476,     0,     0,  1616,     0,     0, -1058,
   -1058, -1058, -1058, -1058,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,     0,  1481,     0,  1476,     0,   129,     0,  1199,
    1017,     0,  1017,  1481,  1017,  1118,   129,     0,     0,  1017,
       0,     0,  1756,     0,   216,   116,     0,     0,     0,     0,
     116,     0,     0,     0,   116,   165,     0,     0,     0,   670,
       0,  1779,  1632,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   378,   789,   123,   581,   219,     0,   341,     0,
     123,  1363,     0,     0,     0,     0,     0,   123,  1737,  1811,
       0,     0,     0,     0,   239,  1744,   239,     0,     0,     0,
       0,     0,   341,  1257,   341,     0,     0,  1261,     0,  1894,
     341,     0,     0,  1476,     0,     0,     0,     0,     0,     0,
     342,     0,   129,  1861,     0,     0,     0,     0,   129,     0,
       0,     0,  1404,  1017,     0,   129,  1819,     0,     0,   165,
     116,   116,   116,     0,     0,     0,   116,   622,  1051,     0,
       0,   165,   239,   116,     0,   442,     0,     0,   522,   495,
     496,   497,   498,   499,   500,   501,   502,   503,   504,   505,
     506,     0,   912,   914,     0,     0,     0,   216,     0,     0,
     344,     0,   344,     0,     0,     0,   216,     0,     0,     0,
       0,  1481,     0,   216,     0,     0,     0,  1481,     0,  1481,
     940,     0,   670,   507,   508,   216,     0,     0,     0,   219,
    1355,     0,   952,     0,     0,     0,   216,     0,   219,     0,
       0,  1481,     0,     0,     0,   219,     0,     0,   123,     0,
       0,     0,     0,     0,     0,     0,   622,   219,   344,  1375,
       0,   239,  1378,     0,   239,     0,     0,     0,   658,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   581,     0,     0,     0,     0,     0,     0,
     123,     0,     0,     0,     0,     0,   129,   123,   509,   510,
       0,     0,     0,   980,   341,     0,     0,     0,  1017,  1017,
       0,   239,     0,     0,   116,     0,     0,  1464,     0,     0,
       0,  1426,     0,  1835,     0,     0,     0,  1179,     0,  1481,
    1737,  1737,   123,     0,  1744,  1744,     0,     0,   129,     0,
    1959,     0,     0,     0,     0,   129,     0,   344,   378,     0,
     344,   216,  1969,     0,   123,     0,   116,     0,     0,    14,
       0,   675,  1972,   116,     0,  1973,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   165,     0,     0,
     129,     0,     0,   219,     0,     0,     0,     0,     0,  1895,
    1460,  1461,     0,     0,  1062,     0,     0,     0,   116,     0,
    1068,     0,   129,   239,     0,   239,  1893,     0,   852,     0,
       0,     0,     0,     0,   123,  1464,     0,     0,     0,   123,
     116,     0,     0,  1465,     0,     0,  1908,     0,  1466,     0,
     439,  1467,   180,    65,    66,    67,  1468,   852,   494,   495,
     496,   497,   498,   499,   500,   501,   502,   503,   504,   505,
     506,     0,     0,     0,   165,  1464,     0,    14,     0,   165,
       0,     0,   129,   165,     0,     0,  1140,   129,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1469,  1470,
     116,  1471,     0,   507,   508,   116,     0,     0,     0,   344,
    1544,   833,  1545,     0,   239,   239,     0,    14,     0,     0,
       0,     0,   440,   239,     0,     0,     0,     0,     0,     0,
       0,  1618,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1465,     0,     0,   216,     0,  1466,     0,   439,  1467,
     180,    65,    66,    67,  1468,     0,     0,     0,  1589,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   165,
     165,   165,     0,     0,     0,   165,   219,     0,   509,   510,
       0,  1465,   165,     0,     0,     0,  1466,     0,   439,  1467,
     180,    65,    66,    67,  1468,     0,  1469,  1470,     0,  1471,
     344,   344,     0,   216,     0,     0,     0,     0,     0,   344,
       0,     0,   670,  1228,     0,     0,     0,     0,     0,     0,
     440,     0,     0,     0,     0,  1635,   205,     0,     0,  1627,
       0,     0,   670,     0,     0,   219,  1469,  1470,   239,  1471,
       0,   670,     0,     0,   216,     0,   216,     0,    50,     0,
       0,     0,     0,     0,     0,     0,   356,   357,     0,  1267,
     440,     0,     0,     0,     0,     0,  1271,     0,     0,  1773,
       0,     0,   217,   217,   216,   852,   219,     0,   219,     0,
       0,     0,   239,     0,   209,   210,   211,   212,   213,   239,
     239,   852,   852,   852,   852,   852,     0,     0,     0,     0,
       0,     0,     0,   852,     0,     0,   219,     0,     0,   358,
       0,     0,    93,    94,     0,    95,   183,    97,     0,   239,
       0,     0,     0,   165,     0,     0,   205,     0,     0,     0,
       0,     0,     0,     0,     0,  1792,     0,     0,     0,     0,
     107,   216,     0,     0,   205,     0,   206,    40,    50,     0,
       0,     0,     0,     0,     0,   239,     0,   216,   216,     0,
       0,     0,     0,     0,     0,   165,    50,     0,  1064,     0,
       0,     0,   165,   219,     0,   344,   344,     0,     0,     0,
       0,   239,   239,     0,   209,   210,   211,   212,   213,   219,
     219,   216,     0,     0,     0,     0,     0,   239,     0,     0,
    1386,     0,   209,   210,   211,   212,   213,   165,     0,   396,
     239,     0,    93,    94,     0,    95,   183,    97,   852,   205,
       0,   239,     0,   658,     0,     0,     0,     0,   768,   165,
      93,    94,     0,    95,   183,    97,     0,  1815,     0,   239,
     107,    50,     0,   239,   397,     0,     0,     0,     0,   875,
     876,     0,     0,     0,     0,     0,   239,     0,   107,  1437,
       0,     0,   804,  1441,   111,   218,   218,   344,  1447,     0,
       0,     0,     0,     0,     0,  1452,   241,   209,   210,   211,
     212,   213,     0,   344,     0,   217,     0,     0,     0,   165,
       0,     0,     0,     0,   165,     0,   344,     0,     0,   216,
     216,     0,   877,     0,     0,    93,    94,     0,    95,   183,
      97,     0,     0,   239,     0,     0,     0,   239,     0,   239,
       0,     0,  1876,     0,     0,   344,     0,     0,     0,     0,
       0,   219,   219,   107,   852,   852,   852,   852,   852,   852,
     216,     0,     0,   852,   852,   852,   852,   852,   852,   852,
     852,   852,   852,   852,   852,   852,   852,   852,   852,   852,
     852,   852,   852,   852,   852,   852,   852,   852,   852,   852,
     852,     0,   658,     0,     0,  1519,     0,     0,     0,  1523,
       0,     0,     0,     0,     0,     0,  1530,   852,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   952,   344,
       0,     0,     0,   344,     0,   833,     0,     0,     0,     0,
    1938,     0,   952,     0,     0,     0,     0,     0,   217,     0,
     239,     0,   239,     0,     0,     0,     0,   217,     0,     0,
       0,  1938,   216,  1963,   217,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   217,     0,     0,   239,
       0,     0,   239,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   219,     0,     0,     0,     0,   239,
     239,   239,   239,   239,   239,     0,     0,   216,   218,   239,
       0,     0,     0,   216,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   216,   216,
       0,   852,     0,     0,     0,     0,   344,     0,   344,   658,
       0,   239,     0,     0,     0,   219,     0,   239,     0,     0,
     852,     0,   852,     0,   451,   452,   453,     0,     0,     0,
     219,   219,   830,     0,     0,   344,     0,     0,   344,     0,
       0,     0,     0,     0,   454,   455,   852,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
       0,   479,   217,     0,     0,     0,     0,     0,     0,     0,
     239,   239,   205,   480,   239,     0,     0,   216,     0,     0,
       0,     0,   831,     0,     0,     0,     0,   344,     0,     0,
       0,     0,     0,   344,    50,     0,     0,     0,     0,     0,
       0,   218,     0,     0,     0,     0,     0,     0,     0,   219,
     218,     0,     0,     0,     0,     0,     0,   218,     0,     0,
       0,     0,     0,     0,     0,   239,     0,   239,     0,   218,
     209,   210,   211,   212,   213,     0,     0,     0,     0,     0,
     218,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   182,     0,     0,    91,   344,   344,    93,    94,
       0,    95,   183,    97,     0,   832,     0,     0,     0,     0,
     239,     0,   239,     0,     0,     0,     0,     0,   852,     0,
     852,     0,   852,     0,     0,     0,   107,   852,   216,     0,
       0,   852,     0,   852,     0,     0,   852,     0,     0,     0,
       0,     0,     0,     0,   925,     0,     0,   239,   239,     0,
       0,   239,     0,     0,     0,   241,     0,     0,   239,     0,
     658,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   217,   522,   495,   496,   497,
     498,   499,   500,   501,   502,   503,   504,   505,   506,     0,
       0,     0,     0,     0,     0,   218,   344,     0,   344,     0,
     239,     0,   239,     0,   239,     0,     0,  1841,  1843,   239,
       0,   216,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   507,   508,     0,     0,   239,     0,     0,   852,     0,
       0,     0,     0,   344,   217,     0,     0,     0,     0,     0,
     239,   239,     0,   658,   344,     0,     0,     0,   239,     0,
     239,     0,   859,     0,     0,     0,   522,   495,   496,   497,
     498,   499,   500,   501,   502,   503,   504,   505,   506,     0,
       0,     0,   239,     0,   239,   217,     0,   217,     0,     0,
     239,   859,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   509,   510,     0,     0,
       0,   507,   508,   239,     0,   217,     0,     0,     0,     0,
     861,   344,     0,     0,     0,     0,     0,     0,     0,     0,
     852,   852,   852,     0,     0,     0,     0,   852,     0,   239,
       0,     0,     0,     0,   344,   239,     0,   239,     0,   887,
     522,   495,   496,   497,   498,   499,   500,   501,   502,   503,
     504,   505,   506,     0,     0,     0,     0,     0,   344,   799,
     344,     0,     0,     0,     0,     0,   344,     0,   218,     0,
       0,     0,   217,   451,   452,   453,   509,   510,     0,     0,
       0,     0,     0,     0,     0,   507,   508,     0,   217,   217,
       0,     0,     0,   454,   455,     0,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,     0,
     479,   344,   205,     0,     0,     0,     0,   218,     0,     0,
       0,     0,   480,     0,     0,     0,     0,   239,     0,   894,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   239,     0,     0,     0,   239,   239,
     509,   510,  1023,     0,     0,     0,     0,     0,   218,     0,
     218,     0,     0,   239,     0,     0,     0,     0,     0,   852,
     209,   210,   211,   212,   213,     0,     0,     0,     0,     0,
     852,     0,     0,     0,     0,     0,   852,     0,   218,   859,
     852,     0,   182,     0,     0,    91,     0,     0,    93,    94,
       0,    95,   183,    97,   282,   859,   859,   859,   859,   859,
       0,     0,   239,   344,     0,     0,     0,   859,     0,     0,
     217,   217,     0,     0,     0,     0,   107,     0,     0,     0,
     344,  1834,     0,  1122,     0,     0,     0,     0,     0,     0,
       0,   284,     0,     0,     0,     0,     0,  1052,     0,  1836,
       0,     0,   852,     0,   205,   218,  1340,     0,     0,     0,
       0,     0,   239,  1075,  1076,  1077,  1078,  1079,     0,  1143,
       0,   218,   218,     0,     0,  1089,    50,     0,     0,   239,
     220,   220,     0,     0,   574,     0,     0,     0,   239,     0,
       0,   245,     0,     0,     0,     0,  1143,     0,   344,     0,
     239,     0,   239,     0,     0,   218,     0,     0,     0,     0,
       0,   567,   209,   210,   211,   212,   213,   568,     0,     0,
       0,   239,     0,   239,     0,     0,     0,     0,     0,     0,
       0,     0,   859,     0,   182,  1186,     0,    91,   335,     0,
      93,    94,     0,    95,   183,    97,     0,     0,     0,     0,
       0,     0,     0,   217,     0,     0,     0,   241,   339,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   107,   340,
    1023,     0,     0,     0,   344,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   344,     0,   344,     0,
    1183,     0,     0,     0,     0,     0,     0,  1092,  1093,  1094,
       0,     0,     0,     0,   217,     0,     0,   344,     0,   344,
       0,     0,     0,   218,   218,     0,     0,     0,  1095,   217,
     217,  1096,  1097,  1098,  1099,  1100,  1101,  1102,  1103,  1104,
    1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,     0,     0,     0,     0,     0,   859,   859,
     859,   859,   859,   859,   218,     0,  1118,   859,   859,   859,
     859,   859,   859,   859,   859,   859,   859,   859,   859,   859,
     859,   859,   859,   859,   859,   859,   859,   859,   859,   859,
     859,   859,   859,   859,   859,     0,  1207,  1208,  1209,   205,
       0,     0,     0,   220,     0,     0,     0,     0,     0,     0,
       0,   859,     0,     0,     0,     0,  1079,  1275,   217,     0,
    1275,    50,     0,     0,     0,  1288,  1291,  1292,  1293,  1295,
    1296,  1297,  1298,  1299,  1300,  1301,  1302,  1303,  1304,  1305,
    1306,  1307,  1308,  1309,  1310,  1311,  1312,  1313,  1314,  1315,
    1316,  1317,  1318,     0,     0,     0,   218,   209,   210,   211,
     212,   213,     0,     0,     0,     0,     0,     0,     0,  1326,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   282,     0,     0,    93,    94,     0,    95,   183,
      97,     0,     0,  1023,  1023,  1023,  1023,  1023,  1023,     0,
       0,   218,     0,  1023,     0,     0,     0,   218,     0,     0,
       0,     0,     0,   107,     0,     0,     0,     0,     0,   284,
       0,     0,   218,   218,     0,   859,     0,     0,     0,     0,
       0,     0,   205,     0,     0,     0,   220,     0,     0,     0,
       0,     0,     0,     0,   859,   220,   859,     0,     0,     0,
       0,     0,   220,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   220,     0,     0,     0,     0,     0,
     859,     0,     0,     0,     0,   245,     0,     0,     0,     0,
       0,     0,     0,  1416,     0,     0,     0,     0,     0,   567,
     209,   210,   211,   212,   213,   568,     0,     0,     0,     0,
       0,     0,  1432,     0,  1433,     0,     0,     0,  1463,     0,
       0,   218,   182,     0,     0,    91,   335,     0,    93,    94,
       0,    95,   183,    97,     0,  1070,     0,     0,  1453,     0,
       0,     0,     0,     0,     0,     0,   339,     0,     0,     0,
       0,     0,   451,   452,   453,     0,   107,   340,     0,     0,
     245,     0,     0,     0,     0,     0,     0,     0,     0,  1023,
       0,  1023,   454,   455,     0,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,     0,   479,
     220,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   480,     0,     0,     0,     0,   282,     0,     0,     0,
       0,     0,   859,     0,   859,     0,   859,     0,   282,     0,
       0,   859,   218,     0,     0,   859,     0,   859,     0,     0,
     859,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1565,   284,     0,  1578,     0,   860,     0,     0,
       0,     0,     0,     0,     0,   284,   205,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   205,     0,
    1547,     0,  1548,     0,  1549,     0,   860,     0,    50,  1550,
       0,     0,     0,  1552,     0,  1553,     0,     0,  1554,     0,
      50,     0,     0,     0,  1023,     0,  1023,     0,  1023,     0,
       0,     0,     0,  1023,     0,   218,     0,     0,     0,     0,
       0,     0,     0,   567,   209,   210,   211,   212,   213,   568,
       0,     0,   859,     0,     0,   567,   209,   210,   211,   212,
     213,   568,   957,     0,  1642,  1643,   182,     0,     0,    91,
     335,     0,    93,    94,  1578,    95,   183,    97,   182,  1429,
       0,    91,   335,     0,    93,    94,     0,    95,   183,    97,
     339,     0,     0,   220,     0,     0,     0,     0,     0,     0,
     107,   340,   339,     0,     0,     0,     0,     0,     0,     0,
    1637,     0,   107,   340,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1023,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   451,
     452,   453,     0,     0,   859,   859,   859,     0,     0,     0,
       0,   859,   220,  1790,     0,     0,     0,     0,     0,   454,
     455,  1578,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,     0,   479,     0,     0,     0,
       0,     0,     0,   220,     0,   220,     0,     0,   480,     0,
       0,     0,  1783,  1784,  1785,     0,     0,     0,     0,  1789,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   220,   860,     0,     0,     0,     0,     0,
     451,   452,   453,     0,     0,     0,     0,     0,     0,     0,
     860,   860,   860,   860,   860,     0,     0,     0,     0,     0,
     454,   455,   860,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,     0,   479,     0,     0,
       0,     0,  1023,  1023,     0,     0,     0,     0,     0,   480,
     220,     0,     0,     0,   205,     0,     0,     0,     0,     0,
       0,     0,     0,   859,     0,     0,   220,   220,     0,     0,
       0,     0,     0,     0,   859,     0,    50,     0,     0,     0,
     859,     0,     0,     0,   859,     0,     0,     0,     0,   996,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     245,  1582,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   209,   210,   211,   212,   213,     0,     0,     0,
       0,  1845,     0,     0,     0,     0,     0,   860,     0,     0,
       0,     0,  1855,     0,     0,     0,     0,     0,  1860,     0,
      93,    94,  1862,    95,   183,    97,   859,     0,     0,     0,
       0,     0,   245,     0,     0,     0,  1905,     0,   451,   452,
     453,     0,     0,     0,     0,     0,     0,     0,   107,  1583,
       0,     0,     0,  1565,     0,     0,     0,     0,   454,   455,
    1000,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,  1897,   479,     0,     0,   220,   220,
       0,     0,     0,     0,     0,     0,     0,   480,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   205,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   860,   860,   860,   860,   860,   860,   245,
      50,     0,   860,   860,   860,   860,   860,   860,   860,   860,
     860,   860,   860,   860,   860,   860,   860,   860,   860,   860,
     860,   860,   860,   860,   860,   860,   860,   860,   860,   860,
       0,     0,     0,     0,     0,     0,   209,   210,   211,   212,
     213,     0,     0,     0,     0,     0,   860,     0,     0,     0,
       0,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,  1742,     0,    93,    94,  1743,    95,   183,    97,
       0,     0,     0,     0,     0,    11,   411,    13,     0,     0,
       0,     0,     0,     0,     0,     0,   751,     0,     0,     0,
       0,   220,   107,  1583,     0,     0,   481,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,   245,     0,     0,    43,
       0,     0,   220,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,   220,   220,    55,
     860,     0,     0,     0,     0,     0,     0,   178,   179,   180,
      65,    66,    67,     0,     0,    69,    70,     0,     0,   860,
       0,   860,     0,     0,     0,   181,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,   860,     0,     0,     0,   182,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   183,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   451,   452,   453,     0,   104,   105,
     106,     0,     0,   107,   108,     0,   220,     0,     0,   111,
     112,     0,   113,   114,   454,   455,     0,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
       0,   479,  1092,  1093,  1094,     0,     0,     0,     0,     0,
       0,     0,     0,   480,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1095,     0,     0,  1096,  1097,  1098,  1099,
    1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,  1118,     0,     0,     0,     0,     0,   860,     0,   860,
       0,   860,    11,    12,    13,     0,   860,   245,     0,     0,
     860,     0,   860,     0,     0,   860,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,  1134,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,    56,    57,    58,
     245,    59,    60,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,    71,    72,    73,   860,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,  1285,
      81,    82,    83,    84,    85,     0,     0,   205,    86,     0,
       0,    87,     0,   205,     0,     0,    88,    89,    90,    91,
      92,     0,    93,    94,     0,    95,    96,    97,    98,    50,
       0,    99,     0,     0,   100,    50,     0,     0,     0,     0,
     101,   102,     0,   103,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,  1153,   111,   112,     0,   113,
     114,     0,     0,     0,     0,   209,   210,   211,   212,   213,
       0,   209,   210,   211,   212,   213,     0,     0,     0,   860,
     860,   860,     0,     0,     0,     0,   860,     0,     0,     0,
       0,     0,     0,    93,    94,  1795,    95,   183,    97,    93,
      94,     0,    95,   183,    97,     0,     0,     0,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,   107,   698,     0,     0,     0,     0,   107,   971,     0,
       0,     0,    11,    12,    13,     0, -1058, -1058, -1058, -1058,
   -1058,   471,   472,   473,   474,   475,   476,   477,   478,     0,
     479,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,   480,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,    56,    57,    58,
       0,    59,    60,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,    71,    72,    73,     0,   860,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,   860,
      81,    82,    83,    84,    85,   860,     0,     0,    86,   860,
       0,    87,     0,     0,     0,     0,    88,    89,    90,    91,
      92,     0,    93,    94,     0,    95,    96,    97,    98,     0,
       0,    99,     0,  1878,   100,     0,     0,     0,     0,     0,
     101,   102,     0,   103,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,  1341,   111,   112,     0,   113,
     114,     0,     5,     6,     7,     8,     9,     0,     0,     0,
       0,   860,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,    56,    57,    58,     0,    59,    60,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
      88,    89,    90,    91,    92,     0,    93,    94,     0,    95,
      96,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,   103,     0,   104,
     105,   106,     0,     0,   107,   108,     0,   109,   110,     0,
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
     109,   110,   677,   111,   112,     0,   113,   114,     5,     6,
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
     107,   108,     0,   109,   110,  1121,   111,   112,     0,   113,
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
     106,     0,     0,   107,   108,     0,   109,   110,  1167,   111,
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
     110,  1240,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,  1242,
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
     108,     0,   109,   110,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,  1417,     0,
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
    1556,   111,   112,     0,   113,   114,     5,     6,     7,     8,
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
       0,   109,   110,  1786,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,  1832,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   182,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   183,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,     0,   111,   112,     0,
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
     105,   106,     0,     0,   107,   108,     0,   109,   110,  1867,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,  1870,
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
     109,   110,     0,   111,   112,     0,   113,   114,     5,     6,
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
     107,   108,     0,   109,   110,  1887,   111,   112,     0,   113,
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
     106,     0,     0,   107,   108,     0,   109,   110,  1904,   111,
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
     110,  1961,   111,   112,     0,   113,   114,     5,     6,     7,
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
     108,     0,   109,   110,  1964,   111,   112,     0,   113,   114,
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
       0,     0,   550,     0,     0,     0,     0,     0,     0,     0,
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
      11,    12,    13,     0,     0,   817,     0,     0,     0,     0,
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
       0,     0,     0,    11,    12,    13,     0,     0,  1054,     0,
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
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,  1631,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,    61,    62,   179,
     180,    65,    66,    67,     0,    68,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     182,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     183,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   108,     0,   109,   110,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,  1778,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
      61,    62,   179,   180,    65,    66,    67,     0,    68,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   182,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   183,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,    61,    62,   179,   180,    65,    66,    67,
       0,    68,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   182,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   183,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,     0,    13,     0,     0,
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
     106,     0,     0,   107,   184,     0,   349,     0,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,  1095,     0,    10,  1096,  1097,  1098,  1099,  1100,
    1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,     0,     0,   692,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1118,    15,    16,     0,     0,     0,     0,    17,     0,    18,
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
       0,    95,   183,    97,     0,   693,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   184,     0,     0,
       0,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,     0,     0,     0,     0,     0,     0,     0,     0,
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
     184,     0,     0,   812,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,  1096,  1097,  1098,  1099,  1100,  1101,  1102,  1103,  1104,
    1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,     0,     0,  1180,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1118,    15,    16,     0,
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
       0,  1181,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   184,     0,     0,     0,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,   411,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
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
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,   451,   452,   453,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   454,   455,     0,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,     0,   479,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,   480,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,   196,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,   178,   179,   180,    65,    66,    67,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     181,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   182,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   183,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,  1193,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   184,
       0,     0,     0,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,     0,   479,     0,   232,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   480,     0,    15,    16,     0,     0,
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
       0,   107,   184,   451,   452,   453,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   454,   455,     0,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,     0,
     479,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,   480,     0,     0,    17,     0,    18,    19,    20,
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
       0,     0,     0,  1204,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   184,     0,   267,   452,   453,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,   454,   455,     0,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,     0,   479,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,   480,     0,    17,     0,
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
     270,     0,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   411,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
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
     107,   108,   451,   452,   453,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   454,   455,     0,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,     0,   479,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,   480,     0,     0,    17,     0,    18,    19,    20,    21,
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
       0,     0,  1234,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   184,   548,     0,     0,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   706,   479,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   480,     0,     0,     0,
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
       8,     9,     0,     0,     0,     0,     0,    10,  1097,  1098,
    1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,     0,
       0,     0,   751,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1118,     0,    15,    16,     0,     0,     0,     0,
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
     184,     0,     0,     0,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,     0,   479,     0,     0,   792,     0,     0,     0,     0,
       0,     0,     0,     0,   480,     0,     0,    15,    16,     0,
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
       0,     0,   107,   184,     0,     0,     0,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,  1098,  1099,  1100,  1101,  1102,  1103,
    1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,     0,     0,     0,     0,   794,     0,
       0,     0,     0,     0,     0,     0,     0,  1118,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
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
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   184,     0,     0,     0,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,     0,   479,     0,     0,
       0,  1231,     0,     0,     0,     0,     0,     0,     0,   480,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
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
     451,   452,   453,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     454,   455,     0,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,     0,   479,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,   480,
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
    1601,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   184,   451,   452,   453,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
     821,     0,    10,   454,   455,     0,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,     0,
     479,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,   480,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,   638,    39,    40,     0,   822,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,   178,   179,
     180,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   181,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     182,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     183,    97,     0,   272,   273,    99,   274,   275,   100,     0,
     276,   277,   278,   279,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   184,     0,     0,   280,   281,
     111,   112,     0,   113,   114,  1099,  1100,  1101,  1102,  1103,
    1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,     0,     0,     0,   283,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1118,     0,     0,
       0,   285,   286,   287,   288,   289,   290,   291,     0,     0,
       0,   205,     0,   206,    40,     0,     0,     0,     0,     0,
       0,     0,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,    50,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   205,   326,     0,
     739,   328,   329,   330,     0,     0,     0,   331,   578,   209,
     210,   211,   212,   213,   579,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,   272,   273,     0,   274,   275,
       0,   580,   276,   277,   278,   279,     0,    93,    94,     0,
      95,   183,    97,   336,     0,   337,     0,     0,   338,     0,
     280,   281,     0,     0,     0,   209,   210,   211,   212,   213,
       0,     0,     0,     0,     0,   107,     0,     0,     0,   740,
       0,   111,     0,     0,     0,     0,     0,   182,     0,   283,
      91,    92,     0,    93,    94,     0,    95,   183,    97,     0,
       0,     0,     0,   285,   286,   287,   288,   289,   290,   291,
       0,     0,     0,   205,     0,   206,    40,     0,     0,     0,
       0,   107,     0,     0,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,    50,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   205,
     326,     0,   327,   328,   329,   330,     0,     0,     0,   331,
     578,   209,   210,   211,   212,   213,   579,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,   272,   273,     0,
     274,   275,     0,   580,   276,   277,   278,   279,     0,    93,
      94,     0,    95,   183,    97,   336,     0,   337,     0,     0,
     338,     0,   280,   281,     0,   282,     0,   209,   210,   211,
     212,   213,     0,     0,     0,     0,     0,   107,     0,     0,
       0,   740,     0,   111,     0,     0,     0,     0,     0,     0,
       0,   283,     0,   436,     0,    93,    94,     0,    95,   183,
      97,     0,   284,     0,     0,   285,   286,   287,   288,   289,
     290,   291,     0,     0,     0,   205,     0,     0,     0,     0,
       0,     0,     0,   107,     0,     0,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,    50,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,     0,   326,     0,     0,   328,   329,   330,     0,     0,
       0,   331,   332,   209,   210,   211,   212,   213,   333,     0,
       0,     0,     0,     0,     0,     0,   205,     0,     0,     0,
       0,     0,     0,     0,     0,   334,  1063,     0,    91,   335,
       0,    93,    94,     0,    95,   183,    97,   336,    50,   337,
       0,     0,   338,   272,   273,     0,   274,   275,     0,   339,
     276,   277,   278,   279,     0,     0,     0,     0,     0,   107,
     340,     0,     0,     0,  1757,     0,     0,     0,   280,   281,
       0,   282,     0,     0,   209,   210,   211,   212,   213,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   182,   283,     0,    91,
       0,     0,    93,    94,     0,    95,   183,    97,   284,     0,
       0,   285,   286,   287,   288,   289,   290,   291,     0,     0,
       0,   205,     0,     0,     0,     0,     0,     0,     0,     0,
     107,     0,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,    50,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,     0,   326,     0,
       0,   328,   329,   330,     0,     0,     0,   331,   332,   209,
     210,   211,   212,   213,   333,     0,     0,     0,     0,     0,
       0,     0,   205,     0,   919,     0,   920,     0,     0,     0,
       0,   334,     0,     0,    91,   335,     0,    93,    94,     0,
      95,   183,    97,   336,    50,   337,     0,     0,   338,   272,
     273,     0,   274,   275,     0,   339,   276,   277,   278,   279,
       0,     0,     0,     0,     0,   107,   340,     0,     0,     0,
    1827,     0,     0,     0,   280,   281,     0,   282,     0,     0,
     209,   210,   211,   212,   213,  1100,  1101,  1102,  1103,  1104,
    1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,   283,     0,     0,     0,     0,    93,    94,
       0,    95,   183,    97,   284,     0,  1118,   285,   286,   287,
     288,   289,   290,   291,     0,     0,     0,   205,     0,     0,
       0,     0,     0,     0,     0,     0,   107,     0,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,    50,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,     0,   326,     0,   327,   328,   329,   330,
       0,     0,     0,   331,   332,   209,   210,   211,   212,   213,
     333,     0,     0,     0,     0,     0,     0,     0,   205,     0,
       0,     0,     0,     0,     0,     0,     0,   334,     0,     0,
      91,   335,     0,    93,    94,     0,    95,   183,    97,   336,
      50,   337,     0,     0,   338,   272,   273,     0,   274,   275,
       0,   339,   276,   277,   278,   279,     0,     0,     0,     0,
       0,   107,   340,     0,     0,     0,     0,     0,     0,     0,
     280,   281,     0,   282,     0,     0,   209,   210,   211,   212,
     213,     0,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   283,
     479,   358,     0,     0,    93,    94,     0,    95,   183,    97,
     284,     0,   480,   285,   286,   287,   288,   289,   290,   291,
       0,     0,     0,   205,     0,     0,     0,     0,     0,     0,
       0,     0,   107,     0,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,    50,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,     0,
     326,     0,     0,   328,   329,   330,     0,     0,     0,   331,
     332,   209,   210,   211,   212,   213,   333,     0,     0,     0,
       0,     0,     0,     0,   205,     0,     0,     0,     0,     0,
       0,     0,     0,   334,     0,     0,    91,   335,     0,    93,
      94,     0,    95,   183,    97,   336,    50,   337,     0,     0,
     338,     0,   272,   273,     0,   274,   275,   339,  1560,   276,
     277,   278,   279,     0,     0,     0,     0,   107,   340,     0,
       0,     0,     0,     0,     0,     0,     0,   280,   281,     0,
     282,     0,   209,   210,   211,   212,   213,     0,     0,  1101,
    1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,   283,   877,     0,     0,
      93,    94,     0,    95,   183,    97,     0,   284,     0,  1118,
     285,   286,   287,   288,   289,   290,   291,     0,     0,     0,
     205,     0,     0,     0,     0,     0,     0,     0,   107,     0,
       0,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,    50,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,     0,   326,     0,     0,
     328,   329,   330,     0,     0,     0,   331,   332,   209,   210,
     211,   212,   213,   333,     0,     0,     0,     0,     0,     0,
       0,   205,     0,     0,     0,     0,     0,     0,     0,     0,
     334,     0,     0,    91,   335,     0,    93,    94,     0,    95,
     183,    97,   336,    50,   337,     0,     0,   338,  1657,  1658,
    1659,  1660,  1661,     0,   339,  1662,  1663,  1664,  1665,     0,
       0,     0,     0,     0,   107,   340,     0,     0,     0,     0,
       0,     0,  1666,  1667,  1668,     0,     0,     0,     0,   209,
     210,   211,   212,   213,     0, -1058, -1058, -1058, -1058,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,  1669,   479,     0,     0,     0,    93,    94,     0,
      95,   183,    97,     0,     0,   480,  1670,  1671,  1672,  1673,
    1674,  1675,  1676,     0,     0,     0,   205,     0,     0,     0,
       0,     0,     0,     0,     0,   107,     0,  1677,  1678,  1679,
    1680,  1681,  1682,  1683,  1684,  1685,  1686,  1687,    50,  1688,
    1689,  1690,  1691,  1692,  1693,  1694,  1695,  1696,  1697,  1698,
    1699,  1700,  1701,  1702,  1703,  1704,  1705,  1706,  1707,  1708,
    1709,  1710,  1711,  1712,  1713,  1714,  1715,  1716,  1717,     0,
       0,     0,  1718,  1719,   209,   210,   211,   212,   213,     0,
    1720,  1721,  1722,  1723,  1724,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1725,  1726,  1727,     0,
     205,     0,    93,    94,     0,    95,   183,    97,  1728,     0,
    1729,  1730,     0,  1731,     0,     0,     0,     0,     0,     0,
    1732,  1733,    50,  1734,     0,  1735,  1736,     0,   272,   273,
     107,   274,   275,     0,     0,   276,   277,   278,   279,     0,
       0,     0,     0,     0,  1569,     0,     0,     0,     0,     0,
       0,     0,     0,   280,   281,     0,     0,  1570,   209,   210,
     211,   212,   213,  1571,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     182,     0,   283,    91,    92,     0,    93,    94,     0,    95,
    1573,    97,     0,     0,     0,     0,   285,   286,   287,   288,
     289,   290,   291,     0,     0,     0,   205,     0,     0,     0,
       0,     0,     0,     0,   107,     0,     0,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,    50,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,     0,   326,     0,   327,   328,   329,   330,     0,
       0,     0,   331,   578,   209,   210,   211,   212,   213,   579,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     272,   273,     0,   274,   275,     0,   580,   276,   277,   278,
     279,     0,    93,    94,     0,    95,   183,    97,   336,     0,
     337,     0,     0,   338,     0,   280,   281,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     107,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   283,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   285,   286,
     287,   288,   289,   290,   291,     0,     0,     0,   205,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
      50,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,     0,   326,     0,  1286,   328,   329,
     330,     0,     0,     0,   331,   578,   209,   210,   211,   212,
     213,   579,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   272,   273,     0,   274,   275,     0,   580,   276,
     277,   278,   279,     0,    93,    94,     0,    95,   183,    97,
     336,     0,   337,     0,     0,   338,     0,   280,   281,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   107,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   283,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     285,   286,   287,   288,   289,   290,   291,     0,     0,     0,
     205,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,    50,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,     0,   326,     0,     0,
     328,   329,   330,     0,     0,     0,   331,   578,   209,   210,
     211,   212,   213,   579,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     580,     0,     0,     0,     0,     0,    93,    94,     0,    95,
     183,    97,   336,     0,   337,     0,     0,   338,   451,   452,
     453,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   107,     0,     0,     0,   454,   455,
       0,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,     0,   479,   451,   452,   453,     0,
       0,     0,     0,     0,     0,     0,     0,   480,     0,     0,
       0,     0,     0,     0,     0,     0,   454,   455,  1421,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,     0,   479,   451,   452,   453,     0,     0,     0,
       0,     0,     0,     0,     0,   480,     0,     0,     0,     0,
       0,     0,     0,     0,   454,   455,     0,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
       0,   479,   451,   452,   453,     0,     0,     0,     0,     0,
       0,     0,     0,   480,     0,     0,     0,     0,     0,     0,
       0,     0,   454,   455,     0,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,  1602,   479,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   480,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   451,   452,   453,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   454,   455,  1422,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,     0,   479,
     451,   452,   453,     0,     0,     0,     0,     0,     0,     0,
       0,   480,     0,     0,     0,     0,     0,     0,     0,     0,
     454,   455,   564,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,     0,   479,   451,   452,
     453,     0,     0,     0,     0,     0,     0,     0,     0,   480,
       0,     0,     0,   830,     0,     0,     0,     0,   454,   455,
     566,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,     0,   479,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   480,     0,     0,
       0,     0,     0,   205,     0,     0,     0,     0,   451,   452,
     453,     0,     0,   831,     0,     0,     0,     0,     0,   282,
       0,     0,     0,     0,     0,    50,     0,     0,   454,   455,
     585,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,     0,   479,   284,     0,     0,     0,
       0,   209,   210,   211,   212,   213,     0,   480,     0,   205,
       0,     0,     0,     0,     0,     0,     0,     0,   589,     0,
       0,     0,     0,   182,     0,     0,    91,     0,     0,    93,
      94,    50,    95,   183,    97,  1294,  1263,     0,     0,  -396,
       0,     0,     0,     0,     0,     0,     0,   439,   179,   180,
      65,    66,    67,   839,   840,     0,     0,   107,     0,   841,
       0,   842,     0,     0,     0,   784,   567,   209,   210,   211,
     212,   213,   568,   843,     0,     0,     0,     0,     0,     0,
       0,    34,    35,    36,   205,     0,     0,     0,     0,   182,
       0,     0,    91,   335,   207,    93,    94,  1568,    95,   183,
      97,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,   339,     0,     0,     0,     0,     0,   440,
       0,     0,     0,   107,   340,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   809,     0,     0,     0,     0,
       0,   844,   845,   846,   847,   848,   849,   205,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,   214,
       0,     0,     0,     0,   182,    89,    90,    91,    92,    50,
      93,    94,     0,    95,   183,    97,     0,     0,     0,    99,
       0,     0,     0,     0,     0,     0,     0,     0,   850,     0,
       0,  1569,     0,   104,     0,     0,     0,     0,   107,   851,
    1048,     0,     0,     0,  1570,   209,   210,   211,   212,   213,
    1571,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   182,     0,     0,
      91,  1572,    29,    93,    94,     0,    95,  1573,    97,     0,
      34,    35,    36,   205,     0,   206,    40,     0,     0,     0,
       0,     0,     0,   207,     0,     0,     0,     0,     0,     0,
       0,   107,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   208, -1058, -1058, -1058, -1058,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1049,
      75,   209,   210,   211,   212,   213,     0,    81,    82,    83,
      84,    85,  1118,     0,     0,     0,     0,     0,   214,     0,
       0,     0,     0,   182,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   183,    97,     0,   839,   840,    99,     0,
       0,     0,   841,     0,   842,     0,     0,     0,     0,     0,
       0,     0,   104,     0,     0,     0,   843,   107,   215,     0,
       0,     0,     0,   111,    34,    35,    36,   205,     0,     0,
       0,   451,   452,   453,     0,     0,     0,   207,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,   454,   455,     0,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,     0,   479,     0,
       0,     0,     0,     0,   844,   845,   846,   847,   848,   849,
     480,    81,    82,    83,    84,    85,     0,     0,     0,  1002,
    1003,     0,   214,     0,     0,     0,     0,   182,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   183,    97,  1004,
       0,     0,    99,     0,     0,     0,     0,  1005,  1006,  1007,
     205,   850,     0,     0,     0,     0,   104,     0,     0,     0,
    1008,   107,   851,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,   525,     0,     0,
      29,     0,     0,     0,     0,     0,     0,     0,    34,    35,
      36,   205,     0,   206,    40,     0,     0,     0,     0,     0,
       0,   207,     0,     0,     0,     0,     0,  1009,  1010,  1011,
    1012,  1013,  1014,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1015,     0,     0,     0,   208,
     182,     0,     0,    91,    92,     0,    93,    94,     0,    95,
     183,    97,     0,     0,     0,     0,     0,     0,    75,   209,
     210,   211,   212,   213,  1016,    81,    82,    83,    84,    85,
       0,     0,     0,     0,   107,     0,   214,     0,     0,     0,
       0,   182,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   183,    97,    29,     0,     0,    99,     0,     0,     0,
       0,    34,    35,    36,   205,     0,   206,    40,     0,     0,
     104,     0,     0,     0,   207,   107,   215,     0,     0,   601,
       0,   111,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   208,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     621,    75,   209,   210,   211,   212,   213,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,   214,
       0,     0,     0,     0,   182,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   183,    97,    29,   991,     0,    99,
       0,     0,     0,     0,    34,    35,    36,   205,     0,   206,
      40,     0,     0,   104,     0,     0,     0,   207,   107,   215,
       0,     0,     0,     0,   111,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   208,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    75,   209,   210,   211,   212,   213,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,   214,     0,     0,     0,     0,   182,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   183,    97,    29,
       0,     0,    99,     0,     0,     0,     0,    34,    35,    36,
     205,     0,   206,    40,     0,     0,   104,     0,     0,     0,
     207,   107,   215,     0,     0,     0,     0,   111,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   208,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1146,    75,   209,   210,
     211,   212,   213,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   214,     0,     0,     0,     0,
     182,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     183,    97,    29,     0,     0,    99,     0,     0,     0,     0,
      34,    35,    36,   205,     0,   206,    40,     0,     0,   104,
       0,     0,     0,   207,   107,   215,     0,     0,     0,     0,
     111,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   208,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,   209,   210,   211,   212,   213,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,   214,     0,
       0,     0,     0,   182,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   183,    97,     0,     0,     0,    99,     0,
       0,   451,   452,   453,     0,     0,     0,     0,     0,     0,
       0,     0,   104,     0,     0,     0,     0,   107,   215,     0,
       0,   454,   455,   111,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,     0,   479,   451,
     452,   453,     0,     0,     0,     0,     0,     0,     0,     0,
     480,     0,     0,     0,     0,     0,     0,     0,     0,   454,
     455,     0,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,     0,   479,     0,     0,     0,
       0,     0,     0,     0,     0,   451,   452,   453,   480,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   454,   455,   534,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,     0,   479,   451,   452,   453,     0,     0,     0,     0,
       0,     0,     0,     0,   480,     0,     0,     0,     0,     0,
       0,     0,     0,   454,   455,   909,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,     0,
     479,     0,     0,     0,     0,     0,     0,     0,     0,   451,
     452,   453,   480,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   454,
     455,   977,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,     0,   479,   451,   452,   453,
       0,     0,     0,     0,     0,     0,     0,     0,   480,     0,
       0,     0,     0,     0,     0,     0,     0,   454,   455,  1033,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,     0,   479,     0,     0,     0,     0,     0,
       0,     0,  1092,  1093,  1094,     0,   480,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1095,     0,  1339,  1096,  1097,  1098,  1099,
    1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,     0,     0,
    1092,  1093,  1094,     0,     0,     0,     0,     0,     0,     0,
       0,  1118,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1095,     0,  1370,  1096,  1097,  1098,  1099,  1100,  1101,
    1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1092,  1093,  1094,     0,  1118,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1095,     0,  1268,  1096,
    1097,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,     0,     0,  1092,  1093,  1094,     0,     0,     0,     0,
       0,     0,     0,     0,  1118,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1095,     0,  1438,  1096,  1097,  1098,
    1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1092,  1093,
    1094,     0,  1118,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1095,
       0,  1449,  1096,  1097,  1098,  1099,  1100,  1101,  1102,  1103,
    1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,     0,     0,  1092,  1093,  1094,     0,
       0,     0,     0,     0,     0,     0,     0,  1118,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1095,     0,  1546,
    1096,  1097,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,     0,    34,    35,    36,   205,     0,   206,    40,
       0,     0,     0,     0,     0,  1118,   207,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,  1638,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   236,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   237,     0,     0,     0,     0,
       0,     0,     0,     0,   209,   210,   211,   212,   213,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,   214,  1640,     0,     0,     0,   182,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   183,    97,     0,     0,
       0,    99,     0,    34,    35,    36,   205,     0,   206,    40,
       0,     0,     0,     0,     0,   104,   652,     0,     0,     0,
     107,   238,     0,     0,     0,     0,   111,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   208,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   209,   210,   211,   212,   213,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,   214,     0,     0,     0,     0,   182,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   183,    97,     0,     0,
       0,    99,     0,    34,    35,    36,   205,     0,   206,    40,
       0,     0,     0,     0,     0,   104,   207,     0,     0,     0,
     107,   653,     0,     0,     0,     0,   111,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   236,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   209,   210,   211,   212,   213,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,   214,     0,     0,     0,     0,   182,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   183,    97,     0,     0,
       0,    99,     0,     0,     0,     0,     0,   451,   452,   453,
       0,     0,     0,     0,     0,   104,     0,     0,     0,     0,
     107,   238,     0,     0,     0,     0,   111,   454,   455,   974,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,     0,   479,   451,   452,   453,     0,     0,
       0,     0,     0,     0,     0,     0,   480,     0,     0,     0,
       0,     0,     0,     0,     0,   454,   455,     0,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,     0,   479,  1092,  1093,  1094,     0,     0,     0,     0,
       0,     0,     0,     0,   480,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1095,  1454,     0,  1096,  1097,  1098,
    1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1093,
    1094,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1118,     0,     0,     0,     0,     0,     0,  1095,
       0,     0,  1096,  1097,  1098,  1099,  1100,  1101,  1102,  1103,
    1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,   453,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1118,     0,     0,
       0,     0,   454,   455,     0,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,  1094,   479,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   480,     0,     0,     0,     0,     0,  1095,     0,     0,
    1096,  1097,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   454,   455,  1118,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,     0,
     479,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   455,   480,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,     0,   479,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   480,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,     0,   479,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   480
};

static const yytype_int16 yycheck[] =
{
       5,     6,     4,     8,     9,    10,    11,    12,    13,   129,
      15,    16,    17,    18,    56,   161,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    98,    31,   669,   186,   102,
     103,     4,     4,   108,   950,   404,    30,   666,   519,    44,
     239,   404,  1170,   404,   166,    56,   538,    52,    33,    54,
      57,   161,    57,   695,    59,   128,     4,   665,   404,   515,
     516,    46,  1331,   108,   939,    30,    51,   187,   479,    30,
       4,   356,   357,   645,    30,   748,   108,   511,   595,   596,
     820,    86,    57,   108,   970,   511,   827,  1047,   544,   796,
     550,  1157,    44,     9,    32,     9,    32,     9,    48,    48,
     986,     9,    14,   108,  1166,     9,    14,     9,     9,     9,
      14,     9,   546,     9,    83,     9,     9,    48,     4,     9,
     546,    48,     9,    70,     9,    48,   586,    38,     9,   251,
       9,     9,     9,   250,    86,     9,     4,     9,     9,   184,
       9,    70,     9,   160,    70,     9,    36,     9,  1034,    70,
      70,    70,   184,    83,    84,    38,    38,   134,   135,   184,
      70,    50,    51,   160,   115,    83,    54,   102,   111,   483,
     215,   160,    83,    70,   543,    90,   166,    83,   166,   184,
       0,   181,   199,   215,   181,    53,   191,    38,    56,   179,
     215,   160,   181,   238,    70,   129,   196,   196,   512,   196,
      83,    83,   199,   517,   786,    73,   238,   667,    60,   199,
     215,   199,    70,  1088,    70,   134,   135,    19,    20,   199,
      70,    70,   196,   200,    70,    70,   786,   178,    96,   164,
      98,    70,    83,   238,   102,   103,    88,   879,    70,    91,
     160,  1465,   157,   193,   193,    70,    70,   197,   253,   196,
     160,   256,   199,   187,    70,    70,   174,    83,   263,   264,
     128,   199,   198,   174,    83,    54,   197,   196,   197,   181,
     197,   201,   389,   199,   197,   161,   196,   165,   199,   199,
     199,   197,   989,   441,   198,   199,   198,  1247,  1557,   199,
     198,   174,   174,   182,   198,   201,   198,   198,   198,   196,
     198,  1367,   198,  1365,   198,   198,   348,   197,  1374,   257,
    1376,   198,   197,   261,     4,  1201,  1057,   198,  1059,   198,
     198,   198,    83,   174,   198,   197,   197,    83,   197,    83,
     197,   376,   200,   197,    70,   197,   160,   348,   967,  1405,
     196,   199,  1566,   199,   376,   160,   196,   196,   174,   199,
    1225,   376,   812,   199,   199,   174,   928,   817,   875,   876,
     199,    90,   435,   521,   160,   102,  1590,   199,  1592,   102,
     375,   376,   196,   493,   199,   199,   165,   382,   383,   384,
     385,   386,   387,   199,   199,  1771,   160,   392,    81,   257,
     432,   180,    14,   261,   106,   107,    38,   265,   134,   135,
     196,   120,   196,   157,   158,   159,   411,   181,   106,   107,
      32,   130,     8,   174,   419,   488,   489,   490,   491,   494,
      70,   432,  1004,   375,   905,   199,   431,   164,   157,    51,
     232,   164,   384,   385,   196,   387,   192,   130,  1500,   196,
    1502,    83,  1828,   199,  1004,  1511,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   485,   480,   102,   482,   483,   484,
     348,   196,  1155,    38,   418,   479,   196,   494,   200,   494,
     495,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   200,  1243,   356,   357,   358,   512,   513,   196,
     515,   516,   517,   518,   479,    83,    84,   522,   479,   494,
     525,   532,   956,   479,    14,     4,    83,   983,    83,   534,
     956,   536,   418,    90,    70,    83,   181,   683,   164,   544,
      19,    20,   485,   742,   396,   552,   164,   552,  1610,   554,
     418,   196,  1614,   134,   135,   672,  1185,   674,   492,  1188,
     428,  1169,   181,   365,   432,   196,    31,   435,   511,    57,
     939,  1446,   374,   683,   376,   774,   939,   196,   939,   381,
     991,    69,   543,   160,   869,    50,   871,   164,    53,   532,
     181,   393,   750,   939,  1054,   196,   601,   121,   134,   135,
     543,   158,   159,   546,   181,   196,   130,   196,   653,   557,
     158,   159,   102,   103,  1131,   196,   196,   485,   486,   487,
     488,   489,   490,   491,   160,   134,   135,    31,   701,   108,
     385,   199,   387,   201,    70,  1217,  1218,  1219,  1220,  1221,
    1222,   181,   164,   511,    70,  1227,    50,   204,   653,    53,
      53,    54,    55,   852,  1529,    70,   196,  1217,  1218,   165,
     196,  1221,    50,    51,   532,   864,    69,  1227,  1085,  1938,
    1087,    32,    83,   790,   791,   797,   198,   199,   546,    90,
     797,   798,   111,    53,    54,    55,    70,    57,   693,   557,
     119,   120,   121,   122,   123,   124,   105,   106,   107,    69,
      83,   706,  1375,    70,  1220,   184,  1222,    90,  1770,   577,
     196,  1203,  1774,    75,    76,    83,   196,   519,    56,  1088,
     410,    38,    90,    75,    76,  1088,   198,  1088,   102,   103,
      83,   599,   600,   132,   133,   740,   215,    90,  1194,   198,
    1369,   198,  1088,   595,   596,   198,  1354,   158,   159,  1205,
     198,    83,    81,   232,    83,    84,   198,   199,    90,   238,
     198,   199,   191,   198,   769,  1246,   634,   635,   119,   120,
     121,   122,   123,   124,   103,   158,   159,    83,   257,   105,
     106,   107,   261,    70,    90,   122,   123,   124,  1916,   157,
     158,   159,   356,   357,   205,  1802,  1803,   683,   198,   804,
     198,  1383,  1930,  1385,   157,   158,   159,  1480,  1798,  1799,
     139,   140,   141,   142,   143,   820,    70,   119,   120,   121,
     122,   123,   124,  1383,    70,  1385,   158,   159,   130,   131,
      70,   816,    70,   701,   837,   838,   165,    70,   167,   168,
     191,   170,   171,   172,   112,   113,   114,   198,  1910,    70,
     199,   160,   158,   159,  1335,   196,  1225,   196,    70,   160,
     196,  1321,  1225,  1925,  1225,   390,   195,   164,   198,   394,
     199,   173,   201,    49,    69,   160,   810,   825,   181,  1225,
     196,   196,   203,  1512,     9,   160,   365,   160,   196,   191,
     692,     8,   196,   198,  1567,   374,   421,   376,   423,   424,
     425,   426,   381,   160,   909,    14,   911,   160,   913,   198,
     198,     9,    14,   199,   393,  1371,   198,   990,   786,   924,
     788,   130,   130,   197,   810,   927,   181,    14,   102,   111,
     197,   942,   866,   938,   197,   197,  1518,   197,  1520,   418,
    1522,   202,   810,     9,   282,  1527,   284,  1407,   196,   751,
     157,   196,     6,   196,   927,   927,   824,   825,  1518,   964,
    1520,   197,  1522,  1423,    94,   197,  1882,  1527,   197,   974,
     197,     9,   977,   198,   979,    14,   666,  1176,   983,   927,
     866,   181,   196,     9,   199,   196,  1902,    83,   198,   132,
     792,   197,   794,   927,    48,  1911,   199,   991,   866,   942,
     198,   197,   340,   197,   196,   873,   874,   198,   197,     9,
     203,   954,     9,   956,   948,   203,   203,   869,   203,   871,
     822,    70,   203,   875,   876,   877,   991,    32,  1033,   133,
     991,   180,   160,   136,   902,   991,     9,   197,   160,  1621,
     519,   927,  1498,   119,   120,   121,   122,   123,   124,    14,
       9,   193,     9,   182,     9,  1040,   197,   132,   112,   927,
      14,  1621,   948,   117,   203,   119,   120,   121,   122,   123,
     124,   125,  1532,  1148,   942,     9,   203,  1446,   557,  1090,
     948,  1541,   200,  1446,    14,  1446,   954,   889,   956,   203,
     197,   429,  1035,  1041,   432,  1555,   197,   203,   160,   196,
    1446,   102,   197,   905,   906,   198,   198,     9,   136,  1043,
     160,  1045,     6,   167,   168,   191,   170,   197,     9,   196,
      70,    70,   990,    50,    51,    52,    53,    54,    55,    70,
      57,    70,    70,   196,  1002,  1003,  1004,   191,   199,     9,
     200,  1148,    69,  1148,     9,    14,   200,   198,   182,   193,
      14,   199,  1154,   199,    48,    14,   203,  1043,   198,  1045,
    1529,   197,    14,   196,    32,   196,  1529,  1035,  1529,    32,
      14,  1631,   196,  1041,   196,  1043,  1181,  1045,    52,   196,
    1822,  1154,  1154,  1529,    70,    70,    78,    79,    80,  1194,
      70,    70,    70,   196,  1776,  1777,   160,     9,  1066,    91,
    1205,  1206,   197,   136,   198,   198,  1154,   196,    14,   160,
     182,   136,     9,   692,   197,    69,  1776,  1777,   112,   203,
    1154,     9,  1090,   117,   200,   119,   120,   121,   122,   123,
     124,   125,    83,   571,   200,   198,     9,   927,  1243,   136,
     196,    83,    14,   196,     9,  1047,  1048,   199,  1253,   197,
    1235,  1119,   198,   145,   146,   147,   148,   149,  1900,   196,
     950,   196,    91,   199,   156,   197,    32,   198,  1154,   136,
     162,   163,   751,   167,   168,   199,   170,   967,   199,  1131,
     203,   157,    77,   198,   176,   197,  1154,   198,   136,   182,
    1238,    32,   197,   197,  1936,   203,     9,   191,   190,     9,
      50,    51,    52,    53,    54,    55,   200,   203,  1768,   136,
       9,     9,   200,   792,   203,   794,  1250,   198,  1778,    69,
     203,   203,   197,   200,   662,   663,   197,   199,    14,    83,
     203,   810,   198,   671,  1339,  1481,   196,   198,     9,   197,
     197,  1346,   197,   822,   197,  1350,   825,  1352,   197,  1217,
    1218,  1219,  1220,  1221,  1222,  1360,  1476,   196,   199,  1227,
     136,     9,   136,  1823,  1250,  1370,  1371,   203,     9,   203,
    1238,   203,   203,   197,    32,   198,   197,   197,  1180,   198,
     198,   136,  1250,   199,   112,   169,   165,   866,   198,    14,
      83,   117,  1260,   197,   197,     6,   136,   199,   197,   136,
     235,   119,   120,   121,   122,   123,   124,    14,  1868,   199,
     889,   181,   130,   131,    83,   198,    14,    14,    83,   197,
     197,   196,   136,    14,   136,    14,   905,   906,    14,  1231,
    1588,   198,     6,     9,   198,   198,     9,    48,   199,    68,
     200,    83,   181,   196,  1246,  1247,    83,  1333,   927,     9,
     199,    19,    20,   171,   198,   173,   115,  1343,   102,  1327,
     160,   182,    30,   102,  1154,   172,    36,    14,   186,   948,
     188,   196,   196,   191,    48,  1935,   198,   197,   182,   182,
    1940,   178,    83,    19,    20,   175,   197,  1560,    56,     9,
    1495,    83,   198,  1498,   199,  1185,   197,    14,  1188,   837,
     838,   112,   197,   195,    83,     9,   117,    14,   119,   120,
     121,   122,   123,   124,   125,  1383,    83,  1385,    14,    83,
      14,    83,  1456,  1131,  1891,   486,   984,  1907,   488,   491,
    1244,  1630,  1902,  1335,   930,   603,  1420,  1617,   112,  1655,
    1568,  1475,  1740,   117,  1947,   119,   120,   121,   122,   123,
     124,   125,  1563,  1613,  1471,  1923,   167,   168,  1752,   170,
    1467,   386,  1041,  1086,  1043,  1221,  1045,  1082,  1047,  1048,
    1456,  1158,  1003,  1216,    56,  1030,  1217,   954,   837,  1934,
     191,   382,   432,  1949,  1857,  1139,  1459,  1119,  1456,   200,
    1538,   929,  1067,   167,   168,  1481,   170,  1465,  1756,  1533,
    1605,    -1,    -1,  1471,    -1,    -1,    -1,   945,  1542,    -1,
      -1,    -1,    -1,    -1,    -1,  1501,    -1,   191,    -1,    -1,
     958,  1507,    -1,  1509,    -1,    -1,   200,    -1,    -1,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,  1333,    -1,  1531,    -1,  1533,    -1,   987,
    1518,    -1,  1520,  1343,  1522,    69,  1542,    -1,    -1,  1527,
      -1,    -1,  1596,    -1,   232,  1533,    -1,    -1,    -1,    -1,
    1538,    -1,    -1,    -1,  1542,  1154,    -1,    -1,    -1,  1369,
      -1,  1629,  1630,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1560,   528,  1628,  1563,   232,    -1,  1566,    -1,
    1634,  1180,    -1,    -1,    -1,    -1,    -1,  1641,  1576,  1751,
      -1,    -1,    -1,    -1,   282,  1583,   284,    -1,    -1,    -1,
      -1,    -1,  1590,  1061,  1592,    -1,    -1,  1065,    -1,  1875,
    1598,    -1,    -1,  1619,    -1,    -1,    -1,    -1,    -1,    -1,
    1751,    -1,  1628,  1816,    -1,    -1,    -1,    -1,  1634,    -1,
      -1,    -1,  1231,  1621,    -1,  1641,  1761,    -1,    -1,  1238,
    1628,  1629,  1630,    -1,    -1,    -1,  1634,  1246,  1247,    -1,
      -1,  1250,   340,  1641,    -1,  1895,    -1,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,   627,   628,    -1,    -1,    -1,   365,    -1,    -1,
     282,    -1,   284,    -1,    -1,    -1,   374,    -1,    -1,    -1,
      -1,  1501,    -1,   381,    -1,    -1,    -1,  1507,    -1,  1509,
     655,    -1,  1512,    59,    60,   393,    -1,    -1,    -1,   365,
    1168,    -1,  1170,    -1,    -1,    -1,   404,    -1,   374,    -1,
      -1,  1531,    -1,    -1,    -1,   381,    -1,    -1,  1782,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1335,   393,   340,  1197,
      -1,   429,  1200,    -1,   432,    -1,    -1,    -1,   404,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1751,    -1,    -1,    -1,    -1,    -1,    -1,
    1824,    -1,    -1,    -1,    -1,    -1,  1782,  1831,   134,   135,
      -1,    -1,    -1,   738,  1772,    -1,    -1,    -1,  1776,  1777,
      -1,   479,    -1,    -1,  1782,    -1,    -1,     6,    -1,    -1,
      -1,  1259,    -1,  1791,    -1,    -1,    -1,  1265,    -1,  1619,
    1798,  1799,  1866,    -1,  1802,  1803,    -1,    -1,  1824,    -1,
    1945,    -1,    -1,    -1,    -1,  1831,    -1,   429,  1816,    -1,
     432,   519,  1957,    -1,  1888,    -1,  1824,    -1,    -1,    48,
      -1,   197,  1967,  1831,    -1,  1970,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1456,    -1,    -1,
    1866,    -1,    -1,   519,    -1,    -1,    -1,    -1,    -1,  1875,
    1328,  1329,    -1,    -1,   829,    -1,    -1,    -1,  1866,    -1,
     835,    -1,  1888,   571,    -1,   573,  1874,    -1,   576,    -1,
      -1,    -1,    -1,    -1,  1948,     6,    -1,    -1,    -1,  1953,
    1888,    -1,    -1,   112,    -1,    -1,  1894,    -1,   117,    -1,
     119,   120,   121,   122,   123,   124,   125,   605,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,    -1,  1533,     6,    -1,    48,    -1,  1538,
      -1,    -1,  1948,  1542,    -1,    -1,   901,  1953,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   167,   168,
    1948,   170,    -1,    59,    60,  1953,    -1,    -1,    -1,   571,
    1428,   573,  1430,    -1,   662,   663,    -1,    48,    -1,    -1,
      -1,    -1,   191,   671,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   200,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,    -1,    -1,   692,    -1,   117,    -1,   119,   120,
     121,   122,   123,   124,   125,    -1,    -1,    -1,  1476,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1628,
    1629,  1630,    -1,    -1,    -1,  1634,   692,    -1,   134,   135,
      -1,   112,  1641,    -1,    -1,    -1,   117,    -1,   119,   120,
     121,   122,   123,   124,   125,    -1,   167,   168,    -1,   170,
     662,   663,    -1,   751,    -1,    -1,    -1,    -1,    -1,   671,
      -1,    -1,  1882,  1028,    -1,    -1,    -1,    -1,    -1,    -1,
     191,    -1,    -1,    -1,    -1,  1543,    81,    -1,    -1,   200,
      -1,    -1,  1902,    -1,    -1,   751,   167,   168,   786,   170,
      -1,  1911,    -1,    -1,   792,    -1,   794,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,   112,    -1,  1074,
     191,    -1,    -1,    -1,    -1,    -1,  1081,    -1,    -1,   200,
      -1,    -1,    19,    20,   822,   823,   792,    -1,   794,    -1,
      -1,    -1,   830,    -1,   139,   140,   141,   142,   143,   837,
     838,   839,   840,   841,   842,   843,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   851,    -1,    -1,   822,    -1,    -1,   164,
      -1,    -1,   167,   168,    -1,   170,   171,   172,    -1,   867,
      -1,    -1,    -1,  1782,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1653,    -1,    -1,    -1,    -1,
     195,   889,    -1,    -1,    81,    -1,    83,    84,   103,    -1,
      -1,    -1,    -1,    -1,    -1,   903,    -1,   905,   906,    -1,
      -1,    -1,    -1,    -1,    -1,  1824,   103,    -1,   830,    -1,
      -1,    -1,  1831,   889,    -1,   837,   838,    -1,    -1,    -1,
      -1,   929,   930,    -1,   139,   140,   141,   142,   143,   905,
     906,   939,    -1,    -1,    -1,    -1,    -1,   945,    -1,    -1,
    1215,    -1,   139,   140,   141,   142,   143,  1866,    -1,   164,
     958,    -1,   167,   168,    -1,   170,   171,   172,   966,    81,
      -1,   969,    -1,   939,    -1,    -1,    -1,    -1,   165,  1888,
     167,   168,    -1,   170,   171,   172,    -1,  1755,    -1,   987,
     195,   103,    -1,   991,   199,    -1,    -1,    -1,    -1,   111,
     112,    -1,    -1,    -1,    -1,    -1,  1004,    -1,   195,  1274,
      -1,    -1,   199,  1278,   201,    19,    20,   929,  1283,    -1,
      -1,    -1,    -1,    -1,    -1,  1290,    30,   139,   140,   141,
     142,   143,    -1,   945,    -1,   232,    -1,    -1,    -1,  1948,
      -1,    -1,    -1,    -1,  1953,    -1,   958,    -1,    -1,  1047,
    1048,    -1,   164,    -1,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,  1061,    -1,    -1,    -1,  1065,    -1,  1067,
      -1,    -1,  1840,    -1,    -1,   987,    -1,    -1,    -1,    -1,
      -1,  1047,  1048,   195,  1082,  1083,  1084,  1085,  1086,  1087,
    1088,    -1,    -1,  1091,  1092,  1093,  1094,  1095,  1096,  1097,
    1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,    -1,  1088,    -1,    -1,  1390,    -1,    -1,    -1,  1394,
      -1,    -1,    -1,    -1,    -1,    -1,  1401,  1135,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1916,  1061,
      -1,    -1,    -1,  1065,    -1,  1067,    -1,    -1,    -1,    -1,
    1928,    -1,  1930,    -1,    -1,    -1,    -1,    -1,   365,    -1,
    1168,    -1,  1170,    -1,    -1,    -1,    -1,   374,    -1,    -1,
      -1,  1949,  1180,  1951,   381,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   393,    -1,    -1,  1197,
      -1,    -1,  1200,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1180,    -1,    -1,    -1,    -1,  1217,
    1218,  1219,  1220,  1221,  1222,    -1,    -1,  1225,   232,  1227,
      -1,    -1,    -1,  1231,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1246,  1247,
      -1,  1249,    -1,    -1,    -1,    -1,  1168,    -1,  1170,  1225,
      -1,  1259,    -1,    -1,    -1,  1231,    -1,  1265,    -1,    -1,
    1268,    -1,  1270,    -1,    10,    11,    12,    -1,    -1,    -1,
    1246,  1247,    31,    -1,    -1,  1197,    -1,    -1,  1200,    -1,
      -1,    -1,    -1,    -1,    30,    31,  1294,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,   519,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1328,  1329,    81,    69,  1332,    -1,    -1,  1335,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    -1,    -1,  1259,    -1,    -1,
      -1,    -1,    -1,  1265,   103,    -1,    -1,    -1,    -1,    -1,
      -1,   365,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1335,
     374,    -1,    -1,    -1,    -1,    -1,    -1,   381,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1383,    -1,  1385,    -1,   393,
     139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,
     404,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   161,    -1,    -1,   164,  1328,  1329,   167,   168,
      -1,   170,   171,   172,    -1,   174,    -1,    -1,    -1,    -1,
    1428,    -1,  1430,    -1,    -1,    -1,    -1,    -1,  1436,    -1,
    1438,    -1,  1440,    -1,    -1,    -1,   195,  1445,  1446,    -1,
      -1,  1449,    -1,  1451,    -1,    -1,  1454,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   200,    -1,    -1,  1465,  1466,    -1,
      -1,  1469,    -1,    -1,    -1,   479,    -1,    -1,  1476,    -1,
    1446,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   692,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,    -1,    -1,    -1,   519,  1428,    -1,  1430,    -1,
    1518,    -1,  1520,    -1,  1522,    -1,    -1,  1792,  1793,  1527,
      -1,  1529,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    59,    60,    -1,    -1,  1543,    -1,    -1,  1546,    -1,
      -1,    -1,    -1,  1465,   751,    -1,    -1,    -1,    -1,    -1,
    1558,  1559,    -1,  1529,  1476,    -1,    -1,    -1,  1566,    -1,
    1568,    -1,   576,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,  1590,    -1,  1592,   792,    -1,   794,    -1,    -1,
    1598,   605,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,    -1,
      -1,    59,    60,  1621,    -1,   822,    -1,    -1,    -1,    -1,
     576,  1543,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1638,  1639,  1640,    -1,    -1,    -1,    -1,  1645,    -1,  1647,
      -1,    -1,    -1,    -1,  1566,  1653,    -1,  1655,    -1,   605,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,    -1,    -1,    -1,  1590,   197,
    1592,    -1,    -1,    -1,    -1,    -1,  1598,    -1,   692,    -1,
      -1,    -1,   889,    10,    11,    12,   134,   135,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    59,    60,    -1,   905,   906,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,  1653,    81,    -1,    -1,    -1,    -1,   751,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,  1755,    -1,   197,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1772,    -1,    -1,    -1,  1776,  1777,
     134,   135,   786,    -1,    -1,    -1,    -1,    -1,   792,    -1,
     794,    -1,    -1,  1791,    -1,    -1,    -1,    -1,    -1,  1797,
     139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,
    1808,    -1,    -1,    -1,    -1,    -1,  1814,    -1,   822,   823,
    1818,    -1,   161,    -1,    -1,   164,    -1,    -1,   167,   168,
      -1,   170,   171,   172,    31,   839,   840,   841,   842,   843,
      -1,    -1,  1840,  1755,    -1,    -1,    -1,   851,    -1,    -1,
    1047,  1048,    -1,    -1,    -1,    -1,   195,    -1,    -1,    -1,
    1772,   200,    -1,   867,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    -1,    -1,    -1,    -1,   823,    -1,  1791,
      -1,    -1,  1880,    -1,    81,   889,   203,    -1,    -1,    -1,
      -1,    -1,  1890,   839,   840,   841,   842,   843,    -1,   903,
      -1,   905,   906,    -1,    -1,   851,   103,    -1,    -1,  1907,
      19,    20,    -1,    -1,   111,    -1,    -1,    -1,  1916,    -1,
      -1,    30,    -1,    -1,    -1,    -1,   930,    -1,  1840,    -1,
    1928,    -1,  1930,    -1,    -1,   939,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
      -1,  1949,    -1,  1951,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   966,    -1,   161,   969,    -1,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1180,    -1,    -1,    -1,   991,   185,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,   196,
    1004,    -1,    -1,    -1,  1916,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1928,    -1,  1930,    -1,
     966,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,  1231,    -1,    -1,  1949,    -1,  1951,
      -1,    -1,    -1,  1047,  1048,    -1,    -1,    -1,    31,  1246,
    1247,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,  1082,  1083,
    1084,  1085,  1086,  1087,  1088,    -1,    69,  1091,  1092,  1093,
    1094,  1095,  1096,  1097,  1098,  1099,  1100,  1101,  1102,  1103,
    1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,    -1,    78,    79,    80,    81,
      -1,    -1,    -1,   232,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1135,    -1,    -1,    -1,    -1,  1082,  1083,  1335,    -1,
    1086,   103,    -1,    -1,    -1,  1091,  1092,  1093,  1094,  1095,
    1096,  1097,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,    -1,    -1,    -1,  1180,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1135,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,  1217,  1218,  1219,  1220,  1221,  1222,    -1,
      -1,  1225,    -1,  1227,    -1,    -1,    -1,  1231,    -1,    -1,
      -1,    -1,    -1,   195,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    -1,  1246,  1247,    -1,  1249,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,   365,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1268,   374,  1270,    -1,    -1,    -1,
      -1,    -1,   381,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   393,    -1,    -1,    -1,    -1,    -1,
    1294,    -1,    -1,    -1,    -1,   404,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1249,    -1,    -1,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
      -1,    -1,  1268,    -1,  1270,    -1,    -1,    -1,  1332,    -1,
      -1,  1335,   161,    -1,    -1,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,   174,    -1,    -1,  1294,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,   195,   196,    -1,    -1,
     479,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1383,
      -1,  1385,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
     519,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      -1,    -1,  1436,    -1,  1438,    -1,  1440,    -1,    31,    -1,
      -1,  1445,  1446,    -1,    -1,  1449,    -1,  1451,    -1,    -1,
    1454,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1466,    68,    -1,  1469,    -1,   576,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,
    1436,    -1,  1438,    -1,  1440,    -1,   605,    -1,   103,  1445,
      -1,    -1,    -1,  1449,    -1,  1451,    -1,    -1,  1454,    -1,
     103,    -1,    -1,    -1,  1518,    -1,  1520,    -1,  1522,    -1,
      -1,    -1,    -1,  1527,    -1,  1529,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
      -1,    -1,  1546,    -1,    -1,   138,   139,   140,   141,   142,
     143,   144,   200,    -1,  1558,  1559,   161,    -1,    -1,   164,
     165,    -1,   167,   168,  1568,   170,   171,   172,   161,   174,
      -1,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
     185,    -1,    -1,   692,    -1,    -1,    -1,    -1,    -1,    -1,
     195,   196,   185,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1546,    -1,   195,   196,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1621,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,  1638,  1639,  1640,    -1,    -1,    -1,
      -1,  1645,   751,  1647,    -1,    -1,    -1,    -1,    -1,    30,
      31,  1655,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,   792,    -1,   794,    -1,    -1,    69,    -1,
      -1,    -1,  1638,  1639,  1640,    -1,    -1,    -1,    -1,  1645,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   822,   823,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     839,   840,   841,   842,   843,    -1,    -1,    -1,    -1,    -1,
      30,    31,   851,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,  1776,  1777,    -1,    -1,    -1,    -1,    -1,    69,
     889,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1797,    -1,    -1,   905,   906,    -1,    -1,
      -1,    -1,    -1,    -1,  1808,    -1,   103,    -1,    -1,    -1,
    1814,    -1,    -1,    -1,  1818,    -1,    -1,    -1,    -1,   200,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     939,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,
      -1,  1797,    -1,    -1,    -1,    -1,    -1,   966,    -1,    -1,
      -1,    -1,  1808,    -1,    -1,    -1,    -1,    -1,  1814,    -1,
     167,   168,  1818,   170,   171,   172,  1880,    -1,    -1,    -1,
      -1,    -1,   991,    -1,    -1,    -1,  1890,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,   196,
      -1,    -1,    -1,  1907,    -1,    -1,    -1,    -1,    30,    31,
     200,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,  1880,    57,    -1,    -1,  1047,  1048,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
     103,    -1,  1091,  1092,  1093,  1094,  1095,  1096,  1097,  1098,
    1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,    -1,    -1,    -1,    -1,    -1,  1135,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,   165,    -1,   167,   168,   169,   170,   171,   172,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,
      -1,  1180,   195,   196,    -1,    -1,   198,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,  1225,    -1,    -1,    91,
      -1,    -1,  1231,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,  1246,  1247,   111,
    1249,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,
     122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,  1268,
      -1,  1270,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,   156,  1294,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,
      -1,    -1,    -1,   185,    10,    11,    12,    -1,   190,   191,
     192,    -1,    -1,   195,   196,    -1,  1335,    -1,    -1,   201,
     202,    -1,   204,   205,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,  1436,    -1,  1438,
      -1,  1440,    27,    28,    29,    -1,  1445,  1446,    -1,    -1,
    1449,    -1,  1451,    -1,    -1,  1454,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,    94,
      -1,    96,    -1,    98,   200,   100,    -1,    -1,   103,   104,
      -1,    -1,    -1,   108,   109,   110,   111,   112,   113,   114,
    1529,   116,   117,   118,   119,   120,   121,   122,   123,   124,
      -1,   126,   127,   128,   129,   130,   131,  1546,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,   197,
     145,   146,   147,   148,   149,    -1,    -1,    81,   153,    -1,
      -1,   156,    -1,    81,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,   173,   103,
      -1,   176,    -1,    -1,   179,   103,    -1,    -1,    -1,    -1,
     185,   186,    -1,   188,    -1,   190,   191,   192,    -1,    -1,
     195,   196,    -1,   198,   199,   200,   201,   202,    -1,   204,
     205,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
      -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,  1638,
    1639,  1640,    -1,    -1,    -1,    -1,  1645,    -1,    -1,    -1,
      -1,    -1,    -1,   167,   168,  1654,   170,   171,   172,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,   195,   196,    -1,    -1,    -1,    -1,   195,   196,    -1,
      -1,    -1,    27,    28,    29,    -1,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    69,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,    94,
      -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,    -1,   108,   109,   110,   111,   112,   113,   114,
      -1,   116,   117,   118,   119,   120,   121,   122,   123,   124,
      -1,   126,   127,   128,   129,   130,   131,    -1,  1797,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,  1808,
     145,   146,   147,   148,   149,  1814,    -1,    -1,   153,  1818,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,   173,    -1,
      -1,   176,    -1,  1842,   179,    -1,    -1,    -1,    -1,    -1,
     185,   186,    -1,   188,    -1,   190,   191,   192,    -1,    -1,
     195,   196,    -1,   198,   199,   200,   201,   202,    -1,   204,
     205,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,  1880,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,
      91,    92,    93,    94,    -1,    96,    -1,    98,    -1,   100,
      -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,   112,   113,   114,    -1,   116,   117,   118,   119,   120,
     121,   122,   123,   124,    -1,   126,   127,   128,   129,   130,
     131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,   153,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,   173,    -1,    -1,   176,    -1,    -1,   179,    -1,
      -1,    -1,    -1,    -1,   185,   186,    -1,   188,    -1,   190,
     191,   192,    -1,    -1,   195,   196,    -1,   198,   199,    -1,
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
     192,    -1,    -1,   195,   196,    -1,   198,   199,   200,   201,
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
      86,    87,    -1,    -1,    -1,    91,    92,    93,    94,    95,
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
     196,    -1,   198,   199,    -1,   201,   202,    -1,   204,   205,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,
      93,    94,    -1,    96,    -1,    98,    -1,   100,   101,    -1,
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
      94,    -1,    96,    -1,    98,    99,   100,    -1,    -1,   103,
     104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,
     114,    -1,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,    -1,   126,   127,   128,    -1,   130,   131,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,   153,
      -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,   173,
      -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,
      -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,
      -1,   195,   196,    -1,   198,   199,    -1,   201,   202,    -1,
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
      -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,    97,
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
     198,   199,    -1,   201,   202,    -1,   204,   205,     3,     4,
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
     192,    -1,    -1,   195,   196,    -1,   198,   199,   200,   201,
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
      93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,
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
      -1,    -1,    -1,    27,    28,    29,    -1,    -1,    32,    -1,
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
      -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,
      91,    92,    93,    94,    -1,    96,    -1,    98,    -1,   100,
      -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,    -1,   113,   114,    -1,   116,    -1,   118,   119,   120,
     121,   122,   123,   124,    -1,   126,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,   153,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    -1,    -1,   176,    -1,    -1,   179,    -1,
      -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,
     191,   192,    -1,    -1,   195,   196,    -1,   198,   199,    -1,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    87,
      -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,
      98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,
     108,   109,   110,   111,    -1,   113,   114,    -1,   116,    -1,
     118,   119,   120,   121,   122,   123,   124,    -1,   126,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,   153,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,
      -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,
      -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,
     198,   199,    -1,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    87,    -1,    -1,    -1,    91,    92,    93,    94,
      -1,    96,    -1,    98,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,   114,
      -1,   116,    -1,   118,   119,   120,   121,   122,   123,   124,
      -1,   126,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,   153,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,
     185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,
     195,   196,    -1,   198,   199,    -1,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    -1,    -1,
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
     192,    -1,    -1,   195,   196,    -1,   198,    -1,    -1,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    31,    -1,    13,    34,    35,    36,    37,    38,
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
      -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     196,    -1,    -1,   199,    -1,   201,   202,    -1,   204,   205,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    50,    51,    -1,
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
      -1,   174,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,
      -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,
      -1,    -1,   195,   196,    -1,    -1,    -1,    -1,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
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
      -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,
     190,   191,   192,    -1,    -1,   195,   196,    10,    11,    12,
      -1,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    69,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,   108,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,   120,   121,   122,   123,   124,    -1,    -1,
     127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,
      -1,    -1,   179,    -1,    -1,    -1,    -1,   200,   185,    -1,
      -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,
      -1,    -1,    -1,    -1,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    50,    51,    -1,    -1,
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
      -1,    -1,    -1,   200,   185,    -1,    -1,    -1,    -1,   190,
     191,   192,    -1,    -1,   195,   196,    -1,   198,    11,    12,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    69,    -1,    56,    -1,
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
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
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
     195,   196,    10,    11,    12,    -1,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    69,    -1,    -1,    56,    -1,    58,    59,    60,    61,
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
      -1,    -1,   200,   185,    -1,    -1,    -1,    -1,   190,   191,
     192,    -1,    -1,   195,   196,   197,    -1,    -1,    -1,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    32,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
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
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    50,    51,    -1,    -1,    -1,    -1,
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
      13,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    50,    51,    -1,
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
      -1,    -1,   195,   196,    -1,    -1,    -1,    -1,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
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
      -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,
     190,   191,   192,    -1,    -1,   195,   196,    -1,    -1,    -1,
      -1,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
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
      10,    11,    12,    -1,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    69,
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
     200,   185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,
      -1,   195,   196,    10,    11,    12,    -1,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      27,    -1,    13,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    69,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,   102,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,     3,     4,   176,     6,     7,   179,    -1,
      10,    11,    12,    13,   185,    -1,    -1,    -1,    -1,   190,
     191,   192,    -1,    -1,   195,   196,    -1,    -1,    28,    29,
     201,   202,    -1,   204,   205,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    81,   128,    -1,
     130,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,   161,    10,    11,    12,    13,    -1,   167,   168,    -1,
     170,   171,   172,   173,    -1,   175,    -1,    -1,   178,    -1,
      28,    29,    -1,    -1,    -1,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,    -1,   199,
      -1,   201,    -1,    -1,    -1,    -1,    -1,   161,    -1,    57,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    -1,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    83,    84,    -1,    -1,    -1,
      -1,   195,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,    81,
     128,    -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,   161,    10,    11,    12,    13,    -1,   167,
     168,    -1,   170,   171,   172,   173,    -1,   175,    -1,    -1,
     178,    -1,    28,    29,    -1,    31,    -1,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,
      -1,   199,    -1,   201,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    68,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   195,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,   128,    -1,    -1,   131,   132,   133,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,   144,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   161,    91,    -1,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,   173,   103,   175,
      -1,    -1,   178,     3,     4,    -1,     6,     7,    -1,   185,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   195,
     196,    -1,    -1,    -1,   200,    -1,    -1,    -1,    28,    29,
      -1,    31,    -1,    -1,   139,   140,   141,   142,   143,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   161,    57,    -1,   164,
      -1,    -1,   167,   168,    -1,   170,   171,   172,    68,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     195,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,    -1,
      -1,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    83,    -1,    85,    -1,    -1,    -1,
      -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,   173,   103,   175,    -1,    -1,   178,     3,
       4,    -1,     6,     7,    -1,   185,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,    -1,
     200,    -1,    -1,    -1,    28,    29,    -1,    31,    -1,    -1,
     139,   140,   141,   142,   143,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    57,    -1,    -1,    -1,    -1,   167,   168,
      -1,   170,   171,   172,    68,    -1,    69,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,   128,    -1,   130,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,   173,
     103,   175,    -1,    -1,   178,     3,     4,    -1,     6,     7,
      -1,   185,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,   195,   196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    29,    -1,    31,    -1,    -1,   139,   140,   141,   142,
     143,    -1,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    57,
      57,   164,    -1,    -1,   167,   168,    -1,   170,   171,   172,
      68,    -1,    69,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   195,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,    -1,    -1,   131,   132,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   161,    -1,    -1,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,   173,   103,   175,    -1,    -1,
     178,    -1,     3,     4,    -1,     6,     7,   185,   186,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,   195,   196,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    29,    -1,
      31,    -1,   139,   140,   141,   142,   143,    -1,    -1,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    57,   164,    -1,    -1,
     167,   168,    -1,   170,   171,   172,    -1,    68,    -1,    69,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,    -1,    -1,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,   173,   103,   175,    -1,    -1,   178,     3,     4,
       5,     6,     7,    -1,   185,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,    -1,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    57,    57,    -1,    -1,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    69,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   195,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   161,   162,   163,    -1,
      81,    -1,   167,   168,    -1,   170,   171,   172,   173,    -1,
     175,   176,    -1,   178,    -1,    -1,    -1,    -1,    -1,    -1,
     185,   186,   103,   188,    -1,   190,   191,    -1,     3,     4,
     195,     6,     7,    -1,    -1,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,   125,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    29,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     161,    -1,    57,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   195,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,    -1,   130,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,   144,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       3,     4,    -1,     6,     7,    -1,   161,    10,    11,    12,
      13,    -1,   167,   168,    -1,   170,   171,   172,   173,    -1,
     175,    -1,    -1,   178,    -1,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     195,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,    -1,   130,   131,   132,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,   161,    10,
      11,    12,    13,    -1,   167,   168,    -1,   170,   171,   172,
     173,    -1,   175,    -1,    -1,   178,    -1,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   195,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,    -1,    -1,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     161,    -1,    -1,    -1,    -1,    -1,   167,   168,    -1,   170,
     171,   172,   173,    -1,   175,    -1,    -1,   178,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   195,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,   200,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,   198,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,   198,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    30,    31,
     198,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    30,    31,
     198,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    68,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,    -1,    69,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,    -1,
      -1,    -1,    -1,   161,    -1,    -1,   164,    -1,    -1,   167,
     168,   103,   170,   171,   172,    32,   174,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,   121,
     122,   123,   124,    50,    51,    -1,    -1,   195,    -1,    56,
      -1,    58,    -1,    -1,    -1,   197,   138,   139,   140,   141,
     142,   143,   144,    70,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    -1,    -1,    -1,    -1,   161,
      -1,    -1,   164,   165,    91,   167,   168,    31,   170,   171,
     172,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,    -1,   191,
      -1,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   197,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,    81,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,   103,
     167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,    -1,
      -1,   125,    -1,   190,    -1,    -1,    -1,    -1,   195,   196,
      38,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,
     164,   165,    70,   167,   168,    -1,   170,   171,   172,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   195,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,   137,
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    69,    -1,    -1,    -1,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    50,    51,   176,    -1,
      -1,    -1,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   190,    -1,    -1,    -1,    70,   195,   196,    -1,
      -1,    -1,    -1,   201,    78,    79,    80,    81,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    91,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
      69,   145,   146,   147,   148,   149,    -1,    -1,    -1,    50,
      51,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    70,
      -1,    -1,   176,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,   185,    -1,    -1,    -1,    -1,   190,    -1,    -1,    -1,
      91,   195,   196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,   119,
     161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,   141,   142,   143,   185,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,   195,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    70,    -1,    -1,   176,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
     190,    -1,    -1,    -1,    91,   195,   196,    -1,    -1,   199,
      -1,   201,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    70,    71,    -1,   176,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,   190,    -1,    -1,    -1,    91,   195,   196,
      -1,    -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,
      -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    70,
      -1,    -1,   176,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,   190,    -1,    -1,    -1,
      91,   195,   196,    -1,    -1,    -1,    -1,   201,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    70,    -1,    -1,   176,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,   190,
      -1,    -1,    -1,    91,   195,   196,    -1,    -1,    -1,    -1,
     201,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   190,    -1,    -1,    -1,    -1,   195,   196,    -1,
      -1,    30,    31,   201,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   136,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,   136,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,   136,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,    -1,    -1,    -1,    69,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   130,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,   156,   136,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,   176,    -1,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,    -1,    -1,    -1,   190,    91,    -1,    -1,    -1,
     195,   196,    -1,    -1,    -1,    -1,   201,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,   176,    -1,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,    -1,    -1,    -1,   190,    91,    -1,    -1,    -1,
     195,   196,    -1,    -1,    -1,    -1,   201,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,   176,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,   190,    -1,    -1,    -1,    -1,
     195,   196,    -1,    -1,    -1,    -1,   201,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    32,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    12,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,    69,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    69,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69
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
     343,   345,   348,   199,   239,   348,   111,   112,   164,   216,
     217,   218,   219,   223,    83,   201,   293,   294,    83,   295,
     121,   130,   120,   130,   196,   196,   196,   196,   213,   265,
     484,   196,   196,    70,    70,    70,    70,    70,   338,    83,
      90,   157,   158,   159,   473,   474,   164,   199,   223,   223,
     213,   266,   484,   165,   196,   484,   484,    83,   192,   199,
     359,    28,   336,   340,   348,   349,   453,   457,   228,   199,
     462,    90,   414,   473,    90,   473,   473,    32,   164,   181,
     485,   196,     9,   198,    38,   245,   165,   264,   484,   119,
     191,   246,   328,   198,   198,   198,   198,   198,   198,   198,
     198,    10,    11,    12,    30,    31,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    57,
      69,   198,    70,    70,   199,   160,   131,   171,   173,   186,
     188,   267,   326,   327,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    59,    60,   134,
     135,   443,    70,   199,   448,   196,   196,    70,   199,   196,
     245,   246,    14,   348,   198,   136,    49,   213,   438,    90,
     336,   349,   160,   453,   136,   203,     9,   423,   260,   336,
     349,   453,   485,   160,   196,   415,   443,   448,   197,   348,
      32,   230,     8,   360,     9,   198,   230,   231,   338,   339,
     348,   213,   279,   234,   198,   198,   198,   138,   144,   505,
     505,   181,   504,   196,   111,   505,    14,   160,   138,   144,
     161,   213,   215,   198,   198,   198,   240,   115,   178,   198,
     216,   218,   216,   218,   223,   199,     9,   424,   198,   102,
     164,   199,   453,     9,   198,    14,     9,   198,   130,   130,
     453,   477,   338,   336,   349,   453,   456,   457,   197,   181,
     257,   137,   453,   466,   467,   348,   368,   369,   338,   389,
     389,   368,   389,   198,    70,   443,   157,   474,    82,   348,
     453,    90,   157,   474,   223,   212,   198,   199,   252,   262,
     398,   400,    91,   196,   361,   362,   364,   407,   411,   459,
     461,   478,    14,   102,   479,   355,   356,   357,   289,   290,
     441,   442,   197,   197,   197,   197,   197,   200,   229,   230,
     247,   254,   261,   441,   348,   202,   204,   205,   213,   486,
     487,   505,    38,   174,   291,   292,   348,   481,   196,   484,
     255,   245,   348,   348,   348,   348,    32,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   412,   348,   348,   463,   463,   348,   469,   470,   130,
     199,   214,   215,   462,   265,   213,   266,   484,   484,   264,
     246,    38,   340,   343,   345,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   165,   199,
     213,   444,   445,   446,   447,   462,   463,   348,   291,   291,
     463,   348,   466,   245,   197,   348,   196,   437,     9,   423,
     197,   197,    38,   348,    38,   348,   415,   197,   197,   197,
     460,   461,   462,   291,   199,   213,   444,   445,   462,   197,
     228,   283,   199,   345,   348,   348,    94,    32,   230,   277,
     198,    27,   102,    14,     9,   197,    32,   199,   280,   505,
      31,    91,   174,   226,   498,   499,   500,   196,     9,    50,
      51,    56,    58,    70,   138,   139,   140,   141,   142,   143,
     185,   196,   224,   375,   378,   381,   384,   387,   393,   408,
     416,   417,   419,   420,   213,   503,   228,   196,   238,   199,
     198,   199,   198,   102,   164,   111,   112,   164,   219,   220,
     221,   222,   223,   219,   213,   348,   294,   417,    83,     9,
     197,   197,   197,   197,   197,   197,   197,   198,    50,    51,
     494,   496,   497,   132,   270,   196,     9,   197,   197,   136,
     203,     9,   423,     9,   423,   203,   203,   203,   203,    83,
      85,   213,   475,   213,    70,   200,   200,   209,   211,    32,
     133,   269,   180,    54,   165,   180,   402,   349,   136,     9,
     423,   197,   160,   505,   505,    14,   360,   289,   228,   193,
       9,   424,   505,   506,   443,   448,   443,   200,     9,   423,
     182,   453,   348,   197,     9,   424,    14,   352,   248,   132,
     268,   196,   484,   348,    32,   203,   203,   136,   200,     9,
     423,   348,   485,   196,   258,   253,   263,    14,   479,   256,
     245,    71,   453,   348,   485,   203,   200,   197,   197,   203,
     200,   197,    50,    51,    70,    78,    79,    80,    91,   138,
     139,   140,   141,   142,   143,   156,   185,   213,   376,   379,
     382,   385,   388,   408,   419,   426,   428,   429,   433,   436,
     213,   453,   453,   136,   268,   443,   448,   197,   348,   284,
      75,    76,   285,   228,   337,   228,   339,   102,    38,   137,
     274,   453,   417,   213,    32,   230,   278,   198,   281,   198,
     281,     9,   423,    91,   226,   136,   160,     9,   423,   197,
     174,   486,   487,   488,   486,   417,   417,   417,   417,   417,
     422,   425,   196,    70,    70,    70,    70,    70,   196,   417,
     160,   199,    10,    11,    12,    31,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    69,   160,
     485,   200,   408,   199,   242,   218,   218,   213,   219,   219,
     223,     9,   424,   200,   200,    14,   453,   198,   182,     9,
     423,   213,   271,   408,   199,   466,   137,   453,    14,   348,
     348,   203,   348,   200,   209,   505,   271,   199,   401,    14,
     197,   348,   361,   462,   198,   505,   193,   200,    32,   492,
     442,    38,    83,   174,   444,   445,   447,   444,   445,   505,
      38,   174,   348,   417,   289,   196,   408,   269,   353,   249,
     348,   348,   348,   200,   196,   291,   270,    32,   269,   505,
      14,   268,   484,   412,   200,   196,    14,    78,    79,    80,
     213,   427,   427,   429,   431,   432,    52,   196,    70,    70,
      70,    70,    70,    90,   157,   196,   160,     9,   423,   197,
     437,    38,   348,   269,   200,    75,    76,   286,   337,   230,
     200,   198,    95,   198,   274,   453,   196,   136,   273,    14,
     228,   281,   105,   106,   107,   281,   200,   505,   182,   136,
     160,   505,   213,   174,   498,     9,   197,   423,   136,   203,
       9,   423,   422,   370,   371,   417,   390,   417,   418,   390,
     370,   390,   361,   363,   365,   197,   130,   214,   417,   471,
     472,   417,   417,   417,    32,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   503,
      83,   243,   200,   200,   222,   198,   417,   497,   102,   103,
     493,   495,     9,   299,   197,   196,   340,   345,   348,   136,
     203,   200,   479,   299,   166,   179,   199,   397,   404,   166,
     199,   403,   136,   198,   492,   505,   360,   506,    83,   174,
      14,    83,   485,   453,   348,   197,   289,   199,   289,   196,
     136,   196,   291,   197,   199,   505,   199,   198,   505,   269,
     250,   415,   291,   136,   203,     9,   423,   428,   431,   372,
     373,   429,   391,   429,   430,   391,   372,   391,   157,   361,
     434,   435,    81,   429,   453,   199,   337,    32,    77,   230,
     198,   339,   273,   466,   274,   197,   417,   101,   105,   198,
     348,    32,   198,   282,   200,   182,   505,   213,   136,   174,
      32,   197,   417,   417,   197,   203,     9,   423,   136,   203,
       9,   423,   203,   203,   203,   136,     9,   423,   197,   136,
     200,     9,   423,   417,    32,   197,   228,   198,   198,   213,
     505,   505,   493,   408,     6,   112,   117,   120,   125,   167,
     168,   170,   200,   300,   325,   326,   327,   332,   333,   334,
     335,   441,   466,   348,   200,   199,   200,    54,   348,   348,
     348,   360,    38,    83,   174,    14,    83,   348,   196,   492,
     197,   299,   197,   289,   348,   291,   197,   299,   479,   299,
     198,   199,   196,   197,   429,   429,   197,   203,     9,   423,
     136,   203,     9,   423,   203,   203,   203,   136,   197,     9,
     423,   299,    32,   228,   198,   197,   197,   197,   235,   198,
     198,   282,   228,   136,   505,   505,   136,   417,   417,   417,
     417,   361,   417,   417,   417,   199,   200,   495,   132,   133,
     186,   214,   482,   505,   272,   408,   112,   335,    31,   125,
     138,   144,   165,   171,   309,   310,   311,   312,   408,   169,
     317,   318,   128,   196,   213,   319,   320,   301,   246,   505,
       9,   198,     9,   198,   198,   479,   326,   197,   296,   165,
     399,   200,   200,    83,   174,    14,    83,   348,   291,   117,
     350,   492,   200,   492,   197,   197,   200,   199,   200,   299,
     289,   136,   429,   429,   429,   429,   361,   200,   228,   233,
     236,    32,   230,   276,   228,   505,   197,   417,   136,   136,
     136,   228,   408,   408,   484,    14,   214,     9,   198,   199,
     482,   479,   312,   181,   199,     9,   198,     3,     4,     5,
       6,     7,    10,    11,    12,    13,    27,    28,    29,    57,
      71,    72,    73,    74,    75,    76,    77,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   137,   138,
     145,   146,   147,   148,   149,   161,   162,   163,   173,   175,
     176,   178,   185,   186,   188,   190,   191,   213,   405,   406,
       9,   198,   165,   169,   213,   320,   321,   322,   198,    83,
     331,   245,   302,   482,   482,    14,   246,   200,   297,   298,
     482,    14,    83,   348,   197,   196,   492,   198,   199,   323,
     350,   492,   296,   200,   197,   429,   136,   136,    32,   230,
     275,   276,   228,   417,   417,   417,   200,   198,   198,   417,
     408,   305,   505,   313,   314,   416,   310,    14,    32,    51,
     315,   318,     9,    36,   197,    31,    50,    53,    14,     9,
     198,   215,   483,   331,    14,   505,   245,   198,    14,   348,
      38,    83,   396,   199,   228,   492,   323,   200,   492,   429,
     429,   228,    99,   241,   200,   213,   226,   306,   307,   308,
       9,   423,     9,   423,   200,   417,   406,   406,    68,   316,
     321,   321,    31,    50,    53,   417,    83,   181,   196,   198,
     417,   484,   417,    83,     9,   424,   228,   200,   199,   323,
      97,   198,   115,   237,   160,   102,   505,   182,   416,   172,
      14,   494,   303,   196,    38,    83,   197,   200,   228,   198,
     196,   178,   244,   213,   326,   327,   182,   417,   182,   287,
     288,   442,   304,    83,   200,   408,   242,   175,   213,   198,
     197,     9,   424,   122,   123,   124,   329,   330,   287,    83,
     272,   198,   492,   442,   506,   197,   197,   198,   195,   489,
     329,    38,    83,   174,   492,   199,   490,   491,   505,   198,
     199,   324,   506,    83,   174,    14,    83,   489,   228,     9,
     424,    14,   493,   228,    38,    83,   174,    14,    83,   348,
     324,   200,   491,   505,   200,    83,   174,    14,    83,   348,
      14,    83,   348,   348
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   206,   208,   207,   209,   209,   210,   210,   210,   210,
     210,   210,   210,   210,   211,   210,   212,   210,   210,   210,
     210,   210,   210,   210,   210,   213,   213,   213,   213,   213,
     213,   213,   213,   213,   213,   213,   213,   213,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   215,   215,   216,   216,   217,   217,   218,   219,
     219,   219,   219,   220,   220,   221,   222,   222,   222,   223,
     223,   224,   224,   224,   225,   226,   227,   227,   228,   228,
     229,   229,   229,   229,   230,   230,   230,   231,   230,   232,
     230,   233,   230,   234,   230,   230,   230,   230,   230,   230,
     230,   230,   230,   230,   230,   230,   230,   230,   230,   230,
     235,   230,   236,   230,   230,   237,   230,   238,   230,   230,
     230,   230,   230,   230,   230,   230,   230,   230,   230,   230,
     230,   230,   230,   240,   239,   241,   241,   243,   242,   244,
     244,   245,   245,   246,   248,   247,   249,   247,   250,   247,
     252,   251,   253,   251,   255,   254,   256,   254,   257,   254,
     258,   254,   260,   259,   262,   261,   263,   261,   264,   264,
     265,   266,   267,   267,   267,   267,   267,   268,   268,   269,
     269,   270,   270,   271,   271,   272,   272,   273,   273,   274,
     274,   274,   275,   275,   276,   276,   277,   277,   278,   278,
     279,   279,   280,   280,   280,   280,   281,   281,   281,   282,
     282,   283,   283,   284,   284,   285,   285,   286,   286,   287,
     287,   287,   287,   287,   287,   287,   287,   288,   288,   288,
     288,   288,   288,   288,   288,   289,   289,   289,   289,   289,
     289,   289,   289,   290,   290,   290,   290,   290,   290,   290,
     290,   291,   291,   292,   292,   292,   292,   292,   292,   293,
     293,   294,   294,   294,   295,   295,   295,   295,   296,   296,
     297,   298,   299,   299,   301,   300,   302,   300,   300,   300,
     300,   303,   300,   304,   300,   300,   300,   300,   300,   300,
     300,   300,   305,   305,   305,   306,   307,   307,   308,   308,
     309,   309,   310,   310,   311,   311,   312,   312,   312,   312,
     312,   312,   312,   313,   313,   314,   315,   315,   316,   316,
     317,   317,   318,   319,   319,   319,   320,   320,   320,   320,
     321,   321,   321,   321,   321,   321,   321,   322,   322,   322,
     323,   323,   324,   324,   325,   325,   326,   326,   327,   327,
     328,   328,   328,   328,   328,   328,   328,   329,   329,   330,
     330,   330,   331,   331,   331,   331,   332,   332,   333,   333,
     334,   334,   335,   336,   336,   336,   336,   336,   336,   337,
     338,   338,   339,   339,   340,   340,   340,   340,   341,   342,
     343,   344,   345,   346,   347,   348,   348,   348,   348,   348,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   350,   350,   352,
     351,   353,   351,   355,   354,   356,   354,   357,   354,   358,
     354,   359,   354,   360,   360,   360,   361,   361,   362,   362,
     363,   363,   364,   364,   365,   365,   366,   367,   367,   368,
     368,   369,   369,   370,   370,   371,   371,   372,   372,   373,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,   389,   389,   390,   390,
     391,   391,   392,   393,   394,   394,   395,   395,   395,   395,
     395,   395,   395,   395,   395,   395,   395,   396,   396,   396,
     396,   397,   398,   398,   399,   399,   400,   400,   401,   401,
     402,   403,   403,   404,   404,   404,   405,   405,   405,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     407,   408,   408,   409,   409,   409,   409,   409,   410,   410,
     411,   411,   411,   411,   412,   412,   412,   413,   413,   413,
     414,   414,   414,   415,   415,   416,   416,   416,   416,   416,
     416,   416,   416,   416,   416,   416,   416,   416,   416,   416,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   418,   418,   419,   420,   420,
     421,   421,   421,   421,   421,   421,   421,   422,   422,   423,
     423,   424,   424,   425,   425,   425,   425,   426,   426,   426,
     426,   426,   427,   427,   427,   427,   428,   428,   429,   429,
     429,   429,   429,   429,   429,   429,   429,   429,   429,   429,
     429,   429,   429,   430,   430,   431,   431,   432,   432,   432,
     432,   433,   433,   434,   434,   435,   435,   436,   436,   437,
     437,   438,   438,   440,   439,   441,   442,   442,   443,   443,
     444,   444,   444,   445,   445,   446,   446,   447,   447,   448,
     448,   449,   449,   450,   450,   451,   451,   452,   452,   453,
     453,   453,   453,   453,   453,   453,   453,   453,   453,   453,
     454,   454,   454,   454,   454,   454,   454,   454,   454,   455,
     455,   455,   455,   455,   455,   455,   455,   455,   456,   457,
     457,   458,   458,   459,   459,   459,   460,   461,   461,   461,
     462,   462,   462,   462,   463,   463,   464,   464,   464,   464,
     464,   464,   465,   465,   465,   465,   465,   466,   466,   466,
     466,   466,   466,   467,   467,   468,   468,   468,   468,   468,
     468,   468,   468,   469,   469,   470,   470,   470,   470,   471,
     471,   472,   472,   472,   472,   473,   473,   473,   473,   474,
     474,   474,   474,   474,   474,   475,   475,   475,   476,   476,
     476,   476,   476,   476,   476,   476,   476,   476,   476,   477,
     477,   478,   478,   479,   479,   480,   480,   480,   480,   481,
     481,   482,   482,   483,   483,   484,   484,   485,   485,   486,
     486,   487,   488,   488,   488,   488,   489,   489,   490,   490,
     491,   491,   492,   492,   493,   493,   494,   495,   495,   496,
     496,   496,   496,   497,   497,   497,   498,   498,   498,   498,
     499,   499,   500,   500,   500,   500,   501,   502,   503,   503,
     504,   504,   505,   505,   505,   505,   505,   505,   505,   505,
     505,   505,   505,   506,   506
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     1,     4,     3,     0,     6,     0,     5,     3,     4,
       4,     6,     7,     7,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     3,     3,     1,     2,     1,
       2,     3,     4,     3,     1,     2,     1,     2,     2,     1,
       3,     1,     3,     2,     2,     2,     5,     4,     2,     0,
       1,     1,     1,     1,     3,     5,     8,     0,     4,     0,
       6,     0,    10,     0,     4,     2,     3,     2,     3,     2,
       3,     3,     3,     3,     3,     3,     5,     1,     1,     1,
       0,     9,     0,    10,     5,     0,    13,     0,     5,     3,
       3,     2,     2,     2,     2,     2,     2,     3,     2,     2,
       3,     2,     2,     0,     4,     9,     0,     0,     4,     2,
       0,     1,     0,     1,     0,     9,     0,    10,     0,    11,
       0,     9,     0,    10,     0,     8,     0,     9,     0,     7,
       0,     8,     0,     8,     0,     7,     0,     8,     1,     1,
       1,     1,     1,     2,     3,     3,     2,     2,     0,     2,
       0,     2,     0,     1,     3,     1,     3,     2,     0,     1,
       2,     4,     1,     4,     1,     4,     1,     4,     1,     4,
       3,     5,     3,     4,     4,     5,     5,     4,     0,     1,
       1,     4,     0,     5,     0,     2,     0,     3,     0,     7,
       8,     6,     2,     5,     6,     4,     0,     4,     5,     7,
       6,     6,     7,     9,     8,     6,     7,     5,     2,     4,
       5,     3,     0,     3,     4,     6,     5,     5,     6,     8,
       7,     2,     0,     1,     2,     2,     3,     4,     4,     3,
       1,     1,     2,     4,     3,     5,     1,     3,     2,     0,
       2,     3,     2,     0,     0,     4,     0,     5,     2,     2,
       2,     0,    11,     0,    12,     3,     3,     3,     4,     4,
       3,     5,     2,     2,     0,     6,     5,     4,     3,     1,
       1,     3,     4,     1,     2,     1,     1,     5,     6,     1,
       1,     4,     1,     1,     3,     2,     2,     0,     2,     0,
       1,     3,     1,     1,     1,     1,     3,     4,     4,     4,
       1,     1,     2,     2,     2,     3,     3,     1,     1,     1,
       1,     3,     1,     3,     1,     1,     1,     0,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     1,
       1,     1,     3,     5,     1,     3,     5,     4,     3,     3,
       3,     4,     3,     3,     3,     2,     2,     1,     1,     3,
       3,     1,     1,     0,     1,     2,     4,     3,     3,     6,
       2,     3,     2,     3,     6,     1,     1,     1,     1,     1,
       6,     3,     4,     6,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     5,     4,     3,     1,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     2,     1,     5,     0,     0,
      12,     0,    13,     0,     4,     0,     7,     0,     5,     0,
       3,     0,     6,     2,     2,     4,     1,     1,     5,     3,
       5,     3,     2,     0,     2,     0,     4,     4,     3,     2,
       0,     5,     3,     2,     0,     5,     3,     2,     0,     5,
       3,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     2,     0,     2,     0,
       2,     0,     4,     4,     4,     4,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     3,     4,     1,
       2,     4,     2,     6,     0,     1,     4,     0,     2,     0,
       1,     1,     3,     1,     3,     1,     1,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       4,     1,     1,     1,     1,     1,     1,     3,     1,     3,
       1,     1,     1,     3,     1,     1,     1,     2,     1,     0,
       0,     1,     1,     3,     0,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     2,
       1,     1,     4,     3,     4,     1,     1,     1,     1,     1,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     5,     4,     3,     1,     3,     3,     1,
       1,     1,     1,     1,     3,     3,     3,     2,     0,     1,
       0,     1,     0,     5,     3,     3,     1,     1,     1,     1,
       3,     2,     1,     1,     1,     1,     1,     3,     1,     1,
       1,     3,     1,     2,     2,     4,     3,     4,     1,     1,
       1,     1,     1,     3,     1,     2,     0,     5,     3,     3,
       1,     3,     1,     2,     0,     5,     3,     2,     0,     3,
       0,     4,     2,     0,     3,     3,     1,     0,     1,     1,
       1,     1,     3,     1,     1,     1,     3,     1,     1,     3,
       3,     2,     4,     2,     4,     5,     5,     5,     5,     1,
       1,     1,     1,     1,     1,     3,     3,     4,     4,     3,
       1,     1,     1,     1,     3,     1,     4,     3,     3,     1,
       1,     1,     1,     1,     3,     3,     4,     4,     3,     1,
       1,     7,     9,     7,     6,     8,     1,     4,     4,     1,
       1,     1,     4,     2,     1,     0,     1,     1,     1,     3,
       3,     3,     0,     1,     1,     3,     3,     2,     3,     6,
       0,     1,     4,     2,     0,     5,     3,     3,     1,     6,
       4,     4,     2,     2,     0,     5,     3,     3,     1,     2,
       0,     5,     3,     3,     1,     2,     2,     1,     2,     1,
       4,     3,     3,     6,     3,     1,     1,     1,     4,     4,
       4,     4,     4,     4,     2,     2,     4,     2,     2,     1,
       3,     3,     3,     0,     2,     5,     6,     6,     7,     1,
       2,     1,     2,     1,     4,     1,     4,     3,     0,     1,
       3,     2,     3,     1,     1,     0,     0,     3,     1,     3,
       3,     2,     0,     2,     2,     2,     2,     1,     2,     4,
       2,     5,     3,     1,     1,     0,     3,     4,     5,     6,
       3,     1,     3,     2,     1,     0,     4,     1,     3,     2,
       4,     5,     2,     2,     1,     1,     1,     1,     3,     2,
       1,     8,     6,     1,     0
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
#line 6837 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 754 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 6845 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 761 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 6851 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 762 "hphp.y" /* yacc.c:1646  */
    { }
#line 6857 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 765 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 6863 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 766 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6869 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 767 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 768 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6881 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 769 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6887 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 770 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 6893 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 771 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 6901 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 774 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 6908 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 776 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 6914 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 6920 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 778 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 6926 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 779 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 6932 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 780 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6939 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 783 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6946 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 786 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6953 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 789 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6961 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 793 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6969 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 797 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6977 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 800 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 6984 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 805 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 6990 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 806 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 6996 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 807 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7002 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 808 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7008 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 809 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7014 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 810 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7020 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 811 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7026 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 812 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7032 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 813 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7038 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 814 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7044 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 815 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7050 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 816 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7056 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 817 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7062 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 896 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7068 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 898 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7074 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 903 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7080 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 904 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7087 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 910 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7093 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 914 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7099 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 915 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7105 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 917 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7111 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 919 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7117 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 924 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7123 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 925 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7130 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 931 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7136 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 935 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClass);}
#line 7143 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 937 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7150 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 939 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7157 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 944 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7163 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 946 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7169 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 949 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7175 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 951 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7181 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 952 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7187 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 957 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7196 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 964 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7205 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 972 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7212 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 975 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7219 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 981 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7225 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 982 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7231 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 985 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7237 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 986 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7243 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 987 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7249 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 988 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7255 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 991 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7261 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 995 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7267 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1000 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7273 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 1001 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7280 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1003 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7288 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1007 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7295 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1010 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7303 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1014 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7310 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1016 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7318 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1019 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7325 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1021 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7333 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1024 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7339 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1025 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7345 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1026 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7351 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1027 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7357 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1028 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7363 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1029 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7369 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1030 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7375 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1031 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7381 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1032 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7387 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1033 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7393 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1034 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7399 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1035 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 7405 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1036 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 7411 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1037 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 7417 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1038 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 7424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1042 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7431 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1044 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 7439 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1049 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7446 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1051 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 7454 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1055 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 7462 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1064 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7468 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1065 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 7474 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1068 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7480 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1069 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 7486 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1070 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 7492 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1071 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 7501 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1075 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7507 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1076 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7513 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1077 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7519 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1078 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7525 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1079 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7531 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1080 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7537 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1081 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7543 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1082 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7549 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1083 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7555 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1084 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 7561 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1085 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7567 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1086 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 7577 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1094 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);}
#line 7583 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1095 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7589 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1104 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 7595 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1105 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7601 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1109 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7608 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1111 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7616 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1117 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7622 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1118 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7628 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1122 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 7634 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1123 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7640 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1127 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 7646 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1133 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7655 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1139 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7664 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1146 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7673 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1152 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7682 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1159 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7691 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1165 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7700 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1173 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 7707 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1177 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 7713 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1181 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 7720 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1185 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 7726 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1191 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 7733 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1194 "hphp.y" /* yacc.c:1646  */
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
#line 7751 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 1209 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 7758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 1212 "hphp.y" /* yacc.c:1646  */
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
#line 7776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 208:
#line 1226 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 7783 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 1229 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7791 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 1234 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 7798 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1237 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7806 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1243 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 7812 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1246 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 7818 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1250 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 7825 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1253 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7836 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1261 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 7843 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1264 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7854 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1272 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7860 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1273 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 7867 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1277 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7873 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1280 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7879 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1283 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 7885 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1284 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 7891 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1285 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 7899 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1288 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 7905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1289 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 7911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1293 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7917 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1294 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7923 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1297 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1298 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7935 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1301 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7941 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1302 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1305 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 7953 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1307 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 7959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 1310 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 7965 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1312 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 7971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 1316 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7977 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1317 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1320 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 7989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1321 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 7995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1322 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8001 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1326 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8007 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1328 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8013 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1331 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1333 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1336 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1338 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8037 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1341 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8043 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1343 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8049 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1347 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8055 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1349 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8062 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1354 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8068 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1355 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8074 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1356 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8080 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1357 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8086 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1362 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8092 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1364 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8098 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1365 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8104 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1368 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8110 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1369 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8116 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1374 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8122 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1375 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8128 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1380 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8134 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1381 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8140 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1384 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8146 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1385 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8152 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1388 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8158 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1389 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8164 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1397 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8171 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1403 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8178 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1409 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8186 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1413 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8192 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1417 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8199 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1422 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8206 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1427 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8214 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1430 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8220 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1436 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8227 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1440 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8234 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1445 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8241 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1450 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8248 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1455 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8255 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1460 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8262 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1466 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8269 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1472 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8276 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1480 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 8283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1485 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 8290 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1490 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 8298 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1494 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8304 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1497 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 8311 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1501 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 8318 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1505 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 8326 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1508 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8332 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1513 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8339 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1516 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8346 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1520 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8353 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1524 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8360 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1528 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8367 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1532 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8374 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1537 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8381 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1542 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8388 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1548 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8394 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1549 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1552 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,false);}
#line 8406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1553 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),true,false);}
#line 8412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1554 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,true);}
#line 8418 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1556 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),false, false);}
#line 8424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1558 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),false,true);}
#line 8430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1560 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),true, false);}
#line 8436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1564 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1565 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 8448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1568 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8454 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1569 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1570 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 8466 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1574 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 8472 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1576 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 8478 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1577 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 8484 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1578 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 8490 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1583 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 8496 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1584 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8502 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1587 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 8509 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1592 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 8515 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1598 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 8521 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1599 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8527 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1602 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[0]));}
#line 8533 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1603 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 8540 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1606 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[-1]));}
#line 8546 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1607 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-3]));}
#line 8553 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1609 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 8560 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1612 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL, true);}
#line 8567 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1614 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8573 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1617 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 8581 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1624 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8590 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1632 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 8598 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1639 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8607 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1644 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 8613 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1646 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 8619 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1648 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 8625 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1650 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 8631 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1652 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 8637 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1653 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 8644 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1656 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 8650 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1659 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 8656 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1660 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 8662 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1661 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8668 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1667 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 8674 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1672 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 8681 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1675 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 8689 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1682 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 8695 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1683 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 8702 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1688 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 8709 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1691 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 8715 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1698 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 8722 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1700 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8728 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1704 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8734 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1709 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8740 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1711 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8746 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1713 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8752 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 359:
#line 1714 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 8763 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1720 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 8769 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1722 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 8775 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1723 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 8781 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1727 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,0,(yyvsp[0]),0);}
#line 8787 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1729 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),0,(yyvsp[0]),0);}
#line 8793 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1734 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8799 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1737 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8805 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1738 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 8811 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1742 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 8817 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1743 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 8823 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1747 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 8830 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1750 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 8837 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1755 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 8844 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1760 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 8850 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1761 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 8857 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1763 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 8863 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1767 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 8869 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1768 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 8875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1769 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 8881 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1770 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 8887 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1774 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8893 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1775 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 8899 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1776 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 8905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 1777 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 8911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1778 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 8917 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1780 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 8923 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1782 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 8929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1786 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 8937 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1789 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 8943 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1790 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 8949 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1794 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8955 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1795 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 8961 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 1799 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8967 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1800 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 8973 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 394:
#line 1803 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8979 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1804 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8985 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1807 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8991 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1808 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8997 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1811 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9003 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1813 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9009 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1816 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9015 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1817 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9021 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1818 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9027 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1819 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9033 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1820 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9039 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1821 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9045 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1822 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9051 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1826 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9057 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1827 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9063 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1830 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9069 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1831 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9075 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9081 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1836 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9087 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1838 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9093 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1839 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9099 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1840 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9105 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1844 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9111 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1846 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9117 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1850 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9123 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1852 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9129 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1856 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9137 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1860 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9144 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1864 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9150 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1868 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9156 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1870 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9162 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1871 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9168 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1872 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9174 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1873 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9180 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1874 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9186 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1877 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9192 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1881 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9198 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1882 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9204 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1886 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9210 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1887 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9216 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 1891 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 9222 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 1892 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 9228 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 1893 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 9234 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 1894 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9240 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 1898 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9246 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 1903 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9252 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 1907 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 9258 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 1911 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9264 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 1915 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 9270 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 1919 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9276 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 1924 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9282 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 1928 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9288 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 1929 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9294 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 1930 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9300 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 1931 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9306 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 1932 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9312 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 1937 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 9318 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 1938 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 9324 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 1939 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 9330 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 1942 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 9336 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 1943 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 9342 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 1944 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 9348 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 1945 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 9354 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 1946 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 9360 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 1947 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 9366 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 1948 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 9372 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 1949 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 9378 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 1950 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 9384 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 1951 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 9390 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 1952 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 9396 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 1953 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 9402 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 1954 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 9408 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 1955 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 9414 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 1956 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 9420 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 1957 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 9426 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 1958 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 9432 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 1959 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 9438 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 1960 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 9444 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 1961 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 9450 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 1962 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 9456 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 9462 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 1964 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 9468 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 1965 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 9474 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 1966 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 9480 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 1967 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 9486 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 1968 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 9492 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 1969 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 9498 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 1970 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 9504 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 1971 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 9510 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 1972 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 9516 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 1973 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 9522 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 1974 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 9528 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 1975 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 9534 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 1976 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 9540 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 1977 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 9546 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 1978 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 9552 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 1979 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 9558 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 1980 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 9564 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 1981 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 9570 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 1982 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 9576 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 1983 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 9582 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 1984 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 9588 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 1985 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 9594 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 1986 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 9601 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 1988 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 9607 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 1989 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 9614 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 1991 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 9620 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 1993 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 9626 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 1994 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9632 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 1995 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 9638 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 1996 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 9644 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 1997 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 9650 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 1998 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9656 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 1999 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 9662 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 9668 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2001 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 9674 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2002 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 9680 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2003 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 9686 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2004 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 9692 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2005 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 9698 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2006 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 9704 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2007 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 9710 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2008 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9716 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2009 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9722 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2010 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9728 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2011 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9734 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2012 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9740 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2013 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9746 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2014 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9752 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2015 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2016 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 9764 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 9770 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2025 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 9782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2026 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9788 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2031 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 9797 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2037 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9808 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2045 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 9817 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2051 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[-12]),
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9828 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2061 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),0,
                                                     NULL,NULL,NULL);}
#line 9841 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2069 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         (yyvsp[-3]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[-3]), nullptr, (yyvsp[-3]));
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[-3]),
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9855 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2079 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9865 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2087 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         (yyvsp[-6]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[-6]), nullptr, (yyvsp[-6]));
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[-6]),
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9879 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2097 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9889 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2103 "hphp.y" /* yacc.c:1646  */
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
#line 9905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2114 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),0,
                                                     NULL,NULL,NULL);}
#line 9918 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2122 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9930 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2129 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9940 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2137 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9952 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2147 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 9958 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2148 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 9964 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2150 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9970 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2154 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 9977 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2156 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2163 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 9989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2166 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 9995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2173 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10001 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2176 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10007 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2181 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10013 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2182 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2187 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2188 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2192 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval), (yyvsp[-1]), T_ARRAY);}
#line 10037 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2196 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10043 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2197 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10049 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2202 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10055 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2203 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10061 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2208 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10067 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2209 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10073 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2214 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10079 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2215 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10085 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2221 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10091 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2223 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10097 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2228 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10103 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2229 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10109 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2235 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10115 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 570:
#line 2237 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10121 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2241 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10127 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 572:
#line 2245 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10133 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2249 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10139 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2253 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10145 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2257 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10151 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
#line 2261 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10157 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2265 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10163 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
#line 2269 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10169 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
#line 2273 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10175 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
#line 2277 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10181 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2281 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10187 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
#line 2285 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10193 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2289 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10199 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2293 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10205 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2297 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10211 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2302 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10217 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2303 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10223 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2308 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10229 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2309 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10235 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2314 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10241 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2315 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10247 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2320 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10255 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2327 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10263 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2334 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10269 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2336 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10275 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2340 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10281 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2341 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10287 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2342 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10293 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2343 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10299 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2344 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10305 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2345 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10311 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2346 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10317 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2347 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10323 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2348 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10330 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2350 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10336 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2351 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10342 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2355 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10348 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2356 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 10354 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2357 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 10360 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2358 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 10366 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2365 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 10372 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2368 "hphp.y" /* yacc.c:1646  */
    { Token t1; _p->onArray(t1,(yyvsp[-1]));
                                         Token t2; _p->onArray(t2,(yyvsp[0]));
                                         Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onCallParam((yyvsp[-1]),NULL,t1,0,0);
                                         _p->onCallParam((yyval), &(yyvsp[-1]),t2,0,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),file,0,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),line,0,0);
                                         (yyval).setText("");}
#line 10386 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2379 "hphp.y" /* yacc.c:1646  */
    { Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onArray((yyvsp[-2]),(yyvsp[-5]));
                                         _p->onArray((yyvsp[-1]),(yyvsp[-3]));
                                         _p->onCallParam((yyvsp[-4]),NULL,(yyvsp[-2]),0,0);
                                         _p->onCallParam((yyval), &(yyvsp[-4]),(yyvsp[-1]),0,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),file,0,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),line,0,0);
                                         (yyval).setText((yyvsp[0]).text());}
#line 10400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2390 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 10406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2391 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 10412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2396 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10418 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2397 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2400 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-1]),0,(yyvsp[0]),0);}
#line 10430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2401 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2404 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10443 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2408 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10451 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2411 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10457 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2414 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         if ((yyvsp[0]).htmlTrim()) {
                                           (yyvsp[0]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));
                                         }
                                       }
#line 10469 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2421 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10475 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2422 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10481 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2426 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10487 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2428 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 10493 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2430 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 10499 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2434 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10505 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2435 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10511 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2436 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10517 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2437 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10523 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2438 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10529 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2439 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10535 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2440 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10541 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2441 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10547 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2442 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10553 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2443 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10559 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2444 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10565 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2445 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10571 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2446 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10577 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2447 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10583 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2448 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10589 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2449 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10595 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2450 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10601 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2451 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10607 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2452 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10613 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2453 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10619 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2454 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10625 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2455 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10631 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2456 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10637 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2457 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10643 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 653:
#line 2458 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10649 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10655 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2460 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10661 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2461 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10667 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2462 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10673 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2463 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10679 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2464 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10685 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2465 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10691 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2466 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10697 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2467 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10703 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2468 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10709 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2469 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10715 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2470 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10721 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2471 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10727 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2472 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10733 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2473 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10739 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2474 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10745 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2475 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10751 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2476 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10757 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2477 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10763 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2478 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10769 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2479 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10775 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2480 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10781 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2481 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10787 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2482 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10793 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10799 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2484 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10805 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2485 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10811 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2486 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10817 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2487 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10823 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2488 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10829 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2489 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10835 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2490 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10841 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2491 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10847 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2492 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10853 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2493 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10859 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2494 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10865 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2495 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10871 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2496 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10877 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2497 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10883 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2498 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10889 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2499 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10895 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2500 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10901 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2501 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10907 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2502 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10913 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2503 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10919 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2504 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10925 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2505 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10931 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2506 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10937 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2507 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10943 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2508 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10949 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2509 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10955 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2510 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10961 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2511 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10967 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2512 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10973 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2513 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10979 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2514 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10985 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2519 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 10991 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2523 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10997 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2524 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11003 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2528 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11009 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2529 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11015 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2530 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11021 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2531 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11028 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2533 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11035 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2537 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11041 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2546 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11047 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2549 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11053 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2550 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11060 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2552 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11067 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2562 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11073 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2566 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11079 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2567 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11085 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2568 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11091 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2572 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11097 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2573 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11103 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2574 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11109 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2578 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11115 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2579 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11121 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2580 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11127 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2584 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11133 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2585 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11139 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2589 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11145 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2590 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11151 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2591 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11157 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2592 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 11164 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2594 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 11170 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2595 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 11176 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2596 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 11182 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2597 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 11188 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2598 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 11194 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2599 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 11200 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2600 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 11206 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2601 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 11212 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2602 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 11218 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2605 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11224 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2607 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11230 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2611 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11236 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2612 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11242 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2614 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11248 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2615 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11254 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2617 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11260 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2618 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11266 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2619 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11272 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2620 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11278 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2621 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11284 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2622 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11290 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2623 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11296 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2624 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11302 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2625 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11308 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2627 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 11314 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2629 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 11320 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2631 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 11326 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2633 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 11332 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2635 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 11338 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2636 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 11344 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2637 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 11350 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2638 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 11356 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2639 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 11362 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2640 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 11368 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2641 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 11374 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2642 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 11380 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2643 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 11386 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2644 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 11392 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2645 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 11398 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2646 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 11404 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2647 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 11410 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2648 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 11416 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2649 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 11422 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2650 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11428 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2651 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11434 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2653 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 11440 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2655 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 11446 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2657 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 11452 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2659 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 11458 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2660 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 11464 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2662 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 11471 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2664 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 11477 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2667 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 11484 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2671 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 11490 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2674 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 11496 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2675 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 11502 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2679 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 11508 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2680 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 11514 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2686 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 11520 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2692 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 11526 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2693 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11532 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2697 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11538 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2698 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11544 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2699 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11550 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2700 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11556 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2701 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 11562 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2702 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 11568 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2704 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 11575 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2709 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11581 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2710 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11587 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2714 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11593 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2715 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11599 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2718 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 11605 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2719 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11611 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2725 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11617 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2727 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11623 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2729 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11629 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2730 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11635 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2734 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11641 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2735 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11647 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2736 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11653 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2739 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11659 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2741 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11665 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2744 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 11671 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2745 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 11677 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2746 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 11683 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2747 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 11689 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2751 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 11696 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2754 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 11704 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2761 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11710 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2762 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11716 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2765 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 11724 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 832:
#line 2768 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 11730 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2769 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11736 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2770 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11742 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2772 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11748 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2773 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11754 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2775 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11760 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2776 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11766 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2777 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11772 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2778 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11778 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2779 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11784 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2780 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11790 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2785 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 11796 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2786 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 11802 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2791 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11808 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2792 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11814 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2797 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11820 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2799 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11826 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2801 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11832 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2802 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11838 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2806 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11844 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2807 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11850 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2812 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11856 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2813 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 11862 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2818 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 11868 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2821 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 11874 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2826 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11880 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2827 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11886 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 2830 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11892 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2831 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 11899 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2838 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 11905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2840 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 11911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2843 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 11917 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2845 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11923 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2848 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2851 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11935 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2852 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11941 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2856 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 11947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2857 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 11953 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 2861 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 11959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2862 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 11965 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2863 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 11971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2867 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11977 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2872 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 11983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2877 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 2878 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 877:
#line 2882 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12001 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 2887 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12007 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 2892 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12013 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 2893 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 2897 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 2899 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 2904 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12037 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 2906 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12043 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 2912 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12057 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
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
#line 12071 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 2938 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12085 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 2950 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12099 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 2962 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12105 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 2963 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12111 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 2964 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12117 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 2965 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12123 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 2966 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12129 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 2967 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12135 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 2969 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12149 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 2986 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12155 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 2988 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12161 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 2990 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12167 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 2991 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12173 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 2995 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12179 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 2996 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12185 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 2997 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12191 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 2998 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12197 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3006 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12211 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3015 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12217 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3017 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12223 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12229 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3022 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12235 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3027 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12241 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3028 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12247 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3029 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3030 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12259 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3031 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12265 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3032 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12271 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3033 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12277 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3035 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3037 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3041 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12295 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3045 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12301 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3046 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12307 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3052 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12313 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3056 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12319 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3063 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 12325 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3072 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 12331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3076 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 12337 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3080 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12343 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3089 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12349 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3090 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12355 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3091 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12361 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3095 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12367 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3096 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 12373 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3097 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 12379 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3099 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]) = 1; _p->onIndirectRef((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12385 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3104 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12391 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3105 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12397 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3116 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12403 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
#line 3117 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12409 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
#line 3118 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12415 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3121 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12429 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
#line 3132 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12435 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
#line 3133 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12441 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3137 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12447 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
#line 3138 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12453 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3141 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12467 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
#line 3150 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12473 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
#line 3154 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 12479 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
#line 3155 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 12485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3157 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 12491 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3158 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 12497 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3159 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 12503 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3160 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 12509 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3165 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12515 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3166 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12521 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3170 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12527 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3171 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12533 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3172 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12539 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3173 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12545 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3176 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 12551 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3178 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 12557 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3179 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 12563 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3180 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 12569 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3185 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12575 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3186 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 12581 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3190 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12587 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3191 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12593 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3192 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12599 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3193 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12605 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3198 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12611 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3199 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 12617 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3204 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12623 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3206 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12629 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3208 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12635 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3209 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12641 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3213 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 12647 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3215 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 12653 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3216 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 12659 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3218 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 12666 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3223 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12672 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3225 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12678 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3227 "hphp.y" /* yacc.c:1646  */
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[-2]),
                                           !(yyvsp[-1]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[0])
                                         );
                                       }
#line 12692 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3237 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 12698 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3239 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 12704 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3240 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12710 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3243 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 12716 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3244 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 12722 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3245 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 12728 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3249 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 12734 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3250 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 12740 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3251 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12746 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3252 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12752 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3253 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3254 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12764 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3255 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 12770 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 995:
#line 3256 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 12776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3257 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 12782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3258 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 12788 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
#line 3259 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 12794 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3263 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12800 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3264 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12806 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3269 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 12812 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3271 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 12818 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1005:
#line 3285 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 12826 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1006:
#line 3290 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 12834 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1007:
#line 3294 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 12842 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1008:
#line 3299 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 12850 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3305 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12856 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3306 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 12862 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3310 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12868 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3311 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 12874 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3317 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 12880 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3321 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 12886 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3327 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 12892 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3331 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 12899 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3338 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3339 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3343 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 12919 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3346 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 12926 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3352 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12932 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3357 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]); }
#line 12938 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3358 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12944 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3359 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12950 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3360 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12956 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1032:
#line 3381 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12962 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1033:
#line 3382 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 12968 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1036:
#line 3391 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12974 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1039:
#line 3402 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 12980 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1040:
#line 3404 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 12986 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1041:
#line 3408 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 12992 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1042:
#line 3411 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 12998 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3415 "hphp.y" /* yacc.c:1646  */
    {}
#line 13004 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3416 "hphp.y" /* yacc.c:1646  */
    {}
#line 13010 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3417 "hphp.y" /* yacc.c:1646  */
    {}
#line 13016 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3423 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13023 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3428 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13033 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3437 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13039 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3443 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13048 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3451 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 13054 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3452 "hphp.y" /* yacc.c:1646  */
    { }
#line 13060 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3458 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 13066 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3460 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 13072 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3461 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 13082 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1055:
#line 3466 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 13089 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1056:
#line 3472 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("array"); }
#line 13096 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1057:
#line 3477 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13102 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1058:
#line 3482 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 13110 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1059:
#line 3486 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13116 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1060:
#line 3491 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 13122 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1061:
#line 3493 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 13128 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1062:
#line 3499 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 13135 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3501 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 13143 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3504 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13149 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3505 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13157 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3508 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13165 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3511 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13171 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3514 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 13179 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1069:
#line 3517 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13186 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3519 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 13195 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3525 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 13204 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3531 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 13214 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3539 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13220 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3540 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13226 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;


#line 13230 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 3543 "hphp.y" /* yacc.c:1906  */

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}
