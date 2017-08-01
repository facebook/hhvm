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
#define YYLAST   18238

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  206
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  301
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1078
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1992

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
     788,   793,   796,   799,   803,   807,   811,   815,   819,   824,
     825,   826,   827,   828,   829,   830,   831,   832,   833,   834,
     835,   836,   840,   841,   842,   843,   844,   845,   846,   847,
     848,   849,   850,   851,   852,   853,   854,   855,   856,   857,
     858,   859,   860,   861,   862,   863,   864,   865,   866,   867,
     868,   869,   870,   871,   872,   873,   874,   875,   876,   877,
     878,   879,   880,   881,   882,   883,   884,   885,   886,   887,
     888,   889,   890,   891,   892,   893,   894,   895,   896,   897,
     898,   899,   900,   901,   902,   906,   910,   911,   915,   916,
     921,   923,   928,   933,   934,   935,   937,   942,   944,   949,
     954,   956,   958,   963,   964,   968,   969,   971,   975,   982,
     989,   993,   999,  1001,  1004,  1005,  1006,  1007,  1010,  1011,
    1015,  1020,  1020,  1026,  1026,  1033,  1032,  1038,  1038,  1043,
    1044,  1045,  1046,  1047,  1048,  1049,  1050,  1051,  1052,  1053,
    1054,  1055,  1056,  1057,  1061,  1059,  1068,  1066,  1073,  1083,
    1077,  1087,  1085,  1089,  1090,  1094,  1095,  1096,  1097,  1098,
    1099,  1100,  1101,  1102,  1103,  1104,  1105,  1113,  1113,  1118,
    1124,  1128,  1128,  1136,  1137,  1141,  1142,  1146,  1152,  1150,
    1166,  1163,  1180,  1177,  1194,  1193,  1202,  1200,  1212,  1211,
    1230,  1228,  1247,  1246,  1255,  1253,  1264,  1264,  1271,  1270,
    1282,  1280,  1293,  1294,  1298,  1301,  1304,  1305,  1306,  1309,
    1310,  1313,  1315,  1318,  1319,  1322,  1323,  1326,  1327,  1331,
    1332,  1337,  1338,  1341,  1342,  1343,  1347,  1348,  1352,  1353,
    1357,  1358,  1362,  1363,  1368,  1369,  1375,  1376,  1377,  1378,
    1381,  1384,  1386,  1389,  1390,  1394,  1396,  1399,  1402,  1405,
    1406,  1409,  1410,  1414,  1420,  1426,  1433,  1435,  1440,  1445,
    1451,  1455,  1459,  1463,  1468,  1473,  1478,  1483,  1489,  1498,
    1503,  1508,  1514,  1516,  1520,  1524,  1529,  1533,  1536,  1539,
    1543,  1547,  1551,  1555,  1560,  1568,  1570,  1573,  1574,  1575,
    1576,  1578,  1580,  1585,  1586,  1589,  1590,  1591,  1595,  1596,
    1598,  1599,  1603,  1605,  1608,  1612,  1618,  1620,  1623,  1623,
    1627,  1626,  1630,  1632,  1635,  1638,  1636,  1653,  1649,  1664,
    1666,  1668,  1670,  1672,  1674,  1676,  1680,  1681,  1682,  1685,
    1691,  1695,  1701,  1704,  1709,  1711,  1716,  1721,  1725,  1726,
    1730,  1731,  1733,  1735,  1741,  1742,  1744,  1748,  1749,  1754,
    1758,  1759,  1763,  1764,  1768,  1770,  1776,  1781,  1782,  1784,
    1788,  1789,  1790,  1791,  1795,  1796,  1797,  1798,  1799,  1800,
    1802,  1807,  1810,  1811,  1815,  1816,  1820,  1821,  1824,  1825,
    1828,  1829,  1832,  1833,  1837,  1838,  1839,  1840,  1841,  1842,
    1843,  1847,  1848,  1851,  1852,  1853,  1856,  1858,  1860,  1861,
    1864,  1866,  1870,  1872,  1876,  1880,  1884,  1889,  1890,  1892,
    1893,  1894,  1895,  1898,  1902,  1903,  1907,  1908,  1912,  1913,
    1914,  1915,  1919,  1923,  1928,  1932,  1936,  1940,  1944,  1949,
    1950,  1951,  1952,  1953,  1957,  1959,  1960,  1961,  1964,  1965,
    1966,  1967,  1968,  1969,  1970,  1971,  1972,  1973,  1974,  1975,
    1976,  1977,  1978,  1979,  1980,  1981,  1982,  1983,  1984,  1985,
    1986,  1987,  1988,  1989,  1990,  1991,  1992,  1993,  1994,  1995,
    1996,  1997,  1998,  1999,  2000,  2001,  2002,  2003,  2004,  2005,
    2006,  2007,  2009,  2010,  2012,  2013,  2015,  2016,  2017,  2018,
    2019,  2020,  2021,  2022,  2023,  2024,  2025,  2026,  2027,  2028,
    2029,  2030,  2031,  2032,  2033,  2034,  2035,  2036,  2037,  2038,
    2039,  2043,  2047,  2052,  2051,  2066,  2064,  2082,  2081,  2100,
    2099,  2118,  2117,  2135,  2135,  2150,  2150,  2168,  2169,  2170,
    2175,  2177,  2181,  2185,  2191,  2195,  2201,  2203,  2207,  2209,
    2213,  2217,  2218,  2222,  2224,  2228,  2230,  2234,  2236,  2240,
    2243,  2248,  2250,  2254,  2257,  2262,  2266,  2270,  2274,  2278,
    2282,  2286,  2290,  2294,  2298,  2302,  2306,  2310,  2314,  2318,
    2322,  2324,  2328,  2330,  2334,  2336,  2340,  2347,  2354,  2356,
    2361,  2362,  2363,  2364,  2365,  2366,  2367,  2368,  2369,  2371,
    2372,  2376,  2377,  2378,  2379,  2383,  2389,  2398,  2411,  2412,
    2415,  2418,  2421,  2422,  2425,  2429,  2432,  2435,  2442,  2443,
    2447,  2448,  2450,  2455,  2456,  2457,  2458,  2459,  2460,  2461,
    2462,  2463,  2464,  2465,  2466,  2467,  2468,  2469,  2470,  2471,
    2472,  2473,  2474,  2475,  2476,  2477,  2478,  2479,  2480,  2481,
    2482,  2483,  2484,  2485,  2486,  2487,  2488,  2489,  2490,  2491,
    2492,  2493,  2494,  2495,  2496,  2497,  2498,  2499,  2500,  2501,
    2502,  2503,  2504,  2505,  2506,  2507,  2508,  2509,  2510,  2511,
    2512,  2513,  2514,  2515,  2516,  2517,  2518,  2519,  2520,  2521,
    2522,  2523,  2524,  2525,  2526,  2527,  2528,  2529,  2530,  2531,
    2532,  2533,  2534,  2535,  2539,  2544,  2545,  2549,  2550,  2551,
    2552,  2554,  2558,  2559,  2570,  2571,  2573,  2575,  2587,  2588,
    2589,  2593,  2594,  2595,  2599,  2600,  2601,  2604,  2606,  2610,
    2611,  2612,  2613,  2615,  2616,  2617,  2618,  2619,  2620,  2621,
    2622,  2623,  2624,  2627,  2632,  2633,  2634,  2636,  2637,  2639,
    2640,  2641,  2642,  2643,  2644,  2645,  2646,  2647,  2649,  2651,
    2653,  2655,  2657,  2658,  2659,  2660,  2661,  2662,  2663,  2664,
    2665,  2666,  2667,  2668,  2669,  2670,  2671,  2672,  2673,  2675,
    2677,  2679,  2681,  2682,  2685,  2686,  2690,  2694,  2696,  2700,
    2701,  2705,  2711,  2714,  2718,  2719,  2720,  2721,  2722,  2723,
    2724,  2729,  2731,  2735,  2736,  2739,  2740,  2744,  2747,  2749,
    2751,  2755,  2756,  2757,  2758,  2761,  2765,  2766,  2767,  2768,
    2772,  2774,  2781,  2782,  2783,  2784,  2789,  2790,  2791,  2792,
    2794,  2795,  2797,  2798,  2799,  2800,  2801,  2805,  2807,  2811,
    2813,  2816,  2819,  2821,  2823,  2826,  2828,  2832,  2834,  2837,
    2840,  2846,  2848,  2851,  2852,  2857,  2860,  2864,  2864,  2869,
    2872,  2873,  2877,  2878,  2882,  2883,  2884,  2888,  2893,  2898,
    2899,  2903,  2908,  2913,  2914,  2918,  2919,  2924,  2926,  2931,
    2942,  2956,  2968,  2983,  2984,  2985,  2986,  2987,  2988,  2989,
    2999,  3008,  3010,  3012,  3016,  3017,  3018,  3019,  3020,  3036,
    3037,  3039,  3041,  3048,  3049,  3050,  3051,  3052,  3053,  3054,
    3055,  3057,  3062,  3066,  3067,  3071,  3074,  3081,  3085,  3094,
    3101,  3109,  3111,  3112,  3116,  3117,  3118,  3120,  3125,  3126,
    3137,  3138,  3139,  3140,  3151,  3154,  3157,  3158,  3159,  3160,
    3171,  3175,  3176,  3177,  3179,  3180,  3181,  3185,  3187,  3190,
    3192,  3193,  3194,  3195,  3198,  3200,  3201,  3205,  3207,  3210,
    3212,  3213,  3214,  3218,  3220,  3223,  3226,  3228,  3230,  3234,
    3235,  3237,  3238,  3244,  3245,  3247,  3257,  3259,  3261,  3264,
    3265,  3266,  3270,  3271,  3272,  3273,  3274,  3275,  3276,  3277,
    3278,  3279,  3280,  3284,  3285,  3289,  3291,  3299,  3301,  3305,
    3309,  3314,  3318,  3326,  3327,  3331,  3332,  3338,  3339,  3348,
    3349,  3357,  3360,  3364,  3367,  3372,  3377,  3379,  3380,  3381,
    3384,  3386,  3392,  3393,  3397,  3398,  3402,  3403,  3407,  3408,
    3411,  3416,  3417,  3421,  3424,  3426,  3430,  3436,  3437,  3438,
    3442,  3446,  3454,  3459,  3471,  3473,  3477,  3480,  3482,  3487,
    3492,  3498,  3501,  3506,  3511,  3513,  3520,  3522,  3525,  3526,
    3529,  3532,  3533,  3538,  3540,  3544,  3550,  3560,  3561
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

#define YYPACT_NINF -1682

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1682)))

#define YYTABLE_NINF -1062

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1062)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1682,   190, -1682, -1682,  5585, 13299, 13299,   -25, 13299, 13299,
   13299, 13299, 11066, 13299, -1682, 13299, 13299, 13299, 13299, 16783,
   16783, 13299, 13299, 13299, 13299, 13299, 13299, 13299, 13299, 11269,
   17448, 13299,    -1,    27, -1682, -1682, -1682,   287, -1682,   234,
   -1682, -1682, -1682,   308, 13299, -1682,    27,    51,   206,   216,
   -1682,    27, 11472, 14576, 11675, -1682, 14329,  4944,   223, 13299,
   15990,    60,    69,   482,    38, -1682, -1682, -1682,   229,   242,
     324,   330, -1682, 14576,   334,   366,   362,   466,   496,   525,
     544, -1682, -1682, -1682, -1682, -1682, 13299,   533,  3889, -1682,
   -1682, 14576, -1682, -1682, -1682, -1682, 14576, -1682, 14576, -1682,
     475,   464, 14576, 14576, -1682,   211, -1682, -1682, 11878, -1682,
   -1682,   474,   478,   543,   543, -1682,   625,   512,   568,   502,
   -1682,   111, -1682,   633, -1682, -1682, -1682, -1682,  4482,   589,
   -1682, -1682,   489,   520,   535,   541,   553,   571,   581,   601,
   15665, -1682, -1682, -1682, -1682,    68,   635,   673,   731,   741,
     750, -1682,   755,   770, -1682,    78,   646, -1682,   692,     5,
   -1682,  1114,   151, -1682, -1682,  3611,   139,   655,   173, -1682,
     143,    79,   659,   200, -1682, -1682,   786, -1682,   697, -1682,
   -1682,   674,   694, -1682, 13299, -1682,   633,   589, 17868,  3913,
   17868, 13299, 17868, 17868, 14859, 14859,   677,  4246, 17868,   822,
   14576,   807,   807,   331,   807, -1682, -1682, -1682, -1682, -1682,
   -1682, -1682, -1682, -1682,    80, 13299,   699, -1682, -1682,   722,
     688,   271,   690,   271,   807,   807,   807,   807,   807,   807,
     807,   807, 16783, 16343,   675,   879,   697, -1682, 13299,   699,
   -1682,   729, -1682,   736,   701, -1682,   154, -1682, -1682, -1682,
     271,   139, -1682, 12081, -1682, -1682, 13299,  9036,   890,   113,
   17868, 10051, -1682, 13299, 13299, 14576, -1682, -1682, 15713,   705,
   -1682, 15783, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682,
   -1682, -1682,  3780, -1682,  3780, -1682, -1682, -1682, -1682, -1682,
   -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682,
   -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682,
   -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682,
   -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682,
   -1682, -1682,    98,    95,   694, -1682, -1682, -1682, -1682,   708,
    3066,   107, -1682, -1682,   746,   894, -1682,   751, 15048, -1682,
     715,   717, 15831, -1682,    -2, 15879, 14224, 14224, 14224, 14576,
   14224,   720,   908,   726, -1682,    67, -1682, 16371,   114, -1682,
     906,   115,   796, -1682,   798, -1682, 16783, 13299, 13299,   733,
     753, -1682, -1682, 16474, 11269, 13299, 13299, 13299, 13299, 13299,
     116,    83,   461, -1682, 13502, 16783,   558, -1682, 14576, -1682,
     243,   512, -1682, -1682, -1682, -1682, 17548,   917,   833, -1682,
   -1682, -1682,   132, 13299,   739,   743, 17868,   744,  2278,   749,
    5788, 13299, -1682,   507,   737,   594,   507,   346,   545, -1682,
   14576,  3780,   754, 10254, 14329, -1682, -1682,  4167, -1682, -1682,
   -1682, -1682, -1682,   633, -1682, -1682, -1682, -1682, -1682, -1682,
   -1682, -1682, -1682, 13299, 13299, 13299, 13299, 12284, 13299, 13299,
   13299, 13299, 13299, 13299, 13299, 13299, 13299, 13299, 13299, 13299,
   13299, 13299, 13299, 13299, 13299, 13299, 13299, 13299, 13299, 13299,
   13299, 17648, 13299, -1682, 13299, 13299, 13299, 13673, 14576, 14576,
   14576, 14576, 14576,  4482,   827,   662,  4740, 13299, 13299, 13299,
   13299, 13299, 13299, 13299, 13299, 13299, 13299, 13299, 13299, -1682,
   -1682, -1682, -1682,  1918, 13299, 13299, -1682, 10254, 10254, 13299,
   13299, 16474,   756,   633, 12487, 15949, -1682, 13299, -1682,   757,
     945,   800,   758,   761, 13825,   271, 12690, -1682, 12893, -1682,
     701,   762,   768,  2754, -1682,    72, 10254, -1682,  3161, -1682,
   -1682, 15997, -1682, -1682, 10457, -1682, 13299, -1682,   876,  9239,
     962,   776, 13486,   961,    84,    76, -1682, -1682, -1682,   795,
   -1682, -1682, -1682,  3780, -1682,  1325,   781,   971, 16268, 14576,
   -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682,   785,
   -1682, -1682,   784,   788,   792,   794,   802,   806,    86,   808,
     811, 16066, 14400, -1682, -1682, 14576, 14576, 13299,   271,    60,
   -1682, 16268,   910, -1682, -1682, -1682,   271,    88,   101,   813,
     814,  2938,   292,   815,   818,   504,   873,   823,   271,   120,
     821, 16952,   817,  1012,  1013,   826,   829,   831,   832, -1682,
    4059, 14576, -1682, -1682,   954,  3552,   459, -1682, -1682, -1682,
     512, -1682, -1682, -1682,   993,   893,   850,   372,   880, 13299,
     903,  1032,   845, -1682,   885, -1682,   164, -1682,  3780,  3780,
    1033,   890,   132, -1682,   853,  1039, -1682,  3780,    93, -1682,
     412,   162, -1682, -1682, -1682, -1682, -1682, -1682, -1682,   771,
    4359, -1682, -1682, -1682, -1682,  1042,   872, -1682, 16783, 13299,
     861,  1054, 17868,  1051, -1682, -1682,   935,  4591, 11660, 18051,
   14859, 18132, 13299, 17820, 18169, 11248, 12668, 13073,  2648, 13669,
   14330, 14330, 14330, 14330,  2695,  2695,  2695,  2695,  2695,   777,
     777,   718,   718,   718,   331,   331,   331, -1682,   807, 17868,
     867,   869, 17000,   875,  1067,     1, 13299,   426,   699,   150,
   -1682, -1682, -1682,  1063,   833, -1682,   633, 16577, -1682, -1682,
   -1682, 14859, 14859, 14859, 14859, 14859, 14859, 14859, 14859, 14859,
   14859, 14859, 14859, 14859, -1682, 13299,   493, -1682,   171, -1682,
     699,   497,   878,  4495,   881,   882,   883,  4669,   130,   888,
   -1682, 17868,  4503, -1682, 14576, -1682,    93,    40, 16783, 17868,
   16783, 17056,   935,    93,   271,   176, -1682,   164,   927,   892,
   13299, -1682,   187, -1682, -1682, -1682,  8833,   661, -1682, -1682,
   17868, 17868,    27, -1682, -1682, -1682, 13299,   988, 16144, 16268,
   14576,  9442,   895,   896, -1682,  1083, 14048,   959, -1682,   938,
   -1682,  1090,   904,  2972,  3780, 16268, 16268, 16268, 16268, 16268,
     911,  1036,  1046,  1052,  1060,  1061,   925, 16268,    -3, -1682,
   -1682, -1682, -1682, -1682, -1682,   199, -1682, 17962, -1682, -1682,
      25, -1682,  5991, 13871,   930, 14400, -1682, 14400, -1682, 14400,
   -1682, 14576, 14576, 14400, -1682, 14400, 14400, 14576, -1682,  1115,
     933, -1682,   437, -1682, -1682, 11050, -1682, 17962,  1120, 16783,
     937, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682,
     955,  1129, 14576, 13871,   940, 16474, 16680,  1127, -1682, 13299,
   -1682, 13299, -1682, 13299, -1682, -1682, -1682, -1682, -1682, -1682,
   -1682, -1682,   942, -1682, 13299, -1682, -1682,  5179, -1682,  3780,
   13871,   944, -1682, -1682, -1682, -1682,  1133,   951, 13299, 17548,
   -1682, -1682, 13673,   953, -1682,  3780, -1682,   956,  6194,  1121,
      70, -1682, -1682,   161,  1918, -1682,  3161, -1682,  3780, -1682,
   -1682,   271, 17868, -1682, 10660, -1682, 16268,    71,   958, 13871,
     893, -1682, -1682, 18169, 13299, -1682, -1682, 13299, -1682, 13299,
   -1682, 11456,   960, 10254,   873,  1125,   893,  3780,  1138,   935,
   14576, 17648,   271, 12065,   963, -1682, -1682,   175,   964, -1682,
   -1682,  1147,  2248,  2248,  4503, -1682, -1682, -1682,  1110,   969,
    1096,  1098,  1099,  1101,  1102,   286,   978,   491, -1682, -1682,
   -1682, -1682, -1682,  1015, -1682, -1682, -1682, -1682,  1167,   980,
     757,   271,   271, 13096,   893,  3161, -1682, -1682, 13283,   682,
      27, 10051, -1682,  6397,   981,  6600,   982, 16144, 16783,   987,
    1048,   271, 17962,  1164, -1682, -1682, -1682, -1682,   574, -1682,
      66,  3780,  1004,  1062,  1040,  3780, 14576,  4042, -1682, -1682,
   -1682,  1193, -1682,  1006,  1042,   691,   691,  1137,  1137, 17208,
    1005,  1198, 16268, 16268, 16268, 16268, 16268, 16268, 17548,  4065,
   15200, 16268, 16268, 16268, 16268, 16025, 16268, 16268, 16268, 16268,
   16268, 16268, 16268, 16268, 16268, 16268, 16268, 16268, 16268, 16268,
   16268, 16268, 16268, 16268, 16268, 16268, 16268, 16268, 16268, 14576,
   -1682, -1682,  1126, -1682, -1682,  1010,  1011,  1014, -1682,  1016,
   -1682, -1682,   463, 16066, -1682,  1017, -1682, 16268,   271, -1682,
   -1682,    90, -1682,   689,  1208, -1682, -1682,   133,  1022,   271,
   10863, 17868, 17104, -1682,  2805, -1682,  5382,   833,  1208, -1682,
     235,   238, -1682, 17868,  1086,  1023, -1682,  1025,  1121, -1682,
    3780,   890,  3780,   309,  1210,  1142,   192, -1682,   699,   195,
   -1682, -1682, 16783, 13299, 17868, 17962,  1029,    71, -1682,  1030,
      71,  1034, 18169, 17868, 17160,  1044, 10254,  1031,  1049,  3780,
    1050,  1045,  3780,   893, -1682,   701,   518, 10254, 13299, -1682,
   -1682, -1682, -1682, -1682, -1682,  1111,  1056,  1223,  1160,  4503,
    4503,  4503,  4503,  4503,  4503,  1095, -1682, 17548,    81,  4503,
   -1682, -1682, -1682, 16783, 17868,  1055, -1682,    27,  1221,  1178,
   10051, -1682, -1682, -1682,  1072, 13299,  1048,   271, 16474, 16144,
    1074, 16268,  6803,   656,  1075, 13299,    85,   290, -1682,  1092,
   -1682,  3780, 14576, -1682,  1136, -1682, -1682,  3293,  1243,  1079,
   16268, -1682, 16268, -1682,  1080,  1077,  1269, 17263,  1078, 17962,
    1273,  1081,  1087,  1089,  1150,  1274,  1097, -1682, -1682, -1682,
   17311,  1103,  1286, 18007, 18095, 10234, 16268, 17916, 12466, 12871,
    5126, 12260, 14506, 14683, 14683, 14683, 14683,  2832,  2832,  2832,
    2832,  2832,  1059,  1059,   691,   691,   691,  1137,  1137,  1137,
    1137, -1682,  1100, -1682,  1108,  1109,  1112,  1117, -1682, -1682,
   17962, 14576,  3780,  3780, -1682,   689, 13871,   742, -1682, 16474,
   -1682, -1682, 14859, 13299,  1113, -1682,  1130,  1144, -1682,   341,
   13299, -1682, -1682, -1682, 13299, -1682, 13299, -1682,   890, -1682,
   -1682,   169,  1284,  1216, 13299, -1682,  1105,   271, 17868,  1121,
    1119, -1682,  1122,    71, 13299, 10254,  1131, -1682, -1682,   833,
   -1682, -1682,  1128,  1132,  1140, -1682,  1135,  4503, -1682,  4503,
   -1682, -1682,  1143,  1139,  1299,  1197,  1148, -1682,  1330,  1149,
    1154,  1155, -1682,  1207,  1163,  1336, -1682, -1682,   271, -1682,
    1302, -1682,  1166, -1682, -1682,  1169,  1170,   136, -1682, -1682,
   17962,  1172,  1174, -1682, 15617, -1682, -1682, -1682, -1682, -1682,
   -1682,  1211,  3780, -1682,  3780, -1682, 17962, 17366, -1682, -1682,
   16268, -1682, 16268, -1682, 16268, -1682, -1682, -1682, -1682, 16268,
   17548, -1682, -1682, 16268, -1682, 16268, -1682, 10640, 16268,  1175,
    7006, -1682, -1682, -1682, -1682,   689, -1682, -1682, -1682, -1682,
     664, 14505, 13871,  1234, -1682,  1648,  1204, 13719, -1682, -1682,
   -1682,   827,  3618,   117,   118,  1177,   833,   662,   142, 17868,
   -1682, -1682, -1682,  1212, 15521, 15569, 17868, -1682,   364,  1362,
    1298, 13299, -1682, 17868, 10254,  1267,  1121,  1659,  1121,  1188,
   17868,  1189, -1682,  1986,  1190,  2165, -1682, -1682,    71, -1682,
   -1682,  1251, -1682, -1682,  4503, -1682,  4503, -1682,  4503, -1682,
   -1682, -1682, -1682,  4503, -1682, 17548, -1682,  2424, -1682,  8833,
   -1682, -1682, -1682, -1682,  9645, -1682, -1682, -1682,  8833,  3780,
   -1682,  1191, 16268, 17414, 17962, 17962, 17962,  1255, 17962, 17469,
   10640, -1682, -1682,   689, 13871, 13871, 14576, -1682,  1378, 15352,
      89, -1682, 14505,   833, 14942, -1682,  1213, -1682,   122,  1194,
     123, -1682, 14858, -1682, -1682, -1682,   124, -1682, -1682,  2318,
   -1682,  1199, -1682,  1312,   633, -1682, 14682, -1682, 14682, -1682,
   -1682,  1382,   827, -1682, 13977, -1682, -1682, -1682, -1682,  1384,
    1316, 13299, -1682, 17868,  1203,  1209,  1121,  1215, -1682,  1267,
    1121, -1682, -1682, -1682, -1682,  2560,  1217,  4503,  1268, -1682,
   -1682, -1682,  1271, -1682,  8833,  9848,  9645, -1682, -1682, -1682,
    8833, -1682, -1682, 17962, 16268, 16268, 16268,  7209,  1219,  1220,
   -1682, 16268, -1682, 13871, -1682, -1682, -1682, -1682, -1682,  3780,
    3835,  1648, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682,
   -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682,
   -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682,
   -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682,
   -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682,
   -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682,
   -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682,
   -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682,
   -1682, -1682, -1682, -1682,   184, -1682,  1204, -1682, -1682, -1682,
   -1682, -1682,    92,   172, -1682,  1394,   125, 15048,  1312,  1399,
   -1682,  3780,   633, -1682, -1682,  1225,  1401, 13299, -1682, 17868,
   -1682,   121,  1226,  3780,   610,  1121,  1215, 14153, -1682,  1121,
   -1682,  4503,  4503, -1682, -1682, -1682, -1682,  7412, 17962, 17962,
   17962, -1682, -1682, -1682, 17962, -1682,  3692,  1410,  1411,  1224,
   -1682, -1682, 16268, 14858, 14858,  1353, -1682,  2318,  2318,   517,
   -1682, -1682, -1682, 16268,  1343, -1682,  1248,  1235,   126, 16268,
   -1682, 14576, -1682, 16268, 17868,  1347, -1682,  1424, -1682,  1425,
   -1682,   152, -1682, -1682, -1682,  1236,   610, -1682,   610, -1682,
   -1682,  7615,  1238,  1322, -1682,  1338,  1278, -1682, -1682,  1342,
    3780,  1264,  3835, -1682, -1682, 17962, -1682, -1682,  1276, -1682,
    1415, -1682, -1682, -1682, -1682, 17962,  1438,   504, -1682, -1682,
   17962,  1259, 17962, -1682,   129,  1262,  7818,  3780, -1682,  3780,
   -1682,  8021, -1682, -1682, -1682,  1258, -1682,  1265,  1282, 14576,
     662,  1280, -1682, -1682, -1682, 16268,  1281,   106, -1682,  1386,
   -1682, -1682, -1682, -1682, -1682, -1682,  8224, -1682, 13871,   930,
   -1682,  1295, 14576,   602, -1682, 17962, -1682,  1275,  1462,   695,
     106, -1682, -1682,  1391, -1682, 13871,  1279, -1682,  1121,   135,
   -1682, -1682, -1682, -1682,  3780, -1682,  1283,  1285,   128, -1682,
    1215,   695,   170,  1121,  1277, -1682,   616,  3780,   395,  1464,
    1398,  1215, -1682, -1682, -1682, -1682,   177,  1470,  1402, 13299,
   -1682,   616,  8427,  8630,   403,  1473,  1408, 13299, -1682, 17868,
   -1682, -1682, -1682,  1480,  1417, 13299, -1682, 17868, 13299, -1682,
   17868, 17868
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,     0,   438,     0,   867,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   958,
     946,     0,   733,     0,   739,   740,   741,    29,   805,   934,
     935,   162,   163,   742,     0,   143,     0,     0,     0,     0,
      30,     0,     0,     0,     0,   197,     0,     0,     0,     0,
       0,     0,   407,   408,   409,   406,   405,   404,     0,     0,
       0,     0,   226,     0,     0,     0,    37,    38,    40,    41,
      39,   746,   748,   749,   743,   744,     0,     0,     0,   750,
     745,     0,   716,    32,    33,    34,    36,    35,     0,   747,
       0,     0,     0,     0,   751,   410,   545,    31,     0,   161,
     133,     0,   734,     0,     0,     4,   123,   125,   804,     0,
     715,     0,     6,   196,     7,     9,     8,    10,     0,     0,
     402,   451,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   449,   923,   924,   527,   521,   522,   523,   524,   525,
     526,   432,   530,     0,   431,   894,   717,   724,     0,   807,
     520,   401,   897,   898,   909,   450,     0,     0,   453,   452,
     895,   896,   893,   930,   933,   510,   806,    11,   407,   408,
     409,     0,     0,    36,     0,   123,   196,     0,   998,   450,
     999,     0,  1001,  1002,   529,   446,     0,   439,   444,     0,
       0,   492,   493,   494,   495,    29,   934,   742,   719,    37,
      38,    40,    41,    39,     0,     0,  1022,   916,   717,     0,
     718,   471,     0,   473,   511,   512,   513,   514,   515,   516,
     517,   519,     0,   962,     0,   814,   729,   216,     0,  1022,
     429,   728,   722,     0,   738,   718,   941,   942,   948,   940,
     730,     0,   430,     0,   732,   518,     0,     0,     0,     0,
     435,     0,   141,   437,     0,     0,   147,   149,     0,     0,
     151,     0,    75,    76,    81,    82,    67,    68,    59,    79,
      90,    91,     0,    62,     0,    66,    74,    72,    93,    85,
      84,    57,    80,   100,   101,    58,    96,    55,    97,    56,
      98,    54,   102,    89,    94,    99,    86,    87,    61,    88,
      92,    53,    83,    69,   103,    77,    70,    60,    47,    48,
      49,    50,    51,    52,    71,   105,   104,   107,    64,    45,
      46,    73,  1069,  1070,    65,  1074,    44,    63,    95,     0,
       0,   123,   106,  1013,  1068,     0,  1071,     0,     0,   153,
       0,     0,     0,   187,     0,     0,     0,     0,     0,     0,
       0,     0,   816,     0,   111,   113,   315,     0,     0,   314,
     320,     0,     0,   227,     0,   230,     0,     0,     0,     0,
    1019,   212,   224,   954,   958,   564,   591,   591,   564,   591,
       0,   983,     0,   753,     0,     0,     0,   981,     0,    16,
       0,   127,   204,   218,   225,   621,   557,     0,  1007,   537,
     539,   541,   871,   438,   451,     0,     0,   449,   450,   452,
       0,     0,   937,   735,     0,   736,     0,     0,     0,   186,
       0,     0,   129,   306,     0,    28,   195,     0,   223,   208,
     222,   407,   410,   196,   403,   176,   177,   178,   179,   180,
     182,   183,   185,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   946,     0,   175,   939,   939,   968,     0,     0,     0,
       0,     0,     0,     0,     0,   400,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   470,
     472,   872,   873,     0,   939,     0,   885,   306,   306,   939,
       0,   954,     0,   196,     0,     0,   155,     0,   869,   864,
     814,     0,   451,   449,     0,   966,     0,   562,   813,   957,
     738,   451,   449,   450,   129,     0,   306,   428,     0,   887,
     731,     0,   133,   266,     0,   544,     0,   158,     0,     0,
     436,     0,     0,     0,     0,     0,   150,   174,   152,  1069,
    1070,  1066,  1067,     0,  1073,  1059,     0,     0,     0,     0,
      78,    43,    65,    42,  1014,   181,   184,   154,   133,     0,
     171,   173,     0,     0,     0,     0,     0,     0,   114,     0,
       0,     0,   815,   112,    18,     0,   108,     0,   316,     0,
     156,     0,     0,   157,   228,   229,  1003,     0,     0,   451,
     449,   450,   453,   452,     0,  1049,   236,     0,   955,     0,
       0,     0,     0,   814,   814,     0,     0,     0,     0,   159,
       0,     0,   752,   982,   805,     0,     0,   980,   810,   979,
     126,     5,    13,    14,     0,   234,     0,     0,   550,     0,
       0,   814,     0,   726,     0,   725,   720,   551,     0,     0,
       0,     0,   871,   133,     0,   816,   870,  1078,   427,   441,
     506,   903,   922,   138,   132,   134,   135,   136,   137,   401,
       0,   528,   808,   809,   124,   814,     0,  1023,     0,     0,
       0,   816,   307,     0,   533,   198,   232,     0,   476,   478,
     477,   489,     0,     0,   509,   474,   475,   479,   481,   480,
     498,   499,   496,   497,   500,   501,   502,   503,   504,   490,
     491,   483,   484,   482,   485,   486,   488,   505,   487,   938,
       0,     0,   972,     0,   814,  1006,     0,  1005,  1022,   900,
     214,   206,   220,     0,  1007,   210,   196,     0,   442,   445,
     447,   455,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   875,     0,   874,   877,   899,   881,
    1022,   878,     0,     0,     0,     0,     0,     0,     0,     0,
    1000,   440,   862,   866,   813,   868,     0,   721,     0,   961,
       0,   960,   232,     0,   721,   945,   944,   930,   933,     0,
       0,   874,   877,   943,   878,   433,   268,   270,   133,   548,
     547,   434,     0,   133,   250,   142,   437,     0,     0,     0,
       0,     0,   262,   262,   148,   814,     0,     0,  1058,     0,
    1055,   814,     0,  1029,     0,     0,     0,     0,     0,   812,
       0,    37,    38,    40,    41,    39,     0,     0,   755,   759,
     760,   761,   762,   763,   765,     0,   754,   131,   803,   764,
    1022,  1072,     0,     0,     0,     0,    21,     0,    22,     0,
      19,     0,   109,     0,    20,     0,     0,     0,   120,   816,
       0,   118,   113,   110,   115,     0,   313,   321,   318,     0,
       0,   992,   997,   994,   993,   996,   995,    12,  1047,  1048,
       0,   814,     0,     0,     0,   954,   951,     0,   561,     0,
     575,   813,   563,   813,   590,   578,   584,   587,   581,   991,
     990,   989,     0,   985,     0,   986,   988,     0,     5,     0,
       0,     0,   615,   616,   624,   623,     0,   449,     0,   813,
     556,   560,     0,     0,  1008,     0,   538,     0,     0,  1036,
     871,   292,  1077,     0,     0,   886,     0,   936,   813,  1025,
    1021,   308,   309,   714,   815,   305,     0,   871,     0,     0,
     234,   535,   200,   508,     0,   598,   599,     0,   596,   813,
     967,     0,     0,   306,   236,     0,   234,     0,     0,   232,
       0,   946,   456,     0,     0,   883,   884,   901,   902,   931,
     932,     0,     0,     0,   850,   821,   822,   823,   830,     0,
      37,    38,    40,    41,    39,     0,     0,   836,   842,   843,
     844,   845,   846,     0,   834,   832,   833,   856,   814,     0,
     864,   965,   964,     0,   234,     0,   888,   737,     0,   272,
       0,     0,   139,     0,     0,     0,     0,     0,     0,     0,
     242,   243,   254,     0,   133,   252,   168,   262,     0,   262,
       0,   813,     0,     0,     0,     0,     0,   813,  1057,  1060,
    1028,   814,  1027,     0,   814,   786,   787,   784,   785,   820,
       0,   814,   812,   568,   593,   593,   568,   593,   559,     0,
       0,   974,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1063,   188,     0,   191,   172,     0,     0,     0,   116,     0,
     121,   122,   114,   815,   119,     0,   317,     0,  1004,   160,
    1020,  1049,  1040,  1044,   235,   237,   327,     0,     0,   952,
       0,   566,     0,   984,     0,    17,     0,  1007,   233,   327,
       0,     0,   721,   553,     0,   727,  1009,     0,  1036,   542,
       0,     0,  1078,     0,   297,   295,   877,   889,  1022,   877,
     890,  1024,     0,     0,   310,   130,     0,   871,   231,     0,
     871,     0,   507,   971,   970,     0,   306,     0,     0,     0,
       0,     0,     0,   234,   202,   738,   876,   306,     0,   826,
     827,   828,   829,   837,   838,   854,     0,   814,     0,   850,
     572,   595,   595,   572,   595,     0,   825,   858,     0,   813,
     861,   863,   865,     0,   959,     0,   876,     0,     0,     0,
       0,   269,   549,   144,     0,   437,   242,   244,   954,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   256,     0,
    1064,     0,     0,  1050,     0,  1056,  1054,   813,     0,     0,
       0,   757,   813,   811,     0,     0,   814,     0,     0,   800,
     814,     0,     0,     0,     0,   814,     0,   766,   801,   802,
     978,     0,   814,   769,   771,   770,     0,     0,   767,   768,
     772,   774,   773,   790,   791,   788,   789,   792,   793,   794,
     795,   796,   781,   782,   776,   777,   775,   778,   779,   780,
     783,  1062,     0,   133,     0,     0,     0,     0,   117,    23,
     319,     0,     0,     0,  1041,  1046,     0,   401,   956,   954,
     443,   448,   454,     0,     0,    15,     0,   401,   627,     0,
       0,   629,   622,   625,     0,   620,     0,  1011,     0,  1037,
     546,     0,   298,     0,     0,   293,     0,   312,   311,  1036,
       0,   327,     0,   871,     0,   306,     0,   928,   327,  1007,
     327,  1010,     0,     0,     0,   457,     0,     0,   840,   813,
     849,   831,     0,     0,   814,     0,     0,   848,   814,     0,
       0,     0,   824,     0,     0,   814,   835,   855,   963,   327,
       0,   133,     0,   265,   251,     0,     0,     0,   241,   164,
     255,     0,     0,   258,     0,   263,   264,   133,   257,  1065,
    1051,     0,     0,  1026,     0,  1076,   819,   818,   756,   576,
     813,   567,     0,   579,   813,   592,   585,   588,   582,     0,
     813,   558,   758,     0,   597,   813,   973,   798,     0,     0,
       0,    24,    25,    26,    27,  1043,  1038,  1039,  1042,   238,
       0,     0,     0,   408,   399,     0,     0,     0,   213,   326,
     328,     0,   398,     0,     0,     0,  1007,   401,     0,   565,
     987,   323,   219,   618,     0,     0,   552,   540,     0,   301,
     291,     0,   294,   300,   306,   532,  1036,   401,  1036,     0,
     969,     0,   927,   401,     0,   401,  1012,   327,   871,   925,
     853,   852,   839,   577,   813,   571,     0,   580,   813,   594,
     586,   589,   583,     0,   841,   813,   857,   401,   133,   271,
     140,   145,   166,   245,     0,   253,   259,   133,   261,     0,
    1052,     0,     0,     0,   570,   799,   555,     0,   977,   976,
     797,   133,   192,  1045,     0,     0,     0,  1015,     0,     0,
       0,   239,     0,  1007,     0,   364,   360,   366,   716,    36,
       0,   354,     0,   359,   363,   376,     0,   374,   379,     0,
     378,     0,   377,     0,   196,   330,     0,   332,     0,   333,
     334,     0,     0,   953,     0,   619,   617,   628,   626,   302,
       0,     0,   289,   299,     0,     0,  1036,  1030,   209,   532,
    1036,   929,   215,   323,   221,   401,     0,     0,     0,   574,
     847,   860,     0,   217,   267,     0,     0,   133,   248,   165,
     260,  1053,  1075,   817,     0,     0,     0,     0,     0,     0,
     426,     0,  1016,     0,   344,   348,   423,   424,   358,     0,
       0,     0,   339,   677,   678,   676,   679,   680,   697,   699,
     698,   668,   640,   638,   639,   658,   673,   674,   634,   645,
     646,   648,   647,   667,   651,   649,   650,   652,   653,   654,
     655,   656,   657,   659,   660,   661,   662,   663,   664,   666,
     665,   635,   636,   637,   641,   642,   644,   682,   683,   687,
     688,   689,   690,   691,   692,   675,   694,   684,   685,   686,
     669,   670,   671,   672,   695,   696,   700,   702,   701,   703,
     704,   681,   706,   705,   708,   710,   709,   643,   713,   711,
     712,   707,   693,   633,   371,   630,     0,   340,   392,   393,
     391,   384,     0,   385,   341,   418,     0,     0,     0,     0,
     422,     0,   196,   205,   322,     0,     0,     0,   290,   304,
     926,     0,     0,     0,     0,  1036,  1030,     0,   211,  1036,
     851,     0,     0,   133,   246,   146,   167,     0,   569,   554,
     975,   190,   342,   343,   421,   240,     0,   814,   814,     0,
     367,   355,     0,     0,     0,   373,   375,     0,     0,   380,
     387,   388,   386,     0,     0,   329,  1017,     0,     0,     0,
     425,     0,   324,     0,   303,     0,   613,   816,   133,   816,
    1032,     0,   394,   133,   199,     0,     0,   207,     0,   573,
     859,     0,     0,   169,   345,   123,     0,   346,   347,     0,
     813,     0,   813,   369,   365,   370,   631,   632,     0,   356,
     389,   390,   382,   383,   381,   419,   416,  1049,   335,   331,
     420,     0,   325,   614,   815,     0,     0,   815,  1031,     0,
    1035,     0,   133,   201,   203,     0,   249,     0,   194,     0,
     401,     0,   361,   368,   372,     0,     0,   871,   337,     0,
     611,   531,   534,  1033,  1034,   395,     0,   247,     0,     0,
     170,   352,     0,   400,   362,   417,  1018,     0,   816,   412,
     871,   612,   536,     0,   193,     0,     0,   351,  1036,   871,
     276,   415,   414,   413,  1078,   411,     0,     0,     0,   350,
    1030,   412,     0,  1036,     0,   349,     0,  1078,     0,   281,
     279,  1030,   133,   396,   133,   336,     0,   282,     0,     0,
     277,     0,     0,     0,     0,   285,   275,     0,   278,   284,
     338,   189,   397,   286,     0,     0,   273,   283,     0,   274,
     288,   287
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1682, -1682, -1682,  -593, -1682, -1682, -1682,    91,   -47,   -41,
     446, -1682,  -266,  -519, -1682, -1682,   358,   240,  1524, -1682,
    2235, -1682,  -489, -1682,    29, -1682, -1682, -1682, -1682, -1682,
   -1682, -1682, -1682, -1682, -1682, -1682,  -417, -1682, -1682,  -146,
     236,    23, -1682, -1682, -1682, -1682, -1682, -1682,    24, -1682,
   -1682, -1682, -1682, -1682, -1682,    30, -1682, -1682,  1018,  1019,
    1024,  -102,  -728,  -894,   510,   573,  -427,   256,  -977, -1682,
    -131, -1682, -1682, -1682, -1682,  -765,    87, -1682, -1682, -1682,
   -1682,  -413, -1682,  -641, -1682,  -440, -1682, -1682,   909, -1682,
    -114, -1682, -1682, -1082, -1682, -1682, -1682, -1682, -1682, -1682,
   -1682, -1682, -1682, -1682,  -149, -1682,   -61, -1682, -1682, -1682,
   -1682, -1682,  -232, -1682,    41,  -981, -1682, -1402,  -446, -1682,
    -147,    65,  -127,  -422, -1682,  -238, -1682, -1682, -1682,    48,
     -24,    11,    44,  -737,   -72, -1682, -1682,    39, -1682,   -20,
   -1682, -1682,    -5,   -38,   -96, -1682, -1682, -1682, -1682, -1682,
   -1682, -1682, -1682, -1682,  -621,  -896, -1682, -1682, -1682, -1682,
   -1682,  1318,  1146, -1682,   440, -1682,   304, -1682, -1682, -1682,
   -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682,
   -1682, -1682, -1682,   340,  -521,  -458, -1682, -1682, -1682, -1682,
   -1682,   368, -1682, -1682, -1682, -1682, -1682, -1682, -1682, -1682,
    -975,  -363,  2426,    35, -1682,   995,  -409, -1682, -1682,  -495,
    3329,  3376, -1682,  -587, -1682, -1682,   447,   155,  -640, -1682,
   -1682,   527,   313,  -474, -1682,   316, -1682, -1682, -1682, -1682,
   -1682,   506, -1682, -1682, -1682,    96,  -922,  -135,  -437,  -434,
   -1682,   578,  -115, -1682, -1682,    36,    43,   611, -1682, -1682,
    1222,   -23, -1682,  -360,    26,  -364,   119,   144, -1682, -1682,
    -480,  1168, -1682, -1682, -1682, -1682, -1682,   710,   572, -1682,
   -1682, -1682,  -358,  -683, -1682,  1123, -1197, -1682,   -73,  -175,
       4,   704, -1682, -1681, -1682,  -338, -1103, -1278,  -326,    99,
   -1682,   402,   479, -1682, -1682, -1682, -1682,   429, -1682,  1165,
   -1150
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   115,   938,   651,   185,  1577,   748,
     361,   362,   363,   364,   889,   890,   891,   117,   118,   119,
     120,   121,   420,   684,   685,   559,   261,  1645,   565,  1554,
    1646,  1898,   874,   354,   588,  1853,  1134,  1333,  1920,   437,
     186,   686,   978,  1201,  1394,   125,   654,   995,   687,   706,
     999,   626,   994,   240,   540,   688,   655,   996,   439,   381,
     403,   128,   980,   941,   914,  1154,  1580,  1260,  1060,  1795,
    1649,   825,  1066,   564,   834,  1068,  1437,   817,  1049,  1052,
    1249,  1927,  1928,   674,   675,   700,   701,   368,   369,   371,
    1614,  1774,  1775,  1347,  1489,  1603,  1768,  1907,  1930,  1806,
    1857,  1858,  1859,  1590,  1591,  1592,  1593,  1808,  1809,  1815,
    1869,  1596,  1597,  1601,  1761,  1762,  1763,  1844,  1965,  1490,
    1491,   187,   130,  1944,  1945,  1766,  1493,  1494,  1495,  1496,
     131,   254,   560,   561,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,  1626,   142,   977,  1200,   143,   671,
     672,   673,   258,   412,   555,   660,   661,  1295,   662,  1296,
     144,   145,   632,   633,  1285,  1286,  1403,  1404,   146,   859,
    1028,   147,   860,  1029,   148,   861,  1030,   149,   862,  1031,
     150,   863,  1032,   635,  1288,  1406,   151,   864,   152,   153,
    1837,   154,   656,  1616,   657,  1170,   946,  1365,  1362,  1754,
    1755,   155,   156,   157,   243,   158,   244,   255,   424,   547,
     159,  1289,  1290,   868,   869,   160,  1090,   969,   603,  1091,
    1035,  1223,  1036,  1407,  1408,  1226,  1227,  1038,  1414,  1415,
    1039,   793,   530,   199,   200,   689,   677,   513,  1186,  1187,
     779,   780,   965,   162,   246,   163,   164,   189,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   740,   250,   251,
     629,   234,   235,   743,   744,  1301,  1302,   396,   397,   932,
     175,   617,   176,   670,   177,   345,  1776,  1827,   382,   432,
     695,   696,  1083,  1784,  1839,  1840,  1181,  1344,   910,  1345,
     911,   912,   840,   841,   842,   346,   347,   871,   574,  1579,
     963
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     188,   190,   444,   192,   193,   194,   195,   197,   198,   342,
     201,   202,   203,   204,   494,   343,   224,   225,   226,   227,
     228,   229,   230,   231,   233,   404,   252,   124,   126,   407,
     408,   957,  1371,   122,   127,   961,   415,   351,  1182,   260,
     522,   788,   666,   663,   257,   802,   665,   268,   667,   271,
     956,   516,   352,  1174,   355,   440,   249,   262,   937,   493,
     444,   975,   266,   816,   544,   242,   247,  1478,  1070,   129,
     417,   998,   737,   248,  1044,  1368,   777,   784,   785,   778,
    1256,   260,   888,   893,   414,   419,  1199,  1357,   259,  1056,
     593,   595,   597,   830,   600,   116,   350,   899,  1663,   872,
     161,  1817,  1210,   416,   -43,  1846,   809,   -78,   832,   -43,
     556,   812,   -78,   589,   813,   548,   -42,  1435,    14,    14,
     434,   -42,   556,   609,   612,   556,  1606,  1608,  1818,   916,
     390,  -357,  1671,  1756,  1824,  1824,   549,  1663,  -600,   916,
     908,   909,   916,   366,   269,   916,   417,   341,  -904,  -906,
    1245,   916,   370,   640,    14,   206,    40, -1022,   374,  1835,
     414,   419,  1416,   514,   380,  -718,  1889,  1909,   375,   605,
     531,   191,  1265,  1266,   511,   512,   590,   533,   431,   416,
      14,   807,  -107,    14,   958,  1129,   525,   402,   881,   380,
       3,   532,   419,   380,   380,   253, -1022,  -107,  1812,  1183,
     542,  -917,  1294,  1820,  1836,  1034,   431,  1508,  1958,   514,
     416,  1298,  1910,  -905,   541,  1974,  1813,   511,   512,   380,
    -912,  -907,  1821,   256,  -947,  1822,   495,   511,   512,  -719,
     422,   606,  -911,   416,   519,  1814,  -919,   393,  -726,  -725,
     123,  -908,  -543,  -607,  1184,  -910,  -950,   263,   551,  1144,
     882,   551,  1509,  1959,  1342,  1343,   641,  -949,   260,   562,
    1975,   367,  -891,  -815,  -916,  -892,  1268,  -815,  -296,  1956,
     519,  1213,  -813,   111,  -915,   833,  1515,  -904,  -906,   573,
    1971,   831,  1428,  1436,  1578,   900,   553,  1664,  1665,  1819,
     558,   529,   515,   -43,   409,  1478,   -78,   707,   901,  1517,
     365,   342,  1263,  -280,  1267,   -42,  1523,   584,  1525,   435,
    -727,   557,   610,   613,   639,  1607,  1609,   917,  1037,  1393,
    -357,  1672,  1757,  1825,  1879,  -296,  1955,  1011,   400,  1053,
    1348,   401,  -815,  1553,  1055,  1185,  1196,  1547,   515,  1613,
     620,  1413,  -905,  1510,  1960,  1166,   993,  -914,   116,  -912,
    -907,  1976,   116,  -947,   619,   623,   563,  -610,  -918,  1100,
    -720,  -911,  -607,   520,   705,   443,  1140,  1141,   444,   518,
    -908,  -921,   260,   416,  -910,  -950,  1235,   789,  -608,   233,
     631,   260,   260,   631,   260,  1666,  -949,   342,   481,   645,
     539,  -891,  1372,   343,  -892,  1503,  1265,  1266,  1101,   520,
     482,  1358,   264,   410,  1363,   511,   512,   430,   197,  1769,
     411,  1770,   265,  1627,  1359,  1629,   690,   404,   753,   754,
     440,   618,   353,   523,   758,   376,   943,  1034,   702,   391,
     634,   634,   385,   634,  1360,  1157,   647,  1364,   377,   583,
     747,   652,   653,  1236,  1893,  1635,  1894,  1619,   708,   709,
     710,   711,   713,   714,   715,   716,   717,   718,   719,   720,
     721,   722,   723,   724,   725,   726,   727,   728,   729,   730,
     731,   732,   733,   734,   735,   736,   760,   738,  1967,   739,
     739,   742,  -609,  1373,  1356,   129,  1983,   342,   518,   905,
    1438,   761,   762,   763,   764,   765,   766,   767,   768,   769,
     770,   771,   772,   773,   394,   395,   405,   249,   676,   739,
     783,   116,   702,   702,   739,   787,   242,   247,  1425,   761,
     378,   694,   791,  1782,   248,   341,   379,  1786,   380,  1189,
     383,   799,  1190,   801,   819,   759,   386,   944,  1620,   605,
    1225,   702,   494,   964,   391,   966,   511,   512,  1872,   820,
     692,   821,   945,  1207,   908,   909,  1380,   206,    40,  1382,
    1370,   391,   384,  1890,  1567,   881,   387,  1873,   423,  1968,
    1874,   806,  -721,   992,  1291,  1262,  1293,  1984,   583,   380,
     751,   380,   380,   380,   380,   666,   663,   493,   824,   665,
     391,   667,  1215,   511,   512,   388,   365,   365,   365,   598,
     365,   430,   895,   372,   776,  1004,   749,  -106,  -919,  1135,
    1000,  1136,   373,  1137,   389,   165,   391,  1139,   642,   394,
     395,   947,  -106,   392,   888,   583,   391,   430,   391,   741,
     221,   223,   781,   426,   982,   647,   394,   395,   650,   811,
     405,   391,  1034,  1034,  1034,  1034,  1034,  1034,   647,  1642,
     116,  -123,  1034,   749,   416,  -123,   123,   429,   782,   936,
     406,   964,   966,   786,   808,   394,   395,   814,  1045,   966,
     870,   436,  -123,   421,  -879,   111,   430,   391,  -882,  1264,
    1265,  1266,  1845,   544,   647,   795,  1848,   445,  1046,  -879,
     393,   394,   395,  -882,   972,  1130,   894,   694,   433,  -880,
      55,   394,   395,   394,   395,  -601,  1524,   983,   441,   179,
     180,    65,    66,    67,  -880,   648,   394,   395,   446,   418,
    1395,   441,   179,   180,    65,    66,    67,   636, -1022,   638,
     756,   931,   933,   447,   666,   663,  1050,  1051,   665,   448,
     667,   991,  1519,  -602,  1125,  1126,  1127,  1507,  1480,   431,
     693,   449,   394,   395,   495,  1225,  1405,  1247,  1248,  1405,
    1128,  1432,  1265,  1266, -1022,  1417,  1386, -1022,   676,   450,
    1003,   478,   479,   480,  1409,   481,  1411,  1396,  1427,   451,
     442,   441,   179,   180,    65,    66,    67,   482,   922,   924,
      14,  1342,  1343,   442,  1952,   418,  1574,  1575,   380,   452,
    1937,  -603,   592,   594,   596,  1048,   599,  1966,  1842,  1843,
    1034,  -604,  1034,  1611,  1963,  1964,   950,  1941,  1942,  1943,
    -605,   260,   425,   427,   428,   484,   418,   475,   476,   477,
     478,   479,   480,  1054,   481,  1950,  1870,  1871,  1866,  1867,
     485,   892,   892,   535,  1470,   486,   482,  1081,  1084,   543,
    1961,   517,   487,   442,  1481,  -913,  -606,  -719,   398,  1482,
    1065,   441,  1483,   180,    65,    66,    67,  1484,   165,  1498,
     521,   528,   165,   666,   663,   526,   482,   665,   537,   667,
     431,   129,   534,  1027,  -917,  1040,   518,  1636,   538,  -717,
     441,    63,    64,    65,    66,    67,   545,   546,   554,   990,
    1667,    72,   488,   567,   575,   747, -1061,   116,   578,  1485,
    1486,   579,  1487,   585,  1161,   586,  1162,   602,   821,   601,
     611,  1063,   116,  1530,   604,  1531,   614,  1214,   615,  1164,
     624,   668,  1549,   442,   625,   669,   678,   129,    55,   691,
     679,   680,  1488,  1173,   490,  1521,   682,  1034,  1558,  1034,
    -128,  1034,   704,   792,   794,   796,  1034,   642,   797,   803,
     124,   126,   442,   116,   643,   804,   122,   127,   649,  1194,
     822,   556,  1138,   694,   826,   829,   573,   843,   608,  1202,
     844,   873,  1203,   875,  1204,  1929,   876,   616,   702,   621,
    1072,   877,   878,   898,   628,   643,  1078,   649,   643,   649,
     649,   879,   129,  1153,   880,   913,   646,   883,  1929,   884,
     902,   903,   906,  1376,   219,   219,   907,  1951,   918,   915,
     920,   921,   923,   129,   934,   939,   940,   249,   116,   925,
     942,   165,   926,   161,   927,   928,   242,   247,  1244,   948,
    -742,   949,   951,   583,   248,   952,   959,   955,   960,   116,
    1034,   968,   123,  1299,   970,   776,   676,   811,   973,  1644,
    1638,  1250,  1639,   974,  1640,   976,  1152,   979,  1650,  1641,
     985,  1175,   986,   676,  1624,   988,   989,   997,  1007,  1008,
    1251,  1005,  1657,   781,   981,   814,  1009,  -723,  1350,  1047,
    1057,   380,  1071,  1067,  1069,  1075,   666,   663,  1076,  1077,
     665,  1079,   667,  1222,  1222,  1027,  1093,  1092,   123,  1122,
    1123,  1124,  1125,  1126,  1127,   892,  1094,   892,   129,   892,
     129,  1098,  1095,   892,  1143,   892,   892,  1142,  1128,  1133,
    1096,  1097,   628,  1145,  1147,  1149,   811,  1150,  1151,  1156,
    1351,  1160,   116,  1169,   116,  1163,   116,  1171,  1172,  1178,
    1480,  1176,  1212,  1180,  1197,  1352,  1206,  1209,  1797,  1217,
    -920,  1218,  1228,  1790,   814,  1229,  1230,  1274,  1231,  1232,
     165,  1233,  1234,   123,  1237,  1238,  1239,  1241,  1261,  1253,
    1255,   666,   663,  1258,  1259,   665,  1270,   667,  1378,   124,
     126,   583,    14,  1240,   123,   122,   127,  1885,  1271,  1888,
    1272,   702,  1277,  1278,  1034,  1034,  1128,  1282,  1281,  1332,
    1334,  1335,   702,  1352,  1336,  1339,  1337,  1346,  1349,   993,
     870,   348,  1366,  1367,  1374,  1375,  1379,   219,  1387,  1381,
    1383,   129,  1399,   441,    63,    64,    65,    66,    67,  1279,
    1385,   222,   222,  1391,    72,   488,  1283,  1397,  1388,  1390,
     260,  1018,  1412,  1421,  1419,  1422,  1481,   116,  1420,  1398,
    1434,  1482,   161,   441,  1483,   180,    65,    66,    67,  1484,
    1424,  1429,  1442,  1433,  1439,  1444,  1445,  1448,  1450,  1423,
    1449,  1453,  1454,  1460,  1456,   489,  1459,   490,  1940,   123,
    1457,   123,  1458,   676,  1462,  1465,   676,  1469,  1511,  1512,
     491,  1514,   492,  1464,  1851,   442,  1471,  1472,  1534,   971,
    1473,  1485,  1486,  1500,  1487,  1474,  1516,  1849,  1850,  1518,
    1027,  1027,  1027,  1027,  1027,  1027,  1526,   129,  1522,  1501,
    1027,  1527,  1529,  1536,  1548,   442,  1528,   217,   217,  1538,
    1532,   116,  1533,  1543,  1502,  1545,  1582,  1559,  1499,  1886,
    1612,  1537,  1540,   116,  1891,  1504,   836,  1541,  1542,  1505,
    1544,  1506,   219,  1441,  1550,   444,  1551,  1552,  1002,  1513,
    1555,   219,  1556,  1595,  1571,  1610,  1621,  1615,   219,  1520,
     702,  1622,  1400,   892,  1625,  1630,  1631,  1637,  1652,  1633,
     219,  1655,  1661,  1670,  1669,  1765,  1771,  1764,  1777,  1778,
    1780,   664,   123,  1916,  1791,  1781,   205,  1792,  1823,  1041,
    1783,  1042,  1492,  1829,  1789,  1833,   837,  1802,  1803,  1860,
    1862,  1868,  1492,  1832,  1864,  1838,  1876,   165,    50,  1877,
    1883,  1878,  1475,  1884,  1887,  1892,  1896,  1897,  1899,  1061,
    -353,  1451,   165,  1497,  1900,  1455,  1902,   571,  1904,   572,
    1461,  1818,  1905,  1497,   222,  1908,  1917,  1466,  1767,  1911,
    1919,  1918,  1924,  1926,   209,   210,   211,   212,   213,  1931,
    1935,  1939,  1938,  1972,  1947,  1973,  1962,  1949,  1969,   676,
    1953,  1970,  1954,   165,  1977,  1978,   182,  1985,  1027,    91,
    1027,  1986,    93,    94,  1988,    95,   183,    97,   123,   838,
    1989,  1338,  1934,  1660,  1208,   577,  1623,   750,  1948,   702,
    1148,   755,  1426,  1168,   752,  1796,   219,  1946,   896,  1787,
     107,  1557,  1811,  1668,  1816,  1980,   628,  1159,  1602,  1957,
    1828,  1583,  1662,  1785,   637,   129,  1292,  1410,  1361,  1284,
    1224,  1401,  1188,   216,   216,  1402,  1242,  1082,   165,  1913,
     217,  1906,   630,  1341,   239,     0,  1276,   703,  1331,  1535,
       0,   116,   495,  1539,     0,     0,     0,     0,     0,   165,
    1546,     0,   341,     0,  1573,     0,     0,     0,  1600,     0,
     239,     0,  1492,  1648,     0,     0,     0,     0,  1492,   222,
    1492,     0,     0,     0,     0,     0,   697,     0,   222,   348,
     622,     0,     0,     0,     0,   222,     0,     0,     0,     0,
       0,     0,  1492,  1497,   129,     0,  1779,   222,     0,  1497,
       0,  1497,     0,   129,   676,  1027,  1831,  1027,     0,  1027,
       0,     0,     0,     0,  1027,     0,     0,     0,     0,     0,
     116,     0,     0,  1497,     0,   116,     0,     0,     0,   116,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   165,     0,   165,  1480,   165,   380,  1061,  1257,
     583,     0,     0,   341,  1794,  1648,     0,     0,     0,  1584,
       0,     0,     0,  1753,     0,   217,     0,     0,     0,     0,
    1760,     0,     0,   219,   217,     0,     0,   341,     0,   341,
    1492,   217,     0,     0,     0,   341,   123,    14,     0,   129,
       0,     0,     0,   217,     0,   129,     0,     0,     0,     0,
     342,     0,   129,     0,     0,     0,  1826,  1604,  1027,   205,
       0,  1497,     0,     0,     0,   116,   116,   116,   835,     0,
       0,   116,     0,   222,     0,     0,     0,     0,   116,     0,
       0,    50,   219,  1922,     0,     0,   216,     0,  1881,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1481,  1834,  1585,     0,     0,  1482,   165,   441,  1483,
     180,    65,    66,    67,  1484,   123,  1586,   209,   210,   211,
     212,   213,  1587,   219,   123,   219,   444,     0,     0,     0,
       0,     0,     0,  1377,     0,     0,   239,     0,   239,   182,
       0,     0,    91,  1588,     0,    93,    94,     0,    95,  1589,
      97,     0,     0,   219,     0,     0,  1485,  1486,     0,  1487,
       0,     0,     0,   953,   954,     0,     0,     0,     0,   217,
       0,     0,   962,   107,     0,     0,     0,     0,  1772,     0,
     442,     0,     0,     0,  1418,     0,     0,     0,   583,  1628,
       0,   165,   129,     0,   239,     0,     0,     0,     0,   628,
    1061,     0,     0,   165,     0,     0,     0,     0,   341,     0,
     123,     0,  1027,  1027,     0,     0,   123,     0,   116,     0,
       0,   216,     0,   123,   219,     0,     0,  1855,     0,     0,
     216,     0,     0,     0,  1753,  1753,     0,   216,  1760,  1760,
     219,   219,     0,     0,     0,     0,   129,     0,     0,   216,
     222,     0,   380,     0,     0,     0,     0,     0,     0,     0,
     216,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   116,     0,   664,     0,     0,     0,     0,     0,
       0,   129,     0,     0,     0,   239,   129,     0,   239,     0,
     628,     0,  1861,  1863,  1979,  1923,     0,     0,     0,     0,
       0,     0,  1987,     0,     0,     0,     0,   116,     0,   222,
    1990,   129,   116,  1991,     0,     0,     0,     0,     0,     0,
    1921,     0,  1480,     0,     0,     0,     0,     0,     0,   205,
       0,   206,    40,   676,     0,   239,     0,   116,   697,   697,
       0,     0,     0,  1936,     0,     0,   217,     0,     0,     0,
     222,    50,   222,     0,     0,     0,   676,     0,     0,     0,
       0,     0,     0,   123,    14,   676,     0,   129,   129,     0,
       0,     0,     0,     0,     0,   216,     0,     0,     0,     0,
     222,     0,   219,   219,     0,     0,     0,   209,   210,   211,
     212,   213,     0,   116,   116,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   217,     0,     0,     0,     0,
       0,   165,     0,   774,     0,    93,    94,   123,    95,   183,
      97,     0,     0,   664,     0,     0,     0,   239,  1481,   239,
       0,     0,   858,  1482,  1167,   441,  1483,   180,    65,    66,
      67,  1484,     0,   107,     0,     0,   217,   775,   217,   111,
    1177,   222,   123,     0,     0,     0,     0,   123,     0,     0,
       0,     0,     0,  1191,     0,   858,     0,   222,   222,     0,
       0,     0,     0,     0,     0,     0,   217,     0,     0,     0,
       0,     0,   123,  1485,  1486,     0,  1487,     0,     0,     0,
     165,     0,  1211,     0,     0,   165,     0,     0,     0,   165,
       0,  1480,     0,     0,     0,     0,     0,   442,     0,     0,
       0,     0,     0,     0,     0,     0,  1632,   219,     0,     0,
       0,     0,   239,   239,     0,     0,     0,     0,     0,     0,
       0,   239,     0,     0,     0,     0,     0,     0,   123,   123,
       0,     0,     0,    14,     0,     0,     0,   217,     0,     0,
       0,     0,   216,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   664,   217,   217,     0,  1269,     0,   219,     0,
    1273,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   219,   219,   165,   165,   165,     0,     0,
       0,   165,     0,     0,     0,     0,     0,     0,   165,     0,
       0,     0,     0,     0,     0,     0,     0,  1481,     0,   222,
     222,   216,  1482,     0,   441,  1483,   180,    65,    66,    67,
    1484,   344,   524,   497,   498,   499,   500,   501,   502,   503,
     504,   505,   506,   507,   508,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   239,     0,     0,     0,
       0,     0,   216,     0,   216,     0,  1219,  1220,  1221,   205,
       0,     0,  1485,  1486,     0,  1487,     0,   509,   510,     0,
       0,     0,     0,     0,   219,  1369,     0,   962,     0,     0,
       0,    50,   216,   858,     0,     0,   442,     0,     0,     0,
     239,     0,     0,     0,     0,  1634,     0,   239,   239,   858,
     858,   858,   858,   858,  1389,   217,   217,  1392,     0,     0,
       0,   858,     0,     0,     0,     0,     0,   209,   210,   211,
     212,   213,     0,     0,     0,     0,     0,   239,     0,   205,
       0,     0,     0,     0,     0,     0,     0,     0,   165,     0,
       0,     0,   511,   512,   222,    93,    94,     0,    95,   183,
      97,    50,     0,   216,     0,     0,     0,     0,     0,     0,
    1480,     0,     0,     0,     0,     0,  1440,   239,     0,   216,
     216,     0,  1191,   107,     0,   218,   218,     0,     0,     0,
       0,     0,     0,     0,     0,   664,   241,   209,   210,   211,
     212,   213,   165,   239,   239,   222,     0,     0,     0,     0,
       0,     0,    14,   216,     0,   681,     0,     0,     0,   239,
     222,   222,     0,  1758,     0,    93,    94,  1759,    95,   183,
      97,     0,   239,     0,     0,     0,     0,   165,     0,     0,
     858,     0,   165,   239,     0,     0,     0,  1476,  1477,     0,
     217,     0,     0,   107,  1599,     0,     0,   344,     0,   344,
       0,   239,     0,     0,     0,   239,     0,   165,     0,     0,
       0,     0,     0,     0,     0,     0,  1481,     0,   239,     0,
     664,  1482,     0,   441,  1483,   180,    65,    66,    67,  1484,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   217,     0,     0,     0,     0,  1480,     0,     0,     0,
       0,   222,     0,     0,     0,   344,   217,   217,     0,     0,
       0,   216,   216,   165,   165,     0,     0,     0,     0,     0,
       0,  1485,  1486,     0,  1487,   239,     0,     0,     0,   239,
       0,   239,     0,     0,     0,     0,     0,  1560,    14,  1561,
       0,     0,     0,     0,     0,   442,   858,   858,   858,   858,
     858,   858,   216,     0,  1643,   858,   858,   858,   858,   858,
     858,   858,   858,   858,   858,   858,   858,   858,   858,   858,
     858,   858,   858,   858,   858,   858,   858,   858,   858,   858,
     858,   858,   858,     0,     0,     0,     0,  1605,   218,     0,
       0,     0,     0,     0,     0,     0,   344,   217,     0,   344,
       0,   858,  1481,     0,     0,     0,     0,  1482,     0,   441,
    1483,   180,    65,    66,    67,  1484,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,   239,   481,   239,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   216,   482,     0,     0,
       0,     0,     0,     0,  1651,     0,     0,  1485,  1486,     0,
    1487,     0,     0,   239,     0,     0,   239,     0, -1062, -1062,
   -1062, -1062, -1062,   473,   474,   475,   476,   477,   478,   479,
     480,   442,   481,   239,   239,   239,   239,   239,   239,     0,
    1788,   216,     0,   239,   482,     0,     0,   216,   524,   497,
     498,   499,   500,   501,   502,   503,   504,   505,   506,   507,
     508,     0,   216,   216,     0,   858,     0,     0,     0,     0,
       0,     0,     0,   218,     0,   239,     0,     0,     0,     0,
       0,   239,   218,     0,   858,     0,   858,     0,   344,   218,
     839,     0,     0,   509,   510,   453,   454,   455,     0,     0,
       0,   218,     0,     0,     0,     0,     0,     0,     0,     0,
     858,     0,   218,     0,  1807,   456,   457,     0,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,     0,   481,     0,     0,     0,   239,   239,     0,     0,
     239,     0,     0,   216,   482, -1062, -1062, -1062, -1062, -1062,
    1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,   511,   512,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1128,     0,   344,   344,     0,     0,   241,     0,     0,
       0,     0,   344,     0,     0,     0,     0,     0,     0,     0,
       0,   239,     0,   239,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1830,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   218,  1841,     0,
       0,   805,   524,   497,   498,   499,   500,   501,   502,   503,
     504,   505,   506,   507,   508,     0,   239,     0,   239,     0,
       0,     0,     0,     0,   858,     0,   858,     0,   858,     0,
       0,     0,     0,   858,   216,     0,     0,   858,     0,   858,
       0,     0,   858,     0,     0,     0,     0,   509,   510,     0,
       0,     0,     0,   282,   865,   239,   239,     0,  1354,   239,
       0,     0,     0,     0,     0,     0,   239,     0,     0,     0,
       0,     0,     0,     0,     0,  1901,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   865,     0,     0,
     284,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1841,   205,  1914,     0,     0,     0,   239,     0,
     239,     0,   239,     0,     0,     0,     0,   239,     0,   216,
       0,  1074,   511,   512,     0,    50,     0,     0,   344,   344,
       0,     0,     0,   239,     0,     0,   858,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   282,   239,   239,
       0,     0,     0,     0,     0,     0,   239,     0,   239,   962,
     569,   209,   210,   211,   212,   213,   570,     0,     0,     0,
       0,     0,   962,     0,   218,     0,     0,     0,     0,     0,
     239,     0,   239,   182,   284,   904,    91,   335,   239,    93,
      94,     0,    95,   183,    97,     0,  1080,   205,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   339,     0,     0,
       0,   239,     0,     0,     0,     0,     0,   107,   340,    50,
       0,     0,     0,     0,   344,     0,     0,   576,   858,   858,
     858,     0,     0,   218,     0,   858,     0,   239,     0,     0,
     344,     0,     0,   239,     0,   239,     0,     0,     0,     0,
       0,     0,     0,   344,   569,   209,   210,   211,   212,   213,
     570,     0,     0,     0,     0,     0,     0,     0,  1033,     0,
       0,     0,     0,     0,   218,     0,   218,   182,     0,     0,
      91,   335,   344,    93,    94,     0,    95,   183,    97,     0,
       0,     0,   205,     0,   206,    40,     0,     0,     0,     0,
       0,   339,     0,     0,   218,   865,     0,     0,     0,     0,
       0,   107,   340,     0,    50,     0,     0,     0,     0,     0,
       0,   865,   865,   865,   865,   865,     0,     0,     0,     0,
       0,     0,     0,   865,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   239,     0,     0,     0,  1132,
     209,   210,   211,   212,   213,     0,   344,   239,     0,     0,
     344,   239,   839,     0,     0,   239,   239,     0,     0,     0,
       0,     0,     0,     0,   282,   218,   774,     0,    93,    94,
     239,    95,   183,    97,     0,     0,   858,     0,     0,  1155,
       0,   218,   218,     0,     0,     0,     0,   858,   220,   220,
       0,     0,     0,   858,     0,     0,   107,   858,     0,   245,
     810,   284,   111,     0,     0,     0,  1155,     0,     0,     0,
       0,     0,     0,     0,   205,   218,     0,     0,     0,     0,
       0,     0,     0,     0,   239,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,   865,     0,     0,  1198,     0,     0,     0,     0,
       0,   239,     0,   239,     0,   344,     0,   344,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   241,     0,   858,
       0,   569,   209,   210,   211,   212,   213,   570,     0,     0,
    1033,     0,   239,     0,   344,     0,     0,   344,     0,     0,
       0,     0,     0,     0,   182,     0,     0,    91,   335,   239,
      93,    94,     0,    95,   183,    97,     0,  1443,   239,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   339,     0,
       0,   239,     0,   218,   218,     0,     0,     0,   107,   340,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   344,     0,     0,     0,
       0,     0,   344,     0,     0,     0,     0,     0,   865,   865,
     865,   865,   865,   865,   218,     0,     0,   865,   865,   865,
     865,   865,   865,   865,   865,   865,   865,   865,   865,   865,
     865,   865,   865,   865,   865,   865,   865,   865,   865,   865,
     865,   865,   865,   865,   865,     0,     0,     0,     0,     0,
       0,   220,   453,   454,   455,     0,     0,     0,     0,     0,
       0,     0,     0,   865,     0,     0,     0,   344,   344,     0,
       0,     0,   456,   457,     0,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,     0,   481,
       0,     0,     0,     0,     0,     0,     0,     0,   218,     0,
       0,   482,     0,     0,     0,   496,   497,   498,   499,   500,
     501,   502,   503,   504,   505,   506,   507,   508,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   282,
       0,     0,     0,     0,     0,  1033,  1033,  1033,  1033,  1033,
    1033,     0,     0,   218,     0,  1033,     0,     0,     0,   218,
     509,   510,     0,     0,     0,     0,     0,   344,     0,   344,
       0,     0,     0,     0,   218,   218,   284,   865,     0,     0,
       0,     0,     0,     0,     0,     0,   220,     0,     0,   205,
       0,     0,     0,     0,     0,   220,   865,     0,   865,     0,
       0,     0,   220,     0,     0,     0,   344,     0,     0,     0,
       0,    50,     0,     0,   220,     0,     0,   344,     0,  -400,
       0,     0,   865,     0,     0,   245,     0,   441,   179,   180,
      65,    66,    67,     0,     0,   511,   512,     0,     0,     0,
       0,     0,   935,     0,     0,     0,   569,   209,   210,   211,
     212,   213,   570,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1479,   205,     0,   218,     0,     0,     0,   182,
       0,     0,    91,   335,     0,    93,    94,     0,    95,   183,
      97,     0,     0,     0,   344,    50,     0,     0,     0,     0,
       0,     0,     0,   339,     0,     0,     0,     0,     0,   442,
     245,   282,     0,   107,   340,     0,     0,   344,     0,     0,
       0,     0,     0,  1033,     0,  1033,     0,     0,     0,     0,
       0,   209,   210,   211,   212,   213,     0,     0,     0,     0,
       0,   344,     0,   344,     0,     0,     0,     0,   284,   344,
     220,     0,     0,   182,     0,     0,    91,     0,     0,    93,
      94,   205,    95,   183,    97,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   865,     0,   865,     0,
     865,     0,     0,    50,     0,   865,   218,   107,     0,   865,
       0,   865,  1854,     0,   865,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   344,     0,     0,   866,  1581,     0,
       0,  1594,     0,    34,    35,    36,     0,     0,   569,   209,
     210,   211,   212,   213,   570,     0,   207,   524,   497,   498,
     499,   500,   501,   502,   503,   504,   505,   506,   507,   508,
     866,   182,     0,     0,    91,   335,     0,    93,    94,     0,
      95,   183,    97,     0,   867,     0,     0,     0,     0,     0,
    1033,     0,  1033,     0,  1033,   339,     0,     0,     0,  1033,
     205,   218,   509,   510,     0,   107,   340,     0,     0,     0,
      81,    82,    83,    84,    85,     0,     0,   897,   865,     0,
       0,   214,    50,     0,     0,     0,     0,    89,    90,     0,
    1658,  1659,     0,     0,     0,     0,   344,     0,     0,     0,
    1594,    99,     0,     0,     0,     0,     0,     0,   344,     0,
       0,     0,   344,     0,     0,   104,     0,   220,   209,   210,
     211,   212,   213,     0,     0,     0,     0,     0,     0,     0,
       0,  1856,     0,     0,     0,     0,     0,   511,   512,     0,
       0,     0,     0,   398,     0,     0,    93,    94,     0,    95,
     183,    97,     0,  1033,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   836,     0,  1102,  1103,  1104,     0,     0,
     865,   865,   865,     0,   107,     0,   220,   865,   399,  1805,
       0,     0,     0,     0,     0,   344,  1105,  1594,     0,  1106,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,
    1127,     0,   344,   205,   344,     0,     0,   220,     0,   220,
       0,     0,     0,   837,  1128,     0,     0,     0,     0,     0,
     205,     0,   929,     0,   930,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   220,   866,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   866,   866,   866,   866,   866,   344,
       0,   209,   210,   211,   212,   213,   866,     0,     0,     0,
       0,     0,   344,     0,     0,     0,     0,     0,   209,   210,
     211,   212,   213,   182,     0,  1062,    91,     0,     0,    93,
      94,     0,    95,   183,    97,     0,  1275,  1033,  1033,     0,
       0,  1085,  1086,  1087,  1088,  1089,    93,    94,   220,    95,
     183,    97,     0,  1099,     0,     0,     0,   107,   865,     0,
       0,     0,     0,     0,   220,   220,     0,     0,   205,   865,
       0,     0,     0,     0,   107,   865,   453,   454,   455,   865,
       0,     0,  1297,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,   456,   457,   245,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,     0,   866,   209,   210,   211,   212,
     213,     0,     0,     0,     0,   482,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     245,   865,     0,     0,    93,    94,     0,    95,   183,    97,
       0,     0,     0,     0,  1933,     0,     0,     0,     0,     0,
       0,     0,  1195,     0,     0,     0,     0,     0,     0,     0,
       0,  1581,   107,   704,     0,     0,     0,     0,     0,   453,
     454,   455,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   527,     0,     0,     0,   220,   220,     0,   456,
     457,     0,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,     0,   481,     0,     0,     0,
       0,   866,   866,   866,   866,   866,   866,   245,   482,     0,
     866,   866,   866,   866,   866,   866,   866,   866,   866,   866,
     866,   866,   866,   866,   866,   866,   866,   866,   866,   866,
     866,   866,   866,   866,   866,   866,   866,   866,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1089,  1287,
       0,     0,  1287,     0,     0,     0,   866,  1300,  1303,  1304,
    1305,  1307,  1308,  1309,  1310,  1311,  1312,  1313,  1314,  1315,
    1316,  1317,  1318,  1319,  1320,  1321,  1322,  1323,  1324,  1325,
    1326,  1327,  1328,  1329,  1330,   453,   454,   455,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   220,     0,  1340,     0,   456,   457,     0,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,     0,   481,  1012,  1013,     0,     0,     0,     0,   967,
       0,     0,     0,   205,   482,     0,   245,     0,     0,     0,
       0,     0,   220,  1014,     0,     0,     0,     0,     0,     0,
       0,  1015,  1016,  1017,   205,    50,     0,   220,   220,     0,
     866,     0,     0,     0,  1018,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,   866,
       0,   866,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   209,   210,   211,   212,   213,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   866,     0,  1430,     0,     0,
       0,  1019,  1020,  1021,  1022,  1023,  1024,   438,     0,    93,
      94,     0,    95,   183,    97,     0,  1446,     0,  1447,  1025,
       0,     0,     0,     0,   182,     0,     0,    91,    92,     0,
      93,    94,   205,    95,   183,    97,     0,   107,   220,   453,
     454,   455,  1467,     0,     0,     0,     0,     0,  1026,     0,
       0,     0,     0,     0,    50,  1006,     0,     0,   107,   456,
     457,     0,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,     0,   481,     0,     0,     0,
     209,   210,   211,   212,   213,     0,     0,     0,   482,     0,
       0,     0,     0,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,    93,    94,
       0,    95,   183,    97,     0,     0,     0,    11,   413,    13,
       0,     0,     0,     0,     0,     0,     0,     0,   757,   866,
       0,   866,     0,   866,     0,     0,   107,   981,   866,   245,
      15,    16,   866,     0,   866,     0,    17,   866,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,  1563,     0,  1564,     0,
    1565,    43,     0,     0,     0,  1566,     0,     0,     0,  1568,
       0,  1569,     0,    50,  1570,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,   178,
     179,   180,    65,    66,    67,     0,     0,    69,    70,  1010,
       0,     0,     0,     0,   245,     0,     0,   181,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,   866,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   182,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   183,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,  1653,     0,
       0,   111,   112,     0,   113,   114,     0,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,     0,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   866,   866,   866,     0,     0,     0,     0,
     866,     0,     0,     0,    15,    16,     0,     0,     0,  1810,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
    1798,  1799,  1800,     0,     0,    43,     0,  1804,     0,     0,
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
     184,   866,   349,     0,     0,   111,   112,     0,   113,   114,
       0,     0,   866,     0,     0,     0,     0,     0,   866,     0,
       0,     0,   866,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,
    1126,  1127,     5,     6,     7,     8,     9,     0,  1865,     0,
       0,  1903,    10,     0,     0,  1128,     0,     0,     0,  1875,
       0,     0,     0,     0,     0,  1880,    11,    12,    13,  1882,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,     0,     0,     0,   866,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,  1925,    50,    51,     0,     0,     0,    52,    53,    54,
      55,    56,    57,    58,     0,    59,    60,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
      88,    89,    90,    91,    92,     0,    93,    94,     0,    95,
      96,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,   103,     0,   104,
     105,   106,     0,     0,   107,   108,     0,   109,   110,  1165,
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
      52,    53,    54,    55,    56,    57,    58,     0,    59,    60,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,    88,    89,    90,    91,    92,     0,    93,
      94,     0,    95,    96,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
     103,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,  1355,   111,   112,     0,   113,   114,     5,     6,
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
       0,     0,     0,    52,    53,    54,    55,    56,    57,    58,
       0,    59,    60,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,    88,    89,    90,    91,
      92,     0,    93,    94,     0,    95,    96,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,   103,     0,   104,   105,   106,     0,     0,
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
     106,     0,     0,   107,   108,     0,   109,   110,   683,   111,
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
     110,  1131,   111,   112,     0,   113,   114,     5,     6,     7,
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
     108,     0,   109,   110,  1179,   111,   112,     0,   113,   114,
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
       0,     0,   107,   108,     0,   109,   110,  1252,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,  1254,    47,     0,    48,     0,
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
       0,    48,     0,    49,  1431,     0,    50,    51,     0,     0,
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
       0,   107,   108,     0,   109,   110,  1572,   111,   112,     0,
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
     105,   106,     0,     0,   107,   108,     0,   109,   110,  1801,
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
      48,  1852,    49,     0,     0,    50,    51,     0,     0,     0,
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
       0,    47,  1895,    48,     0,    49,     0,     0,    50,    51,
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
     106,     0,     0,   107,   108,     0,   109,   110,  1912,   111,
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
     110,  1915,   111,   112,     0,   113,   114,     5,     6,     7,
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
     108,     0,   109,   110,  1932,   111,   112,     0,   113,   114,
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
       0,     0,   107,   108,     0,   109,   110,  1981,   111,   112,
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
    1982,   111,   112,     0,   113,   114,     5,     6,     7,     8,
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
       0,   109,   110,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,   552,     0,
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
       0,   823,     0,     0,     0,     0,     0,     0,     0,     0,
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
      12,    13,     0,     0,  1064,     0,     0,     0,     0,     0,
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
       0,     0,    11,    12,    13,     0,     0,  1647,     0,     0,
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
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
    1793,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,    61,    62,   179,   180,
      65,    66,    67,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,   182,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   183,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   108,     0,   109,   110,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       8,     9,     0,     0,     0,  1105,     0,    10,  1106,  1107,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,
       0,     0,   698,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1128,    15,    16,     0,     0,     0,     0,
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
       0,    93,    94,     0,    95,   183,    97,     0,   699,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     184,     0,     0,     0,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
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
       0,     0,   107,   184,     0,     0,   818,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,  1106,  1107,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,  1123,  1124,  1125,  1126,  1127,     0,     0,  1192,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1128,
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
      95,   183,    97,     0,  1193,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   184,     0,     0,     0,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,   413,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,   104,   105,   106,     0,     0,   107,   108,
     453,   454,   455,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     456,   457,     0,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,     0,   481,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,   482,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,   196,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,   178,   179,   180,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   181,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   182,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   183,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
    1146,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   184,     0,     0,     0,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,     0,   232,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   482,     0,    15,
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
     105,   106,     0,     0,   107,   184,   453,   454,   455,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   456,   457,     0,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,   482,     0,     0,    17,     0,
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
       0,   100,     0,     0,     0,     0,  1205,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   184,     0,
     267,   454,   455,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
     456,   457,     0,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,     0,   481,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,   482,
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
     107,   184,     0,   270,     0,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   413,     0,     0,     0,
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
     106,     0,     0,   107,   108,   453,   454,   455,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   456,   457,     0,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,     0,   481,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,   482,     0,     0,    17,     0,    18,
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
     100,     0,     0,     0,     0,  1216,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   184,   550,     0,
       0,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,  1123,  1124,  1125,  1126,  1127,   712,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1128,
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
     184,     0,     0,     0,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,
    1126,  1127,     0,     0,     0,   757,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1128,     0,    15,    16,     0,
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
       0,     0,     0,    10,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,     0,     0,   798,     0,
       0,     0,     0,     0,     0,     0,     0,   482,     0,     0,
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
       9,     0,     0,     0,     0,     0,    10,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,
    1121,  1122,  1123,  1124,  1125,  1126,  1127,     0,     0,     0,
       0,   800,     0,     0,     0,     0,     0,     0,     0,     0,
    1128,     0,     0,    15,    16,     0,     0,     0,     0,    17,
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
       0,     0,     0,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,     0,     0,     0,  1243,     0,     0,     0,     0,     0,
       0,     0,   482,     0,     0,     0,    15,    16,     0,     0,
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
       0,   107,   184,   453,   454,   455,     0,   111,   112,     0,
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
       0,     0,     0,  1246,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   184,   453,   454,   455,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,   827,     0,    10,   456,   457,     0,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,   482,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,   644,    39,    40,     0,   828,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,   178,   179,   180,    65,    66,    67,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   181,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   182,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   183,    97,     0,   272,   273,    99,   274,
     275,   100,     0,   276,   277,   278,   279,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   184,     0,
       0,   280,   281,   111,   112,     0,   113,   114,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,     0,   481,     0,     0,     0,
     283,     0,     0,     0,     0,     0,     0,     0,   482,     0,
       0,     0,     0,     0,   285,   286,   287,   288,   289,   290,
     291,     0,     0,     0,   205,     0,   206,    40,     0,     0,
       0,     0,     0,     0,     0,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,    50,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     205,   326,     0,   745,   328,   329,   330,     0,     0,     0,
     331,   580,   209,   210,   211,   212,   213,   581,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,   272,   273,
       0,   274,   275,     0,   582,   276,   277,   278,   279,     0,
      93,    94,     0,    95,   183,    97,   336,  1598,   337,     0,
       0,   338,     0,   280,   281,     0,     0,     0,   209,   210,
     211,   212,   213,     0,     0,     0,     0,     0,   107,     0,
       0,     0,   746,     0,   111,     0,     0,     0,     0,     0,
       0,     0,   283,     0,     0,     0,    93,    94,     0,    95,
     183,    97,     0,     0,     0,     0,   285,   286,   287,   288,
     289,   290,   291,     0,     0,     0,   205,     0,   206,    40,
       0,     0,     0,     0,   107,  1599,     0,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,    50,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   205,   326,     0,   327,   328,   329,   330,     0,
       0,     0,   331,   580,   209,   210,   211,   212,   213,   581,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
     272,   273,     0,   274,   275,     0,   582,   276,   277,   278,
     279,     0,    93,    94,     0,    95,   183,    97,   336,     0,
     337,     0,     0,   338,     0,   280,   281,     0,   282,     0,
     209,   210,   211,   212,   213,     0,     0,     0,     0,     0,
     107,     0,     0,     0,   746,     0,   111,     0,     0,     0,
       0,     0,   182,     0,   283,    91,    92,     0,    93,    94,
       0,    95,   183,    97,     0,   284,     0,     0,   285,   286,
     287,   288,   289,   290,   291,     0,     0,     0,   205,     0,
       0,     0,     0,     0,     0,     0,   107,     0,     0,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
      50,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,     0,   326,     0,     0,   328,   329,
     330,     0,     0,     0,   331,   332,   209,   210,   211,   212,
     213,   333,     0,     0,     0,     0,     0,     0,     0,   205,
       0,     0,     0,     0,     0,     0,     0,     0,   334,  1073,
       0,    91,   335,     0,    93,    94,     0,    95,   183,    97,
     336,    50,   337,     0,     0,   338,   272,   273,     0,   274,
     275,     0,   339,   276,   277,   278,   279,     0,     0,     0,
       0,     0,   107,   340,     0,     0,     0,  1773,     0,     0,
       0,   280,   281,     0,   282,     0,     0,   209,   210,   211,
     212,   213,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   182,
     283,     0,    91,     0,     0,    93,    94,     0,    95,   183,
      97,   284,     0,     0,   285,   286,   287,   288,   289,   290,
     291,     0,     0,     0,   205,     0,     0,     0,     0,     0,
       0,     0,     0,   107,     0,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,    50,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
       0,   326,     0,     0,   328,   329,   330,     0,     0,     0,
     331,   332,   209,   210,   211,   212,   213,   333,     0,     0,
       0,     0,     0,     0,     0,   205,     0,     0,     0,     0,
       0,     0,     0,     0,   334,     0,     0,    91,   335,     0,
      93,    94,     0,    95,   183,    97,   336,    50,   337,     0,
       0,   338,   272,   273,     0,   274,   275,     0,   339,   276,
     277,   278,   279,     0,     0,     0,     0,     0,   107,   340,
       0,     0,     0,  1847,     0,     0,     0,   280,   281,     0,
     282,     0,     0,   209,   210,   211,   212,   213,     0, -1062,
   -1062, -1062, -1062,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   283,   481,   359,     0,
       0,    93,    94,     0,    95,   183,    97,   284,     0,   482,
     285,   286,   287,   288,   289,   290,   291,     0,     0,     0,
     205,     0,     0,     0,     0,     0,     0,     0,     0,   107,
       0,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,    50,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,     0,   326,     0,   327,
     328,   329,   330,     0,     0,     0,   331,   332,   209,   210,
     211,   212,   213,   333,     0,     0,     0,     0,     0,     0,
       0,   205,     0,     0,     0,     0,     0,     0,     0,     0,
     334,     0,     0,    91,   335,     0,    93,    94,     0,    95,
     183,    97,   336,    50,   337,     0,     0,   338,   272,   273,
       0,   274,   275,     0,   339,   276,   277,   278,   279,     0,
       0,     0,     0,     0,   107,   340,     0,     0,     0,     0,
       0,     0,     0,   280,   281,     0,   282,     0,     0,   209,
     210,   211,   212,   213,     0,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,
    1126,  1127,   283,     0,   887,     0,     0,    93,    94,     0,
      95,   183,    97,   284,     0,  1128,   285,   286,   287,   288,
     289,   290,   291,     0,     0,     0,   205,     0,     0,     0,
       0,     0,     0,     0,     0,   107,     0,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,    50,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,     0,   326,     0,     0,   328,   329,   330,     0,
       0,     0,   331,   332,   209,   210,   211,   212,   213,   333,
       0,     0,     0,     0,     0,     0,     0,   205,     0,     0,
       0,     0,     0,     0,     0,     0,   334,     0,     0,    91,
     335,     0,    93,    94,     0,    95,   183,    97,   336,    50,
     337,     0,     0,   338,     0,   272,   273,     0,   274,   275,
     339,  1576,   276,   277,   278,   279,     0,     0,     0,     0,
     107,   340,     0,     0,     0,     0,     0,     0,     0,     0,
     280,   281,     0,   282,     0,   209,   210,   211,   212,   213,
       0,     0, -1062, -1062, -1062, -1062,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,   283,
       0,     0,     0,    93,    94,     0,    95,   183,    97,     0,
     284,     0,  1128,   285,   286,   287,   288,   289,   290,   291,
       0,     0,     0,   205,     0,     0,     0,     0,     0,     0,
       0,   107,     0,     0,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,    50,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,     0,
     326,     0,     0,   328,   329,   330,     0,     0,     0,   331,
     332,   209,   210,   211,   212,   213,   333,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   334,     0,     0,    91,   335,     0,    93,
      94,     0,    95,   183,    97,   336,     0,   337,     0,     0,
     338,  1673,  1674,  1675,  1676,  1677,     0,   339,  1678,  1679,
    1680,  1681,     0,     0,     0,     0,     0,   107,   340,     0,
       0,     0,     0,     0,     0,  1682,  1683,  1684,     0,   456,
     457,     0,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,  1685,   481,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   482,  1686,
    1687,  1688,  1689,  1690,  1691,  1692,     0,     0,     0,   205,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1693,  1694,  1695,  1696,  1697,  1698,  1699,  1700,  1701,  1702,
    1703,    50,  1704,  1705,  1706,  1707,  1708,  1709,  1710,  1711,
    1712,  1713,  1714,  1715,  1716,  1717,  1718,  1719,  1720,  1721,
    1722,  1723,  1724,  1725,  1726,  1727,  1728,  1729,  1730,  1731,
    1732,  1733,     0,     0,     0,  1734,  1735,   209,   210,   211,
     212,   213,     0,  1736,  1737,  1738,  1739,  1740,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1741,
    1742,  1743,     0,   205,     0,    93,    94,     0,    95,   183,
      97,  1744,     0,  1745,  1746,     0,  1747,     0,     0,     0,
       0,     0,     0,  1748,  1749,    50,  1750,     0,  1751,  1752,
       0,   272,   273,   107,   274,   275,     0,     0,   276,   277,
     278,   279,     0,     0,     0,     0,     0,  1585,     0,     0,
       0,     0,     0,     0,     0,     0,   280,   281,     0,     0,
    1586,   209,   210,   211,   212,   213,  1587,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   182,     0,   283,    91,    92,     0,    93,
      94,     0,    95,  1589,    97,     0,     0,     0,     0,   285,
     286,   287,   288,   289,   290,   291,     0,     0,     0,   205,
       0,     0,     0,     0,     0,     0,     0,   107,     0,     0,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,    50,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,     0,   326,     0,   327,   328,
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
    1298,   328,   329,   330,     0,     0,     0,   331,   580,   209,
     210,   211,   212,   213,   581,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   272,   273,     0,   274,   275,
       0,   582,   276,   277,   278,   279,     0,    93,    94,     0,
      95,   183,    97,   336,     0,   337,     0,     0,   338,     0,
     280,   281,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   107,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   283,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   285,   286,   287,   288,   289,   290,   291,
       0,     0,     0,   205,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,    50,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,     0,
     326,     0,     0,   328,   329,   330,     0,     0,     0,   331,
     580,   209,   210,   211,   212,   213,   581,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   582,     0,     0,     0,     0,     0,    93,
      94,     0,    95,   183,    97,   336,     0,   337,     0,     0,
     338,   453,   454,   455,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   107,     0,     0,
       0,   456,   457,     0,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,     0,   481,   453,
     454,   455,     0,     0,     0,     0,     0,     0,     0,     0,
     482,     0,     0,     0,     0,     0,     0,     0,     0,   456,
     457,     0,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,     0,   481,   453,   454,   455,
       0,     0,     0,     0,     0,     0,     0,     0,   482,     0,
       0,     0,     0,     0,     0,     0,     0,   456,   457,  1435,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,     0,   481,   453,   454,   455,     0,     0,
       0,     0,     0,     0,     0,     0,   482,     0,     0,     0,
       0,     0,     0,     0,     0,   456,   457,     0,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     480,  1617,   481,   453,   454,   455,     0,     0,     0,     0,
       0,     0,     0,     0,   482,     0,     0,     0,     0,     0,
       0,     0,     0,   456,   457,     0,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,  1618,
     481,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   482,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   453,   454,   455,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   456,   457,  1436,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,   453,   454,   455,     0,     0,     0,     0,     0,     0,
       0,     0,   482,     0,     0,     0,     0,     0,     0,     0,
       0,   456,   457,   483,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,     0,   481,   453,
     454,   455,     0,     0,     0,     0,     0,     0,     0,     0,
     482,     0,     0,     0,     0,     0,     0,     0,     0,   456,
     457,   566,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,     0,   481,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   482,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   453,
     454,   455,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   456,
     457,   568,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,     0,   481,   453,   454,   455,
       0,     0,     0,     0,     0,     0,     0,     0,   482,     0,
       0,     0,     0,     0,     0,     0,     0,   456,   457,   587,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,   480,     0,   481,     0,     0,  1306,     0,     0,
       0,     0,     0,     0,     0,     0,   482,     0,     0,     0,
       0,   205,     0,     0,     0,   845,   846,   591,     0,     0,
       0,   847,     0,   848,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,   849,     0,     0,     0,     0,
       0,   356,   357,    34,    35,    36,   205,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   207,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,   209,
     210,   211,   212,   213,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   790,   205,     0,     0,
       0,   358,     0,     0,   359,     0,     0,    93,    94,     0,
      95,   183,    97,   850,   851,   852,   853,   854,   855,    50,
      81,    82,    83,    84,    85,     0,   360,   885,   886,     0,
       0,   214,  1058,     0,     0,   107,   182,    89,    90,    91,
      92,     0,    93,    94,   815,    95,   183,    97,     0,     0,
       0,    99,     0,     0,     0,   209,   210,   211,   212,   213,
     856,     0,     0,     0,    29,   104,     0,     0,     0,     0,
     107,   857,    34,    35,    36,   205,     0,   206,    40,     0,
     887,     0,     0,    93,    94,   207,    95,   183,    97,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   107,     0,   208,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1059,    75,   209,   210,   211,   212,   213,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
     214,     0,     0,     0,     0,   182,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   183,    97,     0,   845,   846,
      99,     0,     0,     0,   847,     0,   848,     0,     0,     0,
       0,     0,     0,     0,   104,     0,     0,     0,   849,   107,
     215,     0,     0,     0,     0,   111,    34,    35,    36,   205,
       0,     0,     0,   453,   454,   455,     0,     0,     0,   207,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,   456,   457,     0,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,   480,     0,
     481,     0,     0,     0,     0,     0,   850,   851,   852,   853,
     854,   855,   482,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,   214,     0,     0,     0,     0,   182,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   183,
      97,    29,     0,     0,    99,     0,     0,     0,     0,    34,
      35,    36,   205,   856,   206,    40,     0,     0,   104,     0,
       0,     0,   207,   107,   857,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,   536,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     208,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
     209,   210,   211,   212,   213,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,   214,     0,     0,
       0,     0,   182,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   183,    97,    29,     0,     0,    99,     0,     0,
       0,     0,    34,    35,    36,   205,     0,   206,    40,     0,
       0,   104,     0,     0,     0,   207,   107,   215,     0,     0,
     607,     0,   111,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   208,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   627,    75,   209,   210,   211,   212,   213,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
     214,     0,     0,     0,     0,   182,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   183,    97,    29,  1001,     0,
      99,     0,     0,     0,     0,    34,    35,    36,   205,     0,
     206,    40,     0,     0,   104,     0,     0,     0,   207,   107,
     215,     0,     0,     0,     0,   111,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   208,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    75,   209,   210,   211,   212,
     213,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,   214,     0,     0,     0,     0,   182,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   183,    97,
      29,     0,     0,    99,     0,     0,     0,     0,    34,    35,
      36,   205,     0,   206,    40,     0,     0,   104,     0,     0,
       0,   207,   107,   215,     0,     0,     0,     0,   111,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   208,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1158,    75,   209,
     210,   211,   212,   213,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,   214,     0,     0,     0,
       0,   182,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   183,    97,    29,     0,     0,    99,     0,     0,     0,
       0,    34,    35,    36,   205,     0,   206,    40,     0,     0,
     104,     0,     0,     0,   207,   107,   215,     0,     0,     0,
       0,   111,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   208,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,   209,   210,   211,   212,   213,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,   214,
       0,     0,     0,     0,   182,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   183,    97,     0,     0,     0,    99,
       0,     0,   453,   454,   455,     0,     0,     0,     0,     0,
       0,     0,     0,   104,     0,     0,     0,     0,   107,   215,
       0,     0,   456,   457,   111,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,     0,   481,
     453,   454,   455,     0,     0,     0,     0,     0,     0,     0,
       0,   482,     0,     0,     0,     0,     0,     0,     0,     0,
     456,   457,     0,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,     0,   481,     0,     0,
       0,     0,     0,     0,     0,     0,   453,   454,   455,   482,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   456,   457,   919,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,     0,   481,   453,   454,   455,     0,     0,     0,
       0,     0,     0,     0,     0,   482,     0,     0,     0,     0,
       0,     0,     0,     0,   456,   457,   987,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
       0,   481,     0,     0,     0,     0,     0,     0,     0,     0,
     453,   454,   455,   482,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     456,   457,  1043,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,     0,   481,  1102,  1103,
    1104,     0,     0,     0,     0,     0,     0,     0,     0,   482,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1105,
    1353,     0,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,
    1124,  1125,  1126,  1127,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1102,  1103,  1104,     0,  1128,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1105,     0,  1384,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,     0,
       0,  1102,  1103,  1104,     0,     0,     0,     0,     0,     0,
       0,     0,  1128,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1105,     0,  1280,  1106,  1107,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,
    1121,  1122,  1123,  1124,  1125,  1126,  1127,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1102,  1103,  1104,     0,
    1128,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1105,     0,  1452,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,
    1126,  1127,     0,     0,  1102,  1103,  1104,     0,     0,     0,
       0,     0,     0,     0,     0,  1128,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1105,     0,  1463,  1106,  1107,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1102,
    1103,  1104,     0,  1128,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1105,     0,  1562,  1106,  1107,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,
    1123,  1124,  1125,  1126,  1127,     0,    34,    35,    36,   205,
       0,   206,    40,     0,     0,     0,     0,     0,  1128,   207,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1654,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   236,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   237,     0,
       0,     0,     0,     0,     0,     0,     0,   209,   210,   211,
     212,   213,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,   214,  1656,     0,     0,     0,   182,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   183,
      97,     0,     0,     0,    99,     0,    34,    35,    36,   205,
       0,   206,    40,     0,     0,     0,     0,     0,   104,   658,
       0,     0,     0,   107,   238,     0,     0,     0,     0,   111,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   208,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   209,   210,   211,
     212,   213,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,   214,     0,     0,     0,     0,   182,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   183,
      97,     0,     0,     0,    99,     0,    34,    35,    36,   205,
       0,   206,    40,     0,     0,     0,     0,     0,   104,   207,
       0,     0,     0,   107,   659,     0,     0,     0,     0,   111,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   236,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   209,   210,   211,
     212,   213,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,   214,     0,     0,     0,     0,   182,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   183,
      97,     0,     0,     0,    99,     0,     0,     0,     0,     0,
     453,   454,   455,     0,     0,     0,     0,     0,   104,     0,
       0,     0,     0,   107,   238,     0,     0,     0,     0,   111,
     456,   457,   984,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,     0,   481,   453,   454,
     455,     0,     0,     0,     0,     0,     0,     0,     0,   482,
       0,     0,     0,     0,     0,     0,     0,     0,   456,   457,
       0,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,   480,     0,   481,  1102,  1103,  1104,     0,
       0,     0,     0,     0,     0,     0,     0,   482,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1105,  1468,     0,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,
    1126,  1127,  1102,  1103,  1104,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1128,     0,     0,     0,     0,
       0,     0,     0,  1105,     0,     0,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
    1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,  1103,  1104,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1128,     0,     0,     0,     0,     0,     0,  1105,     0,
       0,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,
    1125,  1126,  1127,   455,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1128,     0,     0,     0,
       0,   456,   457,     0,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,  1104,   481,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     482,     0,     0,     0,     0,     0,  1105,     0,     0,  1106,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,
    1127,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   457,  1128,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,     0,   481,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   482,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,     0,   481,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   482
};

static const yytype_int16 yycheck[] =
{
       5,     6,   129,     8,     9,    10,    11,    12,    13,    56,
      15,    16,    17,    18,   161,    56,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    98,    31,     4,     4,   102,
     103,   672,  1182,     4,     4,   675,   108,    57,   960,    44,
     186,   521,   406,   406,    33,   540,   406,    52,   406,    54,
     671,   166,    57,   949,    59,   128,    30,    46,   651,   161,
     187,   701,    51,   552,   239,    30,    30,  1345,   833,     4,
     108,   754,   481,    30,   802,  1178,   513,   517,   518,   513,
    1057,    86,   601,   602,   108,   108,   980,  1169,    44,   826,
     356,   357,   358,     9,   360,     4,    57,     9,     9,   588,
       4,     9,   996,   108,     9,  1786,   546,     9,    32,    14,
       9,   548,    14,   115,   548,   250,     9,    32,    48,    48,
       9,    14,     9,     9,     9,     9,     9,     9,    36,     9,
      86,     9,     9,     9,     9,     9,   251,     9,    70,     9,
      50,    51,     9,    83,    53,     9,   184,    56,    70,    70,
    1044,     9,    83,    70,    48,    83,    84,   160,   120,    38,
     184,   184,    81,    70,    73,   160,    14,    38,   130,   102,
      90,   196,   106,   107,   134,   135,   178,   215,   181,   184,
      48,   545,   181,    48,   673,   160,   191,    96,   102,    98,
       0,   215,   215,   102,   103,   196,   199,   196,    14,    38,
     238,   196,  1098,    31,    83,   792,   181,    38,    38,    70,
     215,   130,    83,    70,   238,    38,    32,   134,   135,   128,
      70,    70,    50,   196,    70,    53,   161,   134,   135,   160,
     111,   164,    70,   238,    70,    51,   196,   157,   160,   160,
       4,    70,     8,    70,    83,    70,    70,   196,   253,   889,
     164,   256,    83,    83,   102,   103,   391,    70,   263,   264,
      83,   201,    70,   193,   196,    70,   200,   197,   197,  1950,
      70,   999,   182,   201,   196,   199,  1379,   199,   199,   181,
    1961,   197,  1259,   198,  1481,   197,   257,   198,   199,   197,
     261,   200,   199,   198,    83,  1573,   198,   443,   197,  1381,
      60,   348,  1067,   197,  1069,   198,  1388,   348,  1390,   198,
     160,   198,   198,   198,   198,   198,   198,   197,   792,  1213,
     198,   198,   198,   198,   198,   193,   198,   197,    88,   818,
     197,    91,   197,   197,   823,   174,   977,  1419,   199,   197,
     378,  1237,   199,   174,   174,   938,   196,   196,   257,   199,
     199,   174,   261,   199,   378,   378,   265,    70,   196,   160,
     160,   199,    70,   199,   437,   129,   885,   886,   495,   196,
     199,   196,   377,   378,   199,   199,    90,   523,    70,   384,
     385,   386,   387,   388,   389,  1582,   199,   434,    57,   394,
     235,   199,    83,   434,   199,    54,   106,   107,   199,   199,
      69,   166,   196,   192,   166,   134,   135,   164,   413,  1606,
     199,  1608,   196,  1516,   179,  1518,   421,   490,   491,   492,
     493,   377,   199,   187,   496,   196,    54,  1014,   433,    83,
     386,   387,    70,   389,   199,   915,    90,   199,   196,   348,
     487,   198,   199,   157,  1846,  1527,  1848,    83,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   496,   482,    83,   484,
     485,   486,    70,   174,  1167,   420,    83,   534,   196,   197,
     200,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   508,   158,   159,   165,   481,   412,   514,
     515,   420,   517,   518,   519,   520,   481,   481,  1255,   524,
     196,   430,   527,  1626,   481,   434,   196,  1630,   437,   966,
     196,   536,   966,   538,   554,   496,    70,   165,   174,   102,
    1014,   546,   689,   678,    83,   680,   134,   135,    31,   554,
     204,   556,   180,   993,    50,    51,  1197,    83,    84,  1200,
    1181,    83,   196,  1841,  1460,   102,    70,    50,    90,   174,
      53,   545,   160,   748,  1095,  1064,  1097,   174,   487,   488,
     489,   490,   491,   492,   493,   949,   949,   689,   559,   949,
      83,   949,  1001,   134,   135,    70,   356,   357,   358,   359,
     360,   164,   607,   121,   513,   780,   487,   181,   196,   875,
     756,   877,   130,   879,    70,     4,    83,   883,   157,   158,
     159,   659,   196,    90,  1143,   534,    83,   164,    83,   485,
      19,    20,   513,    90,   707,    90,   158,   159,   398,   548,
     165,    83,  1229,  1230,  1231,  1232,  1233,  1234,    90,  1545,
     559,   160,  1239,   534,   659,   164,   420,    32,   514,   200,
     196,   796,   797,   519,   545,   158,   159,   548,   803,   804,
     579,    38,   181,   199,   181,   201,   164,    83,   181,   105,
     106,   107,  1785,   858,    90,   530,  1789,   198,   803,   196,
     157,   158,   159,   196,   699,   870,   605,   606,   196,   181,
     111,   158,   159,   158,   159,    70,  1389,   712,   119,   120,
     121,   122,   123,   124,   196,   157,   158,   159,   198,   108,
    1215,   119,   120,   121,   122,   123,   124,   387,   160,   389,
     494,   640,   641,   198,  1098,  1098,    75,    76,  1098,   198,
    1098,   746,  1383,    70,    53,    54,    55,  1368,     6,   181,
     205,   198,   158,   159,   689,  1229,  1230,    75,    76,  1233,
      69,   105,   106,   107,   196,  1239,  1206,   199,   672,   198,
     775,    53,    54,    55,  1232,    57,  1234,  1217,  1258,   198,
     191,   119,   120,   121,   122,   123,   124,    69,   633,   634,
      48,   102,   103,   191,  1944,   184,   132,   133,   707,   198,
     198,    70,   356,   357,   358,   810,   360,  1957,   198,   199,
    1397,    70,  1399,  1496,   198,   199,   661,   122,   123,   124,
      70,   826,   112,   113,   114,    70,   215,    50,    51,    52,
      53,    54,    55,   822,    57,  1938,  1817,  1818,  1813,  1814,
      70,   601,   602,   232,  1333,   199,    69,   843,   844,   238,
    1953,   196,   160,   191,   112,   196,    70,   160,   164,   117,
     831,   119,   120,   121,   122,   123,   124,   125,   257,  1349,
     196,    49,   261,  1237,  1237,   198,    69,  1237,   203,  1237,
     181,   816,   160,   792,   196,   794,   196,  1528,     9,   160,
     119,   120,   121,   122,   123,   124,   160,   196,     8,   744,
    1583,   130,   131,   198,   196,   952,   160,   816,    14,   167,
     168,   160,   170,   198,   919,   198,   921,     9,   923,   199,
      14,   830,   831,  1397,   198,  1399,   130,  1000,   130,   934,
     197,    14,  1421,   191,   181,   102,   197,   872,   111,   202,
     197,   197,   200,   948,   173,  1385,   197,  1534,  1437,  1536,
     196,  1538,   196,   196,     9,   197,  1543,   157,   197,   197,
     937,   937,   191,   872,   392,   197,   937,   937,   396,   974,
      94,     9,   881,   882,   198,    14,   181,   196,   367,   984,
       9,   196,   987,   199,   989,  1907,   198,   376,   993,   378,
     835,   199,   198,    83,   383,   423,   841,   425,   426,   427,
     428,   199,   937,   912,   198,   132,   395,   199,  1930,   198,
     197,   197,   197,  1188,    19,    20,   198,  1939,   197,   196,
     203,     9,     9,   958,    70,    32,   133,  1001,   937,   203,
     180,   420,   203,   937,   203,   203,  1001,  1001,  1043,   136,
     160,     9,   197,   952,  1001,   160,   193,    14,     9,   958,
    1637,     9,   816,  1100,   182,   964,   960,   966,   197,  1548,
    1534,  1050,  1536,     9,  1538,    14,   911,   132,  1557,  1543,
     203,   952,   203,   977,  1514,   200,     9,    14,   197,   197,
    1051,   203,  1571,   964,   196,   966,   203,   160,  1160,   197,
     102,  1000,     9,   198,   198,   136,  1460,  1460,   160,     9,
    1460,   197,  1460,  1012,  1013,  1014,    70,   196,   872,    50,
      51,    52,    53,    54,    55,   875,    70,   877,  1053,   879,
    1055,   196,    70,   883,     9,   885,   886,   887,    69,   199,
      70,    70,   521,   200,    14,   198,  1045,   182,     9,   199,
    1160,    14,  1051,   199,  1053,   203,  1055,    14,   197,   193,
       6,   198,    14,    32,   196,  1160,   196,    32,  1647,   196,
     196,    14,    52,  1637,  1045,   196,    70,  1076,    70,    70,
     559,    70,    70,   937,   196,   160,     9,   197,    14,   198,
     198,  1545,  1545,   196,   136,  1545,   182,  1545,  1193,  1166,
    1166,  1100,    48,  1038,   958,  1166,  1166,  1837,   136,  1839,
     160,  1206,     9,   197,  1791,  1792,    69,     9,   203,    83,
     200,   200,  1217,  1218,   200,   198,   200,     9,   196,   196,
    1129,    56,   136,   198,    14,    83,   197,   232,   197,   199,
     196,  1166,     9,   119,   120,   121,   122,   123,   124,  1084,
     196,    19,    20,   198,   130,   131,  1091,   136,   199,   199,
    1255,    91,   157,    32,   199,    77,   112,  1166,  1247,   203,
    1265,   117,  1166,   119,   120,   121,   122,   123,   124,   125,
     198,   197,   136,   198,   182,    32,   197,   197,     9,  1250,
     203,   203,     9,     9,   203,   171,   136,   173,  1928,  1053,
     203,  1055,   203,  1197,   197,     9,  1200,   197,    14,    83,
     186,   196,   188,   200,  1793,   191,   198,   198,     9,   698,
     198,   167,   168,   200,   170,   198,   197,  1791,  1792,   197,
    1229,  1230,  1231,  1232,  1233,  1234,   198,  1262,   197,   199,
    1239,   199,   197,   136,    32,   191,   196,    19,    20,     9,
     197,  1250,   203,   136,   200,     9,   112,   136,  1353,  1838,
    1497,   203,   203,  1262,  1843,  1360,    31,   203,   203,  1364,
     197,  1366,   367,  1272,   198,  1492,   197,   197,   757,  1374,
     198,   376,   198,   169,   199,   198,    14,   165,   383,  1384,
    1385,    83,  1227,  1143,   117,   197,   197,   136,   197,   199,
     395,   136,    14,   199,   181,    83,    14,   198,    14,    83,
     197,   406,  1166,  1892,   136,   196,    81,   136,    14,   798,
     195,   800,  1347,    14,   197,    14,    91,   198,   198,     9,
       9,    68,  1357,   198,   200,   199,    83,   816,   103,   181,
      83,   196,  1341,     9,     9,   199,   198,   115,   160,   828,
     102,  1286,   831,  1347,   102,  1290,   182,   282,   172,   284,
    1295,    36,    14,  1357,   232,   196,   198,  1302,  1604,   197,
     178,   196,   182,   182,   139,   140,   141,   142,   143,    83,
     175,     9,   197,  1962,    83,  1964,   199,   198,    14,  1383,
     197,    83,   197,   872,    14,    83,   161,    14,  1397,   164,
    1399,    83,   167,   168,    14,   170,   171,   172,  1262,   174,
      83,  1143,  1919,  1576,   994,   340,  1511,   488,  1935,  1514,
     899,   493,  1256,   940,   490,  1646,   521,  1930,   609,  1633,
     195,  1434,  1671,  1584,  1756,  1971,   915,   916,  1487,  1951,
    1768,  1483,  1579,  1629,   388,  1470,  1096,  1233,  1170,  1092,
    1013,  1228,   964,    19,    20,  1229,  1040,   843,   937,  1887,
     232,  1877,   384,  1151,    30,    -1,  1077,   434,  1129,  1404,
      -1,  1470,  1497,  1408,    -1,    -1,    -1,    -1,    -1,   958,
    1415,    -1,  1481,    -1,  1475,    -1,    -1,    -1,  1487,    -1,
      56,    -1,  1517,  1554,    -1,    -1,    -1,    -1,  1523,   367,
    1525,    -1,    -1,    -1,    -1,    -1,   431,    -1,   376,   434,
     378,    -1,    -1,    -1,    -1,   383,    -1,    -1,    -1,    -1,
      -1,    -1,  1547,  1517,  1549,    -1,  1621,   395,    -1,  1523,
      -1,  1525,    -1,  1558,  1528,  1534,  1772,  1536,    -1,  1538,
      -1,    -1,    -1,    -1,  1543,    -1,    -1,    -1,    -1,    -1,
    1549,    -1,    -1,  1547,    -1,  1554,    -1,    -1,    -1,  1558,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1051,    -1,  1053,     6,  1055,  1576,  1057,  1058,
    1579,    -1,    -1,  1582,  1645,  1646,    -1,    -1,    -1,    31,
      -1,    -1,    -1,  1592,    -1,   367,    -1,    -1,    -1,    -1,
    1599,    -1,    -1,   698,   376,    -1,    -1,  1606,    -1,  1608,
    1635,   383,    -1,    -1,    -1,  1614,  1470,    48,    -1,  1644,
      -1,    -1,    -1,   395,    -1,  1650,    -1,    -1,    -1,    -1,
    1767,    -1,  1657,    -1,    -1,    -1,  1767,  1491,  1637,    81,
      -1,  1635,    -1,    -1,    -1,  1644,  1645,  1646,   573,    -1,
      -1,  1650,    -1,   521,    -1,    -1,    -1,    -1,  1657,    -1,
      -1,   103,   757,  1900,    -1,    -1,   232,    -1,  1831,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,  1777,   125,    -1,    -1,   117,  1166,   119,   120,
     121,   122,   123,   124,   125,  1549,   138,   139,   140,   141,
     142,   143,   144,   798,  1558,   800,  1923,    -1,    -1,    -1,
      -1,    -1,    -1,  1192,    -1,    -1,   282,    -1,   284,   161,
      -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,   828,    -1,    -1,   167,   168,    -1,   170,
      -1,    -1,    -1,   668,   669,    -1,    -1,    -1,    -1,   521,
      -1,    -1,   677,   195,    -1,    -1,    -1,    -1,  1612,    -1,
     191,    -1,    -1,    -1,  1243,    -1,    -1,    -1,  1767,   200,
      -1,  1250,  1797,    -1,   340,    -1,    -1,    -1,    -1,  1258,
    1259,    -1,    -1,  1262,    -1,    -1,    -1,    -1,  1787,    -1,
    1644,    -1,  1791,  1792,    -1,    -1,  1650,    -1,  1797,    -1,
      -1,   367,    -1,  1657,   899,    -1,    -1,  1806,    -1,    -1,
     376,    -1,    -1,    -1,  1813,  1814,    -1,   383,  1817,  1818,
     915,   916,    -1,    -1,    -1,    -1,  1851,    -1,    -1,   395,
     698,    -1,  1831,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     406,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1851,    -1,   949,    -1,    -1,    -1,    -1,    -1,
      -1,  1886,    -1,    -1,    -1,   431,  1891,    -1,   434,    -1,
    1349,    -1,  1807,  1808,  1969,  1900,    -1,    -1,    -1,    -1,
      -1,    -1,  1977,    -1,    -1,    -1,    -1,  1886,    -1,   757,
    1985,  1916,  1891,  1988,    -1,    -1,    -1,    -1,    -1,    -1,
    1899,    -1,     6,    -1,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    83,    84,  1907,    -1,   481,    -1,  1916,   843,   844,
      -1,    -1,    -1,  1922,    -1,    -1,   698,    -1,    -1,    -1,
     798,   103,   800,    -1,    -1,    -1,  1930,    -1,    -1,    -1,
      -1,    -1,    -1,  1797,    48,  1939,    -1,  1972,  1973,    -1,
      -1,    -1,    -1,    -1,    -1,   521,    -1,    -1,    -1,    -1,
     828,    -1,  1057,  1058,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,  1972,  1973,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   757,    -1,    -1,    -1,    -1,
      -1,  1470,    -1,   165,    -1,   167,   168,  1851,   170,   171,
     172,    -1,    -1,  1098,    -1,    -1,    -1,   573,   112,   575,
      -1,    -1,   578,   117,   939,   119,   120,   121,   122,   123,
     124,   125,    -1,   195,    -1,    -1,   798,   199,   800,   201,
     955,   899,  1886,    -1,    -1,    -1,    -1,  1891,    -1,    -1,
      -1,    -1,    -1,   968,    -1,   611,    -1,   915,   916,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   828,    -1,    -1,    -1,
      -1,    -1,  1916,   167,   168,    -1,   170,    -1,    -1,    -1,
    1549,    -1,   997,    -1,    -1,  1554,    -1,    -1,    -1,  1558,
      -1,     6,    -1,    -1,    -1,    -1,    -1,   191,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   200,  1192,    -1,    -1,
      -1,    -1,   668,   669,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   677,    -1,    -1,    -1,    -1,    -1,    -1,  1972,  1973,
      -1,    -1,    -1,    48,    -1,    -1,    -1,   899,    -1,    -1,
      -1,    -1,   698,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1237,   915,   916,    -1,  1071,    -1,  1243,    -1,
    1075,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1258,  1259,  1644,  1645,  1646,    -1,    -1,
      -1,  1650,    -1,    -1,    -1,    -1,    -1,    -1,  1657,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,  1057,
    1058,   757,   117,    -1,   119,   120,   121,   122,   123,   124,
     125,    56,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   792,    -1,    -1,    -1,
      -1,    -1,   798,    -1,   800,    -1,    78,    79,    80,    81,
      -1,    -1,   167,   168,    -1,   170,    -1,    59,    60,    -1,
      -1,    -1,    -1,    -1,  1349,  1180,    -1,  1182,    -1,    -1,
      -1,   103,   828,   829,    -1,    -1,   191,    -1,    -1,    -1,
     836,    -1,    -1,    -1,    -1,   200,    -1,   843,   844,   845,
     846,   847,   848,   849,  1209,  1057,  1058,  1212,    -1,    -1,
      -1,   857,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,    -1,   873,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1797,    -1,
      -1,    -1,   134,   135,  1192,   167,   168,    -1,   170,   171,
     172,   103,    -1,   899,    -1,    -1,    -1,    -1,    -1,    -1,
       6,    -1,    -1,    -1,    -1,    -1,  1271,   913,    -1,   915,
     916,    -1,  1277,   195,    -1,    19,    20,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1460,    30,   139,   140,   141,
     142,   143,  1851,   939,   940,  1243,    -1,    -1,    -1,    -1,
      -1,    -1,    48,   949,    -1,   197,    -1,    -1,    -1,   955,
    1258,  1259,    -1,   165,    -1,   167,   168,   169,   170,   171,
     172,    -1,   968,    -1,    -1,    -1,    -1,  1886,    -1,    -1,
     976,    -1,  1891,   979,    -1,    -1,    -1,  1342,  1343,    -1,
    1192,    -1,    -1,   195,   196,    -1,    -1,   282,    -1,   284,
      -1,   997,    -1,    -1,    -1,  1001,    -1,  1916,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,  1014,    -1,
    1545,   117,    -1,   119,   120,   121,   122,   123,   124,   125,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1243,    -1,    -1,    -1,    -1,     6,    -1,    -1,    -1,
      -1,  1349,    -1,    -1,    -1,   340,  1258,  1259,    -1,    -1,
      -1,  1057,  1058,  1972,  1973,    -1,    -1,    -1,    -1,    -1,
      -1,   167,   168,    -1,   170,  1071,    -1,    -1,    -1,  1075,
      -1,  1077,    -1,    -1,    -1,    -1,    -1,  1442,    48,  1444,
      -1,    -1,    -1,    -1,    -1,   191,  1092,  1093,  1094,  1095,
    1096,  1097,  1098,    -1,   200,  1101,  1102,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,
    1126,  1127,  1128,    -1,    -1,    -1,    -1,  1492,   232,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   431,  1349,    -1,   434,
      -1,  1147,   112,    -1,    -1,    -1,    -1,   117,    -1,   119,
     120,   121,   122,   123,   124,   125,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,  1180,    57,  1182,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1192,    69,    -1,    -1,
      -1,    -1,    -1,    -1,  1559,    -1,    -1,   167,   168,    -1,
     170,    -1,    -1,  1209,    -1,    -1,  1212,    -1,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,   191,    57,  1229,  1230,  1231,  1232,  1233,  1234,    -1,
     200,  1237,    -1,  1239,    69,    -1,    -1,  1243,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,  1258,  1259,    -1,  1261,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   367,    -1,  1271,    -1,    -1,    -1,    -1,
      -1,  1277,   376,    -1,  1280,    -1,  1282,    -1,   573,   383,
     575,    -1,    -1,    59,    60,    10,    11,    12,    -1,    -1,
      -1,   395,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1306,    -1,   406,    -1,  1669,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,  1342,  1343,    -1,    -1,
    1346,    -1,    -1,  1349,    69,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,   134,   135,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,   668,   669,    -1,    -1,   481,    -1,    -1,
      -1,    -1,   677,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1397,    -1,  1399,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1771,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   521,  1783,    -1,
      -1,   197,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,  1442,    -1,  1444,    -1,
      -1,    -1,    -1,    -1,  1450,    -1,  1452,    -1,  1454,    -1,
      -1,    -1,    -1,  1459,  1460,    -1,    -1,  1463,    -1,  1465,
      -1,    -1,  1468,    -1,    -1,    -1,    -1,    59,    60,    -1,
      -1,    -1,    -1,    31,   578,  1481,  1482,    -1,   203,  1485,
      -1,    -1,    -1,    -1,    -1,    -1,  1492,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1860,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   611,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1887,    81,  1889,    -1,    -1,    -1,  1534,    -1,
    1536,    -1,  1538,    -1,    -1,    -1,    -1,  1543,    -1,  1545,
      -1,   836,   134,   135,    -1,   103,    -1,    -1,   843,   844,
      -1,    -1,    -1,  1559,    -1,    -1,  1562,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,  1574,  1575,
      -1,    -1,    -1,    -1,    -1,    -1,  1582,    -1,  1584,  1944,
     138,   139,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      -1,    -1,  1957,    -1,   698,    -1,    -1,    -1,    -1,    -1,
    1606,    -1,  1608,   161,    68,   197,   164,   165,  1614,   167,
     168,    -1,   170,   171,   172,    -1,   174,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,
      -1,  1637,    -1,    -1,    -1,    -1,    -1,   195,   196,   103,
      -1,    -1,    -1,    -1,   939,    -1,    -1,   111,  1654,  1655,
    1656,    -1,    -1,   757,    -1,  1661,    -1,  1663,    -1,    -1,
     955,    -1,    -1,  1669,    -1,  1671,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   968,   138,   139,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   792,    -1,
      -1,    -1,    -1,    -1,   798,    -1,   800,   161,    -1,    -1,
     164,   165,   997,   167,   168,    -1,   170,   171,   172,    -1,
      -1,    -1,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,   185,    -1,    -1,   828,   829,    -1,    -1,    -1,    -1,
      -1,   195,   196,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,   845,   846,   847,   848,   849,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   857,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1771,    -1,    -1,    -1,   873,
     139,   140,   141,   142,   143,    -1,  1071,  1783,    -1,    -1,
    1075,  1787,  1077,    -1,    -1,  1791,  1792,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,   899,   165,    -1,   167,   168,
    1806,   170,   171,   172,    -1,    -1,  1812,    -1,    -1,   913,
      -1,   915,   916,    -1,    -1,    -1,    -1,  1823,    19,    20,
      -1,    -1,    -1,  1829,    -1,    -1,   195,  1833,    -1,    30,
     199,    68,   201,    -1,    -1,    -1,   940,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    81,   949,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1860,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,   976,    -1,    -1,   979,    -1,    -1,    -1,    -1,
      -1,  1887,    -1,  1889,    -1,  1180,    -1,  1182,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1001,    -1,  1905,
      -1,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
    1014,    -1,  1918,    -1,  1209,    -1,    -1,  1212,    -1,    -1,
      -1,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,  1935,
     167,   168,    -1,   170,   171,   172,    -1,   174,  1944,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,    -1,
      -1,  1957,    -1,  1057,  1058,    -1,    -1,    -1,   195,   196,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1271,    -1,    -1,    -1,
      -1,    -1,  1277,    -1,    -1,    -1,    -1,    -1,  1092,  1093,
    1094,  1095,  1096,  1097,  1098,    -1,    -1,  1101,  1102,  1103,
    1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,
    1124,  1125,  1126,  1127,  1128,    -1,    -1,    -1,    -1,    -1,
      -1,   232,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1147,    -1,    -1,    -1,  1342,  1343,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1192,    -1,
      -1,    69,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    -1,    -1,  1229,  1230,  1231,  1232,  1233,
    1234,    -1,    -1,  1237,    -1,  1239,    -1,    -1,    -1,  1243,
      59,    60,    -1,    -1,    -1,    -1,    -1,  1442,    -1,  1444,
      -1,    -1,    -1,    -1,  1258,  1259,    68,  1261,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   367,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,   376,  1280,    -1,  1282,    -1,
      -1,    -1,   383,    -1,    -1,    -1,  1481,    -1,    -1,    -1,
      -1,   103,    -1,    -1,   395,    -1,    -1,  1492,    -1,   111,
      -1,    -1,  1306,    -1,    -1,   406,    -1,   119,   120,   121,
     122,   123,   124,    -1,    -1,   134,   135,    -1,    -1,    -1,
      -1,    -1,   200,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1346,    81,    -1,  1349,    -1,    -1,    -1,   161,
      -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,  1559,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,    -1,   191,
     481,    31,    -1,   195,   196,    -1,    -1,  1582,    -1,    -1,
      -1,    -1,    -1,  1397,    -1,  1399,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,
      -1,  1606,    -1,  1608,    -1,    -1,    -1,    -1,    68,  1614,
     521,    -1,    -1,   161,    -1,    -1,   164,    -1,    -1,   167,
     168,    81,   170,   171,   172,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1450,    -1,  1452,    -1,
    1454,    -1,    -1,   103,    -1,  1459,  1460,   195,    -1,  1463,
      -1,  1465,   200,    -1,  1468,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1669,    -1,    -1,   578,  1482,    -1,
      -1,  1485,    -1,    78,    79,    80,    -1,    -1,   138,   139,
     140,   141,   142,   143,   144,    -1,    91,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     611,   161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,   578,    -1,    -1,    -1,    -1,    -1,
    1534,    -1,  1536,    -1,  1538,   185,    -1,    -1,    -1,  1543,
      81,  1545,    59,    60,    -1,   195,   196,    -1,    -1,    -1,
     145,   146,   147,   148,   149,    -1,    -1,   611,  1562,    -1,
      -1,   156,   103,    -1,    -1,    -1,    -1,   162,   163,    -1,
    1574,  1575,    -1,    -1,    -1,    -1,  1771,    -1,    -1,    -1,
    1584,   176,    -1,    -1,    -1,    -1,    -1,    -1,  1783,    -1,
      -1,    -1,  1787,    -1,    -1,   190,    -1,   698,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1806,    -1,    -1,    -1,    -1,    -1,   134,   135,    -1,
      -1,    -1,    -1,   164,    -1,    -1,   167,   168,    -1,   170,
     171,   172,    -1,  1637,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    -1,    10,    11,    12,    -1,    -1,
    1654,  1655,  1656,    -1,   195,    -1,   757,  1661,   199,  1663,
      -1,    -1,    -1,    -1,    -1,  1860,    31,  1671,    -1,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,  1887,    81,  1889,    -1,    -1,   798,    -1,   800,
      -1,    -1,    -1,    91,    69,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    83,    -1,    85,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   828,   829,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   845,   846,   847,   848,   849,  1944,
      -1,   139,   140,   141,   142,   143,   857,    -1,    -1,    -1,
      -1,    -1,  1957,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   161,    -1,   829,   164,    -1,    -1,   167,
     168,    -1,   170,   171,   172,    -1,   174,  1791,  1792,    -1,
      -1,   845,   846,   847,   848,   849,   167,   168,   899,   170,
     171,   172,    -1,   857,    -1,    -1,    -1,   195,  1812,    -1,
      -1,    -1,    -1,    -1,   915,   916,    -1,    -1,    81,  1823,
      -1,    -1,    -1,    -1,   195,  1829,    10,    11,    12,  1833,
      -1,    -1,   197,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    30,    31,   949,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,   976,   139,   140,   141,   142,
     143,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1001,  1905,    -1,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,    -1,  1918,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   976,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1935,   195,   196,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,  1057,  1058,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,  1092,  1093,  1094,  1095,  1096,  1097,  1098,    69,    -1,
    1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,
    1121,  1122,  1123,  1124,  1125,  1126,  1127,  1128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1092,  1093,
      -1,    -1,  1096,    -1,    -1,    -1,  1147,  1101,  1102,  1103,
    1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,
    1124,  1125,  1126,  1127,  1128,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1192,    -1,  1147,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    50,    51,    -1,    -1,    -1,    -1,   200,
      -1,    -1,    -1,    81,    69,    -1,  1237,    -1,    -1,    -1,
      -1,    -1,  1243,    70,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,   103,    -1,  1258,  1259,    -1,
    1261,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,  1280,
      -1,  1282,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1306,    -1,  1261,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,  1280,    -1,  1282,   156,
      -1,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,    -1,
     167,   168,    81,   170,   171,   172,    -1,   195,  1349,    10,
      11,    12,  1306,    -1,    -1,    -1,    -1,    -1,   185,    -1,
      -1,    -1,    -1,    -1,   103,   200,    -1,    -1,   195,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
     139,   140,   141,   142,   143,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,  1450,
      -1,  1452,    -1,  1454,    -1,    -1,   195,   196,  1459,  1460,
      50,    51,  1463,    -1,  1465,    -1,    56,  1468,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,  1450,    -1,  1452,    -1,
    1454,    91,    -1,    -1,    -1,  1459,    -1,    -1,    -1,  1463,
      -1,  1465,    -1,   103,  1468,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,   127,   128,   200,
      -1,    -1,    -1,    -1,  1545,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,  1562,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,   179,
      -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,
     190,   191,   192,    -1,    -1,   195,   196,    -1,  1562,    -1,
      -1,   201,   202,    -1,   204,   205,    -1,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1654,  1655,  1656,    -1,    -1,    -1,    -1,
    1661,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,  1670,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
    1654,  1655,  1656,    -1,    -1,    91,    -1,  1661,    -1,    -1,
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
     196,  1812,   198,    -1,    -1,   201,   202,    -1,   204,   205,
      -1,    -1,  1823,    -1,    -1,    -1,    -1,    -1,  1829,    -1,
      -1,    -1,  1833,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,     3,     4,     5,     6,     7,    -1,  1812,    -1,
      -1,  1862,    13,    -1,    -1,    69,    -1,    -1,    -1,  1823,
      -1,    -1,    -1,    -1,    -1,  1829,    27,    28,    29,  1833,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    -1,    -1,    -1,  1905,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,
      91,    92,    93,    94,    -1,    96,    -1,    98,    -1,   100,
      -1,  1905,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,   112,   113,   114,    -1,   116,   117,   118,   119,   120,
     121,   122,   123,   124,    -1,   126,   127,   128,   129,   130,
     131,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,   153,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,   173,    -1,    -1,   176,    -1,    -1,   179,    -1,
      -1,    -1,    -1,    -1,   185,   186,    -1,   188,    -1,   190,
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
     108,   109,   110,   111,   112,   113,   114,    -1,   116,   117,
     118,   119,   120,   121,   122,   123,   124,    -1,   126,   127,
     128,   129,   130,   131,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,   153,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,   173,    -1,    -1,   176,    -1,
      -1,   179,    -1,    -1,    -1,    -1,    -1,   185,   186,    -1,
     188,    -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,
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
      -1,    -1,    -1,   108,   109,   110,   111,   112,   113,   114,
      -1,   116,   117,   118,   119,   120,   121,   122,   123,   124,
      -1,   126,   127,   128,   129,   130,   131,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,   153,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,   173,    -1,
      -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,
     185,   186,    -1,   188,    -1,   190,   191,   192,    -1,    -1,
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
      -1,    91,    92,    93,    94,    95,    96,    -1,    98,    -1,
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
      -1,    98,    -1,   100,   101,    -1,   103,   104,    -1,    -1,
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
      98,    99,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,
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
      -1,    96,    97,    98,    -1,   100,    -1,    -1,   103,   104,
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
      -1,    -1,    27,    28,    29,    -1,    -1,    32,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,
      92,    93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,
      -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,   111,
      -1,   113,   114,    -1,   116,    -1,   118,   119,   120,   121,
     122,   123,   124,    -1,   126,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,   153,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,   176,    -1,    -1,   179,    -1,    -1,
      -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,
     192,    -1,    -1,   195,   196,    -1,   198,   199,    -1,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
       6,     7,    -1,    -1,    -1,    31,    -1,    13,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    50,    51,    -1,    -1,    -1,    -1,
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
      -1,   167,   168,    -1,   170,   171,   172,    -1,   174,    -1,
     176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,
      -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,
     196,    -1,    -1,    -1,    -1,   201,   202,    -1,   204,   205,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
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
      -1,    -1,   195,   196,    -1,    -1,   199,    -1,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
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
     170,   171,   172,    -1,   174,    -1,   176,    -1,    -1,   179,
      -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,    -1,    -1,
     190,   191,   192,    -1,    -1,   195,   196,    -1,    -1,    -1,
      -1,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,   108,    -1,    -1,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,
     124,    -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,
      -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    -1,
      -1,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,
     200,   185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,
      -1,   195,   196,    -1,    -1,    -1,    -1,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    50,
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
     191,   192,    -1,    -1,   195,   196,    10,    11,    12,    -1,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    69,    -1,    -1,    56,    -1,
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
      -1,   179,    -1,    -1,    -1,    -1,   200,   185,    -1,    -1,
      -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,
     198,    11,    12,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    69,
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
     195,   196,    -1,   198,    -1,    -1,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,
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
     192,    -1,    -1,   195,   196,    10,    11,    12,    -1,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    69,    -1,    -1,    56,    -1,    58,
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
     179,    -1,    -1,    -1,    -1,   200,   185,    -1,    -1,    -1,
      -1,   190,   191,   192,    -1,    -1,   195,   196,   197,    -1,
      -1,    -1,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
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
      54,    55,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
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
      -1,    -1,   195,   196,    -1,    -1,    -1,    -1,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    38,    -1,
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
       7,    -1,    -1,    -1,    -1,    -1,    13,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
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
      -1,    -1,    -1,    -1,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    50,    51,    -1,    -1,
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
     191,   192,    -1,    -1,   195,   196,    10,    11,    12,    -1,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    27,    -1,    13,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    69,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,   102,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,    -1,   145,   146,   147,
     148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,
      -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,     3,     4,   176,     6,
       7,   179,    -1,    10,    11,    12,    13,   185,    -1,    -1,
      -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,    -1,
      -1,    28,    29,   201,   202,    -1,   204,   205,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      81,   128,    -1,   130,   131,   132,   133,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,    -1,   161,    10,    11,    12,    13,    -1,
     167,   168,    -1,   170,   171,   172,   173,   128,   175,    -1,
      -1,   178,    -1,    28,    29,    -1,    -1,    -1,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,   195,    -1,
      -1,    -1,   199,    -1,   201,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    57,    -1,    -1,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    83,    84,
      -1,    -1,    -1,    -1,   195,   196,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    81,   128,    -1,   130,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,   144,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
       3,     4,    -1,     6,     7,    -1,   161,    10,    11,    12,
      13,    -1,   167,   168,    -1,   170,   171,   172,   173,    -1,
     175,    -1,    -1,   178,    -1,    28,    29,    -1,    31,    -1,
     139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,
     195,    -1,    -1,    -1,   199,    -1,   201,    -1,    -1,    -1,
      -1,    -1,   161,    -1,    57,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    -1,    68,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,    -1,    -1,   131,   132,
     133,    -1,    -1,    -1,   137,   138,   139,   140,   141,   142,
     143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,    91,
      -1,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
     173,   103,   175,    -1,    -1,   178,     3,     4,    -1,     6,
       7,    -1,   185,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,   195,   196,    -1,    -1,    -1,   200,    -1,    -1,
      -1,    28,    29,    -1,    31,    -1,    -1,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,
      57,    -1,   164,    -1,    -1,   167,   168,    -1,   170,   171,
     172,    68,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   195,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,    -1,    -1,   131,   132,   133,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,   173,   103,   175,    -1,
      -1,   178,     3,     4,    -1,     6,     7,    -1,   185,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,   195,   196,
      -1,    -1,    -1,   200,    -1,    -1,    -1,    28,    29,    -1,
      31,    -1,    -1,   139,   140,   141,   142,   143,    -1,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    57,    57,   164,    -1,
      -1,   167,   168,    -1,   170,   171,   172,    68,    -1,    69,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,    -1,   130,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,   173,   103,   175,    -1,    -1,   178,     3,     4,
      -1,     6,     7,    -1,   185,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    29,    -1,    31,    -1,    -1,   139,
     140,   141,   142,   143,    -1,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    57,    -1,   164,    -1,    -1,   167,   168,    -1,
     170,   171,   172,    68,    -1,    69,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   195,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,    -1,    -1,   131,   132,   133,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,   144,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,   173,   103,
     175,    -1,    -1,   178,    -1,     3,     4,    -1,     6,     7,
     185,   186,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
     195,   196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    29,    -1,    31,    -1,   139,   140,   141,   142,   143,
      -1,    -1,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    57,
      -1,    -1,    -1,   167,   168,    -1,   170,   171,   172,    -1,
      68,    -1,    69,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   195,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,    -1,    -1,   131,   132,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   161,    -1,    -1,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,   173,    -1,   175,    -1,    -1,
     178,     3,     4,     5,     6,     7,    -1,   185,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,   195,   196,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    57,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,
     162,   163,    -1,    81,    -1,   167,   168,    -1,   170,   171,
     172,   173,    -1,   175,   176,    -1,   178,    -1,    -1,    -1,
      -1,    -1,    -1,   185,   186,   103,   188,    -1,   190,   191,
      -1,     3,     4,   195,     6,     7,    -1,    -1,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,   125,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    29,    -1,    -1,
     138,   139,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   161,    -1,    57,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,
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
     130,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,   161,    10,    11,    12,    13,    -1,   167,   168,    -1,
     170,   171,   172,   173,    -1,   175,    -1,    -1,   178,    -1,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,    -1,
     128,    -1,    -1,   131,   132,   133,    -1,    -1,    -1,   137,
     138,   139,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   161,    -1,    -1,    -1,    -1,    -1,   167,
     168,    -1,   170,   171,   172,   173,    -1,   175,    -1,    -1,
     178,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,   200,    57,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,   200,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,   198,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,   198,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,   198,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,   198,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   198,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    50,    51,   198,    -1,    -1,
      -1,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    70,    -1,    -1,    -1,    -1,
      -1,   111,   112,    78,    79,    80,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,   139,
     140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   197,    81,    -1,    -1,
      -1,   161,    -1,    -1,   164,    -1,    -1,   167,   168,    -1,
     170,   171,   172,   138,   139,   140,   141,   142,   143,   103,
     145,   146,   147,   148,   149,    -1,   186,   111,   112,    -1,
      -1,   156,    38,    -1,    -1,   195,   161,   162,   163,   164,
     165,    -1,   167,   168,   197,   170,   171,   172,    -1,    -1,
      -1,   176,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     185,    -1,    -1,    -1,    70,   190,    -1,    -1,    -1,    -1,
     195,   196,    78,    79,    80,    81,    -1,    83,    84,    -1,
     164,    -1,    -1,   167,   168,    91,   170,   171,   172,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   195,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,    -1,   145,
     146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,
     156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    50,    51,
     176,    -1,    -1,    -1,    56,    -1,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   190,    -1,    -1,    -1,    70,   195,
     196,    -1,    -1,    -1,    -1,   201,    78,    79,    80,    81,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,    69,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    70,    -1,    -1,   176,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,   185,    83,    84,    -1,    -1,   190,    -1,
      -1,    -1,    91,   195,   196,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,    -1,   145,   146,   147,   148,
     149,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,
      -1,    -1,   161,   162,   163,   164,   165,    -1,   167,   168,
      -1,   170,   171,   172,    70,    -1,    -1,   176,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    -1,    83,    84,    -1,
      -1,   190,    -1,    -1,    -1,    91,   195,   196,    -1,    -1,
     199,    -1,   201,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,    -1,   145,
     146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,
     156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    70,    71,    -1,
     176,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,   190,    -1,    -1,    -1,    91,   195,
     196,    -1,    -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,
     143,    -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,
      -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,
     163,   164,   165,    -1,   167,   168,    -1,   170,   171,   172,
      70,    -1,    -1,   176,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,   190,    -1,    -1,
      -1,    91,   195,   196,    -1,    -1,    -1,    -1,   201,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,    -1,   145,   146,   147,   148,   149,
      -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    70,    -1,    -1,   176,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
     190,    -1,    -1,    -1,    91,   195,   196,    -1,    -1,    -1,
      -1,   201,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   190,    -1,    -1,    -1,    -1,   195,   196,
      -1,    -1,    30,    31,   201,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   136,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,   136,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,   136,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
     136,    -1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,   136,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,   136,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,   136,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,    -1,   136,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,   136,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    69,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     136,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,   156,   136,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,   176,    -1,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,    -1,   190,    91,
      -1,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,   201,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,   176,    -1,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,    -1,   190,    91,
      -1,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,   201,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,    -1,   145,   146,   147,   148,   149,    -1,    -1,
      -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,   161,
     162,   163,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,   190,    -1,
      -1,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,   201,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    32,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    12,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    69,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
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
     178,   198,   216,   218,   216,   218,   216,   218,   223,   216,
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
     213,   503,   228,   196,   238,   199,   198,   199,   198,   199,
     198,   102,   164,   199,   198,   111,   112,   164,   219,   220,
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
     485,   200,   408,   199,   242,   218,   218,   218,   213,   218,
     219,   219,   223,     9,   424,   200,   200,    14,   453,   198,
     182,     9,   423,   213,   271,   408,   199,   466,   137,   453,
      14,   348,   348,   203,   348,   200,   209,   505,   271,   199,
     401,    14,   197,   348,   361,   462,   198,   505,   193,   200,
      32,   492,   442,    38,    83,   174,   444,   445,   447,   444,
     445,   505,    38,   174,   348,   417,   289,   196,   408,   269,
     353,   249,   348,   348,   348,   200,   196,   291,   270,    32,
     269,   505,    14,   268,   484,   412,   200,   196,    14,    78,
      79,    80,   213,   427,   427,   429,   431,   432,    52,   196,
      70,    70,    70,    70,    70,    90,   157,   196,   160,     9,
     423,   197,   437,    38,   348,   269,   200,    75,    76,   286,
     337,   230,   200,   198,    95,   198,   274,   453,   196,   136,
     273,    14,   228,   281,   105,   106,   107,   281,   200,   505,
     182,   136,   160,   505,   213,   174,   498,     9,   197,   423,
     136,   203,     9,   423,   422,   370,   371,   417,   390,   417,
     418,   390,   370,   390,   361,   363,   365,   197,   130,   214,
     417,   471,   472,   417,   417,   417,    32,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   503,    83,   243,   200,   200,   200,   200,   222,   198,
     417,   497,   102,   103,   493,   495,     9,   299,   197,   196,
     340,   345,   348,   136,   203,   200,   479,   299,   166,   179,
     199,   397,   404,   166,   199,   403,   136,   198,   492,   505,
     360,   506,    83,   174,    14,    83,   485,   453,   348,   197,
     289,   199,   289,   196,   136,   196,   291,   197,   199,   505,
     199,   198,   505,   269,   250,   415,   291,   136,   203,     9,
     423,   428,   431,   372,   373,   429,   391,   429,   430,   391,
     372,   391,   157,   361,   434,   435,    81,   429,   453,   199,
     337,    32,    77,   230,   198,   339,   273,   466,   274,   197,
     417,   101,   105,   198,   348,    32,   198,   282,   200,   182,
     505,   213,   136,   174,    32,   197,   417,   417,   197,   203,
       9,   423,   136,   203,     9,   423,   203,   203,   203,   136,
       9,   423,   197,   136,   200,     9,   423,   417,    32,   197,
     228,   198,   198,   198,   198,   213,   505,   505,   493,   408,
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
     197,   196,   492,   195,   489,   350,   492,   296,   200,   197,
     429,   136,   136,    32,   230,   275,   276,   228,   417,   417,
     417,   200,   198,   198,   417,   408,   305,   505,   313,   314,
     416,   310,    14,    32,    51,   315,   318,     9,    36,   197,
      31,    50,    53,    14,     9,   198,   215,   483,   331,    14,
     505,   245,   198,    14,   348,    38,    83,   396,   199,   490,
     491,   505,   198,   199,   323,   492,   489,   200,   492,   429,
     429,   228,    99,   241,   200,   213,   226,   306,   307,   308,
       9,   423,     9,   423,   200,   417,   406,   406,    68,   316,
     321,   321,    31,    50,    53,   417,    83,   181,   196,   198,
     417,   484,   417,    83,     9,   424,   228,     9,   424,    14,
     493,   228,   199,   323,   323,    97,   198,   115,   237,   160,
     102,   505,   182,   416,   172,    14,   494,   303,   196,    38,
      83,   197,   200,   491,   505,   200,   228,   198,   196,   178,
     244,   213,   326,   327,   182,   417,   182,   287,   288,   442,
     304,    83,   200,   408,   242,   175,   213,   198,   197,     9,
     424,   122,   123,   124,   329,   330,   287,    83,   272,   198,
     492,   442,   506,   197,   197,   198,   489,   329,    38,    83,
     174,   492,   199,   198,   199,   324,   506,    83,   174,    14,
      83,   489,   228,   228,    38,    83,   174,    14,    83,   348,
     324,   200,   200,    83,   174,    14,    83,   348,    14,    83,
     348,   348
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   206,   208,   207,   209,   209,   210,   210,   210,   210,
     210,   210,   210,   210,   211,   210,   212,   210,   210,   210,
     210,   210,   210,   210,   210,   210,   210,   210,   210,   213,
     213,   213,   213,   213,   213,   213,   213,   213,   213,   213,
     213,   213,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   215,   215,   216,   216,
     217,   217,   218,   219,   219,   219,   219,   220,   220,   221,
     222,   222,   222,   223,   223,   224,   224,   224,   225,   226,
     227,   227,   228,   228,   229,   229,   229,   229,   230,   230,
     230,   231,   230,   232,   230,   233,   230,   234,   230,   230,
     230,   230,   230,   230,   230,   230,   230,   230,   230,   230,
     230,   230,   230,   230,   235,   230,   236,   230,   230,   237,
     230,   238,   230,   230,   230,   230,   230,   230,   230,   230,
     230,   230,   230,   230,   230,   230,   230,   240,   239,   241,
     241,   243,   242,   244,   244,   245,   245,   246,   248,   247,
     249,   247,   250,   247,   252,   251,   253,   251,   255,   254,
     256,   254,   257,   254,   258,   254,   260,   259,   262,   261,
     263,   261,   264,   264,   265,   266,   267,   267,   267,   267,
     267,   268,   268,   269,   269,   270,   270,   271,   271,   272,
     272,   273,   273,   274,   274,   274,   275,   275,   276,   276,
     277,   277,   278,   278,   279,   279,   280,   280,   280,   280,
     281,   281,   281,   282,   282,   283,   283,   284,   284,   285,
     285,   286,   286,   287,   287,   287,   287,   287,   287,   287,
     287,   288,   288,   288,   288,   288,   288,   288,   288,   289,
     289,   289,   289,   289,   289,   289,   289,   290,   290,   290,
     290,   290,   290,   290,   290,   291,   291,   292,   292,   292,
     292,   292,   292,   293,   293,   294,   294,   294,   295,   295,
     295,   295,   296,   296,   297,   298,   299,   299,   301,   300,
     302,   300,   300,   300,   300,   303,   300,   304,   300,   300,
     300,   300,   300,   300,   300,   300,   305,   305,   305,   306,
     307,   307,   308,   308,   309,   309,   310,   310,   311,   311,
     312,   312,   312,   312,   312,   312,   312,   313,   313,   314,
     315,   315,   316,   316,   317,   317,   318,   319,   319,   319,
     320,   320,   320,   320,   321,   321,   321,   321,   321,   321,
     321,   322,   322,   322,   323,   323,   324,   324,   325,   325,
     326,   326,   327,   327,   328,   328,   328,   328,   328,   328,
     328,   329,   329,   330,   330,   330,   331,   331,   331,   331,
     332,   332,   333,   333,   334,   334,   335,   336,   336,   336,
     336,   336,   336,   337,   338,   338,   339,   339,   340,   340,
     340,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     348,   348,   348,   348,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   350,   350,   352,   351,   353,   351,   355,   354,   356,
     354,   357,   354,   358,   354,   359,   354,   360,   360,   360,
     361,   361,   362,   362,   363,   363,   364,   364,   365,   365,
     366,   367,   367,   368,   368,   369,   369,   370,   370,   371,
     371,   372,   372,   373,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
     389,   389,   390,   390,   391,   391,   392,   393,   394,   394,
     395,   395,   395,   395,   395,   395,   395,   395,   395,   395,
     395,   396,   396,   396,   396,   397,   398,   398,   399,   399,
     400,   400,   401,   401,   402,   403,   403,   404,   404,   404,
     405,   405,   405,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   407,   408,   408,   409,   409,   409,
     409,   409,   410,   410,   411,   411,   411,   411,   412,   412,
     412,   413,   413,   413,   414,   414,   414,   415,   415,   416,
     416,   416,   416,   416,   416,   416,   416,   416,   416,   416,
     416,   416,   416,   416,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   418,
     418,   419,   420,   420,   421,   421,   421,   421,   421,   421,
     421,   422,   422,   423,   423,   424,   424,   425,   425,   425,
     425,   426,   426,   426,   426,   426,   427,   427,   427,   427,
     428,   428,   429,   429,   429,   429,   429,   429,   429,   429,
     429,   429,   429,   429,   429,   429,   429,   430,   430,   431,
     431,   432,   432,   432,   432,   433,   433,   434,   434,   435,
     435,   436,   436,   437,   437,   438,   438,   440,   439,   441,
     442,   442,   443,   443,   444,   444,   444,   445,   445,   446,
     446,   447,   447,   448,   448,   449,   449,   450,   450,   451,
     451,   452,   452,   453,   453,   453,   453,   453,   453,   453,
     453,   453,   453,   453,   454,   454,   454,   454,   454,   454,
     454,   454,   454,   455,   455,   455,   455,   455,   455,   455,
     455,   455,   456,   457,   457,   458,   458,   459,   459,   459,
     460,   461,   461,   461,   462,   462,   462,   462,   463,   463,
     464,   464,   464,   464,   464,   464,   465,   465,   465,   465,
     465,   466,   466,   466,   466,   466,   466,   467,   467,   468,
     468,   468,   468,   468,   468,   468,   468,   469,   469,   470,
     470,   470,   470,   471,   471,   472,   472,   472,   472,   473,
     473,   473,   473,   474,   474,   474,   474,   474,   474,   475,
     475,   475,   476,   476,   476,   476,   476,   476,   476,   476,
     476,   476,   476,   477,   477,   478,   478,   479,   479,   480,
     480,   480,   480,   481,   481,   482,   482,   483,   483,   484,
     484,   485,   485,   486,   486,   487,   488,   488,   488,   488,
     489,   489,   490,   490,   491,   491,   492,   492,   493,   493,
     494,   495,   495,   496,   496,   496,   496,   497,   497,   497,
     498,   498,   498,   498,   499,   499,   500,   500,   500,   500,
     501,   502,   503,   503,   504,   504,   505,   505,   505,   505,
     505,   505,   505,   505,   505,   505,   505,   506,   506
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     1,     4,     3,     0,     6,     0,     5,     3,     4,
       4,     4,     4,     6,     7,     7,     7,     7,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     3,
       3,     1,     2,     1,     2,     3,     4,     3,     1,     2,
       1,     2,     2,     1,     3,     1,     3,     2,     2,     2,
       5,     4,     2,     0,     1,     1,     1,     1,     3,     5,
       8,     0,     4,     0,     6,     0,    10,     0,     4,     2,
       3,     2,     3,     2,     3,     3,     3,     3,     3,     3,
       5,     1,     1,     1,     0,     9,     0,    10,     5,     0,
      13,     0,     5,     3,     3,     2,     2,     2,     2,     2,
       2,     3,     2,     2,     3,     2,     2,     0,     4,     9,
       0,     0,     4,     2,     0,     1,     0,     1,     0,    10,
       0,    11,     0,    11,     0,     9,     0,    10,     0,     8,
       0,     9,     0,     7,     0,     8,     0,     8,     0,     7,
       0,     8,     1,     1,     1,     1,     1,     2,     3,     3,
       2,     2,     0,     2,     0,     2,     0,     1,     3,     1,
       3,     2,     0,     1,     2,     4,     1,     4,     1,     4,
       1,     4,     1,     4,     3,     5,     3,     4,     4,     5,
       5,     4,     0,     1,     1,     4,     0,     5,     0,     2,
       0,     3,     0,     7,     8,     6,     2,     5,     6,     4,
       0,     4,     5,     7,     6,     6,     7,     9,     8,     6,
       7,     5,     2,     4,     5,     3,     0,     3,     4,     6,
       5,     5,     6,     8,     7,     2,     0,     1,     2,     2,
       3,     4,     4,     3,     1,     1,     2,     4,     3,     5,
       1,     3,     2,     0,     2,     3,     2,     0,     0,     4,
       0,     5,     2,     2,     2,     0,    11,     0,    12,     3,
       3,     3,     4,     4,     3,     5,     2,     2,     0,     6,
       5,     4,     3,     1,     1,     3,     4,     1,     2,     1,
       1,     5,     6,     1,     1,     4,     1,     1,     3,     2,
       2,     0,     2,     0,     1,     3,     1,     1,     1,     1,
       3,     4,     4,     4,     1,     1,     2,     2,     2,     3,
       3,     1,     1,     1,     1,     3,     1,     3,     1,     1,
       1,     0,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     1,     1,     1,     3,     5,     1,     3,
       5,     4,     3,     3,     3,     4,     3,     3,     3,     2,
       2,     1,     1,     3,     3,     1,     1,     0,     1,     2,
       4,     3,     3,     6,     2,     3,     2,     3,     6,     1,
       1,     1,     1,     1,     6,     3,     4,     6,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     5,     4,     3,
       1,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     2,
       1,     5,     0,     0,    12,     0,    13,     0,     4,     0,
       7,     0,     5,     0,     3,     0,     6,     2,     2,     4,
       1,     1,     5,     3,     5,     3,     2,     0,     2,     0,
       4,     4,     3,     2,     0,     5,     3,     2,     0,     5,
       3,     2,     0,     5,     3,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       2,     0,     2,     0,     2,     0,     4,     4,     4,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     3,     4,     1,     2,     4,     2,     6,     0,     1,
       4,     0,     2,     0,     1,     1,     3,     1,     3,     1,
       1,     3,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     4,     1,     1,     1,     1,     1,
       1,     3,     1,     3,     1,     1,     1,     3,     1,     1,
       1,     2,     1,     0,     0,     1,     1,     3,     0,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     2,     1,     1,     4,     3,     4,     1,
       1,     1,     1,     1,     1,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     5,     4,     3,
       1,     3,     3,     1,     1,     1,     1,     1,     3,     3,
       3,     2,     0,     1,     0,     1,     0,     5,     3,     3,
       1,     1,     1,     1,     3,     2,     1,     1,     1,     1,
       1,     3,     1,     1,     1,     3,     1,     2,     2,     4,
       3,     4,     1,     1,     1,     1,     1,     3,     1,     2,
       0,     5,     3,     3,     1,     3,     1,     2,     0,     5,
       3,     2,     0,     3,     0,     4,     2,     0,     3,     3,
       1,     0,     1,     1,     1,     1,     3,     1,     1,     1,
       3,     1,     1,     3,     3,     2,     4,     2,     4,     5,
       5,     5,     5,     1,     1,     1,     1,     1,     1,     3,
       3,     4,     4,     3,     1,     1,     1,     1,     3,     1,
       4,     3,     3,     1,     1,     1,     1,     1,     3,     3,
       4,     4,     3,     1,     1,     7,     9,     7,     6,     8,
       1,     4,     4,     1,     1,     1,     4,     2,     1,     0,
       1,     1,     1,     3,     3,     3,     0,     1,     1,     3,
       3,     2,     3,     6,     0,     1,     4,     2,     0,     5,
       3,     3,     1,     6,     4,     4,     2,     2,     0,     5,
       3,     3,     1,     2,     0,     5,     3,     3,     1,     2,
       2,     1,     2,     1,     4,     3,     3,     6,     3,     1,
       1,     1,     4,     4,     4,     4,     4,     4,     2,     2,
       4,     2,     2,     1,     3,     3,     3,     0,     2,     5,
       6,     6,     7,     1,     2,     1,     2,     1,     4,     1,
       4,     3,     0,     1,     3,     2,     3,     1,     1,     0,
       0,     3,     1,     3,     3,     2,     0,     2,     2,     2,
       2,     1,     2,     4,     2,     5,     3,     1,     1,     0,
       3,     4,     5,     6,     3,     1,     3,     2,     1,     0,
       4,     1,     3,     2,     4,     5,     2,     2,     1,     1,
       1,     1,     3,     2,     1,     8,     6,     1,     0
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
#line 6869 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 754 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 6877 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 761 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 6883 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 762 "hphp.y" /* yacc.c:1646  */
    { }
#line 6889 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 765 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 6895 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 766 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6901 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 767 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6907 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 768 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6913 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 769 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6919 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 770 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 6925 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 771 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 6933 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 774 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 6940 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 776 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 6946 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 6952 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 778 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 6958 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 779 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 6964 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 780 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6972 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 784 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6981 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 789 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6990 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 794 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6997 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 797 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7004 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 800 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7012 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 804 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7020 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 808 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7028 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 812 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7036 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 816 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7044 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 819 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7051 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 824 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7057 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 825 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7063 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 826 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7069 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 827 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7075 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 828 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7081 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 829 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7087 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 830 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7093 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 831 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7099 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7105 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7111 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 834 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7117 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7123 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7129 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 915 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7135 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 917 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7141 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 922 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7147 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 923 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7154 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 929 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7160 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 933 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7166 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 934 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7172 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 936 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7178 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 938 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7184 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 943 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7190 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 944 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7197 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 950 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7203 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 954 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7210 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 956 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7217 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 958 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7224 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 963 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7230 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 965 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7236 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 968 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7242 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 970 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7248 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 971 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7254 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 976 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7263 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 983 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7272 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 991 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7279 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 994 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7286 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 1000 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7292 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 1001 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7298 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1004 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7304 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 1005 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7310 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1006 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7316 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 1007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7322 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1010 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7328 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1014 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7334 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1019 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7340 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1020 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7347 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1022 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7355 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1026 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7362 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1029 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7370 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1033 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7377 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1035 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7385 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1038 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7392 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1040 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1043 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1044 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1045 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7418 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1046 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1047 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1048 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1049 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1050 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1051 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7454 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1052 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1053 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7466 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1054 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 7472 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1055 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 7478 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1056 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 7484 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1057 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 7491 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1061 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7498 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1063 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 7506 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1068 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7513 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1070 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 7521 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1074 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 7529 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1083 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7535 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1084 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 7541 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1087 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7547 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1088 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 7553 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1089 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 7559 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1090 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 7568 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1094 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7574 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1095 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7580 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1096 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7586 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1097 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7592 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1098 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7598 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1099 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7604 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1100 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7610 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1101 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7616 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1102 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7622 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1103 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 7628 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1104 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7634 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1105 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 7644 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1113 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);}
#line 7650 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1114 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7656 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1123 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 7662 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1124 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7668 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1128 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7675 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1130 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7683 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1136 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7689 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1137 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7695 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1141 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 7701 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1142 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7707 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1146 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 7713 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1152 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7722 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1159 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7731 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1166 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7740 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1173 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7749 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1180 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1186 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7767 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1194 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 7774 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1198 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 7780 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 1202 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 7787 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 1206 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 7793 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 208:
#line 1212 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 7800 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 1215 "hphp.y" /* yacc.c:1646  */
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
#line 7818 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 1230 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 7825 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1233 "hphp.y" /* yacc.c:1646  */
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
#line 7843 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1247 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 7850 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1250 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7858 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1255 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 7865 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1258 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7873 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1264 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 7879 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1267 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 7885 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1271 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 7892 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1274 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7903 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1282 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 7910 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1285 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7921 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1293 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7927 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1294 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 7934 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1298 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7940 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1301 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7946 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1304 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 7952 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1305 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 7958 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1306 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 7966 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1309 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 7972 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1310 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 7978 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1314 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7984 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1315 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7990 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1318 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7996 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1319 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8002 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 1322 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8008 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1323 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8014 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 1326 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 8020 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1328 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8026 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1331 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 8032 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1333 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8038 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1337 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8044 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1338 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8050 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1341 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8056 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1342 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8062 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1343 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8068 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1347 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8074 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1349 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8080 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1352 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8086 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1354 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8092 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1357 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8098 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1359 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8104 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1362 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8110 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1364 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8116 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1368 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8122 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1370 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8129 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1375 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8135 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1376 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8141 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1377 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8147 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1378 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8153 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1383 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8159 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1385 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8165 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1386 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8171 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1389 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8177 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1390 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8183 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1395 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8189 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1396 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8195 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1401 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8201 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1402 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8207 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1405 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8213 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1406 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8219 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1409 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8225 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1410 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8231 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1418 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8238 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1424 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8245 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1430 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1434 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8259 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1438 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8266 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1443 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8273 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1448 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8281 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1451 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8287 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1457 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8294 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1461 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8301 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1466 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8308 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1471 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8315 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1476 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8322 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1481 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8329 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1487 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8336 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1493 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8343 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1501 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 8350 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1506 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 8357 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1511 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 8365 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1515 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8371 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1518 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 8378 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1522 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 8385 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1526 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 8393 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1529 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8399 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1534 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1537 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8413 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1541 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8420 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1545 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8427 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1549 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8434 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1553 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8441 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1558 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1563 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8455 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1569 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8461 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1570 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8467 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1573 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,false);}
#line 8473 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1574 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),true,false);}
#line 8479 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1575 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,true);}
#line 8485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1577 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),false, false);}
#line 8491 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1579 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),false,true);}
#line 8497 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1581 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),true, false);}
#line 8503 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1585 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8509 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1586 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 8515 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1589 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8521 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1590 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8527 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1591 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 8533 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1595 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 8539 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1597 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 8545 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1598 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 8551 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1599 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 8557 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1604 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 8563 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1605 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8569 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1608 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 8576 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1613 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 8582 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1619 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 8588 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1620 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8594 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1623 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[0]));}
#line 8600 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1624 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 8607 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1627 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[-1]));}
#line 8613 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1628 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-3]));}
#line 8620 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1630 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 8627 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1633 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL, true);}
#line 8634 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1635 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8640 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1638 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 8648 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1645 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8657 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1653 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 8665 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1660 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8674 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1665 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 8680 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1667 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 8686 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1669 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 8692 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1671 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 8698 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1673 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 8704 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1674 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 8711 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1677 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 8717 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1680 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 8723 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1681 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 8729 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1682 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8735 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1688 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 8741 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1693 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 8748 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1696 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 8756 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1703 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 8762 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1704 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 8769 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1709 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 8776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1712 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 8782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1719 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 8789 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1721 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8795 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1725 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8801 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1730 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8807 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1732 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8813 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1734 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8819 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1735 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 8830 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1741 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 8836 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1743 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 8842 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1744 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 8848 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1748 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,0,(yyvsp[0]),0);}
#line 8854 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1750 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),0,(yyvsp[0]),0);}
#line 8860 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1755 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8866 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1758 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8872 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1759 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 8878 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1763 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 8884 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1764 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 8890 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1768 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 8897 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1771 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 8904 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1776 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 8911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1781 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 8917 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1782 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 8924 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1784 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 8930 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1788 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 8936 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1789 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 8942 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1790 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 8948 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 1791 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 8954 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1795 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8960 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1796 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 8966 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1797 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 8972 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1798 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 8978 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1799 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 8984 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1801 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 8990 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1803 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 8996 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1807 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 9004 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 1810 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 9010 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1811 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 9016 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 394:
#line 1815 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9022 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1816 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9028 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1820 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9034 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1821 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9040 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1824 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9046 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1825 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9052 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1828 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9058 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1829 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9064 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1832 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9070 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1834 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9076 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9082 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1838 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9088 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1839 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9094 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1840 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9100 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1841 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9106 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1842 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9112 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1843 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9118 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1847 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9124 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1848 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9130 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1851 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9136 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1852 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9142 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1853 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9148 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1857 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9154 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1859 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9160 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1860 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9166 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1861 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9172 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1865 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9178 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1867 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9184 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1871 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9190 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1873 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9196 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1877 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9204 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1881 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9211 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1885 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9217 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1889 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9223 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1891 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9229 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1892 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9235 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1893 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9241 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1894 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9247 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1895 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1898 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9259 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 1902 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9265 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 1903 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9271 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 1907 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9277 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 1908 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 1912 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 9289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 1913 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 9295 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 1914 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 9301 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 1915 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9307 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 1919 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9313 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 1924 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9319 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 1928 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 9325 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 1932 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 1936 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 9337 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 1940 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9343 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 1945 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9349 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 1949 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9355 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 1950 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9361 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 1951 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9367 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 1952 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9373 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 1953 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9379 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 1958 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 9385 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 1959 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 9391 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 1960 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 9397 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 9403 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 1964 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 9409 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 1965 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 9415 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 1966 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 9421 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 1967 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 9427 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 1968 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 9433 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 1969 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 9439 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 1970 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 9445 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 1971 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 9451 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 1972 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 9457 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 1973 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 9463 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 1974 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 9469 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 1975 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 9475 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 1976 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 9481 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 1977 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 9487 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 1978 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 9493 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 1979 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 9499 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 1980 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 9505 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 1981 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 9511 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 1982 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 9517 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 1983 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 9523 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 1984 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 9529 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 1985 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 9535 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 1986 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 9541 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 1987 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 9547 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 1988 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 9553 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 1989 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 9559 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 1990 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 9565 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 1991 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 9571 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 1992 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 9577 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 1993 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 9583 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 1994 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 9589 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 1995 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 9595 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 1996 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 9601 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 1997 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 9607 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 1998 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 9613 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 1999 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 9619 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 9625 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 2001 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 9631 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 2002 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 9637 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 2003 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 9643 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 2004 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 9649 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 2005 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 9655 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 2006 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 9661 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 2007 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 9668 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 2009 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 9674 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2010 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 9681 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2012 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 9687 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2014 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 9693 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2015 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9699 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2016 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 9705 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 9711 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2018 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 9717 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2019 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9723 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2020 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 9729 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2021 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 9735 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2022 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 9741 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2023 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 9747 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2024 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 9753 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2025 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 9759 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2026 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 9765 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2027 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 9771 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2028 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 9777 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2029 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9783 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2030 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9789 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2031 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9795 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2032 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9801 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2033 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9807 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2034 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9813 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2035 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9819 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2036 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9825 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2037 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 9831 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2038 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 9837 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2039 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9843 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2046 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 9849 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2047 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9855 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2052 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 9864 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2058 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2066 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 9884 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2072 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[-12]),
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9895 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2082 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),0,
                                                     NULL,NULL,NULL);}
#line 9908 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2090 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         (yyvsp[-3]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[-3]), nullptr, (yyvsp[-3]));
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[-3]),
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9922 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2100 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9932 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2108 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         (yyvsp[-6]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[-6]), nullptr, (yyvsp[-6]));
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[-6]),
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9946 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2118 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9956 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2124 "hphp.y" /* yacc.c:1646  */
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
#line 9972 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2135 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),0,
                                                     NULL,NULL,NULL);}
#line 9985 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2143 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9997 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2150 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10007 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2158 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2168 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2169 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2171 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10037 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2175 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10044 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2177 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10050 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2184 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10056 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2187 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10062 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2194 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10068 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2197 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10074 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2202 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10080 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2203 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10086 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2208 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10092 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2209 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10098 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2213 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval), (yyvsp[-1]), T_ARRAY);}
#line 10104 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2217 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10110 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2218 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10116 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2223 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10122 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2224 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10128 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2229 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10134 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2230 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10140 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2235 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10146 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2236 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10152 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2242 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10158 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 570:
#line 2244 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10164 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2249 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10170 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 572:
#line 2250 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10176 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2256 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10182 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2258 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10188 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2262 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10194 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
#line 2266 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10200 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2270 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10206 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
#line 2274 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10212 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
#line 2278 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10218 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
#line 2282 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10224 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2286 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10230 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
#line 2290 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10236 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2294 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10242 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2298 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10248 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2302 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10254 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2306 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10260 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2310 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10266 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2314 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10272 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2318 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10278 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2323 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10284 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2324 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10290 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2329 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10296 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2330 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10302 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2335 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10308 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2336 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10314 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2341 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10322 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2348 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10330 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2355 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10336 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2357 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10342 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2361 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10348 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2362 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10354 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2363 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10360 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2364 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10366 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2365 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10372 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2366 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10378 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2367 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10384 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2368 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10390 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2369 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10397 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2371 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10403 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2372 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10409 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2376 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10415 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2377 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 10421 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2378 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 10427 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2379 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 10433 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2386 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 10439 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2389 "hphp.y" /* yacc.c:1646  */
    { Token t1; _p->onArray(t1,(yyvsp[-1]));
                                         Token t2; _p->onArray(t2,(yyvsp[0]));
                                         Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onCallParam((yyvsp[-1]),NULL,t1,0,0);
                                         _p->onCallParam((yyval), &(yyvsp[-1]),t2,0,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),file,0,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),line,0,0);
                                         (yyval).setText("");}
#line 10453 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2400 "hphp.y" /* yacc.c:1646  */
    { Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onArray((yyvsp[-2]),(yyvsp[-5]));
                                         _p->onArray((yyvsp[-1]),(yyvsp[-3]));
                                         _p->onCallParam((yyvsp[-4]),NULL,(yyvsp[-2]),0,0);
                                         _p->onCallParam((yyval), &(yyvsp[-4]),(yyvsp[-1]),0,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),file,0,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),line,0,0);
                                         (yyval).setText((yyvsp[0]).text());}
#line 10467 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2411 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 10473 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2412 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 10479 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2417 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2418 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10491 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2421 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-1]),0,(yyvsp[0]),0);}
#line 10497 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2422 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10503 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2425 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10510 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2429 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10518 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2432 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10524 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2435 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         if ((yyvsp[0]).htmlTrim()) {
                                           (yyvsp[0]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));
                                         }
                                       }
#line 10536 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2442 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10542 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2443 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10548 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2447 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10554 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2449 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 10560 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2451 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 10566 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2455 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10572 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2456 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10578 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2457 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10584 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2458 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10590 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10596 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2460 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10602 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2461 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10608 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2462 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10614 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2463 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10620 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2464 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10626 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2465 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10632 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2466 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10638 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2467 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10644 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2468 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10650 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2469 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10656 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2470 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10662 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2471 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10668 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2472 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10674 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2473 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10680 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2474 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10686 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 653:
#line 2475 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10692 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2476 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10698 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2477 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10704 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2478 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10710 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2479 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10716 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2480 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10722 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2481 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10728 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2482 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10734 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10740 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2484 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10746 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2485 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10752 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2486 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2487 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10764 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2488 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10770 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2489 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2490 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2491 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10788 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2492 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10794 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2493 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10800 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2494 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10806 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2495 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10812 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2496 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10818 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2497 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10824 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2498 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10830 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2499 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10836 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2500 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10842 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2501 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10848 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2502 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10854 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2503 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10860 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2504 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10866 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2505 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10872 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2506 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10878 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2507 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10884 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2508 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10890 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2509 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10896 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2510 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10902 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2511 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10908 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2512 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10914 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2513 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10920 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2514 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10926 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2515 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10932 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2516 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10938 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2517 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10944 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2518 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10950 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2519 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10956 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2520 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10962 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2521 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10968 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2522 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10974 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2523 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10980 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2524 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10986 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2525 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10992 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2526 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10998 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2527 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11004 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2528 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11010 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2529 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11016 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2530 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11022 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2531 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11028 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2532 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11034 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2533 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11040 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2534 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11046 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2535 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11052 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2540 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11058 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2544 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11064 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2545 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11070 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2549 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11076 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2550 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11082 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2551 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11088 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2552 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11095 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2554 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11102 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2558 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11108 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2567 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11114 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2570 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11120 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2571 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11127 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2573 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11134 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2583 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11140 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2587 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11146 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2588 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11152 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2589 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11158 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2593 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11164 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2594 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11170 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2595 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11176 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2599 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11182 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2600 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11188 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2601 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11194 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2605 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11200 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2606 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11206 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2610 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11212 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2611 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11218 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2612 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11224 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2613 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 11231 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2615 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 11237 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2616 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 11243 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2617 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 11249 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2618 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 11255 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2619 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 11261 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2620 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 11267 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2621 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 11273 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2622 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 11279 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2623 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 11285 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2626 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11291 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2628 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11297 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2632 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11303 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2633 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11309 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2635 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11315 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2636 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11321 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2638 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11327 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2639 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11333 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2640 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11339 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2641 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11345 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2642 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11351 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2643 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11357 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2644 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11363 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2645 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11369 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2646 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11375 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2648 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 11381 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2650 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 11387 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2652 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 11393 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2654 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 11399 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2656 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 11405 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2657 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 11411 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2658 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 11417 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2659 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 11423 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2660 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 11429 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2661 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 11435 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2662 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 11441 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2663 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 11447 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2664 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 11453 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2665 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 11459 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2666 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 11465 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2667 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 11471 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2668 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 11477 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2669 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 11483 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2670 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 11489 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2671 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11495 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2672 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11501 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2674 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 11507 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2676 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 11513 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2678 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 11519 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2680 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 11525 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2681 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 11531 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2683 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 11538 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2685 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 11544 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2688 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 11551 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2692 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 11557 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2695 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 11563 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2696 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 11569 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2700 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 11575 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2701 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 11581 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2707 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 11587 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2713 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 11593 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2714 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11599 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2718 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11605 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2719 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11611 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2720 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11617 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2721 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11623 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2722 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 11629 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2723 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 11635 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2725 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 11642 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2730 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11648 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2731 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11654 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2735 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11660 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2736 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11666 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2739 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 11672 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2740 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11678 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2746 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11684 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2748 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11690 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2750 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11696 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2751 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11702 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2755 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11708 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2756 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11714 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2757 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11720 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2760 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11726 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2762 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11732 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2765 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 11738 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2766 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 11744 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2767 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 11750 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2768 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 11756 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2772 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 11763 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2775 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 11771 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2782 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11777 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2783 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11783 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2786 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 11791 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2789 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 11797 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2790 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11803 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2791 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11809 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2793 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11815 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2794 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11821 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2796 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11827 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2797 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11833 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2798 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11839 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2799 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11845 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2800 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11851 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2801 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11857 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2806 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 11863 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2807 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 11869 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2812 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2813 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11881 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2818 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11887 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2820 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11893 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2822 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11899 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2823 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2827 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2828 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11917 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11923 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2834 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 11929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 2839 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 11935 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2842 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 11941 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2847 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2848 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11953 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2851 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2852 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 11966 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2859 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 11972 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2861 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 11978 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2864 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 11984 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2866 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11990 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2869 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11996 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 2872 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12002 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2873 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12008 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2877 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 12014 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2878 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 12020 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2882 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12026 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2883 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 12032 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 2884 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 12038 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 877:
#line 2888 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12044 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 2893 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12050 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 2898 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12056 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 2899 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12062 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 2903 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12068 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 2908 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12074 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 2913 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12080 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 2914 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12086 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 2918 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12092 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
#line 2920 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12098 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 2925 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12104 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 2927 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12110 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 2933 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12124 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 2944 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12138 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 2959 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12152 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 2971 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12166 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 2983 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12172 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 2984 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12178 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 2985 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12184 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 2986 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12190 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 2987 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12196 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 2988 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12202 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 2990 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12216 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 3007 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12222 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3009 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12228 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3011 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12234 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3012 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12240 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3016 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12246 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3017 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12252 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12258 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3019 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12264 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3027 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12278 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3036 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12284 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3038 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12290 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3039 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12296 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3043 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12302 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3048 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12308 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3049 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12314 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3050 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12320 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3051 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12326 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3052 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12332 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3053 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12338 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3054 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12344 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3056 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12350 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3058 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12356 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3062 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12362 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3066 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12368 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3067 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12374 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3073 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12380 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3077 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12386 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3084 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 12392 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3093 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 12398 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3097 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 12404 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3101 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12410 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3110 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12416 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3111 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12422 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3112 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12428 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3116 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12434 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3117 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 12440 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3118 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 12446 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
#line 3120 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]) = 1; _p->onIndirectRef((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12452 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
#line 3125 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12458 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3126 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12464 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
#line 3137 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12470 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
#line 3138 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12476 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3139 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12482 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3142 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12496 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
#line 3153 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12502 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3154 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12508 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
#line 3158 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12514 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
#line 3159 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12520 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3162 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12534 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3171 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12540 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3175 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 12546 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3176 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 12552 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3178 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 12558 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3179 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 12564 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3180 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 12570 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3181 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 12576 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3186 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12582 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3187 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12588 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3191 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12594 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3192 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12600 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3193 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12606 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3194 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12612 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3197 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 12618 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3199 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 12624 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3200 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 12630 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3201 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 12636 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3206 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12642 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3207 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 12648 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3211 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12654 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3212 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12660 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3213 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12666 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3214 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12672 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3219 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12678 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3220 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 12684 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3225 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12690 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3227 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12696 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3229 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12702 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3230 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12708 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3234 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 12714 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3236 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 12720 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3237 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 12726 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3239 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 12733 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3244 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12739 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3246 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12745 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3248 "hphp.y" /* yacc.c:1646  */
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[-2]),
                                           !(yyvsp[-1]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[0])
                                         );
                                       }
#line 12759 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3258 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 12765 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3260 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 12771 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3261 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12777 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3264 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 12783 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3265 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 12789 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3266 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 12795 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3270 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 12801 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3271 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 12807 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3272 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12813 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 995:
#line 3273 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12819 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3274 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12825 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3275 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12831 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
#line 3276 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 12837 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3277 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 12843 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3278 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 12849 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3279 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 12855 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3280 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 12861 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3284 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12867 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1004:
#line 3285 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12873 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1005:
#line 3290 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 12879 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1006:
#line 3292 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 12885 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3306 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 12893 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3311 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 12901 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3315 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 12909 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3320 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 12917 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3326 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12923 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3327 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 12929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3331 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12935 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3332 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 12941 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3338 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 12947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3342 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 12953 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3348 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 12959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3352 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 12966 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3359 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12972 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3360 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12978 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3364 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 12986 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3367 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 12993 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3373 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12999 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3378 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]); }
#line 13005 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1027:
#line 3379 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13011 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1028:
#line 3380 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13017 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1029:
#line 3381 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13023 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1036:
#line 3402 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13029 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1037:
#line 3403 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13035 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1040:
#line 3412 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13041 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3423 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13047 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3425 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13053 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3429 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13059 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3432 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13065 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3436 "hphp.y" /* yacc.c:1646  */
    {}
#line 13071 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3437 "hphp.y" /* yacc.c:1646  */
    {}
#line 13077 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3438 "hphp.y" /* yacc.c:1646  */
    {}
#line 13083 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3444 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13090 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3449 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13100 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3458 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13106 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3464 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13115 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3472 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 13121 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1055:
#line 3473 "hphp.y" /* yacc.c:1646  */
    { }
#line 13127 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1056:
#line 3479 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 13133 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1057:
#line 3481 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 13139 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1058:
#line 3482 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 13149 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1059:
#line 3487 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 13156 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1060:
#line 3493 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("array"); }
#line 13163 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1061:
#line 3498 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13169 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1062:
#line 3503 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 13177 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3507 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13183 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3512 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 13189 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3514 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 13195 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3520 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 13202 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3522 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 13210 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3525 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13216 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1069:
#line 3526 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13224 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3529 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13232 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3532 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13238 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3535 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 13246 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3538 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3540 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 13262 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3546 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 13271 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1076:
#line 3552 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 13281 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1077:
#line 3560 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13287 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1078:
#line 3561 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13293 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;


#line 13297 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 3564 "hphp.y" /* yacc.c:1906  */

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}
