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
using LS = Parser::LabelScopeKind;

using ParamMode = HPHP::ParamMode;

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
    Token params1; _p->onCallParam(params1, NULL, param1, ParamMode::In, false);

    for (unsigned int i = 0; i < classes.size(); i++) {
      Token parent2;  parent2.set(T_STRING, classes[i]);
      Token cls2;     _p->onName(cls2, parent2, Parser::StringName);
      Token fname3;   fname3.setText("__xhpAttributeDeclaration");
      Token param;   _p->onCall(param, 0, fname3, dummy, &cls2);

      Token params; _p->onCallParam(params, &params1, param, ParamMode::In,
                                    false);
      params1 = params;
    }

    Token params2; _p->onCallParam(params2, &params1, arrAttributes,
                                   ParamMode::In, false);

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

#line 660 "hphp.7.tab.cpp" /* yacc.c:339  */

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
    T_INOUT = 323,
    T_HASHBANG = 324,
    T_CHARACTER = 325,
    T_BAD_CHARACTER = 326,
    T_ENCAPSED_AND_WHITESPACE = 327,
    T_CONSTANT_ENCAPSED_STRING = 328,
    T_ECHO = 329,
    T_DO = 330,
    T_WHILE = 331,
    T_ENDWHILE = 332,
    T_FOR = 333,
    T_ENDFOR = 334,
    T_FOREACH = 335,
    T_ENDFOREACH = 336,
    T_DECLARE = 337,
    T_ENDDECLARE = 338,
    T_AS = 339,
    T_SUPER = 340,
    T_SWITCH = 341,
    T_ENDSWITCH = 342,
    T_CASE = 343,
    T_DEFAULT = 344,
    T_BREAK = 345,
    T_GOTO = 346,
    T_CONTINUE = 347,
    T_FUNCTION = 348,
    T_CONST = 349,
    T_RETURN = 350,
    T_TRY = 351,
    T_CATCH = 352,
    T_THROW = 353,
    T_USING = 354,
    T_USE = 355,
    T_GLOBAL = 356,
    T_STATIC = 357,
    T_ABSTRACT = 358,
    T_FINAL = 359,
    T_PRIVATE = 360,
    T_PROTECTED = 361,
    T_PUBLIC = 362,
    T_VAR = 363,
    T_UNSET = 364,
    T_ISSET = 365,
    T_EMPTY = 366,
    T_HALT_COMPILER = 367,
    T_CLASS = 368,
    T_INTERFACE = 369,
    T_EXTENDS = 370,
    T_IMPLEMENTS = 371,
    T_OBJECT_OPERATOR = 372,
    T_NULLSAFE_OBJECT_OPERATOR = 373,
    T_DOUBLE_ARROW = 374,
    T_LIST = 375,
    T_ARRAY = 376,
    T_DICT = 377,
    T_VEC = 378,
    T_VARRAY = 379,
    T_DARRAY = 380,
    T_KEYSET = 381,
    T_CALLABLE = 382,
    T_CLASS_C = 383,
    T_METHOD_C = 384,
    T_FUNC_C = 385,
    T_LINE = 386,
    T_FILE = 387,
    T_COMMENT = 388,
    T_DOC_COMMENT = 389,
    T_OPEN_TAG = 390,
    T_OPEN_TAG_WITH_ECHO = 391,
    T_CLOSE_TAG = 392,
    T_WHITESPACE = 393,
    T_START_HEREDOC = 394,
    T_END_HEREDOC = 395,
    T_DOLLAR_OPEN_CURLY_BRACES = 396,
    T_CURLY_OPEN = 397,
    T_DOUBLE_COLON = 398,
    T_NAMESPACE = 399,
    T_NS_C = 400,
    T_DIR = 401,
    T_NS_SEPARATOR = 402,
    T_XHP_LABEL = 403,
    T_XHP_TEXT = 404,
    T_XHP_ATTRIBUTE = 405,
    T_XHP_CATEGORY = 406,
    T_XHP_CATEGORY_LABEL = 407,
    T_XHP_CHILDREN = 408,
    T_ENUM = 409,
    T_XHP_REQUIRED = 410,
    T_TRAIT = 411,
    T_ELLIPSIS = 412,
    T_INSTEADOF = 413,
    T_TRAIT_C = 414,
    T_HH_ERROR = 415,
    T_FINALLY = 416,
    T_XHP_TAG_LT = 417,
    T_XHP_TAG_GT = 418,
    T_TYPELIST_LT = 419,
    T_TYPELIST_GT = 420,
    T_UNRESOLVED_LT = 421,
    T_COLLECTION = 422,
    T_SHAPE = 423,
    T_TYPE = 424,
    T_UNRESOLVED_TYPE = 425,
    T_NEWTYPE = 426,
    T_UNRESOLVED_NEWTYPE = 427,
    T_COMPILER_HALT_OFFSET = 428,
    T_ASYNC = 429,
    T_LAMBDA_OP = 430,
    T_LAMBDA_CP = 431,
    T_UNRESOLVED_OP = 432,
    T_WHERE = 433
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

#line 903 "hphp.7.tab.cpp" /* yacc.c:358  */

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
#define YYLAST   20018

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  208
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  314
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1124
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  2092

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   433

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    56,   206,     2,   204,    55,    38,   207,
     198,   199,    53,    50,     9,    51,    52,    54,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    32,   200,
      43,    14,    45,    31,    68,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    70,     2,   203,    37,     2,   205,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   201,    36,   202,    58,     2,     2,     2,
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
     194,   195,   196,   197
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   757,   757,   757,   766,   768,   771,   772,   773,   774,
     775,   776,   777,   780,   782,   782,   784,   784,   786,   789,
     794,   799,   802,   805,   809,   813,   817,   821,   825,   830,
     831,   832,   833,   834,   835,   836,   837,   838,   839,   840,
     841,   842,   846,   847,   848,   849,   850,   851,   852,   853,
     854,   855,   856,   857,   858,   859,   860,   861,   862,   863,
     864,   865,   866,   867,   868,   869,   870,   871,   872,   873,
     874,   875,   876,   877,   878,   879,   880,   881,   882,   883,
     884,   885,   886,   887,   888,   889,   890,   891,   892,   893,
     894,   895,   896,   897,   898,   899,   900,   901,   902,   903,
     904,   905,   906,   907,   908,   909,   913,   914,   918,   919,
     923,   924,   929,   931,   936,   941,   942,   943,   945,   950,
     952,   957,   962,   964,   966,   971,   972,   976,   977,   979,
     983,   990,   997,  1001,  1007,  1009,  1013,  1014,  1020,  1022,
    1026,  1028,  1033,  1034,  1035,  1036,  1039,  1040,  1044,  1049,
    1049,  1055,  1055,  1062,  1061,  1067,  1067,  1072,  1073,  1074,
    1075,  1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,
    1085,  1086,  1090,  1088,  1097,  1095,  1102,  1112,  1106,  1116,
    1114,  1118,  1122,  1126,  1130,  1134,  1138,  1142,  1147,  1148,
    1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,
    1162,  1163,  1185,  1191,  1192,  1201,  1203,  1207,  1208,  1209,
    1213,  1214,  1218,  1218,  1223,  1229,  1233,  1233,  1241,  1242,
    1246,  1247,  1251,  1257,  1255,  1272,  1269,  1287,  1284,  1302,
    1301,  1310,  1308,  1320,  1319,  1338,  1336,  1355,  1354,  1363,
    1361,  1372,  1372,  1379,  1378,  1390,  1388,  1401,  1402,  1406,
    1409,  1412,  1413,  1414,  1417,  1418,  1421,  1423,  1426,  1427,
    1430,  1431,  1434,  1435,  1439,  1440,  1445,  1446,  1449,  1450,
    1451,  1455,  1456,  1460,  1461,  1465,  1466,  1470,  1471,  1476,
    1477,  1483,  1484,  1485,  1486,  1489,  1492,  1494,  1497,  1498,
    1502,  1504,  1507,  1510,  1513,  1514,  1517,  1518,  1522,  1528,
    1534,  1541,  1543,  1548,  1553,  1559,  1563,  1568,  1573,  1578,
    1584,  1590,  1596,  1602,  1608,  1615,  1625,  1630,  1635,  1641,
    1643,  1647,  1651,  1656,  1660,  1664,  1668,  1672,  1677,  1682,
    1687,  1692,  1697,  1703,  1712,  1713,  1714,  1718,  1720,  1723,
    1725,  1727,  1729,  1731,  1734,  1737,  1740,  1746,  1747,  1750,
    1751,  1752,  1756,  1757,  1759,  1760,  1764,  1766,  1769,  1773,
    1779,  1781,  1784,  1784,  1788,  1787,  1791,  1793,  1796,  1799,
    1797,  1814,  1811,  1826,  1828,  1830,  1832,  1834,  1836,  1838,
    1842,  1843,  1844,  1847,  1853,  1857,  1863,  1866,  1871,  1873,
    1878,  1883,  1887,  1888,  1892,  1893,  1895,  1897,  1903,  1904,
    1906,  1910,  1911,  1916,  1920,  1921,  1925,  1926,  1930,  1932,
    1938,  1943,  1944,  1946,  1950,  1951,  1952,  1953,  1957,  1958,
    1959,  1960,  1961,  1962,  1964,  1969,  1972,  1973,  1977,  1978,
    1982,  1983,  1986,  1987,  1990,  1991,  1994,  1995,  1999,  2000,
    2001,  2002,  2003,  2004,  2005,  2009,  2010,  2013,  2014,  2015,
    2018,  2020,  2022,  2023,  2026,  2028,  2032,  2034,  2038,  2042,
    2046,  2051,  2055,  2056,  2058,  2059,  2060,  2061,  2064,  2065,
    2069,  2070,  2074,  2075,  2076,  2077,  2081,  2085,  2090,  2094,
    2098,  2102,  2106,  2111,  2115,  2116,  2117,  2118,  2119,  2123,
    2127,  2129,  2130,  2131,  2134,  2135,  2136,  2137,  2138,  2139,
    2140,  2141,  2142,  2143,  2144,  2145,  2146,  2147,  2148,  2149,
    2150,  2151,  2152,  2153,  2154,  2155,  2156,  2157,  2158,  2159,
    2160,  2161,  2162,  2163,  2164,  2165,  2166,  2167,  2168,  2169,
    2170,  2171,  2172,  2173,  2174,  2175,  2176,  2177,  2179,  2180,
    2182,  2183,  2185,  2186,  2187,  2188,  2189,  2190,  2191,  2192,
    2193,  2194,  2195,  2196,  2197,  2198,  2199,  2200,  2201,  2202,
    2203,  2204,  2205,  2206,  2207,  2208,  2209,  2213,  2217,  2222,
    2221,  2237,  2235,  2254,  2253,  2274,  2273,  2293,  2292,  2311,
    2311,  2328,  2328,  2347,  2348,  2349,  2354,  2356,  2360,  2364,
    2370,  2374,  2380,  2382,  2386,  2388,  2392,  2396,  2397,  2401,
    2403,  2407,  2409,  2413,  2415,  2419,  2422,  2427,  2429,  2433,
    2436,  2441,  2445,  2449,  2453,  2457,  2461,  2465,  2469,  2473,
    2477,  2481,  2485,  2489,  2493,  2497,  2501,  2503,  2507,  2509,
    2513,  2515,  2519,  2526,  2533,  2535,  2540,  2541,  2542,  2543,
    2544,  2545,  2546,  2547,  2548,  2550,  2551,  2555,  2556,  2557,
    2558,  2562,  2568,  2581,  2598,  2599,  2602,  2605,  2608,  2609,
    2612,  2616,  2619,  2622,  2629,  2630,  2634,  2635,  2637,  2642,
    2643,  2644,  2645,  2646,  2647,  2648,  2649,  2650,  2651,  2652,
    2653,  2654,  2655,  2656,  2657,  2658,  2659,  2660,  2661,  2662,
    2663,  2664,  2665,  2666,  2667,  2668,  2669,  2670,  2671,  2672,
    2673,  2674,  2675,  2676,  2677,  2678,  2679,  2680,  2681,  2682,
    2683,  2684,  2685,  2686,  2687,  2688,  2689,  2690,  2691,  2692,
    2693,  2694,  2695,  2696,  2697,  2698,  2699,  2700,  2701,  2702,
    2703,  2704,  2705,  2706,  2707,  2708,  2709,  2710,  2711,  2712,
    2713,  2714,  2715,  2716,  2717,  2718,  2719,  2720,  2721,  2722,
    2723,  2724,  2728,  2733,  2734,  2738,  2739,  2740,  2741,  2743,
    2747,  2748,  2759,  2760,  2762,  2764,  2776,  2777,  2778,  2782,
    2783,  2784,  2788,  2789,  2790,  2793,  2795,  2799,  2800,  2801,
    2802,  2804,  2805,  2806,  2807,  2808,  2809,  2810,  2811,  2812,
    2813,  2816,  2821,  2822,  2823,  2825,  2826,  2828,  2829,  2830,
    2831,  2832,  2833,  2834,  2835,  2836,  2838,  2840,  2842,  2844,
    2846,  2847,  2848,  2849,  2850,  2851,  2852,  2853,  2854,  2855,
    2856,  2857,  2858,  2859,  2860,  2861,  2862,  2864,  2866,  2868,
    2870,  2871,  2874,  2875,  2879,  2883,  2885,  2889,  2890,  2894,
    2900,  2903,  2907,  2908,  2909,  2910,  2911,  2912,  2913,  2918,
    2920,  2924,  2925,  2928,  2929,  2933,  2936,  2938,  2940,  2944,
    2945,  2946,  2947,  2950,  2954,  2955,  2956,  2957,  2961,  2963,
    2970,  2971,  2972,  2973,  2978,  2979,  2980,  2981,  2983,  2984,
    2986,  2987,  2988,  2989,  2990,  2994,  2996,  3000,  3002,  3005,
    3008,  3010,  3012,  3015,  3017,  3021,  3023,  3026,  3029,  3035,
    3037,  3040,  3041,  3046,  3049,  3053,  3053,  3058,  3061,  3062,
    3066,  3067,  3071,  3072,  3073,  3077,  3082,  3087,  3088,  3092,
    3097,  3102,  3103,  3107,  3109,  3110,  3115,  3117,  3122,  3133,
    3147,  3159,  3174,  3175,  3176,  3177,  3178,  3179,  3180,  3190,
    3199,  3201,  3203,  3207,  3211,  3212,  3213,  3214,  3215,  3231,
    3232,  3235,  3242,  3243,  3244,  3245,  3246,  3247,  3248,  3249,
    3251,  3256,  3260,  3261,  3265,  3268,  3272,  3279,  3283,  3292,
    3299,  3307,  3309,  3310,  3314,  3315,  3316,  3318,  3323,  3324,
    3335,  3336,  3337,  3338,  3349,  3352,  3355,  3356,  3357,  3358,
    3369,  3373,  3374,  3375,  3377,  3378,  3379,  3383,  3385,  3388,
    3390,  3391,  3392,  3393,  3396,  3398,  3399,  3403,  3405,  3408,
    3410,  3411,  3412,  3416,  3418,  3421,  3424,  3426,  3428,  3432,
    3433,  3435,  3436,  3442,  3443,  3445,  3455,  3457,  3459,  3462,
    3463,  3464,  3468,  3469,  3470,  3471,  3472,  3473,  3474,  3475,
    3476,  3477,  3478,  3482,  3483,  3487,  3489,  3497,  3499,  3503,
    3507,  3512,  3516,  3524,  3525,  3529,  3530,  3536,  3537,  3546,
    3547,  3555,  3558,  3562,  3565,  3570,  3575,  3578,  3581,  3583,
    3585,  3587,  3591,  3593,  3594,  3595,  3598,  3600,  3606,  3607,
    3611,  3612,  3616,  3617,  3621,  3622,  3625,  3630,  3631,  3635,
    3638,  3640,  3644,  3650,  3651,  3652,  3656,  3660,  3668,  3673,
    3685,  3687,  3691,  3694,  3696,  3701,  3706,  3712,  3715,  3720,
    3725,  3727,  3734,  3736,  3739,  3740,  3743,  3746,  3747,  3752,
    3754,  3758,  3764,  3774,  3775
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
  "T_VARIABLE", "T_PIPE_VAR", "T_NUM_STRING", "T_INLINE_HTML", "T_INOUT",
  "T_HASHBANG", "T_CHARACTER", "T_BAD_CHARACTER",
  "T_ENCAPSED_AND_WHITESPACE", "T_CONSTANT_ENCAPSED_STRING", "T_ECHO",
  "T_DO", "T_WHILE", "T_ENDWHILE", "T_FOR", "T_ENDFOR", "T_FOREACH",
  "T_ENDFOREACH", "T_DECLARE", "T_ENDDECLARE", "T_AS", "T_SUPER",
  "T_SWITCH", "T_ENDSWITCH", "T_CASE", "T_DEFAULT", "T_BREAK", "T_GOTO",
  "T_CONTINUE", "T_FUNCTION", "T_CONST", "T_RETURN", "T_TRY", "T_CATCH",
  "T_THROW", "T_USING", "T_USE", "T_GLOBAL", "T_STATIC", "T_ABSTRACT",
  "T_FINAL", "T_PRIVATE", "T_PROTECTED", "T_PUBLIC", "T_VAR", "T_UNSET",
  "T_ISSET", "T_EMPTY", "T_HALT_COMPILER", "T_CLASS", "T_INTERFACE",
  "T_EXTENDS", "T_IMPLEMENTS", "T_OBJECT_OPERATOR",
  "T_NULLSAFE_OBJECT_OPERATOR", "T_DOUBLE_ARROW", "T_LIST", "T_ARRAY",
  "T_DICT", "T_VEC", "T_VARRAY", "T_DARRAY", "T_KEYSET", "T_CALLABLE",
  "T_CLASS_C", "T_METHOD_C", "T_FUNC_C", "T_LINE", "T_FILE", "T_COMMENT",
  "T_DOC_COMMENT", "T_OPEN_TAG", "T_OPEN_TAG_WITH_ECHO", "T_CLOSE_TAG",
  "T_WHITESPACE", "T_START_HEREDOC", "T_END_HEREDOC",
  "T_DOLLAR_OPEN_CURLY_BRACES", "T_CURLY_OPEN", "T_DOUBLE_COLON",
  "T_NAMESPACE", "T_NS_C", "T_DIR", "T_NS_SEPARATOR", "T_XHP_LABEL",
  "T_XHP_TEXT", "T_XHP_ATTRIBUTE", "T_XHP_CATEGORY",
  "T_XHP_CATEGORY_LABEL", "T_XHP_CHILDREN", "T_ENUM", "T_XHP_REQUIRED",
  "T_TRAIT", "\"...\"", "T_INSTEADOF", "T_TRAIT_C", "T_HH_ERROR",
  "T_FINALLY", "T_XHP_TAG_LT", "T_XHP_TAG_GT", "T_TYPELIST_LT",
  "T_TYPELIST_GT", "T_UNRESOLVED_LT", "T_COLLECTION", "T_SHAPE", "T_TYPE",
  "T_UNRESOLVED_TYPE", "T_NEWTYPE", "T_UNRESOLVED_NEWTYPE",
  "T_COMPILER_HALT_OFFSET", "T_ASYNC", "T_LAMBDA_OP", "T_LAMBDA_CP",
  "T_UNRESOLVED_OP", "T_WHERE", "'('", "')'", "';'", "'{'", "'}'", "']'",
  "'$'", "'`'", "'\"'", "'\\''", "$accept", "start", "$@1",
  "top_statement_list", "top_statement", "$@2", "$@3",
  "ident_no_semireserved", "ident_for_class_const", "ident",
  "group_use_prefix", "non_empty_use_declarations", "use_declarations",
  "use_declaration", "non_empty_mixed_use_declarations",
  "mixed_use_declarations", "mixed_use_declaration", "namespace_name",
  "namespace_string", "namespace_string_typeargs",
  "class_namespace_string_typeargs", "constant_declaration",
  "function_statement_list", "function_statement", "inner_statement_list",
  "inner_statement_list_nonempty", "inner_statement", "statement", "$@4",
  "$@5", "$@6", "$@7", "$@8", "$@9", "$@10", "$@11", "using_prologue",
  "opt_await", "using_expr_list", "using_expr", "using_statement_list",
  "try_statement_list", "$@12", "additional_catches",
  "finally_statement_list", "$@13", "optional_finally", "is_reference",
  "function_loc", "function_declaration_statement", "$@14", "$@15", "$@16",
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
  "parameter_list", "non_empty_parameter_list", "inout_variable",
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
  "class_type_constant", "parenthesis_expr_with_parens",
  "expr_with_parens", "expr_list", "for_expr", "yield_expr",
  "yield_assign_expr", "yield_list_assign_expr", "yield_from_expr",
  "yield_from_assign_expr", "await_expr", "await_assign_expr",
  "await_list_assign_expr", "parenthesis_expr", "expr",
  "parenthesis_expr_no_variable", "expr_no_variable", "lambda_use_vars",
  "closure_expression", "$@30", "$@31", "lambda_expression", "$@32",
  "$@33", "$@34", "$@35", "$@36", "lambda_body", "shape_keyname",
  "non_empty_shape_pair_list", "non_empty_static_shape_pair_list",
  "shape_pair_list", "static_shape_pair_list", "shape_literal",
  "array_literal", "dict_pair_list", "non_empty_dict_pair_list",
  "static_dict_pair_list", "non_empty_static_dict_pair_list",
  "static_dict_pair_list_ae", "non_empty_static_dict_pair_list_ae",
  "dict_literal", "static_dict_literal", "static_dict_literal_ae",
  "vec_literal", "static_vec_literal", "static_vec_literal_ae",
  "keyset_literal", "static_keyset_literal", "static_keyset_literal_ae",
  "varray_literal", "static_varray_literal", "static_varray_literal_ae",
  "darray_literal", "static_darray_literal", "static_darray_literal_ae",
  "vec_ks_expr_list", "static_vec_ks_expr_list",
  "static_vec_ks_expr_list_ae", "collection_literal",
  "static_collection_literal", "dim_expr", "dim_expr_base",
  "lexical_var_list", "xhp_tag", "xhp_tag_body", "xhp_opt_end_label",
  "xhp_attributes", "xhp_children", "xhp_attribute_name",
  "xhp_attribute_value", "xhp_child", "xhp_label_ws", "xhp_bareword",
  "simple_function_call", "fully_qualified_class_name",
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
  "parenthesis_variable", "dimmable_variable_no_parens",
  "callable_variable", "lambda_or_closure_with_parens",
  "lambda_or_closure", "object_method_call", "class_method_call",
  "variable_no_objects", "reference_variable", "compound_variable",
  "dim_offset", "variable_no_calls", "dimmable_variable_no_calls",
  "assignment_list", "array_pair_list", "non_empty_array_pair_list",
  "collection_init", "non_empty_collection_init", "static_collection_init",
  "non_empty_static_collection_init", "encaps_list", "encaps_var",
  "encaps_var_offset", "internal_functions", "variable_list",
  "class_constant", "hh_opt_constraint", "hh_type_alias_statement",
  "hh_name_with_type", "hh_constname_with_type", "hh_name_with_typevar",
  "hh_name_no_semireserved_with_typevar", "hh_typeargs_opt",
  "hh_non_empty_type_list", "hh_type_list", "hh_non_empty_func_type_list",
  "hh_func_type_list", "opt_type_constraint_where_clause",
  "non_empty_constraint_list", "hh_generalised_constraint",
  "opt_return_type", "hh_constraint", "hh_typevar_list",
  "hh_non_empty_constraint_list", "hh_non_empty_typevar_list",
  "hh_typevar_variance", "hh_shape_member_type",
  "hh_non_empty_shape_member_list", "hh_shape_member_list",
  "hh_shape_type", "hh_access_type_start", "hh_access_type",
  "array_typelist", "hh_type", "hh_type_opt", YY_NULLPTR
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
     426,   427,   428,   429,   430,   431,   432,   433,    40,    41,
      59,   123,   125,    93,    36,    96,    34,    39
};
# endif

#define YYPACT_NINF -1768

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1768)))

#define YYTABLE_NINF -1125

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1125)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1768,   212, -1768, -1768,  5510, 15019, 15019,    35, 15019, 15019,
   15019, 15019, 12444, 15019, -1768, 15019, 15019, 15019, 15019, 18390,
   18390, 15019, 15019, 15019, 15019, 15019, 15019, 15019, 15019, 12640,
   19187, 15019,   222,   302, -1768, -1768, -1768,   158, -1768,   262,
   -1768, -1768, -1768,   435, 15019, -1768,   302,   336,   347,   379,
   -1768,   302, 12836, 16306, 13041, -1768, 15898, 11250,   382, 15019,
   19496,   364,   112,   471,   329, -1768, -1768, -1768,   404,   407,
     428,   432, -1768, 16306,   442,   451,   478,   587,   606,   612,
     617, -1768, -1768, -1768, -1768, -1768, 15019,   595,  3791, -1768,
   -1768, 16306, -1768, -1768, -1768, -1768, 16306, -1768, 16306, -1768,
     531,   510, 16306, 16306, -1768,   350, -1768, -1768, 13246, -1768,
   -1768,   487,   552,   659,   659, -1768,   684,   567,   255,   541,
   -1768,    88, -1768,   545,   630,   725, -1768, -1768, -1768, -1768,
    1381,   571, -1768,   152, -1768,   572,   582,   585,   590,   593,
     610,   615,   624,  3323, -1768, -1768, -1768, -1768, -1768,   131,
     697,   747,   756,   762,   770, -1768,   775,   785, -1768,   251,
     665, -1768,   708,    25, -1768,  1494,   198, -1768, -1768,  3029,
     152,   152,   675,   138, -1768,   171,   122,   682,   205, -1768,
   -1768,   812, -1768,   724, -1768, -1768,   692,   752, -1768, 15019,
   -1768,   725,   571, 19831,  4623, 19831, 15019, 19831, 19831, 19912,
   19912,   720, 18562, 19831,   863, 16306,   854,   854,   596,   854,
   -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768,    91,
   15019,   744, -1768, -1768,   769,   742,   586,   743,   586,   854,
     854,   854,   854,   854,   854,   854,   854, 18390, 18610,   739,
     934,   724, -1768, 15019,   744, -1768,   787, -1768,   790,   757,
   -1768,   181, -1768, -1768, -1768,   586,   152, -1768, 13442, -1768,
   -1768, 15019, 10020,   948,   100, 19831, 11045, -1768, 15019, 15019,
   16306, -1768, -1768,  3963,   758, -1768, 17248, -1768, -1768, -1768,
   -1768, -1768, -1768, -1768, -1768, -1768, -1768,  4826, -1768,  4826,
   -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768,
   -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768,
   -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768,
   -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768,
   -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768,   124,
     126,   752, -1768, -1768, -1768, -1768,   759,  3481,   133, -1768,
   -1768,   799,   949, -1768,   804, 16625, 15019, -1768,   768,   777,
   17298, -1768,    74, 17368, 15771, 15771, 15771, 16306, 15771,   778,
     969,   780, -1768,   101, -1768, 17977,   114, -1768,   970,   116,
     853, -1768,   856, -1768, 18390, 15019, 15019,   792,   806, -1768,
   -1768, 18078, 12640, 15019, 15019, 15019, 15019, 15019,   127,   456,
     419, -1768, 15215, 18390,   619, -1768, 16306, -1768,   421,   567,
   -1768, -1768, -1768, -1768, 19288,   979,   891, -1768, -1768, -1768,
     113, 15019,   800,   801, 19831,   810,  2370,   811,  6125, 15019,
   -1768,    84,   808,   662,    84,   546,   524, -1768, 16306,  4826,
     813, 11455, 15898, -1768, 13647,   816,   816,   816,   816, -1768,
   -1768, 16327, -1768, -1768, -1768, -1768, -1768,   725, -1768, 15019,
   15019, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768,
   15019, 15019, 15019, 15019, 13843, 15019, 15019, 15019, 15019, 15019,
   15019, 15019, 15019, 15019, 15019, 15019, 15019, 15019, 15019, 15019,
   15019, 15019, 15019, 15019, 15019, 15019, 15019, 15019, 19389, 15019,
   -1768, 15019, 15019, 15019,  4945, 16306, 16306, 16306, 16306, 16306,
    1381,   902,   897,  5126, 15019, 15019, 15019, 15019, 15019, 15019,
   15019, 15019, 15019, 15019, 15019, 15019, -1768, -1768, -1768, -1768,
    2433, -1768, -1768, 11455, 11455, 15019, 15019, 18078,   826,   725,
   14039, 17416, -1768, 15019, -1768,   830,  1023,   874,   835,   837,
   15388,   586, 14235, -1768, 14431, -1768,   757,   844,   845,  2893,
   -1768,    72, 11455, -1768,  3116, -1768, -1768, 17466, -1768, -1768,
   11847, -1768, 15019, -1768,   950, 10225,  1038,   850, 19709,  1040,
     111,    92, -1768, -1768, -1768,   869, -1768, -1768, -1768,  4826,
   -1768,   705,   858,  1048, 17803, 16306, -1768, -1768, -1768, -1768,
   -1768, -1768, -1768, -1768, -1768,   861, -1768, -1768,   864,   870,
     868,   875,   877,   880,   418,   883,   881, 19508, 16673, -1768,
   -1768, 16306, 16306, 15019,   586,   364, -1768, 17803,   978, -1768,
   -1768, -1768,   586,   161,   164,   911,   912,  4032,   375,   913,
     917,   737,   984,   921,   586,   165,   923, 18671,   920,  1116,
    1118,   925,   929,   930,   936, -1768,  1222, 16306, -1768, -1768,
    1070,  3103,    99, -1768, -1768, -1768,   567, -1768, -1768, -1768,
    1109,  1010,   964,   258,   988, 15019,  1015,  1146,   957, -1768,
     996, -1768,   221, -1768,  4826,  4826,  1150,   948,   113, -1768,
     971,  1156, -1768, 17505,    82, -1768,   505,   209, -1768, -1768,
   -1768, -1768, -1768, -1768, -1768,   916,  3788, -1768, -1768, -1768,
   -1768,  1158,   985, -1768, 18390,   712, 15019,   972,  1161, 19831,
    1159,   166,  1165,   976,   977,   980, 19831,   981,  4089,  6330,
   -1768, -1768, -1768, -1768, -1768, -1768,  1043, 16518, 19831,   982,
    4033, 13430, 15203, 19912, 16077, 15019, 19783, 19949, 12619, 14017,
   14212, 13819,  5414, 14602, 14602, 14602, 14602,  3666,  3666,  3666,
    3666,  3666,   882,   882,   860,   860,   860,   596,   596,   596,
   -1768,   854,   987,   989, 18719,   986,  1172,    41, 15019,   248,
     744,   362, -1768, -1768, -1768,  1169,   891, -1768,   725, 18188,
   -1768, -1768, -1768, 19912, 19912, 19912, 19912, 19912, 19912, 19912,
   19912, 19912, 19912, 19912, 19912, 19912, -1768, 15019,   448, -1768,
     226, -1768,   744,   492,   992,   998,  1001,  4294,   169,   997,
   -1768, 19831, 17879, -1768, 16306, -1768,   586,    57, 18390, 19831,
   18390, 18780,  1043,   292,   586,   229, -1768,   221,  1045,  1009,
   15019, -1768,   236, -1768, -1768, -1768,  6535,   784, -1768, -1768,
   19831, 19831,   302, -1768, -1768, -1768, 15019,  1106, 17651, 17803,
   16306, 10430,  1012,  1013, -1768,  1205, 19544,  1078, -1768,  1055,
   -1768,  1209,  1026,  4416,  4826, 17803, 17803, 17803, 17803, 17803,
    1024,  1160,  1167,  1170,  1171,  1175,  1033, 17803,    47, -1768,
   -1768, -1768, -1768, -1768, -1768,     9, -1768, 12819, -1768, -1768,
      49, -1768,  6740, 15593,  1047, 16673, -1768, 16673, -1768, 16673,
   -1768, 16306, 16306, 16673, -1768, 16673, 16673, 16306, -1768,  1240,
    1050, -1768,   420, -1768, -1768,  4780, -1768, 12819,  1242, 18390,
    1054, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768,
    1074,  1252, 16306, 15593,  1062, 18078, 18289,  1253, -1768, 15019,
   -1768, 15019, -1768, 15019, -1768, -1768, -1768, -1768, -1768, -1768,
   -1768, -1768,  1063, -1768, 15019, -1768, -1768,  5715, -1768,  4826,
   15593,  1067, -1768, -1768, -1768, -1768,  1257,  1073, 15019, 19288,
   -1768, -1768,  4945,  1079, -1768,  4826, -1768,  1083,  6945,  1249,
      78, -1768,  4826, -1768,    96,  2433,  2433, -1768,  4826, -1768,
   -1768,   586, -1768, -1768,  1203, 19831, -1768, 11651, -1768, 17803,
   13647,   816, 13647, -1768,   816,   816, -1768, 12052, -1768, -1768,
    7150, -1768,   103,  1076, 15593,  1010, -1768, -1768, -1768, -1768,
   19949, 15019, -1768, -1768, 15019, -1768, 15019, -1768,  5333,  1088,
   11455,   984,  1256,  1010,  4826,  1275,  1043, 16306, 19389,   586,
    5383,  1100,   217,  1104, -1768, -1768,  1290,  2793,  2793, 17879,
   -1768, -1768, -1768,  1254,  1110,  1239,  1243,  1245,  1246,  1248,
      98,  1121,   423, -1768, -1768, -1768, -1768, -1768,  1166, -1768,
   -1768, -1768, -1768,  1311,  1122,   830,   586,   586, 14627,  1010,
    3116, -1768,  3116, -1768, 13027,   786,   302, 11045, -1768,  7355,
    1123,  7560,  1127, 17651, 18390,  1133,  1195,   586, 12819,  1321,
   -1768, -1768, -1768, -1768,   693, -1768,   153,  4826,  1152,  1200,
    1177,  4826, 16306,  1806, -1768, -1768,  4826,  1331,  1144,  1168,
    1173,  1158,   722,   722,  1276,  1276, 18937,  1145,  1342, 17803,
   17803, 17803, 17803, 17803, 17803, 19288,  3420, 16779, 17803, 17803,
   17803, 17803, 17727, 17803, 17803, 17803, 17803, 17803, 17803, 17803,
   17803, 17803, 17803, 17803, 17803, 17803, 17803, 17803, 17803, 17803,
   17803, 17803, 17803, 17803, 17803, 17803, 16306, -1768, -1768,  1269,
   -1768, -1768,  1153,  1155,  1178, -1768,  1179, -1768, -1768,   433,
   19508, -1768,  1162, -1768, 17803,   586, -1768, -1768,   148, -1768,
     702,  1349, -1768, -1768,   172,  1181,   586, 12248, 19831, 18828,
   -1768,  2696, -1768,  5920,   891,  1349, -1768,    46,    61, -1768,
   19831,  1230,  1185, -1768,  1193,  1249, -1768, -1768, -1768, 14823,
    4826,   948, 17553,  1277,    76,  1355,  1292,   261, -1768,   744,
     324, -1768,   744, -1768, 15019, 18390,   712, 15019, 19831, 12819,
   -1768, -1768, -1768,  4779, -1768, -1768, -1768, -1768, -1768, -1768,
    1174,   103, -1768,  1176,   103,  1207, 19949, 19831, 18889,  1208,
   11455,  1204,  1210,  4826,  1211,  1214,  4826,  1010, -1768,   757,
     506, 11455, 15019, -1768, -1768, -1768, -1768, -1768, -1768,  1271,
    1213,  1401,  1325, 17879, 17879, 17879, 17879, 17879, 17879,  1262,
   -1768, 19288,   150, 17879, -1768, -1768, -1768, 18390, 19831,  1224,
   -1768,   302,  1391,  1357, 11045, -1768, -1768, -1768,  1229, 15019,
    1195,   586, 18078, 17651,  1237, 17803,  7765,   723,  1238, 15019,
      97,   356, -1768,  1264, -1768,  4826, 16306, -1768,  1299, -1768,
   -1768, -1768, 17456, -1768,  1415, -1768,  1251, 17803, -1768, 17803,
   -1768,  1260,  1250,  1442, 18997,  1258, 12819,  1443,  1261,  1270,
    1274,  1317,  1454,  1273, -1768, -1768, -1768, 19043,  1272,  1469,
   15002, 19875, 11435, 17803, 12427,  4081,  5456,  3859, 14407, 15384,
   16256, 16256, 16256, 16256,  2186,  2186,  2186,  2186,  2186,  1241,
    1241,   722,   722,   722,  1276,  1276,  1276,  1276, -1768,  1280,
   -1768,  1281,  1282,  1283,  1284, -1768, -1768, 12819, 16306,  4826,
    4826, -1768,   702, 15593,  1263, -1768, 18078, -1768, -1768, 19912,
   15019,  1278, -1768,  1285,  1439, -1768,   244, 15019, -1768, -1768,
   -1768, 15019, -1768, 15019, -1768,   948, 13647,  1288,   339,   816,
     339,   225, -1768, -1768,  4826,   142, -1768,  1475,  1407, 15019,
   -1768,  1293,  1294,  1291,   586,  1203, 19831,  1249,  1296, -1768,
    1297,   103, 15019, 11455,  1298, -1768, -1768,   891, -1768, -1768,
    1301,  1302,  1295, -1768,  1303, 17879, -1768, 17879, -1768, -1768,
    1306,  1304,  1489,  1368,  1305, -1768,  1500,  1308,  1310,  1312,
   -1768,  1376,  1318,  1507, -1768, -1768,   586, -1768,  1486, -1768,
    1327, -1768, -1768,  1322,  1329,   174, -1768, -1768, 12819,  1330,
    1334, -1768, 17200, -1768, -1768, -1768, -1768, -1768, -1768,  1393,
    4826,  4826,  1168,  1360,  4826, -1768, 12819, 19103, -1768, -1768,
   17803, -1768, 17803, -1768, 17803, -1768, -1768, -1768, -1768, 17803,
   19288, -1768, -1768, 17803, -1768, 17803, -1768, 11631, 17803,  1338,
    7970, -1768, -1768, -1768, -1768,   702, -1768, -1768, -1768, -1768,
     678, 16076, 15593,  1428, -1768,  2938,  1371,  3559, -1768, -1768,
   -1768,   902,  4289,   130,   134,  1343,   891,   897,   179, 19831,
   -1768, -1768, -1768,  1378, 17104, 17152, 19831, -1768,  4179, -1768,
    6330,  1463,    93,  1535,  1474, 15019, -1768, 19831, 11455, 11455,
   -1768,  1448,  1249,  2082,  1249,  1370, 19831,  1373, -1768,  2146,
    1369,  2209, -1768, -1768,   103, -1768, -1768,  1435, -1768, -1768,
   17879, -1768, 17879, -1768, 17879, -1768, -1768, -1768, -1768, 17879,
   -1768, 19288, -1768,  2295, -1768,  8175, -1768, -1768, -1768, -1768,
   10635, -1768, -1768, -1768,  6535,  4826, -1768, -1768, -1768,  1383,
   17803, 19149, 12819, 12819, 12819,  1437, 12819, 19209, 11631, -1768,
   -1768,   702, 15593, 15593, 16306, -1768,  1563, 16933,    80, -1768,
   16076,   891, 19461, -1768,  1397, -1768,   137,  1382,   139, -1768,
   16433, -1768, -1768, -1768,   149, -1768, -1768, 16148, -1768,  1384,
   -1768,  1503,   725, -1768, 16255, -1768, 16255, -1768, -1768,  1573,
     902, -1768, 15542, -1768, -1768, -1768, -1768,  3193, -1768,  1574,
    1508, 15019, -1768, 19831,  1396,  1398,  1394,  1249,  1399, -1768,
    1448,  1249, -1768, -1768, -1768, -1768,  2377,  1400, 17879,  1462,
   -1768, -1768, -1768,  1464, -1768,  6535, 10840, 10635, -1768, -1768,
   -1768,  6535, -1768, -1768, 12819, 17803, 17803, 17803,  8380,  1405,
    1406, -1768, 17803, -1768, 15593, -1768, -1768, -1768, -1768, -1768,
    4826,  1768,  2938, -1768, -1768, -1768, -1768, -1768, -1768, -1768,
   -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768,
   -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768,
   -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768,
   -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768,
   -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768,
   -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768,
   -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768,
   -1768, -1768, -1768, -1768, -1768, -1768, -1768,   189, -1768,  1371,
   -1768, -1768, -1768, -1768, -1768,   160,   616, -1768,  1593,   154,
   16625,  1503,  1596, -1768,  4826,   725, -1768, -1768,  1414,  1607,
   15019, -1768, 19831, -1768, -1768,   175,  1421,  4826,   664,  1249,
    1399, 15720, -1768,  1249, -1768, 17879, 17879, -1768, -1768, -1768,
   -1768,  8585, 12819, 12819, 12819, -1768, -1768, -1768, 12819, -1768,
    3469,  1614,  1615,  1423, -1768, -1768, 17803, 16433, 16433,  1560,
   -1768, 16148, 16148,   674, -1768, -1768, -1768, 17803,  1546, -1768,
    1447,  1433,   156, 17803, -1768, 16625, -1768, 17803, 19831,  1550,
   -1768,  1625, -1768,  1626, -1768,   548, -1768, -1768, -1768,  1436,
     664, -1768,   664, -1768, -1768,  8790,  1438,  1520, -1768,  1537,
    1480, -1768, -1768,  1540,  4826,  1460,  1768, -1768, -1768, 12819,
   -1768, -1768,  1471, -1768,  1610, -1768, -1768, -1768, -1768, 12819,
    1635,   737, -1768, -1768, 12819,  1453, 12819, -1768,   188,  1459,
    8995,  4826, -1768,  4826, -1768,  9200, -1768, -1768, -1768,  1452,
   -1768,  1455,  1479, 16306,   897,  1476, -1768, -1768, -1768, 17803,
    1478,   105, -1768,  1581, -1768, -1768, -1768, -1768, -1768, -1768,
    9405, -1768, 15593,  1047, -1768,  1491, 16306,   782, -1768, 12819,
   -1768,  1467,  1661,   633,   105, -1768, -1768,  1591, -1768, 15593,
    1481, -1768,  1249,   106, -1768,  4826, -1768, -1768, -1768,  4826,
   -1768,  1477,  1487,   157, -1768,  1399,   645,  1597,   159,  1249,
    1484, -1768,   688,  4826,  4826, -1768,   325,  1665,  1605,  1399,
   -1768, -1768, -1768, -1768,  1613,   182,  1676,  1618, 15019, -1768,
     688,  9610,  9815, -1768,   331,  1679,  1620, 15019, -1768, 19831,
   -1768, -1768, -1768,  1690,  1622, 15019, -1768, 19831, 15019, -1768,
   19831, 19831
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   203,   472,     0,   905,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   998,
     986,     0,   771,     0,   777,   778,   779,    29,   843,   974,
     975,   170,   171,   780,     0,   151,     0,     0,     0,     0,
      30,     0,     0,     0,     0,   222,     0,     0,     0,     0,
       0,     0,   441,   442,   443,   440,   439,   438,     0,     0,
       0,     0,   251,     0,     0,     0,    37,    38,    40,    41,
      39,   784,   786,   787,   781,   782,     0,     0,     0,   788,
     783,     0,   754,    32,    33,    34,    36,    35,     0,   785,
       0,     0,     0,     0,   789,   444,   581,    31,     0,   169,
     139,     0,   772,     0,     0,     4,   125,   127,   842,     0,
     753,     0,     6,     0,     0,   221,     7,     9,     8,    10,
       0,     0,   436,     0,   486,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   542,   484,   962,   963,   563,   557,
     558,   559,   560,   561,   562,   467,   566,     0,   466,   933,
     755,   762,     0,   845,   556,   435,   936,   937,   949,   485,
       0,     0,     0,   488,   487,   934,   935,   932,   970,   973,
     546,   844,    11,   441,   442,   443,     0,     0,    36,     0,
     125,   221,     0,  1038,   485,  1039,     0,  1041,  1042,   565,
     480,     0,   473,   478,     0,     0,   528,   529,   530,   531,
      29,   974,   780,   757,    37,    38,    40,    41,    39,     0,
       0,  1062,   955,   755,     0,   756,   507,     0,   509,   547,
     548,   549,   550,   551,   552,   553,   555,     0,  1002,     0,
     852,   767,   241,     0,  1062,   464,   766,   760,     0,   776,
     756,   981,   982,   988,   980,   768,     0,   465,     0,   770,
     554,     0,   204,     0,     0,   469,   204,   149,   471,     0,
       0,   155,   157,     0,     0,   159,     0,    75,    76,    81,
      82,    67,    68,    59,    79,    90,    91,     0,    62,     0,
      66,    74,    72,    93,    85,    84,    57,   107,    80,   100,
     101,    58,    96,    55,    97,    56,    98,    54,   102,    89,
      94,    99,    86,    87,    61,    88,    92,    53,    83,    69,
     103,    77,   105,    70,    60,    47,    48,    49,    50,    51,
      52,    71,   106,   104,   109,    64,    45,    46,    73,  1115,
    1116,    65,  1120,    44,    63,    95,     0,     0,   125,   108,
    1053,  1114,     0,  1117,     0,     0,     0,   161,     0,     0,
       0,   212,     0,     0,     0,     0,     0,     0,     0,     0,
     854,     0,   113,   115,   349,     0,     0,   348,   354,     0,
       0,   252,     0,   255,     0,     0,     0,     0,  1059,   237,
     249,   994,   998,   600,   627,   627,   600,   627,     0,  1023,
       0,   791,     0,     0,     0,  1021,     0,    16,     0,   129,
     229,   243,   250,   657,   593,     0,  1047,   573,   575,   577,
     909,   472,   486,     0,     0,   484,   485,   487,   204,     0,
     977,   773,     0,   774,     0,     0,     0,   201,     0,     0,
     131,   338,     0,    28,     0,     0,     0,     0,     0,   202,
     220,     0,   248,   233,   247,   441,   444,   221,   437,   979,
       0,   925,   191,   192,   193,   194,   195,   197,   198,   200,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   986,     0,
     190,   979,   979,  1008,     0,     0,     0,     0,     0,     0,
       0,     0,   434,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   506,   508,   910,   911,
       0,   924,   923,   338,   338,   979,     0,   994,     0,   221,
       0,     0,   163,     0,   907,   902,   852,     0,   486,   484,
       0,  1006,     0,   598,   851,   997,   776,   486,   484,   485,
     131,     0,   338,   463,     0,   926,   769,     0,   139,   291,
       0,   580,     0,   166,     0,   204,   470,     0,     0,     0,
       0,     0,   158,   189,   160,  1115,  1116,  1112,  1113,     0,
    1119,  1105,     0,     0,     0,     0,    78,    43,    65,    42,
    1054,   196,   199,   162,   139,     0,   179,   188,     0,     0,
       0,     0,     0,     0,   116,     0,     0,     0,   853,   114,
      18,     0,   110,     0,   350,     0,   164,     0,     0,   165,
     253,   254,  1043,     0,     0,   486,   484,   485,   488,   487,
       0,  1095,   261,     0,   995,     0,     0,     0,     0,   852,
     852,     0,     0,     0,     0,   167,     0,     0,   790,  1022,
     843,     0,     0,  1020,   848,  1019,   128,     5,    13,    14,
       0,   259,     0,     0,   586,     0,     0,   852,     0,   764,
       0,   763,   758,   587,     0,     0,     0,     0,   909,   135,
       0,   854,   908,  1124,   462,   475,   489,   942,   961,   146,
     138,   142,   143,   144,   145,   435,     0,   564,   846,   847,
     126,   852,     0,  1063,     0,     0,     0,     0,   854,   339,
       0,     0,     0,   486,   208,   209,   207,   484,   485,   204,
     183,   181,   182,   184,   569,   223,   257,     0,   978,     0,
       0,   512,   514,   513,   525,     0,     0,   545,   510,   511,
     515,   517,   516,   534,   535,   532,   533,   536,   537,   538,
     539,   540,   526,   527,   519,   520,   518,   521,   522,   524,
     541,   523,     0,     0,  1012,     0,   852,  1046,     0,  1045,
    1062,   939,   239,   231,   245,     0,  1047,   235,   221,     0,
     476,   479,   481,   491,   494,   495,   496,   497,   498,   499,
     500,   501,   502,   503,   504,   505,   913,     0,   912,   915,
     938,   919,  1062,   916,     0,     0,     0,     0,     0,     0,
    1040,   474,   900,   904,   851,   906,   461,   759,     0,  1001,
       0,  1000,   257,     0,   759,   985,   984,   970,   973,     0,
       0,   912,   915,   983,   916,   483,   293,   295,   135,   584,
     583,   468,     0,   139,   275,   150,   471,     0,     0,     0,
       0,   204,   287,   287,   156,   852,     0,     0,  1104,     0,
    1101,   852,     0,  1075,     0,     0,     0,     0,     0,   850,
       0,    37,    38,    40,    41,    39,     0,     0,   793,   797,
     798,   799,   800,   801,   803,     0,   792,   133,   841,   802,
    1062,  1118,   204,     0,     0,     0,    21,     0,    22,     0,
      19,     0,   111,     0,    20,     0,     0,     0,   122,   854,
       0,   120,   115,   112,   117,     0,   347,   355,   352,     0,
       0,  1032,  1037,  1034,  1033,  1036,  1035,    12,  1093,  1094,
       0,   852,     0,     0,     0,   994,   991,     0,   597,     0,
     611,   851,   599,   851,   626,   614,   620,   623,   617,  1031,
    1030,  1029,     0,  1025,     0,  1026,  1028,   204,     5,     0,
       0,     0,   651,   652,   660,   659,     0,   484,     0,   851,
     592,   596,     0,     0,  1048,     0,   574,     0,   204,  1082,
     909,   319,  1124,  1123,     0,     0,     0,   976,   851,  1065,
    1061,   341,   335,   336,   340,   342,   752,   853,   337,     0,
       0,     0,     0,   461,     0,     0,   489,     0,   943,   211,
     204,   141,   909,     0,     0,   259,   571,   225,   921,   922,
     544,     0,   634,   635,     0,   632,   851,  1007,     0,     0,
     338,   261,     0,   259,     0,     0,   257,     0,   986,   492,
       0,     0,   940,   941,   971,   972,     0,     0,     0,   888,
     859,   860,   861,   868,     0,    37,    38,    40,    41,    39,
       0,     0,   874,   880,   881,   882,   883,   884,     0,   872,
     870,   871,   894,   852,     0,   902,  1005,  1004,     0,   259,
       0,   927,     0,   775,     0,   297,     0,   204,   147,   204,
       0,   204,     0,     0,     0,     0,   267,   268,   279,     0,
     139,   277,   176,   287,     0,   287,     0,   851,     0,     0,
       0,     0,     0,   851,  1103,  1106,  1071,   852,     0,  1066,
       0,   852,   824,   825,   822,   823,   858,     0,   852,   850,
     604,   629,   629,   604,   629,   595,     0,     0,  1014,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1109,   213,     0,
     216,   180,     0,     0,     0,   118,     0,   123,   124,   116,
     853,   121,     0,   351,     0,  1044,   168,  1060,  1095,  1086,
    1090,   260,   262,   361,     0,     0,   992,     0,   602,     0,
    1024,     0,    17,   204,  1047,   258,   361,     0,     0,   759,
     589,     0,   765,  1049,     0,  1082,   578,   134,   136,     0,
       0,     0,  1124,     0,     0,   324,   322,   915,   928,  1062,
     915,   929,  1062,  1064,   979,     0,     0,     0,   343,   132,
     206,   208,   209,   485,   187,   205,   185,   186,   210,   140,
       0,   909,   256,     0,   909,     0,   543,  1011,  1010,     0,
     338,     0,     0,     0,     0,     0,     0,   259,   227,   776,
     914,   338,     0,   864,   865,   866,   867,   875,   876,   892,
       0,   852,     0,   888,   608,   631,   631,   608,   631,     0,
     863,   896,     0,   851,   899,   901,   903,     0,   999,     0,
     914,     0,     0,     0,   204,   294,   585,   152,     0,   471,
     267,   269,   994,     0,     0,     0,   204,     0,     0,     0,
       0,     0,   281,     0,  1110,     0,     0,  1096,     0,  1102,
    1100,  1067,   851,  1073,     0,  1074,     0,     0,   795,   851,
     849,     0,     0,   852,     0,     0,   838,   852,     0,     0,
       0,     0,   852,     0,   804,   839,   840,  1018,     0,   852,
     807,   809,   808,     0,     0,   805,   806,   810,   812,   811,
     828,   829,   826,   827,   830,   831,   832,   833,   834,   819,
     820,   814,   815,   813,   816,   817,   818,   821,  1108,     0,
     139,     0,     0,     0,     0,   119,    23,   353,     0,     0,
       0,  1087,  1092,     0,   435,   996,   994,   477,   482,   490,
       0,     0,    15,     0,   435,   663,     0,     0,   665,   658,
     661,     0,   656,     0,  1051,     0,     0,     0,     0,   542,
       0,   488,  1083,   582,  1124,     0,   325,   326,     0,     0,
     320,     0,     0,     0,   345,   346,   344,  1082,     0,   361,
       0,   909,     0,   338,     0,   968,   361,  1047,   361,  1050,
       0,     0,     0,   493,     0,     0,   878,   851,   887,   869,
       0,     0,   852,     0,     0,   886,   852,     0,     0,     0,
     862,     0,     0,   852,   873,   893,  1003,   361,     0,   139,
       0,   290,   276,     0,     0,     0,   266,   172,   280,     0,
       0,   283,     0,   288,   289,   139,   282,  1111,  1097,     0,
       0,  1070,  1069,     0,     0,  1122,   857,   856,   794,   612,
     851,   603,     0,   615,   851,   628,   621,   624,   618,     0,
     851,   594,   796,     0,   633,   851,  1013,   836,     0,     0,
     204,    24,    25,    26,    27,  1089,  1084,  1085,  1088,   263,
       0,     0,     0,   442,   433,     0,     0,     0,   238,   360,
     362,     0,   432,     0,     0,     0,  1047,   435,     0,   601,
    1027,   357,   244,   654,     0,     0,   588,   576,   485,   137,
     204,     0,     0,   329,   318,     0,   321,   328,   338,   338,
     334,   568,  1082,   435,  1082,     0,  1009,     0,   967,   435,
       0,   435,  1052,   361,   909,   964,   891,   890,   877,   613,
     851,   607,     0,   616,   851,   630,   622,   625,   619,     0,
     879,   851,   895,   435,   139,   204,   148,   153,   174,   270,
     204,   278,   284,   139,   286,     0,  1098,  1068,  1072,     0,
       0,     0,   606,   837,   591,     0,  1017,  1016,   835,   139,
     217,  1091,     0,     0,     0,  1055,     0,     0,     0,   264,
       0,  1047,     0,   398,   394,   400,   754,    36,     0,   388,
       0,   393,   397,   410,     0,   408,   413,     0,   412,     0,
     411,     0,   221,   364,     0,   366,     0,   367,   368,     0,
       0,   993,     0,   655,   653,   664,   662,     0,   330,   331,
       0,     0,   316,   327,     0,     0,     0,  1082,  1076,   234,
     568,  1082,   969,   240,   357,   246,   435,     0,     0,     0,
     610,   885,   898,     0,   242,   292,   204,   204,   139,   273,
     173,   285,  1099,  1121,   855,     0,     0,     0,   204,     0,
       0,   460,     0,  1056,     0,   378,   382,   457,   458,   392,
       0,     0,     0,   373,   713,   714,   712,   715,   716,   733,
     735,   734,   704,   676,   674,   675,   694,   709,   710,   670,
     681,   682,   684,   683,   751,   703,   687,   685,   686,   688,
     689,   690,   691,   692,   693,   695,   696,   697,   698,   699,
     700,   702,   701,   671,   672,   673,   677,   678,   680,   750,
     718,   719,   723,   724,   725,   726,   727,   728,   711,   730,
     720,   721,   722,   705,   706,   707,   708,   731,   732,   736,
     738,   737,   739,   740,   717,   742,   741,   744,   746,   745,
     679,   749,   747,   748,   743,   729,   669,   405,   666,     0,
     374,   426,   427,   425,   418,     0,   419,   375,   452,     0,
       0,     0,     0,   456,     0,   221,   230,   356,     0,     0,
       0,   317,   333,   965,   966,     0,     0,     0,     0,  1082,
    1076,     0,   236,  1082,   889,     0,     0,   139,   271,   154,
     175,   204,   605,   590,  1015,   215,   376,   377,   455,   265,
       0,   852,   852,     0,   401,   389,     0,     0,     0,   407,
     409,     0,     0,   414,   421,   422,   420,     0,     0,   363,
    1057,     0,     0,     0,   459,     0,   358,     0,   332,     0,
     649,   854,   135,   854,  1078,     0,   428,   135,   224,     0,
       0,   232,     0,   609,   897,   204,     0,   177,   379,   125,
       0,   380,   381,     0,   851,     0,   851,   403,   399,   404,
     667,   668,     0,   390,   423,   424,   416,   417,   415,   453,
     450,  1095,   369,   365,   454,     0,   359,   650,   853,     0,
     204,   853,  1077,     0,  1081,   204,   135,   226,   228,     0,
     274,     0,   219,     0,   435,     0,   395,   402,   406,     0,
       0,   909,   371,     0,   647,   567,   570,  1079,  1080,   429,
     204,   272,     0,     0,   178,   386,     0,   434,   396,   451,
    1058,     0,   854,   446,   909,   648,   572,     0,   218,     0,
       0,   385,  1082,   909,   301,  1124,   449,   448,   447,  1124,
     445,     0,     0,     0,   384,  1076,   446,     0,     0,  1082,
       0,   383,     0,  1124,  1124,   307,     0,   306,   304,  1076,
     139,   430,   135,   370,     0,     0,   308,     0,     0,   302,
       0,   204,   204,   312,     0,   311,   300,     0,   303,   310,
     372,   214,   431,   313,     0,     0,   298,   309,     0,   299,
     315,   314
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1768, -1768, -1768,  -575, -1768, -1768, -1768,   104,  -432,   -42,
     488, -1768,  -249,  -513, -1768, -1768,   517,   -22,  1653, -1768,
    2617, -1768,  -839, -1768,  -528, -1768,  -698,    23, -1768, -1768,
   -1768, -1768, -1768, -1768, -1768, -1768,  -913, -1768, -1768,  -882,
    -352, -1768, -1768, -1768,  -305, -1768, -1768,  -163,   128,    30,
   -1768, -1768, -1768, -1768, -1768, -1768,    31, -1768, -1768, -1768,
   -1768, -1768, -1768,    37, -1768, -1768,  1201,  1212,  1206,   -78,
    -731,  -925,   669,   745,  -317,   394, -1004, -1768,   -21, -1768,
   -1768, -1768, -1768,  -775,   211, -1768, -1768, -1768, -1768,  -303,
   -1768,  -645, -1768,   483,  -412, -1768, -1768,  1107, -1768,     7,
   -1768, -1768, -1118, -1768, -1768, -1768, -1768, -1768, -1768, -1768,
   -1768, -1768, -1768,   -28, -1768,    65, -1768, -1768, -1768, -1768,
   -1768,  -114, -1768,   178, -1014, -1768, -1376,  -330, -1768,  -136,
     233,  -129,  -304, -1768,  -120, -1768, -1768, -1768,   183,   -93,
     -75,    63,  -782,   -63, -1768, -1768,    20, -1768,    -9,  -362,
   -1768,    11,    -5,   -77,   -58,    27, -1768, -1768, -1768, -1768,
   -1768, -1768, -1768, -1768, -1768,  -627,  -896, -1768, -1768, -1768,
   -1768, -1768,   197,  1353, -1768,   607, -1768,   454, -1768, -1768,
   -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768, -1768,
   -1768, -1768, -1768, -1768,   322,  -398,  -547, -1768, -1768, -1768,
   -1768, -1768,   535, -1768, -1768, -1768, -1768, -1768, -1768, -1768,
   -1768,  -998,  -347,  2622,    54, -1768,   746,  -428, -1768, -1768,
    -495,  3564,  3613, -1768,   173, -1768, -1768,   618,    51,  -676,
   -1768, -1768,   696,   466,  -556, -1768,   468, -1768, -1768, -1768,
   -1768, -1768,   677, -1768, -1768, -1768,    86,  -919,  -153,  -437,
    -420, -1768,   -71,  -115, -1768, -1768,    55,    56,   710,   -72,
   -1768, -1768,  1352,   -83, -1768,  -341,    29,  -356,   264,  -423,
   -1768, -1768,  -446,  1372, -1768, -1768, -1768, -1768, -1768,   730,
     403, -1768, -1768, -1768,  -334,  -675, -1768,  1323, -1275,  -159,
     -66,  -179,   893, -1768, -1768, -1768, -1767, -1768,  -212,  -821,
   -1344,  -201,   218, -1768,   577,   653, -1768, -1768, -1768, -1768,
     601, -1768,  1314,  -820
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   115,   968,   667,   190,   349,   780,
     369,   370,   371,   372,   919,   920,   921,   117,   118,   119,
     120,   121,   988,  1227,   428,  1020,   700,   701,   575,   266,
    1736,   581,  1640,  1737,  1992,   904,   123,   124,   721,   722,
     730,   362,   604,  1947,  1181,  1400,  2014,   451,   191,   702,
    1023,  1265,  1472,   127,   670,  1042,   703,   736,  1046,   642,
    1041,   245,   556,   704,   671,  1043,   453,   389,   411,   130,
    1025,   971,   944,  1201,  1668,  1324,  1106,  1889,  1740,   855,
    1112,   580,   864,  1114,  1515,   847,  1095,  1098,  1313,  2021,
    2022,   690,   691,  1004,   717,   718,   376,   377,   379,  1702,
    1867,  1868,  1414,  1569,  1691,  1861,  2001,  2024,  1900,  1951,
    1952,  1953,  1678,  1679,  1680,  1681,  1902,  1903,  1909,  1963,
    1684,  1685,  1689,  1854,  1855,  1856,  1938,  2063,  1570,  1571,
     192,   132,  2039,  2040,  1859,  1573,  1574,  1575,  1576,   133,
     134,   576,   577,   135,   136,   137,   138,   139,   140,   141,
     142,   259,   143,   144,   145,  1717,   146,  1022,  1264,   147,
     687,   688,   689,   263,   420,   571,   676,   677,  1362,   678,
    1363,   148,   149,   648,   649,  1352,  1353,  1481,  1482,   150,
     889,  1073,   151,   890,  1074,   152,   891,  1075,   153,   892,
    1076,   154,   893,  1077,   651,  1355,  1484,   155,   894,   156,
     157,  1931,   158,   672,  1704,   673,  1217,   976,  1432,  1429,
    1847,  1848,   159,   160,   161,   248,   162,   249,   260,   432,
     563,   163,  1356,  1357,   898,   899,   164,  1137,   555,   619,
    1138,  1080,  1287,  1081,  1485,  1486,  1290,  1291,  1083,  1492,
    1493,  1084,   823,   546,   204,   205,   705,   693,   530,  1237,
    1238,   811,   812,   461,   166,   251,   167,   168,   194,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   739,
     255,   256,   645,   239,   240,   775,   776,  1368,  1369,   404,
     405,   962,   180,   633,   181,   686,   182,   352,  1869,  1921,
     390,   440,   711,   712,  1127,  1128,  1878,  1933,  1934,  1231,
    1411,   940,  1412,   941,   942,   870,   871,   872,   353,   354,
     901,   590,   993,   994
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     193,   195,   458,   197,   198,   199,   200,   202,   203,  1099,
     206,   207,   208,   209,   350,   991,   229,   230,   231,   232,
     233,   234,   235,   236,   238,   427,   257,   122,   538,   511,
     445,  1021,   412,   422,   126,   128,   415,   416,   373,   265,
     846,   129,  1008,   987,   262,   423,   446,   273,   359,   276,
     425,   447,   360,  1130,   363,   531,   532,   267,   682,   254,
     986,   832,   271,   458,   454,   560,   408,   679,  1558,   409,
     770,  1232,   779,   681,  1102,  1229,   902,   358,   772,   773,
     683,   265,   725,  1221,   247,   252,   253,   510,  1116,  1754,
     165,   818,   967,   809,   731,   732,   733,   442,  1424,  1320,
    1263,  1089,   564,   424,   918,   923,   427,   264,   116,   572,
     810,  1045,   816,  1940,   422,   609,   611,   613,  1274,   616,
     860,   814,   815,   625,   862,   628,    14,   842,  1250,  1513,
    1255,   425,   125,   -78,  1234,   -43,   572,   427,   -78,  1694,
     -43,   565,   -42,  1696,   843,   548,  -391,   -42,  1762,   398,
     839,    14,  -461,    14,    14,   211,    40,   274,  1849,  1447,
     348,    14,   549,  1918,  1309,  1918,  1754,   399,   557,  1911,
     929,  1147,  1233,   572,   946,  1010,  1709,   388,   946,  1235,
    1592,   946,   547,   946,   424,   558,  1229,  -756,   946,  1299,
     605,   541,  -946,   528,   529,   378,  1912,  2056,   938,   939,
     410,  -636,   388,  1906,   621,   837,   388,   388,  -643, -1062,
    1148,  1176,     3,  1929,  1425,   424,   222,   222,   528,   529,
    2074,  1907,   459,  -956,  -109,  1593,  2003,  1426,  -646,  1430,
     439,  1494,   439,   196,   388,   528,   529,   131,   424,  -109,
    1908,  -945,  2057,  1191,   402,   403,   657,  1427, -1062,  1361,
     401,  -987,  1448,   567,   606,  -958,   567,  1300,  1930,   457,
    1329,  1330,  1431,   265,   578,  2075,  1082,   622,  -947,  1710,
    -579,  2004,  1236,  -853,  -757,   535,   111,  -853,  2052,  -943,
    1755,  1756,  1365,  -461,  -763,   569,  1666,  -950,   443,   574,
    1228,   535,  2070,   863,   737,  -643,  -948,  1514,  1583,  -990,
     573,   966,  -323,   639,  -305,  -853,  -989,   589,  -323,   545,
     861,   635,   973,   600,   626,  1277,   629,  1558,  1594,  1506,
     539,  -944,  1259,  -946,   -78,  1101,   -43,   655,   636,  -955,
    1695,  -930,  -851,   -42,  1697,  2058,   534,  -391,  1327,  1763,
    1331,  1603,   373,   373,   373,   614,   373,  1437,  1609,  1850,
    1611,   200,  1471,   460,  1919,  1332,  1973,  2051,  2076,  1913,
     930,   427,   459,   931,   947,  1011,   116,  -758,  1056,   723,
     116,  1415,  -945,  1639,   579,   430,   819,  1260,  1701,  1633,
     265,   424,  -987,   458,   666,   735,   727,   238,   647,   265,
     265,   647,   265,  1213,  -931,  1757,  -953,   661,   512,  -947,
     350,  1228,  1187,  1188,  1435,  1491,   536,  -957,  2066,   459,
    -943,   413,  1445,  -764,  2083,  -960,   202, -1062,  -950,  1862,
     258,  1863,   536,   534,   706,   974,   729,  -948,   528,   529,
    -990,  -108,  -951,   417,   222,   724,   719,  -989,   439,   726,
     975,   412,   785,   786,   454,  -643,  -108,   374,   634,  -954,
     790,   382,  -944, -1062,   738,   740, -1062,   650,   650,   599,
     650,   383,  -930,  1329,  1330,   741,   742,   743,   744,   746,
     747,   748,   749,   750,   751,   752,   753,   754,   755,   756,
     757,   758,   759,   760,   761,   762,   763,   764,   765,   766,
     767,   768,   769,   460,   771,  1726,   738,   738,   774,  1204,
     261,  2067,   399,  1289,   792,  -644,   692,  2084,   793,   794,
     795,   796,   797,   798,   799,   800,   801,   802,   803,   804,
     805,   911,  1523,   621,  -765,  -931,   656,   254,   719,   719,
     738,   817,   116,   791,   268,   793,   911,  1503,   821,  1423,
    1590,   995,   710,   996,   418,   269,   348,   829,   393,   831,
     779,   419,   247,   252,   253,   388,   125,   719,  1516,  1240,
    1040,   849,  1983,  -951,  1987,   850,  1988,   851,   375,   511,
     211,    40,   222,   534,   935,  -645,  1241,   270,   658,   402,
     403,   222,  1326,   361,   912,  -125,   438,   438,   222,  -125,
     836,  1984,   528,   529,   380,   922,   922,   825,   854,   438,
     222,  1039,   384,   381,  1443,   385,  -125,   399,   599,   388,
     783,   388,   388,   388,   388,   663,  1458,   977,   925,  1460,
    1279,   668,   669,   682,  1591,  1047,   386,   510,  1271,   399,
     387,  -917,   679,  1051,   808,   399,  1601,   663,   681,   788,
     391,   528,   529,   431,  1655,   683,  -917,  1914,  1252,   392,
    1252,  1409,  1410,   498,   599,  1240,  1182,   394,  1183,  1254,
    1184,   131,  1256,  1257,  1186,   499,  1915,  -759,   841,  1916,
     424,  1027,  1241,   995,   996,  -920,   395,   918,   399,   116,
    1090,  1092,   396,    55,   402,   403,   400,   397,   429,  -918,
    -920,   111,   455,   184,   185,    65,    66,    67,   413,   900,
     952,   954,   399,  -958,  -918,  1966,   402,   403,   414,   560,
     663,  1005,   402,   403,   169,  1366,   437,   652,  1091,   654,
    2035,  1177,   528,   529,  1967,   924,   710,  1968,   980,   226,
     228,   709,  2053,   438,   222,  1733,   866,  1289,  1483,   441,
    1030,  1483,   399,   444,  1358,   399,  1360,  1495,   449,  1487,
     434,  1489,   708,   663,   401,   402,   403,  2036,  2037,  2038,
     961,   963,   999,   450,   456,   224,   224,  -637,   781,  2036,
    2037,  2038,   462,  1038,   692,  1172,  1173,  1174,   664,   402,
     403,  1718,   463,  1720,  1473,   464,   210,   938,   939,   682,
     465,  1175,  1610,   466,   813,  1002,  1003,   867,   679,  1328,
    1329,  1330,  1050,   659,   681,  1409,  1410,   665,  1587,    50,
     467,   683,  1662,  1663,   781,   468,  1605,  -638,   426,   402,
     403,  1453,   402,   403,   469,   838,  -639,  1037,   844,  1510,
    1329,  1330,  -640,   116,   659,  1094,   665,   659,   665,   665,
    -641,   388,   433,   435,   436,   501,   214,   215,   216,   217,
     218,   265,   608,   610,   612,   502,   615,   125,  1464,  1096,
    1097,  1311,  1312,  1100,  1936,  1937,   503,  1252,   187,  1474,
     504,    91,  1550,   533,    93,    94,  1505,    95,   188,    97,
    -952,   868,  -642,   922,  1111,   922,  -757,   922,  2061,  2062,
     537,   922,  1021,   922,   922,  1189,  1876,  1964,  1965,   426,
    1880,  1699,   107,   455,   184,   185,    65,    66,    67,  1960,
    1961,   222,   544,   495,   496,   497,  1118,   498,   406,  1616,
     542,  1617,  1124,   499,  1239,  1242,  1072,   439,  1085,   499,
     426,   550,   492,   493,   494,   495,   496,   497,   512,   498,
    -956,   534,   553,   554,  1208,   682,  1209,   551,   851,  -755,
     116,   499,   561,   559,   679,   562,   570,   591,   583,  1211,
     681, -1107,   131,   594,  1109,   116,   595,   683,   601,  1727,
    1578,  1635,   169,  1220,   125,   456,   169,   602,   618,   617,
     620,  1278,  2031,   224,   627,   630,   222,  1644,   631,   641,
     122,   640,  1199,   684,   685,  1079,  1758,   126,   128,   694,
     695,  1251,  1248,  1251,   129,   726,   116,   726,   792,   696,
     698,  -130,   793,   707,    55,  1185,   710,   729,   455,   184,
     185,    65,    66,    67,   734,   222,  1266,   222,   822,  1267,
     125,  1268,   824,   658,   826,   719,   827,   455,    63,    64,
      65,    66,    67,   833,   834,   852,  1200,   572,    72,   505,
     856,  1607,   589,   165,   859,   222,   873,   874,  1939,   903,
    1451,   928,  1942,  1452,  1729,   905,  1730,  1229,  1731,   907,
     906,   116,  1229,  1732,   725,   908,   692,   254,   909,   131,
     910,   914,  2023,  1308,   913,   624,   599,   731,   732,   733,
     456,   507,   116,  1980,   632,   125,   637,  1229,  1985,   808,
     808,   644,   247,   252,   253,  2023,  1735,  1314,   692,   456,
     932,   933,   936,   662,  2046,  1741,   125,   937,   943,   945,
    1315,   224,   948,   950,   116,   951,   222,   953,   955,  1665,
     224,  1748,   956,   957,  1304,   131,  1438,   224,   169,   958,
     964,   969,   222,   222,  1417,   970,   972,  2010,   125,   224,
    -780,   388,  1439,   978,   728,   979,   981,  1440,   982,  1229,
     680,  1286,  1286,  1072,   985,   990,   989,   998,   922,  1000,
    1007,  1006,  1884,  1009,  1012,  1013,  1014,  1024,  1343,  1015,
    1016,  1036,  1346,  1044,   682,  1028,  1714,  1715,  1035,  1350,
    1032,  1052,  1033,   679,   841,  1026,   841,  1053,  1418,   681,
     131,   116,  1419,   116,  1054,   116,   683,  -761,  1093,  1103,
    1891,  2045,  1113,  1115,  1117,  2047,  1121,  1122,  1123,  2048,
    1251,   131,  1139,  2072,   726,  1125,  1338,   125,  2059,   125,
    1140,  1145,  1079,  2064,  2065,  1753,   122,  1141,  1665,   738,
    1142,  1143,  1456,   126,   128,  1144,  1222,   644,  1180,  1190,
     129,   599,  1192,   131,  1196,  1979,  1194,  1982,  1197,   813,
     813,  1198,  1665,  1203,  1665,   719,  1210,  1207,  1216,  1560,
    1665,  1218,  1219,  1244,  1261,   682,   719,  1419,  1225,  1223,
     900,  1230,  1228,   224,   679,   169,  1270,  1228,  1273,  1276,
     681,  1169,  1170,  1171,  1172,  1173,  1174,   683,  1281,   165,
     222,   222,  -959,   210,  1282,   959,  1292,   960,  1293,  1294,
    1175,    14,  1228,  1295,   265,  1296,  1297,   116,  1298,  1301,
    1303,  1305,  1498,  1317,  1512,   531,    50,  1319,  1302,  1943,
    1944,  1322,   131,  1323,   131,  1325,  1334,  1501,  1335,  1336,
    1342,   125,  1478,  1344, -1123,  1175,  2034,   692,  1348,  1345,
     692,  1349,  1399,   427,   844,  1401,   844,  1402,  1413,  1945,
    1446,   422,  1406,   214,   215,   216,   217,   218,  1433,  1449,
     355,   227,   227,  1457,  1228,  1450,  1561,  1459,   425,  1416,
    1403,  1404,  1562,  1040,   455,  1563,   185,    65,    66,    67,
    1564,    93,    94,  1434,    95,   188,    97,  1072,  1072,  1072,
    1072,  1072,  1072,  1465,  1531,  1461,  1463,  1072,  1535,  1475,
    1477,  1466,  1468,  1541,  1469,  1579,  1476,  1063,   116,   107,
    1546,  1490,  1584,  1499,  1001,  1497,  1585,   724,  1586,  1502,
     116,   726,  1565,  1566,  1500,  1567,  1507,  1520,  1511,   169,
    1519,  1700,   222,   458,  1597,  1560,   131,  1524,  1517,  1665,
    1525,  1530,  1534,  1529,   125,  1539,   456,  1606,   719,  1528,
     224,  1533,   210,  1540,  1536,  1568,  1079,  1079,  1079,  1079,
    1079,  1079,  1542,  1537,  1544,   448,  1079,  1538,  1545,  1549,
    1580,  1551,  1552,  1553,  1554,    50,  1581,    14,  1589,  1595,
    1596,  1598,  1599,  1614,  1600,  1602,  1604,  1608,  1620,  1049,
    1577,  1612,  1615,  1613,   222,  1618,  1622,  1619,  1623,  1624,
    1577,  1626,  1555,  1627,  1629,  1628,  1631,  1630,  1634,   222,
     222,  1637,   214,   215,   216,   217,   218,  1636,  1638,  1860,
    1641,  1645,  2071,  1621,  1642,   224,  1648,  1625,  1086,  1659,
    1087,  1670,  1683,  1698,  1632,  1703,  1708,   692,   452,  1711,
      93,    94,  1561,    95,   188,    97,   169,  1712,  1562,   131,
     455,  1563,   185,    65,    66,    67,  1564,  1716,  1107,  1721,
    1724,   169,  1722,  1728,   224,  1746,   224,  1752,   107,  1072,
    1760,  1072,  1743,  1761,  1857,  1707,  1858,  1864,  1870,   227,
    1713,  1871,  1875,   719,   719,  1873,  1877,  1874,  1751,  1883,
    1885,   587,  1886,   588,   224,  1896,  1897,  1917,  1565,  1566,
    1923,  1567,   169,   222,  1926,   455,    63,    64,    65,    66,
      67,  1927,  1932,  1954,  1956,  1958,    72,   505,  1962,  1970,
    1971,  1972,   456,  1977,  1978,  1981,  1991,  1986,  1990,  1195,
    -387,  1582,  1993,  1994,  1996,  1998,  1912,  1572,  1079,  1999,
    1079,  2002,  2011,  2012,   116,   644,  1206,  1572,  2005,  2013,
    2018,   593,  2020,  1739,  2025,   348,  2032,   506,  2029,   507,
    2033,  1688,   221,   221,  2042,   224,  2049,   169,   125,  2068,
    2055,  2044,   508,   244,   509,  2060,  2050,   456,  2069,  1577,
    2077,   224,   224,  2085,   116,  1577,  2073,  1577,   169,  1692,
     692,  2078,  1925,  2086,  2088,  2089,  1872,  1405,  2028,   244,
    1272,   787,  2043,   784,  1504,  1215,  1890,   782,   125,  1577,
    1253,  2041,  1253,  1643,  1072,   680,  1072,   227,  1072,  1455,
     169,  1881,   926,  1072,  1905,  1910,   227,  1759,   638,   116,
    2080,  1922,  2054,   227,   116,  1690,  1671,  1879,   116,   653,
    1359,  1488,  1428,   713,  1288,   227,   355,  1351,  1479,  1888,
    1739,  1480,  1306,   125,   646,   720,  1975,  1131,   388,  2007,
    2000,   599,   125,  1661,   348,  1408,  1340,  1398,     0,     0,
       0,     0,     0,   131,  1846,     0,     0,     0,     0,     0,
       0,  1853,     0,  1079,     0,  1079,     0,  1079,   348,     0,
     348,     0,  1079,     0,     0,     0,   348,   169,     0,   169,
     512,   169,  1577,  1107,  1321,     0,     0,     0,  1920,     0,
       0,     0,     0,   131,     0,     0,     0,     0,  1865,     0,
       0,     0,  1072,     0,     0,     0,  1572,   866,     0,   116,
     116,   116,  1572,     0,  1572,   116,    34,    35,    36,   224,
     224,     0,   116,     0,     0,     0,     0,     0,  2016,     0,
     212,     0,     0,   125,     0,  1928,  1572,     0,   131,   125,
       0,     0,     0,     0,     0,     0,   125,   131,     0,     0,
       0,     0,     0,  1920,     0,     0,     0,   210,   458,   227,
     221,   680,     0,     0,     0,     0,     0,     0,   867,     0,
       0,  1079,     0,   865,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,    81,    82,    83,    84,    85,
       0,     0,     0,   169,     0,     0,   219,     0,     0,     0,
       0,     0,    89,    90,     0,     0,     0,     0,     0,  1253,
     244,     0,   244,     0,     0,     0,    99,   214,   215,   216,
     217,   218,  1955,  1957,     0,  1454,     0,     0,     0,  1572,
     104,     0,     0,     0,   599,     0,     0,     0,   131,   187,
       0,     0,    91,     0,   131,    93,    94,     0,    95,   188,
      97,   131,  1339,     0,     0,   348,     0,     0,     0,  1072,
    1072,   224,     0,     0,     0,   116,     0,     0,   983,   984,
     244,     0,     0,   107,  1949,     0,     0,     0,     0,     0,
       0,  1846,  1846,     0,     0,  1853,  1853,  1496,     0,   125,
       0,     0,     0,     0,   169,     0,     0,     0,   221,   599,
       0,     0,   644,  1107,     0,     0,   169,   221,     0,     0,
       0,     0,     0,     0,   221,     0,     0,   680,     0,   116,
       0,     0,     0,   224,     0,     0,   221,     0,  1079,  1079,
       0,     0,     0,  2079,     0,     0,   227,   221,   224,   224,
       0,     0,  2087,   125,     0,     0,     0,     0,     0,     0,
    2090,     0,     0,  2091,   116,     0,     0,   692,  1560,   116,
       0,     0,   244,     0,     0,   244,     0,  2015,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   125,     0,
     692,     0,     0,   125,   116,     0,     0,     0,     0,   692,
    2030,     0,     0,     0,   131,     0,   644,     0,     0,     0,
      14,     0,     0,     0,     0,     0,     0,     0,   125,     0,
       0,   227,     0,     0,     0,     0,  1588,     0,     0,     0,
       0,   244,  1560,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   224,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   116,   116,     0,   131,     0,
     227,     0,   227,     0,     0,     0,     0,  1129,   713,     0,
     221,     0,     0,     0,    14,  1561,     0,     0,     0,   125,
     125,  1562,     0,   455,  1563,   185,    65,    66,    67,  1564,
     227,     0,     0,   131,     0,  1560,     0,     0,   131,     0,
       0,     0,     0,     0,     0,     0,     0,  2017,     0, -1125,
   -1125, -1125, -1125, -1125,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,   244,   131,   244,     0,     0,   888,     0,     0,
       0,  1565,  1566,     0,  1567,  1175,     0,    14,     0,  1561,
     169,     0,     0,     0,     0,  1562,     0,   455,  1563,   185,
      65,    66,    67,  1564,     0,   456,     0,     0,     0,     0,
     888,   227,     0,  1214,  1719,     0,   680,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   227,   227,  1224,
     169,  1560,     0,     0,   131,   131,     0,     0,     0,     0,
       0,     0,  1243,     0,     0,  1565,  1566,     0,  1567,     0,
       0,     0,  1561,     0,     0,     0,     0,     0,  1562,     0,
     455,  1563,   185,    65,    66,    67,  1564,   244,   244,   456,
       0,     0,     0,    14,     0,   169,   244,     0,  1723,     0,
     169,     0,     0,     0,   169,     0,     0,     0,  1275,     0,
       0,     0,     0,     0,     0,     0,     0,   221,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   680,  1565,  1566,
       0,  1567,     0,  1560,   540,   514,   515,   516,   517,   518,
     519,   520,   521,   522,   523,   524,   525,     0,     0,     0,
       0,     0,   456,     0,     0,     0,     0,     0,  1561,     0,
       0,  1725,     0,     0,  1562,     0,   455,  1563,   185,    65,
      66,    67,  1564,     0,     0,    14,     0,     0,     0,   526,
     527,  1333,     0,     0,     0,  1337,     0,     0,     0,     0,
    1341,     0,   221,     0,     0,   169,   169,   169,     0,     0,
       0,   169,     0,     0,     0,   227,   227,     0,   169,     0,
       0,     0,     0,     0,  1565,  1566,     0,  1567,     0,     0,
       0,     0,     0,     0,     0,   244,     0,     0,     0,     0,
       0,   221,     0,   221,     0,     0,     0,     0,   456,     0,
    1561,     0,     0,     0,     0,     0,  1562,  1734,   455,  1563,
     185,    65,    66,    67,  1564,     0,   528,   529,     0,     0,
       0,   221,   888,     0,   210,     0,   211,    40,     0,   244,
       0,     0,     0,     0,     0,     0,   244,   244,   888,   888,
     888,   888,   888,     0,     0,     0,     0,    50,     0,     0,
     888,     0,     0,     0,  1442,     0,  1565,  1566,     0,  1567,
       0,     0,     0,     0,     0,     0,   244,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   697,
     456,     0,     0,     0,   214,   215,   216,   217,   218,  1882,
       0,  1441,   221,     0,     0,     0,     0,  1467,     0,     0,
    1470,     0,     0,     0,     0,     0,   244,   227,   221,   221,
     806,   169,    93,    94,     0,    95,   188,    97,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   244,   244,     0,     0,     0,     0,     0,     0,
     107,     0,   221,     0,   807,     0,     0,   111,   244,     0,
       0,   223,   223,     0,     0,   244,     0,     0,     0,  1518,
       0,   244,   246,     0,     0,   169,  1522,     0,     0,   227,
       0,     0,   888,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   351,   227,   227,     0,   244,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     169,     0,     0,     0,     0,   169,     0,   244,     0,     0,
       0,   244,     0,     0,     0,     0,   470,   471,   472,     0,
       0,     0,   244,     0,     0,     0,     0,     0,     0,     0,
     169,     0,     0,  1556,  1557,     0,   473,   474,     0,   475,
     476,   477,   478,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,     0,   498,     0,     0,   221,   221,     0,     0,
       0,     0,     0,     0,     0,   499,     0,     0,   227,     0,
     244,     0,     0,     0,   244,     0,   244,     0,     0,   244,
       0,   169,   169,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   888,   888,   888,   888,   888,   888,   221,     0,
       0,   888,   888,   888,   888,   888,   888,   888,   888,   888,
     888,   888,   888,   888,   888,   888,   888,   888,   888,   888,
     888,   888,   888,   888,   888,   888,   888,   888,   888,     0,
       0,     0,     0,     0,  1646,  1647,     0,     0,  1649,     0,
       0,     0,     0,     0,     0,     0,     0,   888,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   223,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1283,  1284,  1285,   210,  1667,     0,     0,     0,     0,
       0,     0,     0,   244,     0,   244,  1693,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,   221,  1421,
       0,     0,     0,     0,   351,     0,   351,   540,   514,   515,
     516,   517,   518,   519,   520,   521,   522,   523,   524,   525,
       0,     0,     0,     0,     0,     0,   244,     0,     0,   244,
       0,     0,     0,     0,   214,   215,   216,   217,   218,     0,
       0,     0,     0,     0,     0,     0,   244,   244,   244,   244,
     244,   244,   526,   527,   221,     0,   244,     0,     0,  1742,
     221,     0,    93,    94,   351,    95,   188,    97,     0,  1672,
       0,     0,     0,     0,     0,   221,   221,     0,   888,     0,
       0,     0,     0,     0,  1667,     0,     0,     0,   244,     0,
     107,     0,     0,     0,     0,   244,     0,   223,     0,     0,
     888,     0,   888,     0,     0,     0,   223,     0,  1667,     0,
    1667,     0,     0,   223,     0,     0,  1667,     0,     0,   210,
       0,     0,     0,     0,     0,   223,   888,     0,     0,   528,
     529,     0,     0,     0,     0,     0,   223,     0,     0,     0,
       0,     0,    50,   513,   514,   515,   516,   517,   518,   519,
     520,   521,   522,   523,   524,   525,   351,     0,     0,   351,
       0,     0,   244,   244,     0,  1673,   244,     0,     0,   221,
       0,     0,     0,     0,  1901,     0,     0,     0,  1674,   214,
     215,   216,   217,   218,  1675,     0,     0,     0,   526,   527,
       0,     0,   835,     0,     0,     0,     0,   244,     0,     0,
       0,   187,     0,     0,    91,  1676,     0,    93,    94,     0,
      95,  1677,    97,   470,   471,   472,     0,     0,     0,     0,
     246,     0,     0,     0,     0,     0,     0,     0,   244,     0,
     244,     0,     0,   473,   474,   107,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   223,
     498,     0,     0,     0,     0,   528,   529,     0,     0,     0,
       0,     0,   499,   244,   244,     0,     0,   244,  1924,     0,
       0,     0,     0,   888,     0,   888,     0,   888,     0,     0,
       0,  1935,   888,   221,     0,  1667,   888,   210,   888,   211,
      40,   888,     0,   470,   471,   472,   351,     0,   869,     0,
       0,     0,     0,     0,   244,   244,   895,     0,   244,     0,
      50,     0,     0,   473,   474,   244,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   895,
     498,     0,     0,     0,     0,     0,     0,   214,   215,   216,
     217,   218,   499,     0,     0,     0,     0,     0,  1995,     0,
       0,     0,     0,   244,     0,   244,     0,   244,     0,     0,
       0,     0,   244,   806,   221,    93,    94,     0,    95,   188,
      97,     0,     0,     0,     0,  1935,     0,  2008,   244,     0,
       0,   351,   351,   888,     0,   965,     0,     0,     0,     0,
     351,     0,     0,   107,     0,   244,   244,   840,     0,     0,
     111,     0,     0,   244,     0,   244,     0,     0,     0,     0,
       0,     0,     0,   470,   471,   472,   223,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   244,     0,   244,
       0,     0,     0,   473,   474,   244,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,     0,
     498,   244,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   499,   500,     0,  1029,     0,     0,   888,   888,
     888,     0,     0,     0,     0,   888,     0,   244,     0,     0,
       0,   223,     0,   244,     0,   244,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1149,  1150,  1151,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1078,     0,     0,     0,     0,     0,
     223,  1152,   223,     0,  1153,  1154,  1155,  1156,  1157,  1158,
    1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,     0,     0,     0,     0,
     223,   895,     0,  1120,     0,     0,     0,     0,     0,  1175,
     351,   351,     0,     0,     0,     0,     0,   895,   895,   895,
     895,   895,     0,     0,     0,     0,     0,     0,     0,   895,
       0,     0,   287,     0,     0,     0,     0,   244,     0,     0,
       0,     0,     0,   500,     0,  1179,     0,     0,     0,     0,
     244,     0,     0,     0,   244,     0,     0,     0,   244,   244,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   289,
     210,   223,     0,   244,     0,     0,     0,     0,     0,   888,
       0,     0,   210,     0,     0,  1202,     0,   223,   223,     0,
     888,     0,     0,    50,     0,     0,   888,     0,     0,     0,
     888,     0,     0,   225,   225,    50,   351,     0,     0,     0,
       0,     0,  1202,   592,   250,     0,     0,     0,     0,     0,
       0,   223,   351,     0,     0,     0,     0,   244,     0,   351,
     214,   215,   216,   217,   218,   351,     0,     0,     0,  1364,
       0,   585,   214,   215,   216,   217,   218,   586,     0,     0,
       0,   895,   187,     0,   244,    91,   244,     0,    93,    94,
     210,    95,   188,    97,   187,     0,  1262,    91,   342,     0,
      93,    94,   888,    95,   188,    97,     0,     0,     0,     0,
       0,   351,     0,    50,     0,   244,   107,     0,   346,     0,
     246,  1948,     0,     0,     0,     0,     0,     0,   107,   347,
       0,  1078,   244,     0,     0,     0,     0,     0,   244,  1686,
       0,     0,   244,     0,     0,     0,     0,     0,     0,     0,
     214,   215,   216,   217,   218,     0,   244,   244,     0, -1125,
   -1125, -1125, -1125, -1125,   490,   491,   492,   493,   494,   495,
     496,   497,     0,   498,     0,   223,   223,     0,    93,    94,
       0,    95,   188,    97,   351,   499,     0,     0,   351,     0,
     869,     0,     0,   351,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   107,  1687,     0,     0,
       0,   895,   895,   895,   895,   895,   895,   223,     0,     0,
     895,   895,   895,   895,   895,   895,   895,   895,   895,   895,
     895,   895,   895,   895,   895,   895,   895,   895,   895,   895,
     895,   895,   895,   895,   895,   895,   895,   895,   470,   471,
     472,   225,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   895,     0,   473,   474,
       0,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,     0,   498,     0,   351,     0,   351,
       0,     0,     0,     0,     0,     0,     0,   499,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   223,     0,     0,
       0,     0,   210,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     351,     0,     0,   351,     0,    50,  1156,  1157,  1158,  1159,
    1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1078,  1078,  1078,  1078,  1078,
    1078,     0,     0,   223,     0,  1078,     0,     0,  1175,   223,
       0,     0,   214,   215,   216,   217,   218,     0,     0,   225,
       0,     0,     0,     0,   223,   223,     0,   895,   225,     0,
       0,     0,   351,     0,     0,   225,     0,   406,     0,   351,
      93,    94,     0,    95,   188,    97,     0,   225,     0,   895,
       0,   895,     0,   470,   471,   472,     0,     0,   250,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   107,     0,
     997,     0,   407,   473,   474,   895,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,     0,
     498,     0,     0,     0,     0,     0,   351,   351,     0,     0,
       0,     0,   499,     0,     0,  1559,     0,     0,   223,     0,
       0,     0,     0,   470,   471,   472,   540,   514,   515,   516,
     517,   518,   519,   520,   521,   522,   523,   524,   525,     0,
       0,   351,   250,   473,   474,     0,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,     0,
     498,   526,   527,     0,     0,     0,     0,  1078,     0,  1078,
       0,   225,   499,  1017,   514,   515,   516,   517,   518,   519,
     520,   521,   522,   523,   524,   525,  1154,  1155,  1156,  1157,
    1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,   351,   351,     0,
       0,   351,     0,     0,     0,     0,     0,     0,   526,   527,
    1175,     0,   895,     0,   895,     0,   895,     0,   896,     0,
       0,   895,   223,   582,     0,   895,     0,   895,   528,   529,
     895,     0,     0,     0,     0,     0,     0,     0,   351,     0,
       0,     0,     0,     0,  1669,     0,     0,  1682,     0,   351,
       0,   896,     0,  1017,   514,   515,   516,   517,   518,   519,
     520,   521,   522,   523,   524,   525,     0,   897,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   528,   529,     0,     0,     0,
       0,   934,     0,     0,     0,  1029,     0,     0,   526,   527,
     927,     0,  1078,     0,  1078,     0,  1078,     0,     0,     0,
       0,  1078,     0,   223,     0,     0,     0,     0,     0,     0,
       0,     0,   351,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   895,     0,     0,     0,     0,     0,   225,     0,
       0,     0,     0,     0,  1749,  1750,     0,   351,  1018,     0,
       0,     0,     0,     0,  1682,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   470,   471,   472,     0,     0,     0,
       0,   351,     0,   351,     0,   528,   529,     0,     0,   351,
     287,     0,     0,     0,   473,   474,     0,   475,   476,   477,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
    1078,   498,     0,   225,     0,     0,     0,   289,     0,     0,
       0,     0,     0,   499,     0,     0,     0,   895,   895,   895,
     210,     0,     0,     0,   895,     0,  1899,   351,   697,     0,
       0,     0,     0,     0,  1682,     0,     0,     0,     0,     0,
       0,     0,   225,    50,   225,     0,     0,     0,     0,     0,
       0,  -434,     0,     0,     0,     0,     0,     0,     0,     0,
     455,   184,   185,    65,    66,    67,     0,     0,     0,     0,
       0,     0,   225,   896,     0,     0,     0,     0,     0,   585,
     214,   215,   216,   217,   218,   586,     0,     0,     0,   896,
     896,   896,   896,   896,     0,     0,     0,   287,     0,     0,
       0,   896,   187,     0,     0,    91,   342,     0,    93,    94,
       0,    95,   188,    97,     0,     0,     0,     0,     0,     0,
       0,     0,  1108,     0,     0,     0,   346,     0,     0,     0,
       0,   351,   456,     0,   289,     0,   107,   347,  1132,  1133,
    1134,  1135,  1136,   225,   351,     0,  1055,   210,   351,     0,
    1146,     0,     0,  1126,     0,     0,     0,  1078,  1078,   225,
     225,     0,     0,     0,     0,     0,     0,  1950,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,   895,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   895,
       0,     0,     0,   250,     0,   895,     0,     0,     0,   895,
       0,     0,     0,     0,     0,     0,   585,   214,   215,   216,
     217,   218,   586,     0,     0,     0,     0,     0,     0,     0,
       0,   351,     0,   896,     0,     0,     0,     0,     0,   187,
       0,     0,    91,   342,     0,    93,    94,     0,    95,   188,
      97,     0, -1124,     0,     0,     0,     0,     0,   351,     0,
     351,     0,     0,   346,     0,     0,     0,     0,     0,     0,
       0,     0,   250,   107,   347,     0,     0,     0,     0,     0,
       0,   895,  1249,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  2027,     0,     0,   540,   514,   515,
     516,   517,   518,   519,   520,   521,   522,   523,   524,   525,
       0,  1669,   351,     0,     0,     0,   351,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   225,   225,     0,
     351,   351,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   526,   527,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   896,   896,   896,   896,   896,   896,   250,
       0,     0,   896,   896,   896,   896,   896,   896,   896,   896,
     896,   896,   896,   896,   896,   896,   896,   896,   896,   896,
     896,   896,   896,   896,   896,   896,   896,   896,   896,   896,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1136,  1354,     0,     0,  1354,     0,   896,   528,
     529,  1367,  1370,  1371,  1372,  1374,  1375,  1376,  1377,  1378,
    1379,  1380,  1381,  1382,  1383,  1384,  1385,  1386,  1387,  1388,
    1389,  1390,  1391,  1392,  1393,  1394,  1395,  1396,  1397,     0,
     470,   471,   472,  1017,   514,   515,   516,   517,   518,   519,
     520,   521,   522,   523,   524,   525,     0,  1407,     0,   225,
     473,   474,     0,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,     0,   498,   526,   527,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   499,
       0,     0,     0,     0,     0,     0,     0,   287,     0,     0,
       0,     0,     0,     0,     0,   250,     0,     0,     0,     0,
       0,   225,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   225,   225,     0,   896,
       0,     0,     0,     0,   289,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   210,     0,     0,
       0,   896,     0,   896,     0,   528,   529,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,   896,  1508,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   277,   278,
       0,   279,   280,     0,     0,   281,   282,   283,   284,     0,
    1526,     0,  1527,     0,     0,     0,   585,   214,   215,   216,
     217,   218,   586,   285,   286,     0,     0,     0,     0,     0,
     225,     0,  1193,     0,     0,     0,  1547,     0,     0,   187,
       0,     0,    91,   342,     0,    93,    94,     0,    95,   188,
      97,     0,   288,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   346,     0,     0,   290,   291,   292,   293,
     294,   295,   296,   107,   347,     0,   210,     0,   211,    40,
       0,     0,   297,     0,     0,     0,     0,     0,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,    50,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,     0,   333,     0,   777,   335,   336,
     337,     0,     0,     0,   338,   596,   214,   215,   216,   217,
     218,   597,     0,     0,   896,     0,   896,     0,   896,     0,
       0,     0,     0,   896,   250,     0,     0,   896,   598,   896,
       0,     0,   896,     0,    93,    94,     0,    95,   188,    97,
     343,     0,   344,     0,     0,   345,     0,     0,     0,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,   107,  1651,     0,  1652,   778,  1653,     0,   111,
       0,     0,  1654,   356,   421,    13,  1656,     0,  1657,     0,
       0,  1658,     0,     0,   789,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,   250,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,   896,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,     0,   183,   184,   185,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,  1744,     0,   186,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   187,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   188,
      97,     0,     0,     0,    99,     0,     0,   100,     0,   896,
     896,   896,     0,   101,     0,     0,   896,     0,   104,   105,
     106,     0,     0,   107,   108,  1904,     0,     0,     0,     0,
     111,   112,   113,   114,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   470,   471,   472,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1892,  1893,
    1894,     0,     0,   473,   474,  1898,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,     0,
     498,     0,     0,   470,   471,   472,     0,     0,     0,     0,
       0,     0,   499,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   473,   474,     0,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,     0,
     498,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   499,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     896,   498,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   896,     0,   499,     0,     0,     0,   896,     0,     0,
       0,   896,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,
    1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,     0,     5,     6,     7,     8,     9,     0,  1959,
    1997,     0,     0,    10,     0,  1175,     0,     0,     0,     0,
    1969,     0,     0,     0,     0,  1269,  1974,    11,    12,    13,
    1976,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,   896,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,  1280,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,  2019,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,    56,    57,    58,     0,    59,  -204,    60,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,    88,    89,    90,    91,    92,     0,    93,
      94,     0,    95,    96,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
     103,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,     0,     0,   111,   112,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,    56,    57,
      58,     0,    59,     0,    60,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,    88,    89,
      90,    91,    92,     0,    93,    94,     0,    95,    96,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,   103,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,  1212,     0,   111,
     112,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,    56,    57,    58,     0,    59,     0,    60,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,    88,    89,    90,    91,    92,     0,    93,
      94,     0,    95,    96,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
     103,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,  1422,     0,   111,   112,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,   699,     0,   111,
     112,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   187,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   188,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,  1019,     0,   111,   112,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,  -204,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,     0,     0,   111,
     112,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   187,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   188,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,  1178,     0,   111,   112,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,  1226,     0,   111,
     112,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   187,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   188,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,  1258,     0,   111,   112,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,  1316,     0,   111,
     112,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,  1318,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   187,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   188,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,     0,     0,   111,   112,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,  1509,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,     0,     0,   111,
     112,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   187,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   188,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,  1660,     0,   111,   112,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,  -296,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,     0,     0,   111,
     112,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   187,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   188,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,  1895,     0,   111,   112,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,  1946,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,     0,     0,   111,
     112,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,  1989,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   187,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   188,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,     0,     0,   111,   112,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,  2006,     0,   111,
     112,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   187,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   188,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,  2009,     0,   111,   112,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,  2026,     0,   111,
     112,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   187,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   188,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,  2081,     0,   111,   112,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,  2082,     0,   111,
     112,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,   568,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,   184,   185,    65,    66,    67,     0,    68,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   187,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   188,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,     0,     0,   111,   112,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,   853,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,   184,   185,    65,
      66,    67,     0,    68,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,     0,     0,   111,
     112,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,  1110,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,   184,   185,    65,    66,    67,     0,    68,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   187,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   188,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,     0,     0,   111,   112,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,  1738,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,   184,   185,    65,
      66,    67,     0,    68,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,     0,     0,   111,
     112,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,  1887,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,   184,   185,    65,    66,    67,     0,    68,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   187,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   188,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,     0,     0,   111,   112,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,   184,   185,    65,
      66,    67,     0,    68,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,     0,     0,   111,
     112,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   356,     0,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
       0,   183,   184,   185,    65,    66,    67,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   186,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   187,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   188,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   189,     0,
     357,     0,     0,     0,   111,   112,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,  1152,     0,    10,  1153,
    1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,
    1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,     0,     0,   714,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1175,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,   715,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,     0,   183,   184,   185,    65,
      66,    67,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   186,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
       0,   716,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   189,     5,     6,     7,     8,     9,   111,
     112,   113,   114,     0,    10,  1153,  1154,  1155,  1156,  1157,
    1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,     0,     0,  1245,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1175,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,  1246,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,     0,   183,   184,   185,    65,    66,    67,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     186,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   187,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   188,    97,     0,  1247,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   189,
       5,     6,     7,     8,     9,   111,   112,   113,   114,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   356,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,   183,   184,
     185,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   186,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     187,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     188,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   189,     0,     0,   848,     0,
       0,   111,   112,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   356,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     789,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,     0,   183,   184,   185,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   186,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   187,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   188,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     189,     5,     6,     7,     8,     9,   111,   112,   113,   114,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   356,   421,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,   183,
     184,   185,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   186,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   187,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   188,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,  1149,  1150,  1151,
     104,   105,   106,     0,     0,   107,   108,     5,     6,     7,
       8,     9,   111,   112,   113,   114,     0,    10,  1152,  1548,
       0,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,
    1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,  1175,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,   201,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,     0,   183,   184,   185,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   186,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   187,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   188,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   189,     5,     6,     7,     8,     9,   111,   112,
     113,   114,     0,    10,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,     0,   498,     0,   237,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   499,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
       0,   183,   184,   185,    65,    66,    67,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   186,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   187,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   188,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,  1149,
    1150,  1151,   104,   105,   106,     0,     0,   107,   189,     5,
       6,     7,     8,     9,   111,   112,   113,   114,     0,    10,
    1152,     0,     0,  1153,  1154,  1155,  1156,  1157,  1158,  1159,
    1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,  1175,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,     0,   183,   184,   185,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   186,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   187,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   188,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   189,     0,   272,   470,   471,   472,
     111,   112,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,   473,   474,     0,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,     0,   498,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,   499,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,     0,   183,   184,   185,    65,    66,    67,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     186,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   187,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   188,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,  1310,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   189,
       0,   275,     0,     0,     0,   111,   112,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   421,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,     0,   183,   184,   185,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   186,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   187,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   188,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,   471,   472,   107,   108,     5,     6,     7,     8,     9,
     111,   112,   113,   114,     0,    10,     0,     0,     0,     0,
     473,   474,     0,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,     0,   498,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,   499,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,     0,   183,   184,   185,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   186,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   187,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   188,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     189,   566,     0,     0,     0,     0,   111,   112,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   356,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,   183,   184,
     185,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   186,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     187,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     188,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   189,     5,     6,     7,     8,
       9,   111,   112,   113,   114,     0,    10,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   745,   498,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   499,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,     0,   183,   184,   185,    65,    66,    67,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   186,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   187,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   188,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   189,     5,     6,     7,     8,     9,   111,   112,   113,
     114,     0,    10,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,     0,   498,     0,     0,   789,     0,     0,
       0,     0,     0,     0,     0,     0,   499,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,     0,
     183,   184,   185,    65,    66,    67,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   186,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   187,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   188,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   189,     5,     6,
       7,     8,     9,   111,   112,   113,   114,     0,    10,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,     0,   498,
       0,     0,     0,   828,     0,     0,     0,     0,     0,     0,
       0,   499,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,     0,   183,   184,   185,    65,
      66,    67,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   186,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   189,     5,     6,     7,     8,     9,   111,
     112,   113,   114,     0,    10,  1157,  1158,  1159,  1160,  1161,
    1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,     0,     0,     0,     0,     0,     0,   830,
       0,     0,     0,     0,     0,     0,  1175,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,     0,   183,   184,   185,    65,    66,    67,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     186,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   187,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   188,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   189,
       5,     6,     7,     8,     9,   111,   112,   113,   114,     0,
      10, -1125, -1125, -1125, -1125,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,     0,   498,
       0,     0,     0,     0,     0,  1307,     0,     0,     0,     0,
       0,   499,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,   183,   184,
     185,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   186,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     187,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     188,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   189,     5,     6,     7,     8,
       9,   111,   112,   113,   114,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     356,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,     0,   183,   184,   185,    65,    66,    67,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   186,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   187,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   188,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,  1150,  1151,   104,   105,   106,     0,     0,
     107,  1436,     5,     6,     7,     8,     9,   111,   112,   113,
     114,     0,    10,  1152,     0,     0,  1153,  1154,  1155,  1156,
    1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
    1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,  1175,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,     0,
     183,   184,   185,    65,    66,    67,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   186,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   187,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   188,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,   472,   107,   189,     5,     6,
       7,     8,     9,   111,   112,   113,   114,     0,    10,     0,
       0,     0,     0,   473,   474,     0,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,     0,
     498,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,   499,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,   660,    39,    40,
       0,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,     0,   183,   184,   185,    65,
      66,    67,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   186,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
       0,   277,   278,    99,   279,   280,   100,     0,   281,   282,
     283,   284,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   189,     0,     0,   285,   286,     0,   111,
     112,   113,   114,  1158,  1159,  1160,  1161,  1162,  1163,  1164,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
       0,     0,     0,     0,     0,   288,     0,     0,     0,     0,
       0,     0,     0,  1175,     0,     0,     0,     0,     0,   290,
     291,   292,   293,   294,   295,   296,     0,     0,     0,   210,
       0,   211,    40,     0,     0,   297,     0,     0,     0,     0,
       0,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,    50,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,     0,   333,     0,
     334,   335,   336,   337,     0,     0,     0,   338,   596,   214,
     215,   216,   217,   218,   597,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   277,   278,     0,   279,   280,
       0,   598,   281,   282,   283,   284,     0,    93,    94,     0,
      95,   188,    97,   343,     0,   344,     0,     0,   345,     0,
     285,   286,     0,   287,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   107,     0,     0,     0,   778,
       0,     0,   111,     0,     0,     0,     0,     0,     0,   288,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     289,     0,     0,   290,   291,   292,   293,   294,   295,   296,
       0,     0,     0,   210,     0,     0,     0,     0,     0,   297,
       0,     0,     0,     0,     0,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,    50,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,     0,   333,     0,   210,   335,   336,   337,     0,     0,
       0,   338,   339,   214,   215,   216,   217,   218,   340,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,   341,     0,     0,    91,   342,
       0,    93,    94,     0,    95,   188,    97,   343,     0,   344,
       0,     0,   345,   277,   278,     0,   279,   280,     0,   346,
     281,   282,   283,   284,   214,   215,   216,   217,   218,   107,
     347,     0,     0,     0,  1866,     0,     0,     0,   285,   286,
       0,   287,     0,     0,     0,     0,   187,     0,     0,    91,
      92,     0,    93,    94,     0,    95,   188,    97,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   288,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   289,     0,
     107,   290,   291,   292,   293,   294,   295,   296,     0,     0,
       0,   210,     0,     0,     0,     0,     0,   297,     0,     0,
       0,     0,     0,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,    50,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,     0,
     333,     0,   210,   335,   336,   337,     0,     0,     0,   338,
     339,   214,   215,   216,   217,   218,   340,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,   341,     0,     0,    91,   342,     0,    93,
      94,     0,    95,   188,    97,   343,     0,   344,     0,     0,
     345,   277,   278,     0,   279,   280,     0,   346,   281,   282,
     283,   284,   214,   215,   216,   217,   218,   107,   347,     0,
       0,     0,  1941,     0,     0,     0,   285,   286,     0,   287,
       0,     0,     0,     0,     0,     0,     0,   367,     0,     0,
      93,    94,     0,    95,   188,    97,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   288,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   289,     0,   107,   290,
     291,   292,   293,   294,   295,   296,     0,     0,     0,   210,
       0,     0,     0,     0,     0,   297,     0,     0,     0,     0,
       0,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,    50,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,     0,   333,     0,
     334,   335,   336,   337,     0,     0,     0,   338,   339,   214,
     215,   216,   217,   218,   340,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   341,     0,     0,    91,   342,     0,    93,    94,     0,
      95,   188,    97,   343,     0,   344,     0,     0,   345,   277,
     278,     0,   279,   280,     0,   346,   281,   282,   283,   284,
       0,     0,     0,     0,     0,   107,   347,     0,     0,     0,
       0,     0,     0,     0,   285,   286,     0,   287,   474,     0,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   288,   498,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   289,     0,   499,   290,   291,   292,
     293,   294,   295,   296,     0,     0,     0,   210,     0,     0,
       0,     0,     0,   297,     0,     0,     0,     0,     0,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
      50,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,     0,   333,     0,     0,   335,
     336,   337,     0,     0,     0,   338,   339,   214,   215,   216,
     217,   218,   340,     0,     0,     0,     0,     0,     0,   210,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   341,
       0,     0,    91,   342,     0,    93,    94,     0,    95,   188,
      97,   343,    50,   344,     0,     0,   345,     0,   277,   278,
       0,   279,   280,   346,  1664,   281,   282,   283,   284,     0,
       0,     0,     0,   107,   347,     0,     0,     0,     0,     0,
       0,     0,     0,   285,   286,     0,   287,     0,     0,   214,
     215,   216,   217,   218,     0, -1125, -1125, -1125, -1125,  1162,
    1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,   288,     0,     0,  1851,     0,    93,    94,  1852,
      95,   188,    97,   289,     0,  1175,   290,   291,   292,   293,
     294,   295,   296,     0,     0,     0,   210,     0,     0,     0,
       0,     0,   297,     0,     0,   107,  1687,     0,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,    50,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,     0,   333,     0,   210,   335,   336,
     337,     0,     0,     0,   338,   339,   214,   215,   216,   217,
     218,   340,     0,     0,     0,     0,     0,     0,   210,     0,
      50,     0,     0,     0,     0,     0,     0,     0,   341,     0,
       0,    91,   342,     0,    93,    94,     0,    95,   188,    97,
     343,    50,   344,     0,     0,   345,  1764,  1765,  1766,  1767,
    1768,     0,   346,  1769,  1770,  1771,  1772,   214,   215,   216,
     217,   218,   107,   347,     0,     0,     0,     0,     0,     0,
    1773,  1774,  1775,     0,     0,     0,     0,     0,   214,   215,
     216,   217,   218,     0,     0,    93,    94,     0,    95,   188,
      97,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1776,     0,     0,     0,     0,     0,    93,    94,     0,    95,
     188,    97,     0,   107,  1777,  1778,  1779,  1780,  1781,  1782,
    1783,     0,     0,     0,   210,     0,     0,     0,     0,     0,
    1784,     0,     0,     0,   107,   734,  1785,  1786,  1787,  1788,
    1789,  1790,  1791,  1792,  1793,  1794,  1795,    50,  1796,  1797,
    1798,  1799,  1800,  1801,  1802,  1803,  1804,  1805,  1806,  1807,
    1808,  1809,  1810,  1811,  1812,  1813,  1814,  1815,  1816,  1817,
    1818,  1819,  1820,  1821,  1822,  1823,  1824,  1825,  1826,     0,
       0,     0,  1827,  1828,   214,   215,   216,   217,   218,     0,
    1829,  1830,  1831,  1832,  1833,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1834,  1835,  1836,   210,
       0,     0,    93,    94,     0,    95,   188,    97,  1837,     0,
    1838,  1839,     0,  1840,     0,     0,     0,     0,     0,     0,
    1841,  1842,    50,  1843,     0,  1844,  1845,     0,   277,   278,
     107,   279,   280,     0,     0,   281,   282,   283,   284,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   285,   286,     0,     0,     0,     0,   214,
     215,   216,   217,   218,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   288,     0,     0,     0,     0,    93,    94,     0,
      95,   188,    97,     0,     0,     0,   290,   291,   292,   293,
     294,   295,   296,     0,     0,     0,   210,     0,     0,     0,
       0,     0,   297,     0,     0,   107,  1026,     0,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,    50,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   210,   333,     0,   334,   335,   336,
     337,     0,     0,     0,   338,   596,   214,   215,   216,   217,
     218,   597,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,   277,   278,     0,   279,   280,     0,   598,   281,
     282,   283,   284,     0,    93,    94,     0,    95,   188,    97,
     343,     0,   344,     0,     0,   345,     0,   285,   286,     0,
       0,     0,     0,     0,   214,   215,   216,   217,   218,     0,
       0,     0,   107,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   288,     0,     0,   917,
       0,     0,    93,    94,     0,    95,   188,    97,     0,     0,
     290,   291,   292,   293,   294,   295,   296,     0,     0,     0,
     210,     0,     0,     0,     0,     0,   297,     0,     0,     0,
     107,     0,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,    50,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,     0,   333,
       0,  1365,   335,   336,   337,     0,     0,     0,   338,   596,
     214,   215,   216,   217,   218,   597,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   277,   278,     0,   279,
     280,     0,   598,   281,   282,   283,   284,     0,    93,    94,
       0,    95,   188,    97,   343,     0,   344,     0,     0,   345,
       0,   285,   286,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   107,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     288,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   290,   291,   292,   293,   294,   295,
     296,     0,     0,     0,   210,     0,     0,     0,     0,     0,
     297,     0,     0,     0,     0,     0,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,    50,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,     0,   333,     0,     0,   335,   336,   337,     0,
       0,     0,   338,   596,   214,   215,   216,   217,   218,   597,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   598,     0,     0,     0,
       0,     0,    93,    94,     0,    95,   188,    97,   343,     0,
     344,     0,     0,   345,   470,   471,   472,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     107,     0,     0,     0,   473,   474,     0,   475,   476,   477,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
       0,   498,   470,   471,   472,     0,     0,     0,     0,     0,
       0,     0,     0,   499,     0,     0,     0,     0,     0,     0,
       0,     0,   473,   474,     0,   475,   476,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,     0,   498,
     470,   471,   472,     0,     0,     0,     0,     0,     0,     0,
       0,   499,     0,     0,     0,     0,     0,     0,     0,     0,
     473,   474,  1513,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,     0,   498,   470,   471,
     472,     0,     0,     0,     0,     0,     0,     0,     0,   499,
       0,     0,     0,     0,     0,     0,     0,     0,   473,   474,
       0,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,     0,   498,  1705,     0,   470,   471,
     472,     0,     0,     0,     0,     0,     0,   499,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   473,   474,
       0,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,  1706,   498,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   499,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   470,   471,
     472,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   473,   474,
    1514,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,     0,   498,   470,   471,   472,     0,
       0,     0,     0,     0,     0,     0,     0,   499,     0,     0,
       0,     0,     0,     0,     0,     0,   473,   474,   584,   475,
     476,   477,   478,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,     0,   498,     0,     0,   470,   471,   472,     0,
       0,     0,     0,     0,     0,   499,     0,   287,     0,     0,
       0,     0,     0,     0,     0,     0,   473,   474,   603,   475,
     476,   477,   478,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,     0,   498,   289,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   499,   287,   210,     0,     0,
       0,     0,     0,  1521,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,   607,     0,
       0,     0,     0,   289,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   287,     0,   210,     0,     0,     0,
       0,     0,   992,     0,     0,     0,   585,   214,   215,   216,
     217,   218,   586,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,   820,     0,     0,     0,   187,
       0,   289,    91,   342,     0,    93,    94,     0,    95,   188,
      97,     0, -1124,     0,   210,     0,     0,     0,     0,     0,
    1444,     0,     0,   346,     0,   585,   214,   215,   216,   217,
     218,   586,     0,   107,   347,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,   845,     0,     0,   187,     0,
       0,    91,   342,     0,    93,    94,     0,    95,   188,    97,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1104,
       0,     0,   346,   585,   214,   215,   216,   217,   218,   586,
       0,     0,   107,   347,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   187,     0,     0,    91,
     342,    29,    93,    94,     0,    95,   188,    97,     0,    34,
      35,    36,   210,     0,   211,    40,     0,     0,     0,     0,
     346,     0,     0,   212,     0,     0,     0,     0,     0,     0,
     107,   347,     0,     0,     0,    50,     0,     0,     0,  1373,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   213,     0,     0,     0,     0,   875,   876,     0,
       0,     0,     0,   877,     0,   878,     0,     0,     0,     0,
    1105,    75,   214,   215,   216,   217,   218,   879,    81,    82,
      83,    84,    85,     0,     0,    34,    35,    36,   210,   219,
       0,     0,     0,     0,   187,    89,    90,    91,    92,   212,
      93,    94,     0,    95,   188,    97,     0,     0,     0,    99,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   104,     0,     0,     0,     0,   107,   220,
       0,     0,     0,   875,   876,   111,     0,     0,     0,   877,
       0,   878,     0,     0,     0,     0,     0,   880,   881,   882,
     883,   884,   885,   879,    81,    82,    83,    84,    85,     0,
       0,    34,    35,    36,   210,   219,     0,     0,     0,     0,
     187,    89,    90,    91,    92,   212,    93,    94,     0,    95,
     188,    97,     0,     0,     0,    99,     0,    50,     0,     0,
       0,     0,     0,     0,   886,     0,     0,     0,     0,   104,
       0,     0,     0,     0,   107,   887,     0,     0,     0,  1057,
    1058,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   880,   881,   882,   883,   884,   885,  1059,
      81,    82,    83,    84,    85,     0,     0,  1060,  1061,  1062,
     210,   219,     0,     0,     0,     0,   187,    89,    90,    91,
      92,  1063,    93,    94,     0,    95,   188,    97,     0,     0,
       0,    99,     0,    50,     0,     0,     0,     0,     0,     0,
     886,     0,     0,     0,     0,   104,     0,     0,     0,     0,
     107,   887,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1064,
    1065,  1066,  1067,  1068,  1069,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1070,     0,     0,
       0,     0,   187,     0,     0,    91,    92,    29,    93,    94,
       0,    95,   188,    97,     0,    34,    35,    36,   210,     0,
     211,    40,     0,     0,     0,     0,  1071,     0,     0,   212,
       0,     0,     0,     0,     0,     0,   107,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   213,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,   214,   215,
     216,   217,   218,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   219,     0,     0,     0,     0,
     187,    89,    90,    91,    92,     0,    93,    94,    29,    95,
     188,    97,     0,     0,     0,    99,    34,    35,    36,   210,
       0,   211,    40,     0,     0,     0,     0,     0,     0,   104,
     212,     0,     0,     0,   107,   220,     0,     0,   623,     0,
       0,   111,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   213,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   643,    75,   214,
     215,   216,   217,   218,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,   219,     0,     0,     0,
       0,   187,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   188,    97,     0,     0,     0,    99,     0,    29,  1048,
       0,     0,     0,     0,     0,     0,    34,    35,    36,   210,
     104,   211,    40,     0,     0,   107,   220,     0,     0,     0,
     212,     0,   111,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   213,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    75,   214,
     215,   216,   217,   218,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,   219,     0,     0,     0,
       0,   187,    89,    90,    91,    92,     0,    93,    94,    29,
      95,   188,    97,     0,     0,     0,    99,    34,    35,    36,
     210,     0,   211,    40,     0,     0,     0,     0,     0,     0,
     104,   212,     0,     0,     0,   107,   220,     0,     0,     0,
       0,     0,   111,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     213,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1205,    75,
     214,   215,   216,   217,   218,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,   219,     0,     0,
       0,     0,   187,    89,    90,    91,    92,     0,    93,    94,
      29,    95,   188,    97,     0,     0,     0,    99,    34,    35,
      36,   210,     0,   211,    40,     0,     0,     0,     0,     0,
       0,   104,   212,     0,     0,     0,   107,   220,     0,     0,
       0,     0,     0,   111,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   213,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,   214,   215,   216,   217,   218,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,   219,     0,
       0,     0,     0,   187,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   188,    97,     0,     0,     0,    99,     0,
       0,     0,   470,   471,   472,     0,     0,     0,     0,     0,
       0,     0,   104,     0,     0,     0,     0,   107,   220,     0,
       0,     0,   473,   474,   111,   475,   476,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,     0,   498,
     470,   471,   472,     0,     0,     0,     0,     0,     0,     0,
       0,   499,     0,     0,     0,     0,     0,     0,     0,     0,
     473,   474,     0,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,     0,   498,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   499,
       0,   470,   471,   472,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     543,   473,   474,     0,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,     0,   498,   470,
     471,   472,     0,     0,     0,     0,     0,     0,     0,     0,
     499,     0,     0,     0,     0,     0,     0,     0,   552,   473,
     474,     0,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,     0,   498,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   499,     0,
     470,   471,   472,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   949,
     473,   474,     0,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,     0,   498,   470,   471,
     472,     0,     0,     0,     0,     0,     0,     0,     0,   499,
       0,     0,     0,     0,     0,     0,     0,  1034,   473,   474,
       0,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,     0,   498,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   499,     0,   470,
     471,   472,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1088,   473,
     474,     0,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,     0,   498,  1149,  1150,  1151,
       0,     0,     0,     0,     0,     0,     0,     0,   499,     0,
       0,     0,     0,     0,     0,     0,  1420,     0,  1152,     0,
       0,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,
    1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1175,  1149,  1150,  1151,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1462,  1152,     0,
       0,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,
    1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,  1149,  1150,  1151,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1175,     0,     0,     0,
       0,     0,     0,     0,  1152,  1347,     0,  1153,  1154,  1155,
    1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1175,  1149,  1150,  1151,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1152,  1532,     0,  1153,  1154,  1155,
    1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1149,
    1150,  1151,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1175,     0,     0,     0,     0,     0,     0,     0,
    1152,  1543,     0,  1153,  1154,  1155,  1156,  1157,  1158,  1159,
    1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1175,  1149,
    1150,  1151,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1152,  1650,     0,  1153,  1154,  1155,  1156,  1157,  1158,  1159,
    1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,    34,    35,    36,   210,     0,
     211,    40,     0,     0,     0,     0,     0,     0,  1175,   212,
       0,     0,     0,     0,     0,     0,     0,  1745,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   241,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   242,
       0,     0,     0,     0,     0,     0,     0,     0,   214,   215,
     216,   217,   218,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   219,     0,  1747,     0,     0,
     187,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     188,    97,     0,     0,     0,    99,    34,    35,    36,   210,
       0,   211,    40,     0,     0,     0,     0,     0,     0,   104,
     674,     0,     0,     0,   107,   243,     0,     0,     0,     0,
       0,   111,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   213,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   214,
     215,   216,   217,   218,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,   219,     0,     0,     0,
       0,   187,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   188,    97,     0,     0,     0,    99,    34,    35,    36,
     210,     0,   211,    40,     0,     0,     0,     0,     0,     0,
     104,   212,     0,     0,     0,   107,   675,     0,     0,     0,
       0,     0,   111,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     241,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     214,   215,   216,   217,   218,     0,    81,    82,    83,    84,
      85,     0,   210,     0,     0,     0,     0,   219,     0,     0,
       0,     0,   187,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   188,    97,     0,    50,     0,    99,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   210,     0,     0,
       0,   104,     0,     0,     0,     0,   107,   243,  1673,   210,
       0,     0,     0,   111,     0,     0,     0,     0,     0,     0,
      50,  1674,   214,   215,   216,   217,   218,  1675,   364,   365,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
     915,   916,     0,     0,   187,   210,     0,    91,    92,     0,
      93,    94,     0,    95,  1677,    97,  1119,   214,   215,   216,
     217,   218,     0,     0,     0,     0,     0,     0,    50,   214,
     215,   216,   217,   218,     0,     0,     0,     0,   107,   366,
       0,     0,   367,     0,     0,    93,    94,     0,    95,   188,
      97,     0,     0,     0,   917,     0,     0,    93,    94,     0,
      95,   188,    97,     0,   368,   214,   215,   216,   217,   218,
       0,     0,     0,   107,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   107,     0,   187,     0,     0,
      91,     0,     0,    93,    94,     0,    95,   188,    97,   470,
     471,   472,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   857,     0,     0,   473,
     474,   107,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,     0,   498,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   499,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   470,   471,   472,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   858,   473,   474,  1031,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,     0,
     498,   470,   471,   472,     0,     0,     0,     0,     0,     0,
       0,     0,   499,     0,     0,     0,     0,     0,     0,     0,
       0,   473,   474,     0,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,  1151,   498,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     499,     0,     0,     0,     0,     0,  1152,     0,     0,  1153,
    1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,
    1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   473,   474,  1175,   475,   476,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,     0,   498,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   499,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,     0,   498,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   499
};

static const yytype_int16 yycheck[] =
{
       5,     6,   131,     8,     9,    10,    11,    12,    13,   848,
      15,    16,    17,    18,    56,   691,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   108,    31,     4,   191,   165,
     123,   729,    98,   108,     4,     4,   102,   103,    60,    44,
     568,     4,   718,   688,    33,   108,   123,    52,    57,    54,
     108,   123,    57,   873,    59,   170,   171,    46,   414,    30,
     687,   556,    51,   192,   130,   244,    88,   414,  1412,    91,
     498,   990,   504,   414,   856,   988,   604,    57,   501,   502,
     414,    86,   444,   979,    30,    30,    30,   165,   863,     9,
       4,   537,   667,   530,   446,   447,   448,     9,  1216,  1103,
    1025,   832,   255,   108,   617,   618,   189,    44,     4,     9,
     530,   786,   535,  1880,   189,   364,   365,   366,  1043,   368,
       9,   533,   534,     9,    32,     9,    48,   564,  1010,    32,
    1012,   189,     4,     9,    38,     9,     9,   220,    14,     9,
      14,   256,     9,     9,   564,   220,     9,    14,     9,    86,
     562,    48,    70,    48,    48,    83,    84,    53,     9,    83,
      56,    48,   220,     9,  1089,     9,     9,    83,   243,     9,
       9,   162,   992,     9,     9,     9,    83,    73,     9,    83,
      38,     9,    91,     9,   189,   243,  1099,   162,     9,    91,
     116,   196,    70,   136,   137,    83,    36,    38,    50,    51,
      96,    70,    98,    14,   103,   561,   102,   103,    70,   162,
     201,   162,     0,    38,   168,   220,    19,    20,   136,   137,
      38,    32,    70,   198,   183,    83,    38,   181,    70,   168,
     183,    81,   183,   198,   130,   136,   137,     4,   243,   198,
      51,    70,    83,   919,   160,   161,   399,   201,   201,  1145,
     159,    70,   176,   258,   180,   198,   261,   159,    83,   131,
     107,   108,   201,   268,   269,    83,   822,   166,    70,   176,
       8,    83,   176,   195,   162,    70,   204,   199,  2045,    70,
     200,   201,   132,   201,   162,   262,  1561,    70,   200,   266,
     988,    70,  2059,   201,   457,    70,    70,   200,    54,    70,
     200,   202,   199,   386,   199,   199,    70,   183,   195,   205,
     199,   386,    54,   355,   200,  1046,   200,  1661,   176,  1323,
     192,    70,  1020,   201,   200,   853,   200,   200,   386,   198,
     200,    70,   184,   200,   200,   176,   198,   200,  1113,   200,
    1115,  1459,   364,   365,   366,   367,   368,  1229,  1466,   200,
    1468,   356,  1277,   201,   200,   202,   200,   200,   176,   199,
     199,   444,    70,   199,   199,   199,   262,   162,   199,   444,
     266,   199,   201,   199,   270,   111,   539,  1022,   199,  1497,
     385,   386,   201,   512,   406,   451,   444,   392,   393,   394,
     395,   396,   397,   968,    70,  1670,   198,   402,   165,   201,
     442,  1099,   915,   916,  1225,  1301,   201,   198,    83,    70,
     201,   167,  1232,   162,    83,   198,   421,   162,   201,  1694,
     198,  1696,   201,   198,   429,   167,   201,   201,   136,   137,
     201,   183,    70,    83,   237,   444,   441,   201,   183,   444,
     182,   507,   508,   509,   510,    70,   198,    83,   385,   198,
     513,   122,   201,   198,   459,   460,   201,   394,   395,   355,
     397,   132,   201,   107,   108,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   201,   499,  1613,   501,   502,   503,   945,
     198,   176,    83,  1059,   513,    70,   420,   176,   513,   514,
     515,   516,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   103,  1342,   103,   162,   201,    70,   498,   533,   534,
     535,   536,   428,   513,   198,   540,   103,  1319,   543,  1214,
     201,   694,   438,   696,   194,   198,   442,   552,    70,   554,
     982,   201,   498,   498,   498,   451,   428,   562,   202,   996,
     198,   570,    14,   201,  1940,   570,  1942,   572,   204,   705,
      83,    84,   375,   198,   199,    70,   996,   198,   159,   160,
     161,   384,  1110,   201,   166,   162,   166,   166,   391,   166,
     561,  1935,   136,   137,   123,   617,   618,   546,   575,   166,
     403,   780,   198,   132,  1231,   198,   183,    83,   504,   505,
     506,   507,   508,   509,   510,    91,  1261,   675,   623,  1264,
    1048,   200,   201,   979,  1444,   788,   198,   705,  1040,    83,
     198,   183,   979,   812,   530,    83,  1457,    91,   979,   511,
     198,   136,   137,    91,  1540,   979,   198,    31,  1010,   198,
    1012,   103,   104,    57,   550,  1092,   905,    70,   907,  1011,
     909,   428,  1014,  1015,   913,    69,    50,   162,   564,    53,
     675,   737,  1092,   826,   827,   183,    70,  1190,    83,   575,
     833,   834,    70,   112,   160,   161,    91,    70,   201,   183,
     198,   204,   121,   122,   123,   124,   125,   126,   167,   595,
     649,   650,    83,   198,   198,    31,   160,   161,   198,   888,
      91,   716,   160,   161,     4,  1147,    32,   395,   833,   397,
      87,   900,   136,   137,    50,   621,   622,    53,   677,    19,
      20,   207,    87,   166,   537,  1631,    31,  1293,  1294,   198,
     745,  1297,    83,   198,  1142,    83,  1144,  1303,   118,  1296,
      91,  1298,   206,    91,   159,   160,   161,   124,   125,   126,
     656,   657,   711,    38,   193,    19,    20,    70,   504,   124,
     125,   126,   200,   778,   688,    53,    54,    55,   159,   160,
     161,  1602,   200,  1604,  1279,   200,    81,    50,    51,  1145,
     200,    69,  1467,   200,   530,    83,    84,    92,  1145,   106,
     107,   108,   807,   400,  1145,   103,   104,   404,  1435,   104,
     200,  1145,   134,   135,   550,   200,  1461,    70,   108,   160,
     161,  1244,   160,   161,   200,   561,    70,   776,   564,   106,
     107,   108,    70,   729,   431,   840,   433,   434,   435,   436,
      70,   737,   112,   113,   114,    70,   141,   142,   143,   144,
     145,   856,   364,   365,   366,    70,   368,   729,  1270,    75,
      76,    75,    76,   852,   200,   201,   201,  1229,   163,  1281,
     162,   166,  1400,   198,   169,   170,  1322,   172,   173,   174,
     198,   176,    70,   905,   861,   907,   162,   909,   200,   201,
     198,   913,  1590,   915,   916,   917,  1717,  1911,  1912,   189,
    1721,  1576,   197,   121,   122,   123,   124,   125,   126,  1907,
    1908,   714,    49,    53,    54,    55,   865,    57,   166,  1475,
     200,  1477,   871,    69,   995,   996,   822,   183,   824,    69,
     220,   162,    50,    51,    52,    53,    54,    55,   705,    57,
     198,   198,   203,     9,   949,  1301,   951,   237,   953,   162,
     846,    69,   162,   243,  1301,   198,     8,   198,   200,   964,
    1301,   162,   729,    14,   860,   861,   162,  1301,   200,  1614,
    1416,  1499,   262,   978,   846,   193,   266,   200,     9,   201,
     200,  1047,   200,   237,    14,   132,   789,  1515,   132,   183,
     967,   199,   941,    14,   103,   822,  1671,   967,   967,   199,
     199,  1010,  1007,  1012,   967,  1010,   902,  1012,  1017,   199,
     199,   198,  1017,   205,   112,   911,   912,   201,   121,   122,
     123,   124,   125,   126,   198,   828,  1031,   830,   198,  1034,
     902,  1036,     9,   159,   199,  1040,   199,   121,   122,   123,
     124,   125,   126,   199,   199,    95,   942,     9,   132,   133,
     200,  1463,   183,   967,    14,   858,   198,     9,  1879,   198,
    1239,    83,  1883,  1242,  1620,   201,  1622,  1980,  1624,   201,
     200,   967,  1985,  1629,  1436,   200,   990,  1048,   201,   846,
     200,   200,  2001,  1088,   201,   375,   982,  1439,  1440,  1441,
     193,   175,   988,  1932,   384,   967,   386,  2010,  1937,   995,
     996,   391,  1048,  1048,  1048,  2024,  1634,  1096,  1022,   193,
     199,   199,   199,   403,  2033,  1643,   988,   200,   134,   198,
    1097,   375,   199,   203,  1020,     9,   929,     9,   203,  1561,
     384,  1659,   203,   203,  1083,   902,  1229,   391,   428,   203,
      70,    32,   945,   946,  1207,   135,   182,  1986,  1020,   403,
     162,  1047,  1229,   138,   444,     9,   199,  1229,   162,  2072,
     414,  1057,  1058,  1059,    14,     9,   195,     9,  1190,   184,
       9,   199,  1728,    14,     9,   199,   199,   134,  1127,   199,
     199,     9,  1131,    14,  1540,   203,  1598,  1599,   202,  1138,
     203,   199,   203,  1540,  1090,   198,  1092,   199,  1207,  1540,
     967,  1097,  1207,  1099,   203,  1101,  1540,   162,   199,   103,
    1738,  2032,   200,   200,     9,  2035,   138,   162,     9,  2039,
    1229,   988,   198,  2062,  1229,   199,  1122,  1099,  2049,  1101,
      70,   198,  1059,  2053,  2054,  1667,  1213,    70,  1670,  1244,
      70,    70,  1247,  1213,  1213,    70,   982,   537,   201,     9,
    1213,  1147,   202,  1020,   200,  1931,    14,  1933,   184,   995,
     996,     9,  1694,   201,  1696,  1270,   203,    14,   201,     6,
    1702,    14,   199,    70,   198,  1631,  1281,  1282,   195,   200,
    1176,    32,  1980,   537,  1631,   575,   198,  1985,    32,    14,
    1631,    50,    51,    52,    53,    54,    55,  1631,   198,  1213,
    1103,  1104,   198,    81,    14,    83,    52,    85,   198,    70,
      69,    48,  2010,    70,  1319,    70,    70,  1213,    70,   198,
       9,   199,  1311,   200,  1329,  1440,   104,   200,   162,  1885,
    1886,   198,  1099,   138,  1101,    14,   184,  1314,   138,   162,
       9,  1213,  1291,   199,   176,    69,  2022,  1261,   203,   176,
    1264,     9,    83,  1436,  1090,   202,  1092,   202,     9,  1887,
      83,  1436,   200,   141,   142,   143,   144,   145,   138,    14,
      56,    19,    20,   199,  2072,    83,   113,   201,  1436,   198,
     202,   202,   119,   198,   121,   122,   123,   124,   125,   126,
     127,   169,   170,   200,   172,   173,   174,  1293,  1294,  1295,
    1296,  1297,  1298,   199,  1353,   198,   198,  1303,  1357,   138,
       9,   201,   201,  1362,   200,  1420,   203,    92,  1314,   197,
    1369,   159,  1427,    32,   714,   201,  1431,  1436,  1433,   200,
    1326,  1436,   169,   170,    77,   172,   199,   138,   200,   729,
    1336,  1577,  1245,  1572,  1449,     6,  1213,    32,   184,  1881,
     199,     9,     9,   203,  1326,   138,   193,  1462,  1463,   199,
     714,   203,    81,     9,   203,   202,  1293,  1294,  1295,  1296,
    1297,  1298,   199,   203,   202,   123,  1303,   203,     9,   199,
     202,   200,   200,   200,   200,   104,   201,    48,   200,    14,
      83,   198,   198,   198,   203,   199,   199,   199,     9,   789,
    1414,   200,   199,   201,  1307,   199,   138,   203,   203,     9,
    1424,   203,  1408,   203,   138,   203,     9,   199,    32,  1322,
    1323,   199,   141,   142,   143,   144,   145,   200,   199,  1692,
     200,   138,  2060,  1482,   200,   789,   176,  1486,   828,   201,
     830,   113,   171,   200,  1493,   167,    83,  1461,   167,    14,
     169,   170,   113,   172,   173,   174,   846,    83,   119,  1326,
     121,   122,   123,   124,   125,   126,   127,   119,   858,   199,
     201,   861,   199,   138,   828,   138,   830,    14,   197,  1475,
     183,  1477,   199,   201,   200,  1590,    83,    14,    14,   237,
    1595,    83,   198,  1598,  1599,   199,   197,   199,  1664,   199,
     138,   287,   138,   289,   858,   200,   200,    14,   169,   170,
      14,   172,   902,  1416,   200,   121,   122,   123,   124,   125,
     126,    14,   201,     9,     9,   202,   132,   133,    68,    83,
     183,   198,   193,    83,     9,     9,   116,   201,   200,   929,
     103,   202,   162,   103,   184,   174,    36,  1414,  1475,    14,
    1477,   198,   200,   198,  1550,   945,   946,  1424,   199,   180,
     184,   347,   184,  1640,    83,  1561,   199,   173,   177,   175,
       9,  1567,    19,    20,    83,   929,   199,   967,  1550,    14,
      83,   200,   188,    30,   190,   201,   199,   193,    83,  1603,
      14,   945,   946,    14,  1590,  1609,    83,  1611,   988,  1571,
    1614,    83,  1865,    83,    14,    83,  1711,  1190,  2013,    56,
    1041,   510,  2029,   507,  1320,   970,  1737,   505,  1590,  1633,
    1010,  2024,  1012,  1512,  1620,   979,  1622,   375,  1624,  1246,
    1020,  1724,   625,  1629,  1762,  1849,   384,  1672,   386,  1635,
    2070,  1861,  2046,   391,  1640,  1567,  1563,  1720,  1644,   396,
    1143,  1297,  1217,   439,  1058,   403,   442,  1139,  1292,  1736,
    1737,  1293,  1085,  1635,   392,   442,  1925,   874,  1664,  1981,
    1971,  1667,  1644,  1555,  1670,  1198,  1123,  1176,    -1,    -1,
      -1,    -1,    -1,  1550,  1680,    -1,    -1,    -1,    -1,    -1,
      -1,  1687,    -1,  1620,    -1,  1622,    -1,  1624,  1694,    -1,
    1696,    -1,  1629,    -1,    -1,    -1,  1702,  1097,    -1,  1099,
    1577,  1101,  1726,  1103,  1104,    -1,    -1,    -1,  1860,    -1,
      -1,    -1,    -1,  1590,    -1,    -1,    -1,    -1,  1700,    -1,
      -1,    -1,  1728,    -1,    -1,    -1,  1603,    31,    -1,  1735,
    1736,  1737,  1609,    -1,  1611,  1741,    78,    79,    80,  1103,
    1104,    -1,  1748,    -1,    -1,    -1,    -1,    -1,  1994,    -1,
      92,    -1,    -1,  1735,    -1,  1870,  1633,    -1,  1635,  1741,
      -1,    -1,    -1,    -1,    -1,    -1,  1748,  1644,    -1,    -1,
      -1,    -1,    -1,  1925,    -1,    -1,    -1,    81,  2017,   537,
     237,  1145,    -1,    -1,    -1,    -1,    -1,    -1,    92,    -1,
      -1,  1728,    -1,   589,    -1,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,  1213,    -1,    -1,   158,    -1,    -1,    -1,
      -1,    -1,   164,   165,    -1,    -1,    -1,    -1,    -1,  1229,
     287,    -1,   289,    -1,    -1,    -1,   178,   141,   142,   143,
     144,   145,  1901,  1902,    -1,  1245,    -1,    -1,    -1,  1726,
     192,    -1,    -1,    -1,  1860,    -1,    -1,    -1,  1735,   163,
      -1,    -1,   166,    -1,  1741,   169,   170,    -1,   172,   173,
     174,  1748,   176,    -1,    -1,  1881,    -1,    -1,    -1,  1885,
    1886,  1245,    -1,    -1,    -1,  1891,    -1,    -1,   684,   685,
     347,    -1,    -1,   197,  1900,    -1,    -1,    -1,    -1,    -1,
      -1,  1907,  1908,    -1,    -1,  1911,  1912,  1307,    -1,  1891,
      -1,    -1,    -1,    -1,  1314,    -1,    -1,    -1,   375,  1925,
      -1,    -1,  1322,  1323,    -1,    -1,  1326,   384,    -1,    -1,
      -1,    -1,    -1,    -1,   391,    -1,    -1,  1301,    -1,  1945,
      -1,    -1,    -1,  1307,    -1,    -1,   403,    -1,  1885,  1886,
      -1,    -1,    -1,  2068,    -1,    -1,   714,   414,  1322,  1323,
      -1,    -1,  2077,  1945,    -1,    -1,    -1,    -1,    -1,    -1,
    2085,    -1,    -1,  2088,  1980,    -1,    -1,  2001,     6,  1985,
      -1,    -1,   439,    -1,    -1,   442,    -1,  1993,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1980,    -1,
    2024,    -1,    -1,  1985,  2010,    -1,    -1,    -1,    -1,  2033,
    2016,    -1,    -1,    -1,  1891,    -1,  1416,    -1,    -1,    -1,
      48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2010,    -1,
      -1,   789,    -1,    -1,    -1,    -1,  1436,    -1,    -1,    -1,
      -1,   498,     6,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1416,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  2071,  2072,    -1,  1945,    -1,
     828,    -1,   830,    -1,    -1,    -1,    -1,   873,   874,    -1,
     537,    -1,    -1,    -1,    48,   113,    -1,    -1,    -1,  2071,
    2072,   119,    -1,   121,   122,   123,   124,   125,   126,   127,
     858,    -1,    -1,  1980,    -1,     6,    -1,    -1,  1985,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1994,    -1,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,   589,  2010,   591,    -1,    -1,   594,    -1,    -1,
      -1,   169,   170,    -1,   172,    69,    -1,    48,    -1,   113,
    1550,    -1,    -1,    -1,    -1,   119,    -1,   121,   122,   123,
     124,   125,   126,   127,    -1,   193,    -1,    -1,    -1,    -1,
     627,   929,    -1,   969,   202,    -1,  1540,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   945,   946,   985,
    1590,     6,    -1,    -1,  2071,  2072,    -1,    -1,    -1,    -1,
      -1,    -1,   998,    -1,    -1,   169,   170,    -1,   172,    -1,
      -1,    -1,   113,    -1,    -1,    -1,    -1,    -1,   119,    -1,
     121,   122,   123,   124,   125,   126,   127,   684,   685,   193,
      -1,    -1,    -1,    48,    -1,  1635,   693,    -1,   202,    -1,
    1640,    -1,    -1,    -1,  1644,    -1,    -1,    -1,  1044,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   714,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1631,   169,   170,
      -1,   172,    -1,     6,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
      -1,    -1,   193,    -1,    -1,    -1,    -1,    -1,   113,    -1,
      -1,   202,    -1,    -1,   119,    -1,   121,   122,   123,   124,
     125,   126,   127,    -1,    -1,    48,    -1,    -1,    -1,    59,
      60,  1117,    -1,    -1,    -1,  1121,    -1,    -1,    -1,    -1,
    1126,    -1,   789,    -1,    -1,  1735,  1736,  1737,    -1,    -1,
      -1,  1741,    -1,    -1,    -1,  1103,  1104,    -1,  1748,    -1,
      -1,    -1,    -1,    -1,   169,   170,    -1,   172,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   822,    -1,    -1,    -1,    -1,
      -1,   828,    -1,   830,    -1,    -1,    -1,    -1,   193,    -1,
     113,    -1,    -1,    -1,    -1,    -1,   119,   202,   121,   122,
     123,   124,   125,   126,   127,    -1,   136,   137,    -1,    -1,
      -1,   858,   859,    -1,    81,    -1,    83,    84,    -1,   866,
      -1,    -1,    -1,    -1,    -1,    -1,   873,   874,   875,   876,
     877,   878,   879,    -1,    -1,    -1,    -1,   104,    -1,    -1,
     887,    -1,    -1,    -1,  1230,    -1,   169,   170,    -1,   172,
      -1,    -1,    -1,    -1,    -1,    -1,   903,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   199,
     193,    -1,    -1,    -1,   141,   142,   143,   144,   145,   202,
      -1,  1229,   929,    -1,    -1,    -1,    -1,  1273,    -1,    -1,
    1276,    -1,    -1,    -1,    -1,    -1,   943,  1245,   945,   946,
     167,  1891,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   969,   970,    -1,    -1,    -1,    -1,    -1,    -1,
     197,    -1,   979,    -1,   201,    -1,    -1,   204,   985,    -1,
      -1,    19,    20,    -1,    -1,   992,    -1,    -1,    -1,  1335,
      -1,   998,    30,    -1,    -1,  1945,  1342,    -1,    -1,  1307,
      -1,    -1,  1009,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    56,  1322,  1323,    -1,  1024,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1980,    -1,    -1,    -1,    -1,  1985,    -1,  1044,    -1,    -1,
      -1,  1048,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,  1059,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    2010,    -1,    -1,  1409,  1410,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,  1103,  1104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,  1416,    -1,
    1117,    -1,    -1,    -1,  1121,    -1,  1123,    -1,    -1,  1126,
      -1,  2071,  2072,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1139,  1140,  1141,  1142,  1143,  1144,  1145,    -1,
      -1,  1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,
    1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
    1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,    -1,
      -1,    -1,    -1,    -1,  1520,  1521,    -1,    -1,  1524,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1194,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   237,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,  1561,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1230,    -1,  1232,  1572,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,  1245,   203,
      -1,    -1,    -1,    -1,   287,    -1,   289,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,    -1,    -1,    -1,  1273,    -1,    -1,  1276,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1293,  1294,  1295,  1296,
    1297,  1298,    59,    60,  1301,    -1,  1303,    -1,    -1,  1645,
    1307,    -1,   169,   170,   347,   172,   173,   174,    -1,    31,
      -1,    -1,    -1,    -1,    -1,  1322,  1323,    -1,  1325,    -1,
      -1,    -1,    -1,    -1,  1670,    -1,    -1,    -1,  1335,    -1,
     197,    -1,    -1,    -1,    -1,  1342,    -1,   375,    -1,    -1,
    1347,    -1,  1349,    -1,    -1,    -1,   384,    -1,  1694,    -1,
    1696,    -1,    -1,   391,    -1,    -1,  1702,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,   403,  1373,    -1,    -1,   136,
     137,    -1,    -1,    -1,    -1,    -1,   414,    -1,    -1,    -1,
      -1,    -1,   104,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,   439,    -1,    -1,   442,
      -1,    -1,  1409,  1410,    -1,   127,  1413,    -1,    -1,  1416,
      -1,    -1,    -1,    -1,  1760,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,   146,    -1,    -1,    -1,    59,    60,
      -1,    -1,   199,    -1,    -1,    -1,    -1,  1444,    -1,    -1,
      -1,   163,    -1,    -1,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,    10,    11,    12,    -1,    -1,    -1,    -1,
     498,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1475,    -1,
    1477,    -1,    -1,    30,    31,   197,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,   537,
      57,    -1,    -1,    -1,    -1,   136,   137,    -1,    -1,    -1,
      -1,    -1,    69,  1520,  1521,    -1,    -1,  1524,  1864,    -1,
      -1,    -1,    -1,  1530,    -1,  1532,    -1,  1534,    -1,    -1,
      -1,  1877,  1539,  1540,    -1,  1881,  1543,    81,  1545,    83,
      84,  1548,    -1,    10,    11,    12,   589,    -1,   591,    -1,
      -1,    -1,    -1,    -1,  1561,  1562,   594,    -1,  1565,    -1,
     104,    -1,    -1,    30,    31,  1572,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,   627,
      57,    -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,    69,    -1,    -1,    -1,    -1,    -1,  1954,    -1,
      -1,    -1,    -1,  1620,    -1,  1622,    -1,  1624,    -1,    -1,
      -1,    -1,  1629,   167,  1631,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,    -1,  1981,    -1,  1983,  1645,    -1,
      -1,   684,   685,  1650,    -1,   202,    -1,    -1,    -1,    -1,
     693,    -1,    -1,   197,    -1,  1662,  1663,   201,    -1,    -1,
     204,    -1,    -1,  1670,    -1,  1672,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,   714,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1694,    -1,  1696,
      -1,    -1,    -1,    30,    31,  1702,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,  1728,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,   200,    -1,   202,    -1,    -1,  1745,  1746,
    1747,    -1,    -1,    -1,    -1,  1752,    -1,  1754,    -1,    -1,
      -1,   789,    -1,  1760,    -1,  1762,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   822,    -1,    -1,    -1,    -1,    -1,
     828,    31,   830,    -1,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
     858,   859,    -1,   866,    -1,    -1,    -1,    -1,    -1,    69,
     873,   874,    -1,    -1,    -1,    -1,    -1,   875,   876,   877,
     878,   879,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   887,
      -1,    -1,    31,    -1,    -1,    -1,    -1,  1864,    -1,    -1,
      -1,    -1,    -1,   200,    -1,   903,    -1,    -1,    -1,    -1,
    1877,    -1,    -1,    -1,  1881,    -1,    -1,    -1,  1885,  1886,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      81,   929,    -1,  1900,    -1,    -1,    -1,    -1,    -1,  1906,
      -1,    -1,    81,    -1,    -1,   943,    -1,   945,   946,    -1,
    1917,    -1,    -1,   104,    -1,    -1,  1923,    -1,    -1,    -1,
    1927,    -1,    -1,    19,    20,   104,   969,    -1,    -1,    -1,
      -1,    -1,   970,   112,    30,    -1,    -1,    -1,    -1,    -1,
      -1,   979,   985,    -1,    -1,    -1,    -1,  1954,    -1,   992,
     141,   142,   143,   144,   145,   998,    -1,    -1,    -1,   199,
      -1,   140,   141,   142,   143,   144,   145,   146,    -1,    -1,
      -1,  1009,   163,    -1,  1981,   166,  1983,    -1,   169,   170,
      81,   172,   173,   174,   163,    -1,  1024,   166,   167,    -1,
     169,   170,  1999,   172,   173,   174,    -1,    -1,    -1,    -1,
      -1,  1044,    -1,   104,    -1,  2012,   197,    -1,   187,    -1,
    1048,   202,    -1,    -1,    -1,    -1,    -1,    -1,   197,   198,
      -1,  1059,  2029,    -1,    -1,    -1,    -1,    -1,  2035,   130,
      -1,    -1,  2039,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,    -1,  2053,  2054,    -1,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,  1103,  1104,    -1,   169,   170,
      -1,   172,   173,   174,  1117,    69,    -1,    -1,  1121,    -1,
    1123,    -1,    -1,  1126,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   197,   198,    -1,    -1,
      -1,  1139,  1140,  1141,  1142,  1143,  1144,  1145,    -1,    -1,
    1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,
    1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,    10,    11,
      12,   237,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1194,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,  1230,    -1,  1232,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1245,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1273,    -1,    -1,  1276,    -1,   104,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,  1293,  1294,  1295,  1296,  1297,
    1298,    -1,    -1,  1301,    -1,  1303,    -1,    -1,    69,  1307,
      -1,    -1,   141,   142,   143,   144,   145,    -1,    -1,   375,
      -1,    -1,    -1,    -1,  1322,  1323,    -1,  1325,   384,    -1,
      -1,    -1,  1335,    -1,    -1,   391,    -1,   166,    -1,  1342,
     169,   170,    -1,   172,   173,   174,    -1,   403,    -1,  1347,
      -1,  1349,    -1,    10,    11,    12,    -1,    -1,   414,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   197,    -1,
     202,    -1,   201,    30,    31,  1373,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,  1409,  1410,    -1,    -1,
      -1,    -1,    69,    -1,    -1,  1413,    -1,    -1,  1416,    -1,
      -1,    -1,    -1,    10,    11,    12,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,  1444,   498,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    59,    60,    -1,    -1,    -1,    -1,  1475,    -1,  1477,
      -1,   537,    69,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,  1520,  1521,    -1,
      -1,  1524,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,
      69,    -1,  1530,    -1,  1532,    -1,  1534,    -1,   594,    -1,
      -1,  1539,  1540,   200,    -1,  1543,    -1,  1545,   136,   137,
    1548,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1561,    -1,
      -1,    -1,    -1,    -1,  1562,    -1,    -1,  1565,    -1,  1572,
      -1,   627,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,   594,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   136,   137,    -1,    -1,    -1,
      -1,   199,    -1,    -1,    -1,   202,    -1,    -1,    59,    60,
     627,    -1,  1620,    -1,  1622,    -1,  1624,    -1,    -1,    -1,
      -1,  1629,    -1,  1631,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1645,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1650,    -1,    -1,    -1,    -1,    -1,   714,    -1,
      -1,    -1,    -1,    -1,  1662,  1663,    -1,  1670,   199,    -1,
      -1,    -1,    -1,    -1,  1672,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,  1694,    -1,  1696,    -1,   136,   137,    -1,    -1,  1702,
      31,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    1728,    57,    -1,   789,    -1,    -1,    -1,    68,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,  1745,  1746,  1747,
      81,    -1,    -1,    -1,  1752,    -1,  1754,  1760,   199,    -1,
      -1,    -1,    -1,    -1,  1762,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   828,   104,   830,    -1,    -1,    -1,    -1,    -1,
      -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     121,   122,   123,   124,   125,   126,    -1,    -1,    -1,    -1,
      -1,    -1,   858,   859,    -1,    -1,    -1,    -1,    -1,   140,
     141,   142,   143,   144,   145,   146,    -1,    -1,    -1,   875,
     876,   877,   878,   879,    -1,    -1,    -1,    31,    -1,    -1,
      -1,   887,   163,    -1,    -1,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   859,    -1,    -1,    -1,   187,    -1,    -1,    -1,
      -1,  1864,   193,    -1,    68,    -1,   197,   198,   875,   876,
     877,   878,   879,   929,  1877,    -1,   202,    81,  1881,    -1,
     887,    -1,    -1,    87,    -1,    -1,    -1,  1885,  1886,   945,
     946,    -1,    -1,    -1,    -1,    -1,    -1,  1900,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1906,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1917,
      -1,    -1,    -1,   979,    -1,  1923,    -1,    -1,    -1,  1927,
      -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1954,    -1,  1009,    -1,    -1,    -1,    -1,    -1,   163,
      -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,   176,    -1,    -1,    -1,    -1,    -1,  1981,    -1,
    1983,    -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1048,   197,   198,    -1,    -1,    -1,    -1,    -1,
      -1,  1999,  1009,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  2012,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,  2029,  2035,    -1,    -1,    -1,  2039,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1103,  1104,    -1,
    2053,  2054,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    59,    60,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1139,  1140,  1141,  1142,  1143,  1144,  1145,
      -1,    -1,  1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,
    1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1139,  1140,    -1,    -1,  1143,    -1,  1194,   136,
     137,  1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,
    1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
    1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,    -1,
      10,    11,    12,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,  1194,    -1,  1245,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    59,    60,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1301,    -1,    -1,    -1,    -1,
      -1,  1307,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1322,  1323,    -1,  1325,
      -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,
      -1,  1347,    -1,  1349,    -1,   136,   137,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,  1373,  1325,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,    -1,    -1,    10,    11,    12,    13,    -1,
    1347,    -1,  1349,    -1,    -1,    -1,   140,   141,   142,   143,
     144,   145,   146,    28,    29,    -1,    -1,    -1,    -1,    -1,
    1416,    -1,   202,    -1,    -1,    -1,  1373,    -1,    -1,   163,
      -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   187,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,   197,   198,    -1,    81,    -1,    83,    84,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,    -1,   130,    -1,   132,   133,   134,
     135,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,  1530,    -1,  1532,    -1,  1534,    -1,
      -1,    -1,    -1,  1539,  1540,    -1,    -1,  1543,   163,  1545,
      -1,    -1,  1548,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,   197,  1530,    -1,  1532,   201,  1534,    -1,   204,
      -1,    -1,  1539,    27,    28,    29,  1543,    -1,  1545,    -1,
      -1,  1548,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,  1631,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,  1650,    -1,    -1,    -1,    92,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,   122,   123,
     124,   125,   126,    -1,    -1,   129,   130,    -1,    -1,    -1,
      -1,    -1,    -1,  1650,    -1,   139,   140,   141,   142,   143,
     144,   145,    -1,   147,   148,   149,   150,   151,    -1,    -1,
      -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,
     164,   165,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,   178,    -1,    -1,   181,    -1,  1745,
    1746,  1747,    -1,   187,    -1,    -1,  1752,    -1,   192,   193,
     194,    -1,    -1,   197,   198,  1761,    -1,    -1,    -1,    -1,
     204,   205,   206,   207,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1745,  1746,
    1747,    -1,    -1,    30,    31,  1752,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    1906,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1917,    -1,    69,    -1,    -1,    -1,  1923,    -1,    -1,
      -1,  1927,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,     3,     4,     5,     6,     7,    -1,  1906,
    1956,    -1,    -1,    13,    -1,    69,    -1,    -1,    -1,    -1,
    1917,    -1,    -1,    -1,    -1,   202,  1923,    27,    28,    29,
    1927,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,  1999,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,   202,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,    99,
      -1,   101,  1999,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,   113,   114,   115,    -1,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,   131,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,
     190,    -1,   192,   193,   194,    -1,    -1,   197,   198,    -1,
     200,   201,    -1,    -1,   204,   205,   206,   207,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,
      95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,   113,   114,
     115,    -1,   117,    -1,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,   131,   132,   133,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,   188,    -1,   190,    -1,   192,   193,   194,
      -1,    -1,   197,   198,    -1,   200,   201,   202,    -1,   204,
     205,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,   113,   114,   115,    -1,   117,    -1,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,   131,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,
     190,    -1,   192,   193,   194,    -1,    -1,   197,   198,    -1,
     200,   201,   202,    -1,   204,   205,   206,   207,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,
      95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,
     115,    -1,   117,    -1,    -1,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,    -1,   132,   133,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,   194,
      -1,    -1,   197,   198,    -1,   200,   201,   202,    -1,   204,
     205,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      -1,    -1,   192,   193,   194,    -1,    -1,   197,   198,    -1,
     200,   201,   202,    -1,   204,   205,   206,   207,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,
      95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,
     115,    -1,   117,   118,    -1,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,    -1,   132,   133,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,   194,
      -1,    -1,   197,   198,    -1,   200,   201,    -1,    -1,   204,
     205,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      -1,    -1,   192,   193,   194,    -1,    -1,   197,   198,    -1,
     200,   201,   202,    -1,   204,   205,   206,   207,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,
      95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,
     115,    -1,   117,    -1,    -1,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,    -1,   132,   133,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,   194,
      -1,    -1,   197,   198,    -1,   200,   201,   202,    -1,   204,
     205,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      -1,    -1,   192,   193,   194,    -1,    -1,   197,   198,    -1,
     200,   201,   202,    -1,   204,   205,   206,   207,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,
      95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,
     115,    -1,   117,    -1,    -1,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,    -1,   132,   133,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,   194,
      -1,    -1,   197,   198,    -1,   200,   201,   202,    -1,   204,
     205,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    -1,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      -1,    -1,   192,   193,   194,    -1,    -1,   197,   198,    -1,
     200,   201,    -1,    -1,   204,   205,   206,   207,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,
      95,    -1,    97,    -1,    99,    -1,   101,   102,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,
     115,    -1,   117,    -1,    -1,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,    -1,   132,   133,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,   194,
      -1,    -1,   197,   198,    -1,   200,   201,    -1,    -1,   204,
     205,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      -1,    -1,   192,   193,   194,    -1,    -1,   197,   198,    -1,
     200,   201,   202,    -1,   204,   205,   206,   207,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    77,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,
      95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,
     115,    -1,   117,    -1,    -1,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,    -1,   132,   133,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,   194,
      -1,    -1,   197,   198,    -1,   200,   201,    -1,    -1,   204,
     205,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      -1,    -1,   192,   193,   194,    -1,    -1,   197,   198,    -1,
     200,   201,   202,    -1,   204,   205,   206,   207,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,
      95,    -1,    97,    -1,    99,   100,   101,    -1,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,
     115,    -1,   117,    -1,    -1,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,    -1,   132,   133,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,   194,
      -1,    -1,   197,   198,    -1,   200,   201,    -1,    -1,   204,
     205,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    98,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      -1,    -1,   192,   193,   194,    -1,    -1,   197,   198,    -1,
     200,   201,    -1,    -1,   204,   205,   206,   207,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,
      95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,
     115,    -1,   117,    -1,    -1,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,    -1,   132,   133,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,   194,
      -1,    -1,   197,   198,    -1,   200,   201,   202,    -1,   204,
     205,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      -1,    -1,   192,   193,   194,    -1,    -1,   197,   198,    -1,
     200,   201,   202,    -1,   204,   205,   206,   207,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,
      95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,
     115,    -1,   117,    -1,    -1,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,    -1,   132,   133,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,   194,
      -1,    -1,   197,   198,    -1,   200,   201,   202,    -1,   204,
     205,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      -1,    -1,   192,   193,   194,    -1,    -1,   197,   198,    -1,
     200,   201,   202,    -1,   204,   205,   206,   207,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,
      95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,
     115,    -1,   117,    -1,    -1,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,    -1,   132,   133,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,   194,
      -1,    -1,   197,   198,    -1,   200,   201,   202,    -1,   204,
     205,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      -1,    -1,   192,   193,   194,    -1,    -1,   197,   198,    -1,
     200,   201,    -1,    -1,   204,   205,   206,   207,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,
      95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,
     115,    -1,   117,    -1,    -1,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,   194,
      -1,    -1,   197,   198,    -1,   200,   201,    -1,    -1,   204,
     205,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      -1,    -1,   192,   193,   194,    -1,    -1,   197,   198,    -1,
     200,   201,    -1,    -1,   204,   205,   206,   207,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,
      95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,
     115,    -1,   117,    -1,    -1,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,   194,
      -1,    -1,   197,   198,    -1,   200,   201,    -1,    -1,   204,
     205,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,    99,
      -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,
     110,   111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,
     120,   121,   122,   123,   124,   125,   126,    -1,   128,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      -1,    -1,   192,   193,   194,    -1,    -1,   197,   198,    -1,
     200,   201,    -1,    -1,   204,   205,   206,   207,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,
      95,    -1,    97,    -1,    99,    -1,   101,    -1,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,
     115,    -1,   117,    -1,    -1,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,   194,
      -1,    -1,   197,   198,    -1,   200,   201,    -1,    -1,   204,
     205,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   121,   122,   123,   124,   125,   126,    -1,    -1,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      -1,    -1,   192,   193,   194,    -1,    -1,   197,   198,    -1,
     200,    -1,    -1,    -1,   204,   205,   206,   207,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    31,    -1,    13,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    92,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   121,   122,   123,   124,
     125,   126,    -1,    -1,   129,   130,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
      -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,   176,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,   194,
      -1,    -1,   197,   198,     3,     4,     5,     6,     7,   204,
     205,   206,   207,    -1,    13,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    87,    -1,
      -1,    -1,    -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   121,   122,   123,   124,   125,   126,    -1,    -1,
     129,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,   158,
      -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,   176,    -1,   178,
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,    -1,
      -1,    -1,    -1,   192,   193,   194,    -1,    -1,   197,   198,
       3,     4,     5,     6,     7,   204,   205,   206,   207,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,   122,
     123,   124,   125,   126,    -1,    -1,   129,   130,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,   178,    -1,    -1,   181,    -1,
      -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,   192,
     193,   194,    -1,    -1,   197,   198,    -1,    -1,   201,    -1,
      -1,   204,   205,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   121,   122,   123,   124,   125,   126,    -1,
      -1,   129,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,    -1,   147,
     148,   149,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,
     158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,
     178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,
      -1,    -1,    -1,    -1,   192,   193,   194,    -1,    -1,   197,
     198,     3,     4,     5,     6,     7,   204,   205,   206,   207,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,
     122,   123,   124,   125,   126,    -1,    -1,   129,   130,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,    -1,    10,    11,    12,
     192,   193,   194,    -1,    -1,   197,   198,     3,     4,     5,
       6,     7,   204,   205,   206,   207,    -1,    13,    31,    32,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    69,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,   109,    -1,    -1,   112,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   121,   122,   123,   124,   125,
     126,    -1,    -1,   129,   130,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
      -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,
      -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,    -1,
      -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,
      -1,   187,    -1,    -1,    -1,    -1,   192,   193,   194,    -1,
      -1,   197,   198,     3,     4,     5,     6,     7,   204,   205,
     206,   207,    -1,    13,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   121,   122,   123,   124,   125,   126,    -1,    -1,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,    -1,    10,
      11,    12,   192,   193,   194,    -1,    -1,   197,   198,     3,
       4,     5,     6,     7,   204,   205,   206,   207,    -1,    13,
      31,    -1,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    69,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,   122,   123,
     124,   125,   126,    -1,    -1,   129,   130,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,    -1,   147,   148,   149,   150,   151,    -1,    -1,
      -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,
     164,   165,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,
      -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,
     194,    -1,    -1,   197,   198,    -1,   200,    10,    11,    12,
     204,   205,   206,   207,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    69,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   121,   122,   123,   124,   125,   126,    -1,    -1,
     129,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,   158,
      -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   202,
      -1,    -1,    -1,   192,   193,   194,    -1,    -1,   197,   198,
      -1,   200,    -1,    -1,    -1,   204,   205,   206,   207,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,   122,   123,
     124,   125,   126,    -1,    -1,   129,   130,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,    -1,   147,   148,   149,   150,   151,    -1,    -1,
      -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,
     164,   165,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,
      -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,
     194,    11,    12,   197,   198,     3,     4,     5,     6,     7,
     204,   205,   206,   207,    -1,    13,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    69,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   121,   122,   123,   124,   125,   126,    -1,
      -1,   129,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,    -1,   147,
     148,   149,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,
     158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,
     178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,
      -1,    -1,    -1,    -1,   192,   193,   194,    -1,    -1,   197,
     198,   199,    -1,    -1,    -1,    -1,   204,   205,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,   122,
     123,   124,   125,   126,    -1,    -1,   129,   130,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,   178,    -1,    -1,   181,    -1,
      -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,   192,
     193,   194,    -1,    -1,   197,   198,     3,     4,     5,     6,
       7,   204,   205,   206,   207,    -1,    13,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    32,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   121,   122,   123,   124,   125,   126,
      -1,    -1,   129,   130,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,    -1,    -1,    -1,    -1,   192,   193,   194,    -1,    -1,
     197,   198,     3,     4,     5,     6,     7,   204,   205,   206,
     207,    -1,    13,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     121,   122,   123,   124,   125,   126,    -1,    -1,   129,   130,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,    -1,
     181,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,
      -1,   192,   193,   194,    -1,    -1,   197,   198,     3,     4,
       5,     6,     7,   204,   205,   206,   207,    -1,    13,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   121,   122,   123,   124,
     125,   126,    -1,    -1,   129,   130,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
      -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,   194,
      -1,    -1,   197,   198,     3,     4,     5,     6,     7,   204,
     205,   206,   207,    -1,    13,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   121,   122,   123,   124,   125,   126,    -1,    -1,
     129,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,    -1,    -1,    -1,   158,
      -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,    -1,
      -1,    -1,    -1,   192,   193,   194,    -1,    -1,   197,   198,
       3,     4,     5,     6,     7,   204,   205,   206,   207,    -1,
      13,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,   122,
     123,   124,   125,   126,    -1,    -1,   129,   130,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,   178,    -1,    -1,   181,    -1,
      -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,   192,
     193,   194,    -1,    -1,   197,   198,     3,     4,     5,     6,
       7,   204,   205,   206,   207,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   121,   122,   123,   124,   125,   126,
      -1,    -1,   129,   130,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,    -1,    -1,    11,    12,   192,   193,   194,    -1,    -1,
     197,   198,     3,     4,     5,     6,     7,   204,   205,   206,
     207,    -1,    13,    31,    -1,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    69,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     121,   122,   123,   124,   125,   126,    -1,    -1,   129,   130,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,    -1,
     181,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,
      -1,   192,   193,   194,    -1,    12,   197,   198,     3,     4,
       5,     6,     7,   204,   205,   206,   207,    -1,    13,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    69,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   121,   122,   123,   124,
     125,   126,    -1,    -1,   129,   130,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
      -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,     3,     4,   178,     6,     7,   181,    -1,    10,    11,
      12,    13,   187,    -1,    -1,    -1,    -1,   192,   193,   194,
      -1,    -1,   197,   198,    -1,    -1,    28,    29,    -1,   204,
     205,   206,   207,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    83,    84,    -1,    -1,    87,    -1,    -1,    -1,    -1,
      -1,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,    -1,   130,    -1,
     132,   133,   134,   135,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,   163,    10,    11,    12,    13,    -1,   169,   170,    -1,
     172,   173,   174,   175,    -1,   177,    -1,    -1,   180,    -1,
      28,    29,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   197,    -1,    -1,    -1,   201,
      -1,    -1,   204,    -1,    -1,    -1,    -1,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,    -1,   130,    -1,    81,   133,   134,   135,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,   175,    -1,   177,
      -1,    -1,   180,     3,     4,    -1,     6,     7,    -1,   187,
      10,    11,    12,    13,   141,   142,   143,   144,   145,   197,
     198,    -1,    -1,    -1,   202,    -1,    -1,    -1,    28,    29,
      -1,    31,    -1,    -1,    -1,    -1,   163,    -1,    -1,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
     197,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,
      -1,    -1,    -1,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,    -1,
     130,    -1,    81,   133,   134,   135,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,   146,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   163,    -1,    -1,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,   177,    -1,    -1,
     180,     3,     4,    -1,     6,     7,    -1,   187,    10,    11,
      12,    13,   141,   142,   143,   144,   145,   197,   198,    -1,
      -1,    -1,   202,    -1,    -1,    -1,    28,    29,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,   197,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,
      -1,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,    -1,   130,    -1,
     132,   133,   134,   135,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   163,    -1,    -1,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,    -1,   177,    -1,    -1,   180,     3,
       4,    -1,     6,     7,    -1,   187,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   197,   198,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    29,    -1,    31,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    57,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    -1,    69,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    -1,   130,    -1,    -1,   133,
     134,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,
      -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,   175,   104,   177,    -1,    -1,   180,    -1,     3,     4,
      -1,     6,     7,   187,   188,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,   197,   198,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    29,    -1,    31,    -1,    -1,   141,
     142,   143,   144,   145,    -1,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    57,    -1,    -1,   167,    -1,   169,   170,   171,
     172,   173,   174,    68,    -1,    69,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,   197,   198,    -1,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,    -1,   130,    -1,    81,   133,   134,
     135,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,
      -1,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,   104,   177,    -1,    -1,   180,     3,     4,     5,     6,
       7,    -1,   187,    10,    11,    12,    13,   141,   142,   143,
     144,   145,   197,   198,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,    -1,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    -1,   169,   170,    -1,   172,
     173,   174,    -1,   197,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,   197,   198,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   163,   164,   165,    81,
      -1,    -1,   169,   170,    -1,   172,   173,   174,   175,    -1,
     177,   178,    -1,   180,    -1,    -1,    -1,    -1,    -1,    -1,
     187,   188,   104,   190,    -1,   192,   193,    -1,     3,     4,
     197,     6,     7,    -1,    -1,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    29,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    57,    -1,    -1,    -1,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,   197,   198,    -1,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,    81,   130,    -1,   132,   133,   134,
     135,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,   163,    10,
      11,    12,    13,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,   177,    -1,    -1,   180,    -1,    28,    29,    -1,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,    -1,
      -1,    -1,   197,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,   166,
      -1,    -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
     197,    -1,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,    -1,   130,
      -1,   132,   133,   134,   135,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,
       7,    -1,   163,    10,    11,    12,    13,    -1,   169,   170,
      -1,   172,   173,   174,   175,    -1,   177,    -1,    -1,   180,
      -1,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   197,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,    -1,   130,    -1,    -1,   133,   134,   135,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,   146,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,    -1,
      -1,    -1,   169,   170,    -1,   172,   173,   174,   175,    -1,
     177,    -1,    -1,   180,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     197,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,   202,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,   202,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
     200,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   200,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   200,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    31,    81,    -1,    -1,
      -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,
      -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,   140,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,   199,    -1,    -1,    -1,   163,
      -1,    68,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,   176,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,   187,    -1,   140,   141,   142,   143,   144,
     145,   146,    -1,   197,   198,    -1,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   199,    -1,    -1,   163,    -1,
      -1,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,
      -1,    -1,   187,   140,   141,   142,   143,   144,   145,   146,
      -1,    -1,   197,   198,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,   166,
     167,    70,   169,   170,    -1,   172,   173,   174,    -1,    78,
      79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
     187,    -1,    -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,
     197,   198,    -1,    -1,    -1,   104,    -1,    -1,    -1,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   121,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    70,   147,   148,
     149,   150,   151,    -1,    -1,    78,    79,    80,    81,   158,
      -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,    92,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   192,    -1,    -1,    -1,    -1,   197,   198,
      -1,    -1,    -1,    50,    51,   204,    -1,    -1,    -1,    56,
      -1,    58,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,    70,   147,   148,   149,   150,   151,    -1,
      -1,    78,    79,    80,    81,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    92,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,   178,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,   192,
      -1,    -1,    -1,    -1,   197,   198,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   140,   141,   142,   143,   144,   145,    70,
     147,   148,   149,   150,   151,    -1,    -1,    78,    79,    80,
      81,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    92,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,   178,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,
     187,    -1,    -1,    -1,    -1,   192,    -1,    -1,    -1,    -1,
     197,   198,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,
     141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,    -1,    -1,   166,   167,    70,   169,   170,
      -1,   172,   173,   174,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,   187,    -1,    -1,    92,
      -1,    -1,    -1,    -1,    -1,    -1,   197,    -1,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    70,   172,
     173,   174,    -1,    -1,    -1,   178,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,   192,
      92,    -1,    -1,    -1,   197,   198,    -1,    -1,   201,    -1,
      -1,   204,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,   178,    -1,    70,    71,
      -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
     192,    83,    84,    -1,    -1,   197,   198,    -1,    -1,    -1,
      92,    -1,   204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    70,
     172,   173,   174,    -1,    -1,    -1,   178,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
     192,    92,    -1,    -1,    -1,   197,   198,    -1,    -1,    -1,
      -1,    -1,   204,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      70,   172,   173,   174,    -1,    -1,    -1,   178,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,   192,    92,    -1,    -1,    -1,   197,   198,    -1,    -1,
      -1,    -1,    -1,   204,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   192,    -1,    -1,    -1,    -1,   197,   198,    -1,
      -1,    -1,    30,    31,   204,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    31,    -1,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,    31,    -1,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,   138,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,   138,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,   138,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,   138,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    69,    92,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,    -1,    -1,    -1,   158,    -1,   138,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,   178,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,   192,
      92,    -1,    -1,    -1,   197,   198,    -1,    -1,    -1,    -1,
      -1,   204,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,   178,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
     192,    92,    -1,    -1,    -1,   197,   198,    -1,    -1,    -1,
      -1,    -1,   204,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    81,    -1,    -1,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,    -1,   104,    -1,   178,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,
      -1,   192,    -1,    -1,    -1,    -1,   197,   198,   127,    81,
      -1,    -1,    -1,   204,    -1,    -1,    -1,    -1,    -1,    -1,
     104,   140,   141,   142,   143,   144,   145,   146,   112,   113,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     112,   113,    -1,    -1,   163,    81,    -1,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    92,   141,   142,   143,
     144,   145,    -1,    -1,    -1,    -1,    -1,    -1,   104,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,   197,   163,
      -1,    -1,   166,    -1,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,
     172,   173,   174,    -1,   188,   141,   142,   143,   144,   145,
      -1,    -1,    -1,   197,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   197,    -1,   163,    -1,    -1,
     166,    -1,    -1,   169,   170,    -1,   172,   173,   174,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    30,
      31,   197,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    12,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    69,    33,    34,    35,    36,    37,
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
       0,   209,   210,     0,   211,     3,     4,     5,     6,     7,
      13,    27,    28,    29,    48,    50,    51,    56,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    70,
      71,    72,    73,    74,    78,    79,    80,    81,    82,    83,
      84,    86,    88,    92,    93,    94,    95,    97,    99,   101,
     104,   105,   109,   110,   111,   112,   113,   114,   115,   117,
     119,   120,   121,   122,   123,   124,   125,   126,   128,   129,
     130,   131,   132,   133,   139,   140,   141,   142,   143,   144,
     145,   147,   148,   149,   150,   151,   155,   158,   163,   164,
     165,   166,   167,   169,   170,   172,   173,   174,   175,   178,
     181,   187,   188,   190,   192,   193,   194,   197,   198,   200,
     201,   204,   205,   206,   207,   212,   215,   225,   226,   227,
     228,   229,   235,   244,   245,   256,   257,   261,   264,   271,
     277,   338,   339,   347,   348,   351,   352,   353,   354,   355,
     356,   357,   358,   360,   361,   362,   364,   367,   379,   380,
     387,   390,   393,   396,   399,   405,   407,   408,   410,   420,
     421,   422,   424,   429,   434,   454,   462,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     490,   492,   494,   121,   122,   123,   139,   163,   173,   198,
     215,   256,   338,   360,   466,   360,   198,   360,   360,   360,
     360,   109,   360,   360,   452,   453,   360,   360,   360,   360,
      81,    83,    92,   121,   141,   142,   143,   144,   145,   158,
     198,   226,   380,   421,   424,   429,   466,   470,   466,   360,
     360,   360,   360,   360,   360,   360,   360,    38,   360,   481,
     482,   121,   132,   198,   226,   269,   421,   422,   423,   425,
     429,   463,   464,   465,   474,   478,   479,   360,   198,   359,
     426,   198,   359,   371,   349,   360,   237,   359,   198,   198,
     198,   359,   200,   360,   215,   200,   360,     3,     4,     6,
       7,    10,    11,    12,    13,    28,    29,    31,    57,    68,
      71,    72,    73,    74,    75,    76,    77,    87,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   130,   132,   133,   134,   135,   139,   140,
     146,   163,   167,   175,   177,   180,   187,   198,   215,   216,
     217,   228,   495,   516,   517,   520,    27,   200,   354,   356,
     360,   201,   249,   360,   112,   113,   163,   166,   188,   218,
     219,   220,   221,   225,    83,   204,   304,   305,    83,   306,
     123,   132,   122,   132,   198,   198,   198,   198,   215,   275,
     498,   198,   198,    70,    70,    70,    70,    70,   349,    83,
      91,   159,   160,   161,   487,   488,   166,   201,   225,   225,
     215,   276,   498,   167,   198,   498,   498,    83,   194,   201,
     372,    28,   348,   351,   360,   362,   466,   471,   232,   201,
     476,    91,   427,   487,    91,   487,   487,    32,   166,   183,
     499,   198,     9,   200,   198,   347,   361,   467,   470,   118,
      38,   255,   167,   274,   498,   121,   193,   256,   339,    70,
     201,   461,   200,   200,   200,   200,   200,   200,   200,   200,
      10,    11,    12,    30,    31,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    57,    69,
     200,    70,    70,   201,   162,   133,   173,   175,   188,   190,
     277,   337,   338,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    59,    60,   136,   137,
     456,   461,   461,   198,   198,    70,   201,   198,   255,   256,
      14,   360,   200,   138,    49,   215,   451,    91,   348,   362,
     162,   466,   138,   203,     9,   436,   270,   348,   362,   466,
     499,   162,   198,   428,   456,   461,   199,   360,    32,   235,
       8,   373,     9,   200,   235,   236,   349,   350,   360,   215,
     289,   239,   200,   200,   200,   140,   146,   520,   520,   183,
     519,   198,   112,   520,    14,   162,   140,   146,   163,   215,
     217,   200,   200,   200,   250,   116,   180,   200,   218,   220,
     218,   220,   218,   220,   225,   218,   220,   201,     9,   437,
     200,   103,   166,   201,   466,     9,   200,    14,     9,   200,
     132,   132,   466,   491,   349,   348,   362,   466,   470,   471,
     199,   183,   267,   139,   466,   480,   481,   360,   381,   382,
     349,   402,   402,   381,   402,   200,    70,   456,   159,   488,
      82,   360,   466,    91,   159,   488,   225,   214,   200,   201,
     262,   272,   411,   413,    92,   198,   374,   375,   377,   420,
     424,   473,   475,   492,    14,   103,   493,   368,   369,   370,
     299,   300,   454,   455,   199,   199,   199,   199,   199,   202,
     234,   235,   257,   264,   271,   454,   360,   205,   206,   207,
     215,   500,   501,   520,    38,    87,   176,   302,   303,   360,
     495,   246,   247,   348,   356,   357,   360,   362,   466,   201,
     248,   248,   248,   248,   198,   498,   265,   255,   360,   477,
     360,   360,   360,   360,   360,    32,   360,   360,   360,   360,
     360,   360,   360,   360,   360,   360,   360,   360,   360,   360,
     360,   360,   360,   360,   360,   360,   360,   360,   360,   360,
     425,   360,   477,   477,   360,   483,   484,   132,   201,   216,
     217,   476,   275,   215,   276,   498,   498,   274,   256,    38,
     351,   354,   356,   360,   360,   360,   360,   360,   360,   360,
     360,   360,   360,   360,   360,   360,   167,   201,   215,   457,
     458,   459,   460,   476,   302,   302,   477,   360,   480,   255,
     199,   360,   198,   450,     9,   436,   199,   199,    38,   360,
      38,   360,   428,   199,   199,   199,   474,   475,   476,   302,
     201,   215,   457,   458,   476,   199,   232,   293,   201,   356,
     360,   360,    95,    32,   235,   287,   200,    27,   103,    14,
       9,   199,    32,   201,   290,   520,    31,    92,   176,   228,
     513,   514,   515,   198,     9,    50,    51,    56,    58,    70,
     140,   141,   142,   143,   144,   145,   187,   198,   226,   388,
     391,   394,   397,   400,   406,   421,   429,   430,   432,   433,
     215,   518,   232,   198,   243,   201,   200,   201,   200,   201,
     200,   103,   166,   201,   200,   112,   113,   166,   221,   222,
     223,   224,   225,   221,   215,   360,   305,   430,    83,     9,
     199,   199,   199,   199,   199,   199,   199,   200,    50,    51,
     509,   511,   512,   134,   280,   198,     9,   199,   199,   138,
     203,     9,   436,     9,   436,   203,   203,   203,   203,    83,
      85,   215,   489,   215,    70,   202,   202,   211,   213,    32,
     135,   279,   182,    54,   167,   182,   415,   362,   138,     9,
     436,   199,   162,   520,   520,    14,   373,   299,   230,   195,
       9,   437,    87,   520,   521,   456,   456,   202,     9,   436,
     184,   466,    83,    84,   301,   360,   199,     9,   437,    14,
       9,   199,     9,   199,   199,   199,   199,    14,   199,   202,
     233,   234,   365,   258,   134,   278,   198,   498,   203,   202,
     360,    32,   203,   203,   138,   202,     9,   436,   360,   499,
     198,   268,   263,   273,    14,   493,   266,   255,    71,   466,
     360,   499,   199,   199,   203,   202,   199,    50,    51,    70,
      78,    79,    80,    92,   140,   141,   142,   143,   144,   145,
     158,   187,   215,   389,   392,   395,   398,   401,   421,   432,
     439,   441,   442,   446,   449,   215,   466,   466,   138,   278,
     456,   461,   456,   199,   360,   294,    75,    76,   295,   230,
     359,   232,   350,   103,    38,   139,   284,   466,   430,   215,
      32,   235,   288,   200,   291,   200,   291,     9,   436,    92,
     228,   138,   162,     9,   436,   199,    87,   502,   503,   520,
     521,   500,   430,   430,   430,   430,   430,   435,   438,   198,
      70,    70,    70,    70,    70,   198,   430,   162,   201,    10,
      11,    12,    31,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    69,   162,   499,   202,   421,
     201,   252,   220,   220,   220,   215,   220,   221,   221,   225,
       9,   437,   202,   202,    14,   466,   200,   184,     9,   436,
     215,   281,   421,   201,   480,   139,   466,    14,   360,   360,
     203,   360,   202,   211,   520,   281,   201,   414,    14,   199,
     360,   374,   476,   200,   520,   195,   202,   231,   234,   244,
      32,   507,   455,   521,    38,    83,   176,   457,   458,   460,
     457,   458,   460,   520,    70,    38,    87,   176,   360,   430,
     247,   356,   357,   466,   248,   247,   248,   248,   202,   234,
     299,   198,   421,   279,   366,   259,   360,   360,   360,   202,
     198,   302,   280,    32,   279,   520,    14,   278,   498,   425,
     202,   198,    14,    78,    79,    80,   215,   440,   440,   442,
     444,   445,    52,   198,    70,    70,    70,    70,    70,    91,
     159,   198,   162,     9,   436,   199,   450,    38,   360,   279,
     202,    75,    76,   296,   359,   235,   202,   200,    96,   200,
     284,   466,   198,   138,   283,    14,   232,   291,   106,   107,
     108,   291,   202,   520,   184,   138,   162,   520,   215,   176,
     513,   520,     9,   436,   199,   176,   436,   138,   203,     9,
     436,   435,   383,   384,   430,   403,   430,   431,   403,   383,
     403,   374,   376,   378,   199,   132,   216,   430,   485,   486,
     430,   430,   430,    32,   430,   430,   430,   430,   430,   430,
     430,   430,   430,   430,   430,   430,   430,   430,   430,   430,
     430,   430,   430,   430,   430,   430,   430,   430,   518,    83,
     253,   202,   202,   202,   202,   224,   200,   430,   512,   103,
     104,   508,   510,     9,   310,   199,   198,   351,   356,   360,
     138,   203,   202,   493,   310,   168,   181,   201,   410,   417,
     168,   201,   416,   138,   200,   507,   198,   247,   347,   361,
     467,   470,   520,   373,    87,   521,    83,    83,   176,    14,
      83,   499,   499,   477,   466,   301,   360,   199,   299,   201,
     299,   198,   138,   198,   302,   199,   201,   520,   201,   200,
     520,   279,   260,   428,   302,   138,   203,     9,   436,   441,
     444,   385,   386,   442,   404,   442,   443,   404,   385,   404,
     159,   374,   447,   448,    81,   442,   466,   201,   359,    32,
      77,   235,   200,   350,   283,   480,   284,   199,   430,   102,
     106,   200,   360,    32,   200,   292,   202,   184,   520,   215,
     138,    87,   520,   521,    32,   199,   430,   430,   199,   203,
       9,   436,   138,   203,     9,   436,   203,   203,   203,   138,
       9,   436,   199,   138,   202,     9,   436,   430,    32,   199,
     232,   200,   200,   200,   200,   215,   520,   520,   508,   421,
       6,   113,   119,   122,   127,   169,   170,   172,   202,   311,
     336,   337,   338,   343,   344,   345,   346,   454,   480,   360,
     202,   201,   202,    54,   360,   360,   360,   373,   466,   200,
     201,   521,    38,    83,   176,    14,    83,   360,   198,   198,
     203,   507,   199,   310,   199,   299,   360,   302,   199,   310,
     493,   310,   200,   201,   198,   199,   442,   442,   199,   203,
       9,   436,   138,   203,     9,   436,   203,   203,   203,   138,
     199,     9,   436,   310,    32,   232,   200,   199,   199,   199,
     240,   200,   200,   292,   232,   138,   520,   520,   176,   520,
     138,   430,   430,   430,   430,   374,   430,   430,   430,   201,
     202,   510,   134,   135,   188,   216,   496,   520,   282,   421,
     113,   346,    31,   127,   140,   146,   167,   173,   320,   321,
     322,   323,   421,   171,   328,   329,   130,   198,   215,   330,
     331,   312,   256,   520,     9,   200,     9,   200,   200,   493,
     337,   199,   307,   167,   412,   202,   202,   360,    83,    83,
     176,    14,    83,   360,   302,   302,   119,   363,   507,   202,
     507,   199,   199,   202,   201,   202,   310,   299,   138,   442,
     442,   442,   442,   374,   202,   232,   238,   241,    32,   235,
     286,   232,   520,   199,   430,   138,   138,   138,   232,   421,
     421,   498,    14,   216,     9,   200,   201,   496,   493,   323,
     183,   201,     9,   200,     3,     4,     5,     6,     7,    10,
      11,    12,    13,    27,    28,    29,    57,    71,    72,    73,
      74,    75,    76,    77,    87,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   139,   140,   147,
     148,   149,   150,   151,   163,   164,   165,   175,   177,   178,
     180,   187,   188,   190,   192,   193,   215,   418,   419,     9,
     200,   167,   171,   215,   331,   332,   333,   200,    83,   342,
     255,   313,   496,   496,    14,   256,   202,   308,   309,   496,
      14,    83,   360,   199,   199,   198,   507,   197,   504,   363,
     507,   307,   202,   199,   442,   138,   138,    32,   235,   285,
     286,   232,   430,   430,   430,   202,   200,   200,   430,   421,
     316,   520,   324,   325,   429,   321,    14,    32,    51,   326,
     329,     9,    36,   199,    31,    50,    53,    14,     9,   200,
     217,   497,   342,    14,   520,   255,   200,    14,   360,    38,
      83,   409,   201,   505,   506,   520,   200,   201,   334,   507,
     504,   202,   507,   442,   442,   232,   100,   251,   202,   215,
     228,   317,   318,   319,     9,   436,     9,   436,   202,   430,
     419,   419,    68,   327,   332,   332,    31,    50,    53,   430,
      83,   183,   198,   200,   430,   497,   430,    83,     9,   437,
     230,     9,   437,    14,   508,   230,   201,   334,   334,    98,
     200,   116,   242,   162,   103,   520,   184,   429,   174,    14,
     509,   314,   198,    38,    83,   199,   202,   506,   520,   202,
     230,   200,   198,   180,   254,   215,   337,   338,   184,   430,
     184,   297,   298,   455,   315,    83,   202,   421,   252,   177,
     215,   200,   199,     9,   437,    87,   124,   125,   126,   340,
     341,   297,    83,   282,   200,   507,   455,   521,   521,   199,
     199,   200,   504,    87,   340,    83,    38,    83,   176,   507,
     201,   200,   201,   335,   521,   521,    83,   176,    14,    83,
     504,   232,   230,    83,    38,    83,   176,    14,    83,   360,
     335,   202,   202,    83,   176,    14,    83,   360,    14,    83,
     360,   360
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   208,   210,   209,   211,   211,   212,   212,   212,   212,
     212,   212,   212,   212,   213,   212,   214,   212,   212,   212,
     212,   212,   212,   212,   212,   212,   212,   212,   212,   215,
     215,   215,   215,   215,   215,   215,   215,   215,   215,   215,
     215,   215,   216,   216,   216,   216,   216,   216,   216,   216,
     216,   216,   216,   216,   216,   216,   216,   216,   216,   216,
     216,   216,   216,   216,   216,   216,   216,   216,   216,   216,
     216,   216,   216,   216,   216,   216,   216,   216,   216,   216,
     216,   216,   216,   216,   216,   216,   216,   216,   216,   216,
     216,   216,   216,   216,   216,   216,   216,   216,   216,   216,
     216,   216,   216,   216,   216,   216,   216,   216,   217,   217,
     218,   218,   219,   219,   220,   221,   221,   221,   221,   222,
     222,   223,   224,   224,   224,   225,   225,   226,   226,   226,
     227,   228,   229,   229,   230,   230,   231,   231,   232,   232,
     233,   233,   234,   234,   234,   234,   235,   235,   235,   236,
     235,   237,   235,   238,   235,   239,   235,   235,   235,   235,
     235,   235,   235,   235,   235,   235,   235,   235,   235,   235,
     235,   235,   240,   235,   241,   235,   235,   242,   235,   243,
     235,   235,   235,   235,   235,   235,   235,   235,   235,   235,
     235,   235,   235,   235,   235,   235,   235,   235,   235,   235,
     235,   235,   244,   245,   245,   246,   246,   247,   247,   247,
     248,   248,   250,   249,   251,   251,   253,   252,   254,   254,
     255,   255,   256,   258,   257,   259,   257,   260,   257,   262,
     261,   263,   261,   265,   264,   266,   264,   267,   264,   268,
     264,   270,   269,   272,   271,   273,   271,   274,   274,   275,
     276,   277,   277,   277,   277,   277,   278,   278,   279,   279,
     280,   280,   281,   281,   282,   282,   283,   283,   284,   284,
     284,   285,   285,   286,   286,   287,   287,   288,   288,   289,
     289,   290,   290,   290,   290,   291,   291,   291,   292,   292,
     293,   293,   294,   294,   295,   295,   296,   296,   297,   297,
     297,   297,   297,   297,   297,   297,   298,   298,   298,   298,
     298,   298,   298,   298,   298,   298,   299,   299,   299,   299,
     299,   299,   299,   299,   300,   300,   300,   300,   300,   300,
     300,   300,   300,   300,   301,   301,   301,   302,   302,   303,
     303,   303,   303,   303,   303,   303,   303,   304,   304,   305,
     305,   305,   306,   306,   306,   306,   307,   307,   308,   309,
     310,   310,   312,   311,   313,   311,   311,   311,   311,   314,
     311,   315,   311,   311,   311,   311,   311,   311,   311,   311,
     316,   316,   316,   317,   318,   318,   319,   319,   320,   320,
     321,   321,   322,   322,   323,   323,   323,   323,   323,   323,
     323,   324,   324,   325,   326,   326,   327,   327,   328,   328,
     329,   330,   330,   330,   331,   331,   331,   331,   332,   332,
     332,   332,   332,   332,   332,   333,   333,   333,   334,   334,
     335,   335,   336,   336,   337,   337,   338,   338,   339,   339,
     339,   339,   339,   339,   339,   340,   340,   341,   341,   341,
     342,   342,   342,   342,   343,   343,   344,   344,   345,   345,
     346,   347,   348,   348,   348,   348,   348,   348,   349,   349,
     350,   350,   351,   351,   351,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   360,   360,   360,   360,   361,
     362,   362,   362,   362,   362,   362,   362,   362,   362,   362,
     362,   362,   362,   362,   362,   362,   362,   362,   362,   362,
     362,   362,   362,   362,   362,   362,   362,   362,   362,   362,
     362,   362,   362,   362,   362,   362,   362,   362,   362,   362,
     362,   362,   362,   362,   362,   362,   362,   362,   362,   362,
     362,   362,   362,   362,   362,   362,   362,   362,   362,   362,
     362,   362,   362,   362,   362,   362,   362,   362,   362,   362,
     362,   362,   362,   362,   362,   362,   362,   363,   363,   365,
     364,   366,   364,   368,   367,   369,   367,   370,   367,   371,
     367,   372,   367,   373,   373,   373,   374,   374,   375,   375,
     376,   376,   377,   377,   378,   378,   379,   380,   380,   381,
     381,   382,   382,   383,   383,   384,   384,   385,   385,   386,
     386,   387,   388,   389,   390,   391,   392,   393,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   402,   403,   403,
     404,   404,   405,   406,   407,   407,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   409,   409,   409,
     409,   410,   411,   411,   412,   412,   413,   413,   414,   414,
     415,   416,   416,   417,   417,   417,   418,   418,   418,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   419,   419,   419,   419,   419,
     419,   419,   420,   421,   421,   422,   422,   422,   422,   422,
     423,   423,   424,   424,   424,   424,   425,   425,   425,   426,
     426,   426,   427,   427,   427,   428,   428,   429,   429,   429,
     429,   429,   429,   429,   429,   429,   429,   429,   429,   429,
     429,   429,   430,   430,   430,   430,   430,   430,   430,   430,
     430,   430,   430,   430,   430,   430,   430,   430,   430,   430,
     430,   430,   430,   430,   430,   430,   430,   430,   430,   430,
     430,   430,   430,   430,   430,   430,   430,   430,   430,   430,
     430,   430,   430,   430,   430,   430,   430,   431,   431,   432,
     433,   433,   434,   434,   434,   434,   434,   434,   434,   435,
     435,   436,   436,   437,   437,   438,   438,   438,   438,   439,
     439,   439,   439,   439,   440,   440,   440,   440,   441,   441,
     442,   442,   442,   442,   442,   442,   442,   442,   442,   442,
     442,   442,   442,   442,   442,   443,   443,   444,   444,   445,
     445,   445,   445,   446,   446,   447,   447,   448,   448,   449,
     449,   450,   450,   451,   451,   453,   452,   454,   455,   455,
     456,   456,   457,   457,   457,   458,   458,   459,   459,   460,
     460,   461,   461,   462,   462,   462,   463,   463,   464,   464,
     465,   465,   466,   466,   466,   466,   466,   466,   466,   466,
     466,   466,   466,   467,   468,   468,   468,   468,   468,   468,
     468,   468,   469,   469,   469,   469,   469,   469,   469,   469,
     469,   470,   471,   471,   472,   472,   472,   473,   473,   473,
     474,   475,   475,   475,   476,   476,   476,   476,   477,   477,
     478,   478,   478,   478,   478,   478,   479,   479,   479,   479,
     479,   480,   480,   480,   480,   480,   480,   481,   481,   482,
     482,   482,   482,   482,   482,   482,   482,   483,   483,   484,
     484,   484,   484,   485,   485,   486,   486,   486,   486,   487,
     487,   487,   487,   488,   488,   488,   488,   488,   488,   489,
     489,   489,   490,   490,   490,   490,   490,   490,   490,   490,
     490,   490,   490,   491,   491,   492,   492,   493,   493,   494,
     494,   494,   494,   495,   495,   496,   496,   497,   497,   498,
     498,   499,   499,   500,   500,   501,   502,   502,   502,   502,
     502,   502,   503,   503,   503,   503,   504,   504,   505,   505,
     506,   506,   507,   507,   508,   508,   509,   510,   510,   511,
     511,   511,   511,   512,   512,   512,   513,   513,   513,   513,
     514,   514,   515,   515,   515,   515,   516,   517,   518,   518,
     519,   519,   520,   520,   520,   520,   520,   520,   520,   520,
     520,   520,   520,   521,   521
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
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     3,     3,     1,     2,     1,     2,     3,     4,     3,
       1,     2,     1,     2,     2,     1,     3,     1,     3,     2,
       2,     2,     5,     4,     2,     0,     1,     3,     2,     0,
       2,     1,     1,     1,     1,     1,     3,     5,     8,     0,
       4,     0,     6,     0,    10,     0,     4,     2,     3,     2,
       3,     2,     3,     3,     3,     3,     3,     3,     5,     1,
       1,     1,     0,     9,     0,    10,     5,     0,    13,     0,
       5,     3,     3,     3,     3,     5,     5,     5,     3,     3,
       2,     2,     2,     2,     2,     2,     3,     2,     2,     3,
       2,     2,     2,     1,     0,     3,     3,     1,     1,     1,
       3,     2,     0,     4,     9,     0,     0,     4,     2,     0,
       1,     0,     1,     0,    10,     0,    11,     0,    11,     0,
       9,     0,    10,     0,     8,     0,     9,     0,     7,     0,
       8,     0,     8,     0,     7,     0,     8,     1,     1,     1,
       1,     1,     2,     3,     3,     2,     2,     0,     2,     0,
       2,     0,     1,     3,     1,     3,     2,     0,     1,     2,
       4,     1,     4,     1,     4,     1,     4,     1,     4,     3,
       5,     3,     4,     4,     5,     5,     4,     0,     1,     1,
       4,     0,     5,     0,     2,     0,     3,     0,     7,     8,
       6,     2,     5,     6,     4,     0,     4,     4,     5,     7,
       6,     6,     6,     7,     9,     8,     6,     7,     5,     2,
       4,     5,     3,     0,     3,     4,     4,     6,     5,     5,
       6,     6,     8,     7,     4,     1,     1,     2,     0,     1,
       2,     2,     2,     3,     4,     4,     4,     3,     1,     1,
       2,     4,     3,     5,     1,     3,     2,     0,     2,     3,
       2,     0,     0,     4,     0,     5,     2,     2,     2,     0,
      11,     0,    12,     3,     3,     3,     4,     4,     3,     5,
       2,     2,     0,     6,     5,     4,     3,     1,     1,     3,
       4,     1,     2,     1,     1,     5,     6,     1,     1,     4,
       1,     1,     3,     2,     2,     0,     2,     0,     1,     3,
       1,     1,     1,     1,     3,     4,     4,     4,     1,     1,
       2,     2,     2,     3,     3,     1,     1,     1,     1,     3,
       1,     3,     1,     1,     1,     0,     1,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     0,     1,     1,     1,
       3,     5,     1,     3,     5,     4,     3,     3,     3,     4,
       3,     3,     3,     3,     2,     2,     1,     1,     3,     1,
       1,     0,     1,     2,     4,     3,     3,     6,     2,     3,
       2,     3,     6,     3,     1,     1,     1,     1,     1,     3,
       6,     3,     4,     6,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     1,     5,     4,     3,     1,     2,     2,     2,
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
       1,     3,     3,     2,     2,     2,     2,     4,     5,     5,
       5,     5,     1,     1,     1,     1,     1,     1,     3,     3,
       4,     4,     3,     3,     1,     1,     1,     1,     3,     1,
       4,     3,     1,     1,     1,     1,     1,     3,     3,     4,
       4,     3,     1,     1,     7,     9,     9,     7,     6,     8,
       1,     4,     4,     1,     1,     1,     4,     2,     1,     0,
       1,     1,     1,     3,     3,     3,     0,     1,     1,     3,
       3,     2,     3,     6,     0,     1,     4,     2,     0,     5,
       3,     3,     1,     6,     4,     4,     2,     2,     0,     5,
       3,     3,     1,     2,     0,     5,     3,     3,     1,     2,
       2,     1,     2,     1,     4,     3,     3,     6,     3,     1,
       1,     1,     4,     4,     4,     4,     4,     4,     2,     2,
       4,     2,     2,     1,     3,     3,     3,     0,     2,     5,
       6,     6,     7,     1,     2,     1,     2,     1,     4,     1,
       4,     3,     0,     1,     3,     2,     1,     2,     4,     3,
       3,     1,     4,     2,     2,     0,     0,     3,     1,     3,
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
#line 757 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
#line 7284 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 760 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 7292 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 767 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 7298 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 768 "hphp.y" /* yacc.c:1646  */
    { }
#line 7304 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 771 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 7310 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 772 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7316 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 773 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7322 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 774 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7328 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 775 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7334 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 776 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 7340 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 7348 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 780 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 7355 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 782 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 7361 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 783 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7367 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 784 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 7373 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 785 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7379 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 786 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7387 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 790 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7396 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 795 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7405 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 800 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 803 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7419 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 806 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7427 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 810 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7435 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 814 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7443 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 818 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7451 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 822 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7459 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 825 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7466 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 830 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7472 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 831 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7478 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7484 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7490 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 834 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7496 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7502 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7508 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7514 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 838 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7520 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 839 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7526 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 840 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7532 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 841 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7538 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 842 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7544 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 923 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7550 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 925 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7556 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 930 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7562 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 931 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7569 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 937 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7575 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 941 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7581 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 942 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7587 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 944 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7593 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 946 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7599 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 951 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7605 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 952 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7612 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 958 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7618 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 962 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7625 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 964 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7632 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 966 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7639 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 971 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7645 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 973 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7651 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 976 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7657 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 978 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7663 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 979 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7669 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 984 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7678 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 991 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7687 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 999 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7694 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 1002 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7701 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1008 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 7707 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 1009 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval)); }
#line 7713 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 1014 "hphp.y" /* yacc.c:1646  */
    {
                                         _p->onUsing((yyval), (yyvsp[-2]), true, (yyvsp[-1]), nullptr);
                                       }
#line 7721 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1021 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7727 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1022 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7733 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1027 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 7739 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1028 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval), (yyval), (yyvsp[0])); }
#line 7746 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1033 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7752 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1034 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1035 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7764 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1036 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7770 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1039 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1043 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1048 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7788 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1049 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7795 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1051 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7803 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1055 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7810 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1058 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7818 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1062 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7825 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1064 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7833 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1067 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7840 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1069 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7848 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1072 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7854 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1073 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7860 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1074 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7866 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1075 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7872 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1076 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7878 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1077 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7884 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1078 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7890 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1079 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7896 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1080 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7902 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1081 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7908 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1082 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7914 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1083 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 7920 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1084 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 7926 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1085 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 7932 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1086 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 7939 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1090 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7946 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1092 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 7954 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1097 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7961 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1099 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 7969 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1103 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 7977 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1112 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1113 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 7989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1116 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false); }
#line 7995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1117 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 8001 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1119 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8009 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1123 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8017 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1127 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1131 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8033 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1135 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8041 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1139 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8049 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1144 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8057 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1147 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 8063 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1148 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 8072 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1152 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8078 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1153 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8084 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1154 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8090 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1155 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8096 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1156 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8102 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1157 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8108 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1158 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 8114 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1159 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8120 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1160 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8126 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1161 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 8132 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1162 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8138 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1163 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 8148 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1185 "hphp.y" /* yacc.c:1646  */
    { _p->pushLabelScope(LS::Using);
                                         _p->onNewLabelScope(false);
                                         (yyval) = (yyvsp[-1]); }
#line 8156 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1191 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1; }
#line 8162 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1192 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8168 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1201 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), nullptr, (yyvsp[-2]));
                                         _p->onExprListElem((yyval), &(yyval), (yyvsp[0])); }
#line 8175 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 1203 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0])); }
#line 8181 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 1213 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8187 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1214 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8193 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1218 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false); }
#line 8199 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1219 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8205 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1228 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 8211 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1229 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8217 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1233 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::Finally);}
#line 8224 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1235 "hphp.y" /* yacc.c:1646  */
    { _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->popLabelScope();
                                         _p->onCompleteLabelScope(false);}
#line 8232 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1241 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8238 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1242 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8244 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1246 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 8250 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1247 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8256 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1251 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 8262 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1257 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8271 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1264 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8281 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1272 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8290 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1279 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8300 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1287 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8309 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1293 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8319 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1302 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8326 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1306 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 8332 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1310 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8339 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1314 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 8345 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1320 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8352 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1323 "hphp.y" /* yacc.c:1646  */
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
#line 8370 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 1338 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8377 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1341 "hphp.y" /* yacc.c:1646  */
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
#line 8395 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 1355 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8402 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1358 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8410 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1363 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8417 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1366 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8425 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1372 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 8431 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1375 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 8437 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1379 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8444 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1382 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8455 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1390 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8462 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1393 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8473 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1401 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8479 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1402 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 8486 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1406 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8492 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1409 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8498 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1412 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 8504 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1413 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 8510 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1414 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 8518 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1417 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 8524 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1418 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 8530 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1422 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8536 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1423 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8542 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1426 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8548 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1427 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8554 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1430 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8560 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1431 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8566 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1434 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 8572 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1436 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8578 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1439 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 8584 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1441 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8590 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1445 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8596 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1446 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8602 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1449 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8608 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1450 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8614 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1451 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8620 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1455 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8626 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1457 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8632 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1460 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8638 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1462 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8644 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1465 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8650 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1467 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8656 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1470 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8662 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1472 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8668 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1476 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8674 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1478 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8681 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8687 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1484 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8693 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1485 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8699 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1486 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8705 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1491 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8711 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1493 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8717 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1494 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8723 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1497 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8729 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1498 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8735 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1503 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8741 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1504 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8747 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1509 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8753 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1510 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8759 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1513 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8765 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1514 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8771 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1517 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8777 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1518 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8783 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1526 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8790 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1532 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8797 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1538 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8805 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1542 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8811 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1546 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8818 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1551 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8825 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1556 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8833 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1559 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8839 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1565 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8847 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1570 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8855 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1575 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8863 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1581 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8871 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1587 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8879 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1593 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8887 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1599 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8895 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1605 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8903 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1612 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1619 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8919 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1628 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 8926 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1633 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 8933 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1638 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 8941 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1642 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1645 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 8954 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1649 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 8961 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1653 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 8969 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1656 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8975 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1661 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1665 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8991 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1669 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8999 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1674 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9007 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1679 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9015 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1684 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 9023 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1689 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1694 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9039 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1700 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9047 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1706 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9055 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1712 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 9061 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1713 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 9067 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1714 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 9073 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1719 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9079 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1720 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9085 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1723 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,false);}
#line 9092 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1725 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::InOut,false);}
#line 9099 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1727 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::Ref,false);}
#line 9106 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1729 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,true);}
#line 9113 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1732 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::In,false);}
#line 9120 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1735 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::In,true);}
#line 9127 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1738 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::Ref,false);}
#line 9134 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1741 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::InOut,false);}
#line 9141 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1746 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9147 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1747 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 9153 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1750 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9159 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1751 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 9165 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1752 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 9171 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1756 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9177 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1758 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9183 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1759 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 9189 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1760 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9195 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1765 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9201 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1766 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9207 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1769 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9214 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 359:
#line 1774 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9220 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1780 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9226 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1781 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9232 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1784 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[0]));}
#line 9238 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1785 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 9245 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1788 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[-1]));}
#line 9251 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1789 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-3]));}
#line 9258 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1791 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9265 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1794 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL, true);}
#line 9272 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1796 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9278 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1799 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9286 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1806 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9296 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1814 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9304 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1821 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9314 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1827 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 9320 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1829 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9326 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1831 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9332 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1833 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 9338 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1835 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 9344 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1836 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 9351 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1839 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 9357 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1842 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9363 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1843 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9369 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1844 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 9375 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 1850 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 9381 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1855 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 9388 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1858 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 9396 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1865 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 9402 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1866 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 9409 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1871 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 9416 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1874 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 9422 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1881 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 9429 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1883 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 9435 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 1887 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9441 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 394:
#line 1892 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9447 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1894 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9453 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1896 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9459 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1897 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 9470 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1903 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 9476 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1905 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 9482 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1906 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 9488 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1910 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,0,(yyvsp[0]),0);}
#line 9494 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1912 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),0,(yyvsp[0]),0);}
#line 9500 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1917 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9506 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1920 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9512 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1921 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 9518 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1925 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 9524 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1926 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 9530 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1930 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 9537 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1933 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 9544 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1938 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 9551 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1943 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 9557 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1944 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 9564 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1946 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 9570 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1950 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 9576 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1951 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 9582 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1952 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 9588 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1953 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 9594 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1957 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9600 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1958 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 9606 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1959 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 9612 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1960 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 9618 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1961 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 9624 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 9630 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1965 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 9636 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1969 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 9644 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1972 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 9650 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1973 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 9656 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1977 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9662 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1978 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9668 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1982 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9674 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1983 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9680 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1986 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9686 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1987 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9692 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 1990 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9698 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 1991 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9704 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 1994 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9710 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 1996 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9716 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 1999 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9722 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9728 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 2001 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9734 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 2002 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9740 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 2003 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9746 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 2004 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9752 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 2005 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 2009 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9764 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 2010 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9770 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 2013 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 2014 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 2015 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9788 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 2019 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9794 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 2021 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9800 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 2022 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9806 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 2023 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9812 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 2027 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9818 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 2029 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9824 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 2033 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9830 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 2035 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9836 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 2039 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9844 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 2043 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9851 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 2047 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9857 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 2051 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9863 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 2055 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9869 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 2057 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 2058 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9881 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 2059 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9887 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 2060 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9893 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 2061 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9899 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 2064 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 2065 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 2069 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9917 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 2070 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9923 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 2074 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 9929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 2075 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 9935 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 2076 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 9941 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 2077 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 2081 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9953 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 2086 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 2090 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 9965 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 2094 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 2098 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 9977 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 2102 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 2107 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 2111 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 2115 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10001 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 2116 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10007 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 2117 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10013 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 2118 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 2119 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 2123 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 2128 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 10037 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 2129 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 10043 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 2130 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 10049 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 2133 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 10055 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 2134 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 10061 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 2135 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 10067 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 2136 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 10073 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 2137 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 10079 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 2138 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 10085 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 2139 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 10091 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 2140 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 10097 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 2141 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 10103 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 2142 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 10109 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2143 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 10115 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2144 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 10121 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2145 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 10127 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2146 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 10133 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2147 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 10139 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2148 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 10145 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2149 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 10151 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2150 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 10157 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2151 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 10163 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2152 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 10169 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2153 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 10175 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2154 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 10181 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2155 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 10187 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2156 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 10193 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2157 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 10199 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2158 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 10205 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2159 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 10211 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2160 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 10217 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2161 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 10223 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2162 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 10229 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2163 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 10235 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2164 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 10241 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2165 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 10247 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2166 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 10253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2167 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 10259 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2168 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 10265 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2169 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 10271 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2170 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 10277 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2171 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 10283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2172 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 10289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2173 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 10295 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2174 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 10301 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2175 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 10307 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2176 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 10313 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2177 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 10320 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2179 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 10326 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2180 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 10333 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2182 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 10339 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2184 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 10345 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2185 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10351 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2186 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 10357 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2187 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 10363 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2188 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 10369 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2189 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10375 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2190 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 10381 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2191 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 10387 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2192 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 10393 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2193 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 10399 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2194 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 10405 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2195 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 10411 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2196 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 10417 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2197 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 10423 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2198 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 10429 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2199 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10435 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2200 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10441 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2201 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10447 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2202 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10453 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2203 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10459 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2204 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10465 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2205 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10471 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2206 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10477 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2207 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 10483 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2208 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 10489 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2209 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10495 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2216 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 10501 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2217 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10507 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2222 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10516 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 570:
#line 2228 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10528 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2237 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10537 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 572:
#line 2243 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[-12]),
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10549 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2254 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,NULL,NULL);}
#line 10563 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2263 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         (yyvsp[-3]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[-3]), nullptr, (yyvsp[-3]));
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[-3]),
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10578 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2274 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10588 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
#line 2282 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         (yyvsp[-6]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[-6]), nullptr, (yyvsp[-6]));
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[-6]),
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10603 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2293 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10613 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
#line 2299 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v; Token w; Token x;
                                         Token y;
                                         (yyvsp[-4]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[-4]), nullptr, (yyvsp[-4]));
                                         _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[-4]),
                                                            u,v,w,(yyvsp[-1]),x);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);
                                         _p->onCall((yyval),1,(yyval),y,NULL);}
#line 10630 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
#line 2311 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,NULL,NULL);}
#line 10644 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
#line 2320 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10657 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2328 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10667 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
#line 2336 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10680 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2347 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10686 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2348 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10692 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2350 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10698 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2354 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10705 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2356 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10711 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2363 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10717 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2366 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10723 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2373 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10729 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2376 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10735 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2381 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10741 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2382 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10747 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2387 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10753 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2388 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10759 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2392 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval), (yyvsp[-1]), T_ARRAY);}
#line 10765 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2396 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10771 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2397 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10777 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2402 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10783 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2403 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10789 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2408 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10795 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2409 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10801 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2414 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10807 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2415 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10813 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2421 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10819 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2423 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10825 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2428 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10831 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2429 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10837 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2435 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10843 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2437 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10849 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2441 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10855 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2445 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10861 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2449 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10867 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2453 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10873 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2457 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10879 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2461 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10885 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2465 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10891 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2469 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10897 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2473 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10903 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2477 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10909 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2481 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10915 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2485 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10921 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2489 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10927 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2493 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10933 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2497 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10939 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2502 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10945 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2503 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10951 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2508 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10957 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2509 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10963 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2514 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10969 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2515 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10975 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2520 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2527 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10991 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2534 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10997 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2536 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 11003 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2540 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11009 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2541 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11015 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2542 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11021 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2543 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11027 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2544 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11033 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2545 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11039 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2546 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11045 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2547 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11051 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2548 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 11058 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2550 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11064 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2551 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11070 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2555 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11076 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2556 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 11082 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2557 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 11088 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2558 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 11094 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2565 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 11100 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2568 "hphp.y" /* yacc.c:1646  */
    { Token t1; _p->onArray(t1,(yyvsp[-1]));
                                         Token t2; _p->onArray(t2,(yyvsp[0]));
                                         Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onCallParam((yyvsp[-1]),NULL,t1,
                                                         ParamMode::In,0);
                                         _p->onCallParam((yyval), &(yyvsp[-1]),t2,
                                                         ParamMode::In,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),file,
                                                         ParamMode::In,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),line,
                                                         ParamMode::In,0);
                                         (yyval).setText("");}
#line 11118 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 653:
#line 2583 "hphp.y" /* yacc.c:1646  */
    { Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onArray((yyvsp[-2]),(yyvsp[-5]));
                                         _p->onArray((yyvsp[-1]),(yyvsp[-3]));
                                         _p->onCallParam((yyvsp[-4]),NULL,(yyvsp[-2]),
                                                         ParamMode::In,0);
                                         _p->onCallParam((yyval), &(yyvsp[-4]),(yyvsp[-1]),
                                                         ParamMode::In,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),file,
                                                         ParamMode::In,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),line,
                                                         ParamMode::In,0);
                                         (yyval).setText((yyvsp[0]).text());}
#line 11136 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2598 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 11142 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2599 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 11148 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2604 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11154 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2605 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11160 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2608 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-1]),0,(yyvsp[0]),0);}
#line 11166 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2609 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11172 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2612 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11179 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2616 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11187 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2619 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11193 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2622 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         if ((yyvsp[0]).htmlTrim()) {
                                           (yyvsp[0]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));
                                         }
                                       }
#line 11205 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2629 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11211 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2630 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11217 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2634 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11223 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2636 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 11229 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2638 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 11235 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2642 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11241 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2643 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11247 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2644 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2645 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11259 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2646 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11265 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2647 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11271 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2648 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11277 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2649 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2650 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2651 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11295 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2652 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11301 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2653 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11307 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2654 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11313 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2655 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11319 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2656 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11325 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2657 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2658 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11337 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2659 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11343 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2660 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11349 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2661 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11355 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2662 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11361 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2663 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11367 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2664 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11373 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2665 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11379 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2666 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11385 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2667 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11391 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2668 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11397 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2669 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11403 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2670 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11409 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2671 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11415 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2672 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11421 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2673 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11427 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2674 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11433 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2675 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11439 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2676 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11445 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2677 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11451 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2678 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11457 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2679 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11463 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2680 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11469 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2681 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11475 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2682 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11481 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2683 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11487 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2684 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11493 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2685 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11499 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2686 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11505 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2687 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11511 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2688 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11517 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2689 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11523 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2690 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11529 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2691 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11535 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2692 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11541 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2693 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11547 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2694 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11553 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2695 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11559 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2696 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11565 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2697 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11571 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2698 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11577 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2699 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11583 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2700 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11589 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2701 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11595 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2702 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11601 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2703 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11607 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2704 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11613 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2705 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11619 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2706 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11625 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2707 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11631 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2708 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11637 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2709 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11643 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2710 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11649 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2711 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11655 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2712 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11661 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2713 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11667 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2714 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11673 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2715 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11679 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2716 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11685 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2717 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11691 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2718 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11697 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2719 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11703 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2720 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11709 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2721 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11715 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2722 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11721 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2723 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11727 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2724 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11733 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2729 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11739 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2733 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11745 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2734 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11751 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2738 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11757 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2739 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11763 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2740 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11769 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2741 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2743 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11783 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2747 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11789 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2756 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11795 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2759 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11801 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2760 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11808 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2762 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11815 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2772 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11821 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2776 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11827 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2777 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11833 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2778 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11839 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2782 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11845 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2783 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11851 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2784 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11857 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2788 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11863 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2789 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11869 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2790 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2794 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11881 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2795 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11887 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2799 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11893 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2800 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11899 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2801 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2802 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 11912 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2804 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 11918 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2805 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 11924 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2806 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 11930 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2807 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 11936 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2808 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 11942 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2809 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 11948 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2810 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 11954 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2811 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 11960 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2812 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 11966 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2815 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11972 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2817 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11978 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2821 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11984 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2822 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11990 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2824 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11996 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2825 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12002 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2827 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12008 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2828 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12014 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2829 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12020 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2830 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12026 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2831 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12032 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12038 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12044 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2834 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12050 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12056 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2837 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 12062 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2839 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 12068 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2841 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 12074 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2843 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 12080 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2845 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 12086 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2846 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 12092 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2847 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 12098 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2848 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 12104 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2849 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 12110 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2850 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 12116 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2851 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 12122 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2852 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 12128 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2853 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 12134 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2854 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 12140 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2855 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 12146 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2856 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 12152 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2857 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 12158 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2858 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 12164 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2859 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 12170 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2860 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12176 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2861 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12182 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2863 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 12188 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2865 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 12194 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2867 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 12200 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2869 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 12206 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2870 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 12212 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2872 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 12219 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 832:
#line 2874 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 12225 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2877 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 12232 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2881 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 12238 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2884 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 12244 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2885 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 12250 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2889 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12256 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2890 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12262 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2896 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12268 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2902 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12274 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2903 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12280 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2907 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12286 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2908 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12292 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2909 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12298 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2910 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12304 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2911 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 12310 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2912 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 12316 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2914 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 12323 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2919 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12329 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2920 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12335 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2924 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12341 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2925 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12347 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2928 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 12353 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2929 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12359 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2935 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12365 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2937 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12371 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2939 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12377 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2940 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12383 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 2944 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 12389 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2945 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 12395 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2946 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 12401 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2949 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12407 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2951 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12413 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2954 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 12419 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2955 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 12425 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2956 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 12431 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2957 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12437 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2961 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 12444 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2964 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 12452 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2971 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12458 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2972 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12464 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2975 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 12472 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2978 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12478 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2979 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12484 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 2980 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12490 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 877:
#line 2982 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12496 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 2983 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12502 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 2985 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12508 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 2986 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12514 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 2987 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12520 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 2988 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12526 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 2989 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12532 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 2990 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12538 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 2995 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12544 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
#line 2996 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12550 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 3001 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12556 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 3002 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12562 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 3007 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12568 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 3009 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12574 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 3011 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12580 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 3012 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12586 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 3016 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12592 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 3017 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12598 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 3022 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12604 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 3023 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12610 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 3028 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12616 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 3031 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12622 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 3036 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12628 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 3037 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12634 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3040 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12640 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3041 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 12647 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3048 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 12653 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3050 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 12659 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3053 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 12665 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3055 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12671 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3058 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12677 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3061 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12683 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3062 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12689 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3066 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 12695 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3067 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 12701 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3071 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12707 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3072 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 12713 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3073 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 12719 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3077 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12725 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3082 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12731 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3087 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12737 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3088 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12743 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3092 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12749 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3097 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12755 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3102 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12761 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3103 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12767 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3108 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12773 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3109 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12779 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3111 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12785 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3116 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12791 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3118 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12797 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3124 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12811 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3135 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12825 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3150 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12839 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3162 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12853 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3174 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12859 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3175 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12865 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3176 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12871 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3177 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12877 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3178 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12883 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
#line 3179 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12889 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
#line 3181 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12903 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3198 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12909 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
#line 3200 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12915 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
#line 3202 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12921 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3203 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12927 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3207 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12933 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
#line 3211 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12939 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3212 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12945 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
#line 3213 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12951 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
#line 3214 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12957 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
#line 3222 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3231 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12977 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3233 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3237 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3242 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3243 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13001 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3244 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13007 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3245 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13013 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3246 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3247 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3248 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3250 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13037 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3252 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13043 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3256 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13049 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3260 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13055 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3261 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13061 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3267 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13067 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3271 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13073 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3275 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13079 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3282 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 13085 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3291 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 13091 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3295 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 13097 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3299 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13103 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3308 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13109 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3309 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13115 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3310 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13121 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3314 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13127 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3315 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 13133 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3316 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 13139 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3318 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]) = 1; _p->onIndirectRef((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 13145 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3323 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13151 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3324 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13157 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3335 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13163 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3336 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13169 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3337 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13175 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3340 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13189 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3351 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13195 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3352 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13201 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3356 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13207 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3357 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13213 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3360 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13227 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3369 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13233 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3373 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 13239 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3374 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 13245 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3376 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 13251 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3377 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 13257 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 995:
#line 3378 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 13263 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3379 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 13269 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3384 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13275 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
#line 3385 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13281 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3389 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13287 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3390 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13293 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3391 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13299 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3392 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13305 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3395 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13311 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1004:
#line 3397 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 13317 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1005:
#line 3398 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13323 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1006:
#line 3399 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 13329 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1007:
#line 3404 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13335 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1008:
#line 3405 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13341 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3409 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13347 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3410 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13353 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3411 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13359 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3412 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13365 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3417 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13371 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3418 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13377 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3423 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13383 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3425 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13389 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3427 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13395 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3428 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13401 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3432 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 13407 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3434 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 13413 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3435 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 13419 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3437 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 13426 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3442 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13432 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3444 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13438 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3446 "hphp.y" /* yacc.c:1646  */
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[-2]),
                                           !(yyvsp[-1]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[0])
                                         );
                                       }
#line 13452 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3456 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 13458 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1027:
#line 3458 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 13464 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1028:
#line 3459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13470 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1029:
#line 3462 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 13476 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1030:
#line 3463 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 13482 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1031:
#line 3464 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 13488 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1032:
#line 3468 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 13494 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1033:
#line 3469 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 13500 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1034:
#line 3470 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13506 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1035:
#line 3471 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13512 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1036:
#line 3472 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13518 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1037:
#line 3473 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13524 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1038:
#line 3474 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 13530 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1039:
#line 3475 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 13536 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1040:
#line 3476 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 13542 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1041:
#line 3477 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 13548 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1042:
#line 3478 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 13554 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3482 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 13560 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3483 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 13566 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3488 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13572 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3490 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13578 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3504 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13586 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3509 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 13594 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3513 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13602 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3518 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 13610 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3524 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13616 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3525 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13622 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1055:
#line 3529 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13628 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1056:
#line 3530 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13634 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1057:
#line 3536 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13640 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1058:
#line 3540 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 13646 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1059:
#line 3546 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13652 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1060:
#line 3550 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 13659 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1061:
#line 3557 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13665 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1062:
#line 3558 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13671 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3562 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 13679 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3565 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 13686 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3571 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13692 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3575 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13700 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3578 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13708 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3581 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-3]); }
#line 13715 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1069:
#line 3583 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13722 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3585 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13729 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3587 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13735 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3592 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-3]); }
#line 13741 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3593 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13747 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3594 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13753 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3595 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13759 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1082:
#line 3616 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13765 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1083:
#line 3617 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13771 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1086:
#line 3626 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13777 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1089:
#line 3637 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13783 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1090:
#line 3639 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13789 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1091:
#line 3643 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13795 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1092:
#line 3646 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13801 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1093:
#line 3650 "hphp.y" /* yacc.c:1646  */
    {}
#line 13807 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1094:
#line 3651 "hphp.y" /* yacc.c:1646  */
    {}
#line 13813 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1095:
#line 3652 "hphp.y" /* yacc.c:1646  */
    {}
#line 13819 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1096:
#line 3658 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13826 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1097:
#line 3663 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13836 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1098:
#line 3672 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13842 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1099:
#line 3678 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13851 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1100:
#line 3686 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 13857 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1101:
#line 3687 "hphp.y" /* yacc.c:1646  */
    { }
#line 13863 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1102:
#line 3693 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 13869 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1103:
#line 3695 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 13875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1104:
#line 3696 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 13885 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1105:
#line 3701 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 13892 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1106:
#line 3707 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("array"); }
#line 13899 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1107:
#line 3712 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1108:
#line 3717 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 13913 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1109:
#line 3721 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13919 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1110:
#line 3726 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 13925 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1111:
#line 3728 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 13931 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1112:
#line 3734 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 13938 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1113:
#line 3736 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 13946 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1114:
#line 3739 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13952 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1115:
#line 3740 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13960 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1116:
#line 3743 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13968 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1117:
#line 3746 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13974 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1118:
#line 3749 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 13982 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1119:
#line 3752 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1120:
#line 3754 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 13998 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1121:
#line 3760 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 14007 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1122:
#line 3766 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 14017 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1123:
#line 3774 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14023 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1124:
#line 3775 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 14029 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;


#line 14033 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 3778 "hphp.y" /* yacc.c:1906  */

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}
