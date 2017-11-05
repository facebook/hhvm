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
#define yyparse         Compiler5parse
#define yylex           Compiler5lex
#define yyerror         Compiler5error
#define yydebug         Compiler5debug
#define yynerrs         Compiler5nerrs


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

#line 660 "hphp.5.tab.cpp" /* yacc.c:339  */

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
   by #include "hphp.5.tab.hpp".  */
#ifndef YY_YY_HPHP_Y_INCLUDED
# define YY_YY_HPHP_Y_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int Compiler5debug;
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
    T_TUPLE = 424,
    T_TYPE = 425,
    T_UNRESOLVED_TYPE = 426,
    T_NEWTYPE = 427,
    T_UNRESOLVED_NEWTYPE = 428,
    T_COMPILER_HALT_OFFSET = 429,
    T_ASYNC = 430,
    T_LAMBDA_OP = 431,
    T_LAMBDA_CP = 432,
    T_UNRESOLVED_OP = 433,
    T_WHERE = 434
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



int Compiler5parse (HPHP::HPHP_PARSER_NS::Parser *_p);

#endif /* !YY_YY_HPHP_Y_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 904 "hphp.5.tab.cpp" /* yacc.c:358  */

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
#define YYLAST   20298

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  209
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  318
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1133
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  2110

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   434

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    56,   207,     2,   205,    55,    38,   208,
     199,   200,    53,    50,     9,    51,    52,    54,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    32,   201,
      43,    14,    45,    31,    68,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    70,     2,   204,    37,     2,   206,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   202,    36,   203,    58,     2,     2,     2,
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
     194,   195,   196,   197,   198
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   758,   758,   758,   767,   769,   772,   773,   774,   775,
     776,   777,   778,   781,   783,   783,   785,   785,   787,   790,
     795,   800,   803,   806,   810,   814,   818,   822,   826,   831,
     832,   833,   834,   835,   836,   837,   838,   839,   840,   841,
     842,   843,   847,   848,   849,   850,   851,   852,   853,   854,
     855,   856,   857,   858,   859,   860,   861,   862,   863,   864,
     865,   866,   867,   868,   869,   870,   871,   872,   873,   874,
     875,   876,   877,   878,   879,   880,   881,   882,   883,   884,
     885,   886,   887,   888,   889,   890,   891,   892,   893,   894,
     895,   896,   897,   898,   899,   900,   901,   902,   903,   904,
     905,   906,   907,   908,   909,   910,   911,   915,   916,   920,
     921,   925,   926,   931,   933,   938,   943,   944,   945,   947,
     952,   954,   959,   964,   966,   968,   973,   974,   978,   979,
     981,   985,   992,   999,  1003,  1009,  1011,  1015,  1016,  1022,
    1024,  1028,  1030,  1035,  1036,  1037,  1038,  1041,  1042,  1046,
    1051,  1051,  1057,  1057,  1064,  1063,  1069,  1069,  1074,  1075,
    1076,  1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,
    1086,  1087,  1088,  1092,  1090,  1099,  1097,  1104,  1114,  1108,
    1118,  1116,  1120,  1124,  1128,  1132,  1136,  1140,  1144,  1149,
    1150,  1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,
    1163,  1164,  1165,  1187,  1193,  1194,  1203,  1205,  1209,  1210,
    1211,  1215,  1216,  1220,  1220,  1225,  1231,  1235,  1235,  1243,
    1244,  1248,  1249,  1253,  1259,  1257,  1274,  1271,  1289,  1286,
    1304,  1303,  1312,  1310,  1322,  1321,  1340,  1338,  1357,  1356,
    1365,  1363,  1374,  1374,  1381,  1380,  1392,  1390,  1403,  1404,
    1408,  1411,  1414,  1415,  1416,  1419,  1420,  1423,  1425,  1428,
    1429,  1432,  1433,  1436,  1437,  1441,  1442,  1447,  1448,  1451,
    1452,  1453,  1457,  1458,  1462,  1463,  1467,  1468,  1472,  1473,
    1478,  1479,  1485,  1486,  1487,  1488,  1491,  1494,  1496,  1499,
    1500,  1504,  1506,  1509,  1512,  1515,  1516,  1519,  1520,  1524,
    1530,  1536,  1543,  1545,  1550,  1555,  1561,  1565,  1570,  1575,
    1580,  1586,  1592,  1598,  1604,  1610,  1617,  1627,  1632,  1637,
    1643,  1645,  1649,  1653,  1658,  1662,  1666,  1670,  1674,  1679,
    1684,  1689,  1694,  1699,  1705,  1714,  1715,  1716,  1720,  1722,
    1725,  1727,  1729,  1731,  1733,  1736,  1739,  1742,  1748,  1749,
    1752,  1753,  1754,  1758,  1759,  1761,  1762,  1766,  1768,  1771,
    1775,  1781,  1783,  1786,  1786,  1790,  1789,  1793,  1795,  1798,
    1801,  1799,  1816,  1813,  1828,  1830,  1832,  1834,  1836,  1838,
    1840,  1844,  1845,  1846,  1849,  1855,  1859,  1865,  1868,  1873,
    1875,  1880,  1885,  1889,  1890,  1894,  1895,  1897,  1899,  1905,
    1906,  1908,  1912,  1913,  1918,  1922,  1923,  1927,  1928,  1932,
    1934,  1940,  1945,  1946,  1948,  1952,  1953,  1954,  1955,  1959,
    1960,  1961,  1962,  1963,  1964,  1966,  1971,  1974,  1975,  1979,
    1980,  1984,  1985,  1988,  1989,  1992,  1993,  1996,  1997,  2001,
    2002,  2003,  2004,  2005,  2006,  2007,  2011,  2012,  2015,  2016,
    2017,  2020,  2022,  2024,  2025,  2028,  2030,  2034,  2036,  2040,
    2044,  2048,  2053,  2057,  2058,  2060,  2061,  2062,  2063,  2066,
    2067,  2071,  2072,  2076,  2077,  2078,  2079,  2083,  2087,  2092,
    2096,  2100,  2104,  2108,  2113,  2117,  2118,  2119,  2120,  2121,
    2125,  2129,  2131,  2132,  2133,  2136,  2137,  2138,  2139,  2140,
    2141,  2142,  2143,  2144,  2145,  2146,  2147,  2148,  2149,  2150,
    2151,  2152,  2153,  2154,  2155,  2156,  2157,  2158,  2159,  2160,
    2161,  2162,  2163,  2164,  2165,  2166,  2167,  2168,  2169,  2170,
    2171,  2172,  2173,  2174,  2175,  2176,  2177,  2178,  2179,  2181,
    2182,  2184,  2185,  2187,  2188,  2189,  2190,  2191,  2192,  2193,
    2194,  2195,  2196,  2197,  2198,  2199,  2200,  2201,  2202,  2203,
    2204,  2205,  2206,  2207,  2208,  2209,  2210,  2211,  2212,  2216,
    2220,  2225,  2224,  2240,  2238,  2257,  2256,  2277,  2276,  2296,
    2295,  2314,  2314,  2331,  2331,  2350,  2351,  2352,  2357,  2359,
    2363,  2367,  2373,  2377,  2383,  2385,  2389,  2391,  2395,  2399,
    2400,  2404,  2406,  2410,  2412,  2416,  2418,  2422,  2425,  2430,
    2432,  2436,  2439,  2444,  2448,  2452,  2456,  2460,  2464,  2468,
    2472,  2476,  2480,  2484,  2488,  2492,  2496,  2500,  2504,  2508,
    2512,  2516,  2518,  2522,  2524,  2528,  2530,  2534,  2541,  2548,
    2550,  2555,  2556,  2557,  2558,  2559,  2560,  2561,  2562,  2563,
    2564,  2566,  2567,  2571,  2572,  2573,  2574,  2578,  2584,  2597,
    2614,  2615,  2618,  2621,  2624,  2625,  2628,  2632,  2635,  2638,
    2645,  2646,  2650,  2651,  2653,  2658,  2659,  2660,  2661,  2662,
    2663,  2664,  2665,  2666,  2667,  2668,  2669,  2670,  2671,  2672,
    2673,  2674,  2675,  2676,  2677,  2678,  2679,  2680,  2681,  2682,
    2683,  2684,  2685,  2686,  2687,  2688,  2689,  2690,  2691,  2692,
    2693,  2694,  2695,  2696,  2697,  2698,  2699,  2700,  2701,  2702,
    2703,  2704,  2705,  2706,  2707,  2708,  2709,  2710,  2711,  2712,
    2713,  2714,  2715,  2716,  2717,  2718,  2719,  2720,  2721,  2722,
    2723,  2724,  2725,  2726,  2727,  2728,  2729,  2730,  2731,  2732,
    2733,  2734,  2735,  2736,  2737,  2738,  2739,  2740,  2744,  2749,
    2750,  2754,  2755,  2756,  2757,  2759,  2763,  2764,  2775,  2776,
    2778,  2780,  2792,  2793,  2794,  2798,  2799,  2800,  2804,  2805,
    2806,  2809,  2811,  2815,  2816,  2817,  2818,  2820,  2821,  2822,
    2823,  2824,  2825,  2826,  2827,  2828,  2829,  2832,  2837,  2838,
    2839,  2841,  2842,  2844,  2845,  2846,  2847,  2848,  2849,  2850,
    2851,  2852,  2853,  2855,  2857,  2859,  2861,  2863,  2864,  2865,
    2866,  2867,  2868,  2869,  2870,  2871,  2872,  2873,  2874,  2875,
    2876,  2877,  2878,  2879,  2881,  2883,  2885,  2887,  2888,  2891,
    2892,  2896,  2900,  2902,  2906,  2907,  2911,  2917,  2920,  2924,
    2925,  2926,  2927,  2928,  2929,  2930,  2935,  2937,  2941,  2942,
    2945,  2946,  2950,  2953,  2955,  2957,  2961,  2962,  2963,  2964,
    2967,  2971,  2972,  2973,  2974,  2978,  2980,  2987,  2988,  2989,
    2990,  2995,  2996,  2997,  2998,  3000,  3001,  3003,  3004,  3005,
    3006,  3007,  3008,  3012,  3014,  3018,  3020,  3023,  3026,  3028,
    3030,  3033,  3035,  3039,  3041,  3044,  3047,  3053,  3055,  3058,
    3059,  3064,  3067,  3071,  3071,  3076,  3079,  3080,  3084,  3085,
    3089,  3090,  3091,  3095,  3097,  3105,  3106,  3110,  3112,  3120,
    3121,  3125,  3127,  3128,  3133,  3135,  3140,  3151,  3165,  3177,
    3192,  3193,  3194,  3195,  3196,  3197,  3198,  3208,  3217,  3219,
    3221,  3225,  3229,  3230,  3231,  3232,  3233,  3249,  3250,  3260,
    3261,  3262,  3263,  3264,  3265,  3266,  3267,  3269,  3274,  3278,
    3279,  3283,  3286,  3290,  3297,  3301,  3310,  3317,  3319,  3325,
    3327,  3328,  3332,  3333,  3334,  3341,  3342,  3347,  3348,  3353,
    3354,  3355,  3356,  3367,  3370,  3373,  3374,  3375,  3376,  3387,
    3391,  3392,  3393,  3395,  3396,  3397,  3401,  3403,  3406,  3408,
    3409,  3410,  3411,  3414,  3416,  3417,  3421,  3423,  3426,  3428,
    3429,  3430,  3434,  3436,  3439,  3442,  3444,  3446,  3450,  3451,
    3453,  3454,  3460,  3461,  3463,  3473,  3475,  3477,  3480,  3481,
    3482,  3486,  3487,  3488,  3489,  3490,  3491,  3492,  3493,  3494,
    3495,  3496,  3500,  3501,  3505,  3507,  3515,  3517,  3521,  3525,
    3530,  3534,  3542,  3543,  3547,  3548,  3554,  3555,  3564,  3565,
    3573,  3576,  3580,  3583,  3588,  3593,  3596,  3599,  3601,  3603,
    3605,  3609,  3611,  3612,  3613,  3616,  3618,  3624,  3625,  3629,
    3630,  3634,  3635,  3639,  3640,  3643,  3648,  3649,  3653,  3656,
    3658,  3662,  3668,  3669,  3670,  3674,  3678,  3686,  3691,  3703,
    3705,  3709,  3712,  3714,  3719,  3724,  3730,  3733,  3738,  3743,
    3745,  3752,  3754,  3757,  3758,  3761,  3764,  3765,  3770,  3772,
    3776,  3782,  3792,  3793
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
  "T_TYPELIST_GT", "T_UNRESOLVED_LT", "T_COLLECTION", "T_SHAPE", "T_TUPLE",
  "T_TYPE", "T_UNRESOLVED_TYPE", "T_NEWTYPE", "T_UNRESOLVED_NEWTYPE",
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
  "tuple_literal", "static_tuple_literal", "static_tuple_literal_ae",
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
  "dim_offset", "simple_indirect_reference", "variable_no_calls",
  "dimmable_variable_no_calls", "assignment_list", "array_pair_list",
  "non_empty_array_pair_list", "collection_init",
  "non_empty_collection_init", "static_collection_init",
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
     426,   427,   428,   429,   430,   431,   432,   433,   434,    40,
      41,    59,   123,   125,    93,    36,    96,    34,    39
};
# endif

#define YYPACT_NINF -1764

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1764)))

#define YYTABLE_NINF -1134

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1134)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1764,   219, -1764, -1764,  5679, 14983, 14983,    47, 14983, 14983,
   14983, 14983, 12584, 14983, -1764, 14983, 14983, 14983, 14983, 18317,
   18317, 14983, 14983, 14983, 14983, 14983, 14983, 14983, 14983, 12763,
   19115, 14983,   243,   258, -1764, -1764, -1764,   151, -1764,   247,
   -1764, -1764, -1764,   232, 14983, -1764,   258,   311,   350,   359,
   -1764,   258, 12942,  3653, 13121, -1764, 15822, 11447,   382, 14983,
   19364,    80,    98,    85,   328, -1764, -1764, -1764,   393,   395,
     420,   435, -1764,  3653,   438,   440,   460,   556,   583,   597,
     602, -1764, -1764, -1764, -1764, -1764, 14983,   496,  4048, -1764,
   -1764,  3653, -1764, -1764, -1764, -1764,  3653, -1764,  3653, -1764,
     512,   502,   505,  3653,  3653, -1764,   344, -1764, -1764, 13327,
   -1764, -1764,   489,   553,   609,   609, -1764,   678,   549,    30,
     521, -1764,   100, -1764,   528,   620,   702, -1764, -1764, -1764,
   -1764,  2618,   600, -1764,   165, -1764,   542,   560,   564,   566,
     573,   575,   577,   585, 17227, -1764, -1764, -1764, -1764, -1764,
     110,   687,   719,   721,   730,   733,   738, -1764,   766,   768,
   -1764,    82,   608, -1764,   686,    29, -1764,  1357,    90, -1764,
   -1764,  3289,   165,   165,   669,   160, -1764,   179,   175,   671,
     207, -1764,   364, -1764,   802, -1764,   717, -1764, -1764,   681,
     711, -1764, 14983, -1764,   702,   600, 19652,  4008, 19652, 14983,
   19652, 19652, 16371, 16371,   683, 18490, 19652,   833,  3653,   818,
     818,   361,   818, -1764, -1764, -1764, -1764, -1764, -1764, -1764,
   -1764, -1764,   116, 14983,   713, -1764, -1764,   735,   700,   331,
     701,   331,   818,   818,   818,   818,   818,   818,   818,   818,
   18317, 18538,   699,   892,   717, -1764, 14983,   713, -1764,   745,
   -1764,   746,   710, -1764,   180, -1764, -1764, -1764,   331,   165,
   -1764, 13506, -1764, -1764, 14983, 10211,   902,   104, 19652, 11241,
   -1764, 14983, 14983,  3653, -1764, -1764, 17278,   714, -1764, 17348,
   -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764,
   17601, -1764, 17601, -1764, -1764, -1764, -1764, -1764, -1764, -1764,
   -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764,
   -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764,
   -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764,
   -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764,
   -1764, -1764,   124,   125,   711, -1764, -1764, -1764, -1764,   722,
   -1764,  4441,   127, -1764, -1764,   754,   903, -1764,   758,  5113,
   14983, -1764,   723,   725, 17396, -1764,    63, 17447,  4361,  4361,
    4361,  3653,  4361,   728,   913,   726, -1764,   103, -1764, 17984,
     114, -1764,   909,   123,   799, -1764,   801, -1764, 18317, 14983,
   14983,   736,   760, -1764, -1764, 18060, 12763, 14983, 14983, 14983,
   14983, 14983,   131,   495,   445, -1764, 15162, 18317,   646, -1764,
    3653, -1764,   482,   549, -1764, -1764, -1764, -1764, 19217, 14983,
     930,   843, -1764, -1764, -1764,   161, 14983,   751,   752, 19652,
     756,  2037,   759,  6297, 14983,    91,   755,   658,    91,   500,
     454, -1764,  3653, 17601,   761, 11626, 15822, -1764, 13712,   765,
     765,   765,   765, -1764, -1764,  3519, -1764, -1764, -1764, -1764,
   -1764,   702, -1764, 14983, 14983, -1764, -1764, -1764, -1764, -1764,
   -1764, -1764, -1764, -1764, 14983, 14983, 14983, 14983, 13891, 14983,
   14983, 14983, 14983, 14983, 14983, 14983, 14983, 14983, 14983, 14983,
   14983, 14983, 14983, 14983, 14983, 14983, 14983, 14983, 14983, 14983,
   14983, 14983, 19293, 14983, -1764, 14983, 14983, 14983, 15335,  3653,
    3653,  3653,  3653,  3653,  2618,   863,   816,  5268, 14983, 14983,
   14983, 14983, 14983, 14983, 14983, 14983, 14983, 14983, 14983, 14983,
   -1764, -1764, -1764, -1764,  1251, -1764, -1764, 11626, 11626, 14983,
   14983,   489,   195, 18060,   778,   702, 14070,  4044, -1764, 14983,
   -1764,   780,   976,   827,   790,   791, 15489,   331, 14249, -1764,
   14428, -1764,   710,   792,   795,  2361, -1764,   372, 11626, -1764,
    4961, -1764, -1764, 17517, -1764, -1764, 12002, -1764, 14983, -1764,
     901, 10417,   989,   798, 19530,   991,   117,    89, -1764, -1764,
   -1764,   823, -1764, -1764, -1764, 17601, -1764,  3504,   808,  1007,
   17832,  3653, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764,
   -1764,   821, -1764, -1764,   819,   822,   824,   829,   826,   830,
     112,   831,   834,  5340, 15383, -1764, -1764,  3653,  3653, 14983,
     331,    80, -1764, 17832,   941, -1764, -1764, -1764,   331,   145,
     149,   825,   837,  2518,   188,   838,   839,   531,   900,   845,
     331,   152,   842, 18599,   848,  1037,  1038,   850,   859,   862,
     867, -1764,  3155,  3653, -1764, -1764,   979,  3232,    59, -1764,
   -1764, -1764,   549, -1764, -1764, -1764,  1021,   932,   890,   392,
     912, 14983,   489,   937,  1071,   882, -1764,   921, -1764,   195,
   -1764,   887, 17601, 17601,  1075,   902,   161, -1764,   894,  1082,
   -1764, 17437,   229, -1764,   367,   177, -1764, -1764, -1764, -1764,
   -1764, -1764, -1764,  1081,  3517, -1764, -1764, -1764, -1764,  1083,
     917, -1764, 18317,   339, 14983,   893,  1095, 19652,  1091,   153,
    1098,   911,   916,   923, 19652,   925,  2570,  6503, -1764, -1764,
   -1764, -1764, -1764, -1764,   975,  4194, 19652,   908,  4615, 19791,
   19882, 16371, 16002, 14983, 19604, 19963, 15336, 20068, 20135, 20167,
   19300, 20198, 20198, 20198, 20198,  3951,  3951,  3951,  3951,  3951,
     919,   919,   697,   697,   697,   361,   361,   361, -1764,   818,
     915,   922, 18647,   910,  1112,    27, 14983,    37,   713,    49,
     195, -1764, -1764, -1764,  1113,   843, -1764,   702, 18165, -1764,
   -1764, -1764, 16371, 16371, 16371, 16371, 16371, 16371, 16371, 16371,
   16371, 16371, 16371, 16371, 16371, -1764, 14983,   250,   202, -1764,
   -1764,   713,   262,   931,   933,   926,  4978,   156,   939, -1764,
   19652, 17908, -1764,  3653, -1764,   331,    62, 18317, 19652, 18317,
   18708,   975,   236,   331,   213,   980,   943, 14983, -1764,   217,
   -1764, -1764, -1764,  6709,   612, -1764, -1764, 19652, 19652,   258,
   -1764, -1764, -1764, 14983,  1041, 17756, 17832,  3653, 10623,   944,
     945, -1764,  1138, 15715,  1011, -1764,   988, -1764,  1143,   953,
    1609, 17601, 17832, 17832, 17832, 17832, 17832,   956,  1087,  1089,
    1097,  1102,  1103,   981,   982, 17832,   410, -1764, -1764, -1764,
   -1764, -1764, -1764, -1764,    78, -1764, 19746, -1764, -1764,    39,
   -1764,  6915,  3762,   984, 15383, -1764, 15383, -1764, 15383, -1764,
    3653,  3653, 15383, -1764, 15383, 15383,  3653, -1764,  1173,   985,
   -1764,   286, -1764, -1764, 16888, -1764, 19746,  1170, 18317,   986,
   -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764,  1008,
    1184,  3653,  3762,   992, 18060, 18241,  1181, -1764, 14983, -1764,
   14983, -1764, 14983, -1764, -1764, -1764, -1764, -1764, -1764, -1764,
   -1764,   993, -1764, 14983, -1764, -1764,  5885, -1764, 17601,  3762,
     994, -1764, -1764, -1764, -1764,  1186,  1001, 14983, 19217, -1764,
   -1764, 15335, -1764,  1015, -1764, 17601, -1764,  1012,  7121,  1180,
     108, -1764, 17601, -1764,   144,  1251,  1251, -1764, 17601, -1764,
   -1764,   331, -1764, -1764,  1150, 19652, -1764, 11805, -1764, 17832,
   13712,   765, 13712, -1764,   765,   765, -1764, 12208, -1764, -1764,
    7327, -1764,   111,  1022,  3762,   932, -1764, -1764, -1764, -1764,
   19963, 14983, -1764, -1764, 14983, -1764, 14983, -1764, 16936,  1029,
   11626,   900,  1202,   932, 17601,  1221,   975,  3653, 19293,   331,
   16984,  1039,   184,  1043, -1764, -1764,  1229,  1792,  1792, 17908,
   -1764, -1764, -1764,  1193,  1047,  1178,  1183,  1187,  1190,  1191,
     134,  1050,  1051,   414, -1764, -1764, -1764, -1764, -1764, -1764,
    1092, -1764, -1764, -1764, -1764,  1242,  1062,   780,   331,   331,
   14607,   932,  4961, -1764,  4961, -1764, 17032,   623,   258, 11241,
   -1764,  7533,  1066,  7739,  1068, 17756, 18317,  1073,  1135,   331,
   19746,  1263, -1764, -1764, -1764, -1764,   707, -1764,    68, 17601,
    1096,  1147,  1124, 17601,  3653,  3711, -1764, -1764, 17601,  1278,
    1093,  1115,  1121,  1083,   718,   718,  1230,  1230, 18865,  1099,
    1292, 17832, 17832, 17832, 17832, 17832, 17832, 19217, 17832,  3403,
   16563, 17832, 17832, 17832, 17832, 17680, 17832, 17832, 17832, 17832,
   17832, 17832, 17832, 17832, 17832, 17832, 17832, 17832, 17832, 17832,
   17832, 17832, 17832, 17832, 17832, 17832, 17832, 17832, 17832,  3653,
   -1764, -1764,  1219, -1764, -1764,  1104,  1105,  1106, -1764,  1107,
   -1764, -1764,   296,  5340, -1764,  1110, -1764, 17832,   331, -1764,
   -1764,   154, -1764,   642,  1297, -1764, -1764,   162,  1114,   331,
   12405, 19652, 18756, -1764,  2866, -1764,  6091,   843,  1297, -1764,
      22,    32, -1764, 19652,  1176,  1116, -1764,  1119,  1180, -1764,
   -1764, -1764, 14804, 17601,   902, 17553,  1233,   426,  1303,  1238,
     226, -1764,   713,   233, -1764,   713, -1764, 14983, 18317,   339,
   14983, 19652, 19746, -1764, -1764, -1764,  4183, -1764, -1764, -1764,
   -1764, -1764, -1764,  1122,   111, -1764,  1125,   111,  1127, 19963,
   19652, 18817,  1130, 11626,  1131,  1128, 17601,  1134,  1123, 17601,
     932, -1764,   710,   439, 11626, 14983, -1764, -1764, -1764, -1764,
   -1764, -1764,  1201,  1136,  1332,  1250, 17908, 17908, 17908, 17908,
   17908, 17908,  1188, -1764, 19217, 17908,   105, 17908, -1764, -1764,
   -1764, 18317, 19652,  1141, -1764,   258,  1313,  1271, 11241, -1764,
   -1764, -1764,  1148, 14983,  1135,   331, 18060, 17756,  1151, 17832,
    7945,   739,  1149, 14983,   132,    76, -1764,  1168, -1764, 17601,
    3653, -1764,  1215, -1764, -1764, -1764,  4290, -1764,  1322, -1764,
    1156, 17832, -1764, 17832, -1764,  1157,  1154,  1350, 18925,  1158,
   19746,  1355,  1161,  1162,  1165,  1232,  1366,  1177,  1189, -1764,
   -1764, -1764, 18971,  1185,  1367, 19838, 19926, 16192, 17832, 19700,
   20034, 20102, 17764, 18249, 18068, 20229, 20229, 20229, 20229,  3108,
    3108,  3108,  3108,  3108,  1172,  1172,   718,   718,   718,  1230,
    1230,  1230,  1230, -1764,  1197, -1764,  1198,  1203,  1204,  1205,
   -1764, -1764, 19746,  3653, 17601, 17601, -1764,   642,  3762,  1755,
   -1764, 18060, -1764, -1764, 16371, 14983,  1195, -1764,  1182,  2025,
   -1764,   115, 14983, -1764, -1764, -1764, 14983, -1764, 14983, -1764,
     902, 13712,  1206,   336,   765,   336,   218, -1764, -1764, 17601,
     155, -1764,  1365,  1298, 14983, -1764,  1192,  1209,  1199,   331,
    1150, 19652,  1180,  1210, -1764,  1213,   111, 14983, 11626,  1216,
   -1764, -1764,   843, -1764, -1764,  1208,  1220,  1218, -1764,  1227,
   17908, -1764, 17908, -1764, -1764,  1228,  1231,  1377,  1252,  1234,
   -1764,  1422,  1235,  1236,  1240, -1764,  1294,  1247,  1424,  1248,
   -1764, -1764,   331, -1764,  1404, -1764,  1253, -1764, -1764,  1255,
    1257,   163, -1764, -1764, 19746,  1259,  1262, -1764, 17179, -1764,
   -1764, -1764, -1764, -1764, -1764,  1314, 17601, 17601,  1115,  1275,
   17601, -1764, 19746, 19031, -1764, -1764, 17832, -1764, 17832, -1764,
   17832, -1764, -1764, -1764, -1764, 17832, 19217, -1764, -1764, -1764,
   17832, -1764, 17832, -1764, 19999, 17832,  1264,  8151, -1764, -1764,
   -1764, -1764,   642, -1764, -1764, -1764, -1764,   649, 16001,  3762,
    1351, -1764,  2669,  1299,   720, -1764, -1764, -1764,   863,  4115,
     133,   135,  1268,   843,   816,   164, 19652, -1764, -1764, -1764,
    1307, 17083, 17131, 19652, -1764,  2835, -1764,  6503,  1382,   457,
    1461,  1393, 14983, -1764, 19652, 11626, 11626, -1764,  1369,  1180,
    2158,  1180,  1284, 19652,  1285, -1764,  2195,  1289,  2291, -1764,
   -1764,   111, -1764, -1764,  1348, -1764, -1764, 17908, -1764, 17908,
   -1764, 17908, -1764, -1764, -1764, -1764, 17908, -1764, 19217, -1764,
   -1764,  2447, -1764,  8357, -1764, -1764, -1764, -1764, 10829, -1764,
   -1764, -1764,  6709, 17601, -1764, -1764, -1764,  1293, 17832, 19077,
   19746, 19746, 19746,  1354, 19746, 19137, 19999, -1764, -1764,   642,
    3762,  3762,  3653, -1764,  1481, 16717,    96, -1764, 16001,   843,
   16456, -1764,  1316, -1764,   138,  1295,   139, -1764, 16370, -1764,
   -1764, -1764,   140, -1764, -1764,  2885, -1764,  1301, -1764,  1413,
     702, -1764, 16191, -1764, 16191, -1764, -1764,  1486,   863, -1764,
    5492, -1764, -1764, -1764, -1764,  3055, -1764,  1489,  1423, 14983,
   -1764, 19652,  1309,  1317,  1308,  1180,  1315, -1764,  1369,  1180,
   -1764, -1764, -1764, -1764,  2551,  1321, 17908,  1376, -1764, -1764,
   -1764,  1384, -1764,  6709, 11035, 10829, -1764, -1764, -1764,  6709,
   -1764, -1764, 19746, 17832, 17832, 17832,  8563,  1323,  1326, -1764,
   17832, -1764,  3762, -1764, -1764, -1764, -1764, -1764, 17601,  2579,
    2669, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764,
   -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764,
   -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764,
   -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764,
   -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764,
   -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764,
   -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764,
   -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764,
   -1764, -1764, -1764, -1764, -1764,   584, -1764,  1299, -1764, -1764,
   -1764, -1764, -1764,   121,   525, -1764,  1515,   141,  5113,  1413,
    1517, -1764, 17601,   702, -1764, -1764,  1333,  1521, 14983, -1764,
   19652, -1764, -1764,   150,  1334, 17601,   621,  1180,  1315, 15643,
   -1764,  1180, -1764, 17908, 17908, -1764, -1764, -1764, -1764,  8769,
   19746, 19746, 19746, -1764, -1764, -1764, 19746, -1764,   590,  1528,
    1530,  1337, -1764, -1764, 17832, 16370, 16370,  1476, -1764,  2885,
    2885,   557, -1764, -1764, -1764, 17832,  1464, -1764,  1372,  1360,
     142, 17832, -1764,  5113, -1764, 17832, 19652,  1466, -1764,  1541,
   -1764,  1547, -1764,   492, -1764, -1764, -1764,  1361,   621, -1764,
     621, -1764, -1764,  8975,  1364,  1450, -1764,  1467,  1407, -1764,
   -1764,  1469, 17601,  1389,  2579, -1764, -1764, 19746, -1764, -1764,
    1401, -1764,  1540, -1764, -1764, -1764, -1764, 19746,  1564,   531,
   -1764, -1764, 19746,  1380, 19746, -1764,   193,  1381,  9181, 17601,
   -1764, 17601, -1764,  9387, -1764, -1764, -1764,  1379, -1764,  1383,
    1403,  3653,   816,  1400, -1764, -1764, -1764, 17832,  1405,   119,
   -1764,  1502, -1764, -1764, -1764, -1764, -1764, -1764,  9593, -1764,
    3762,   984, -1764,  1410,  3653,   704, -1764, 19746, -1764,  1390,
    1582,   503,   119, -1764, -1764,  1509, -1764,  3762,  1392, -1764,
    1180,   130, -1764, 17601, -1764, -1764, -1764, 17601, -1764,  1394,
    1395,   146, -1764,  1315,   715,  1513,   159,  1180,  1396, -1764,
     655, 17601, 17601, -1764,   464,  1585,  1518,  1315, -1764, -1764,
   -1764, -1764,  1520,   185,  1591,  1523, 14983, -1764,   655,  9799,
   10005, -1764,   487,  1594,  1529, 14983, -1764, 19652, -1764, -1764,
   -1764,  1599,  1534, 14983, -1764, 19652, 14983, -1764, 19652, 19652
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   204,   473,     0,   913,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1007,
     995,     0,   777,     0,   783,   784,   785,    29,   850,   982,
     983,   171,   172,   786,     0,   152,     0,     0,     0,     0,
      30,     0,     0,     0,     0,   223,     0,     0,     0,     0,
       0,     0,   442,   443,   444,   441,   440,   439,     0,     0,
       0,     0,   252,     0,     0,     0,    37,    38,    40,    41,
      39,   790,   792,   793,   787,   788,     0,     0,     0,   794,
     789,     0,   760,    32,    33,    34,    36,    35,     0,   791,
       0,     0,     0,     0,     0,   795,   445,   583,    31,     0,
     170,   140,   987,   778,     0,     0,     4,   126,   128,   849,
       0,   759,     0,     6,     0,     0,   222,     7,     9,     8,
      10,     0,     0,   437,     0,   487,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   543,   485,   969,   970,   565,
     558,   559,   560,   561,   564,   562,   563,   468,   568,     0,
     467,   941,   761,   768,     0,   852,   557,   436,   944,   945,
     957,   486,     0,     0,     0,   489,   488,   942,   943,   940,
     977,   981,     0,   547,   851,    11,   442,   443,   444,     0,
       0,    36,     0,   126,   222,     0,  1047,   486,  1048,     0,
    1050,  1051,   567,   481,     0,   474,   479,     0,     0,   529,
     530,   531,   532,    29,   982,   786,   763,    37,    38,    40,
      41,    39,     0,     0,  1071,   962,   761,     0,   762,   508,
       0,   510,   548,   549,   550,   551,   552,   553,   554,   556,
       0,  1011,     0,   859,   773,   242,     0,  1071,   465,   772,
     766,     0,   782,   762,   990,   991,   997,   989,   774,     0,
     466,     0,   776,   555,     0,   205,     0,     0,   470,   205,
     150,   472,     0,     0,   156,   158,     0,     0,   160,     0,
      75,    76,    82,    83,    67,    68,    59,    80,    91,    92,
       0,    62,     0,    66,    74,    72,    94,    86,    85,    57,
     108,    81,   101,   102,    58,    97,    55,    98,    56,    99,
      54,   103,    90,    95,   100,    87,    88,    61,    89,    93,
      53,    84,    69,   104,    77,   106,    70,    60,    47,    48,
      49,    50,    51,    52,    71,   107,   105,   110,    64,    45,
      46,    73,  1124,  1125,    65,  1129,    44,    63,    96,     0,
      79,     0,   126,   109,  1062,  1123,     0,  1126,     0,     0,
       0,   162,     0,     0,     0,   213,     0,     0,     0,     0,
       0,     0,     0,     0,   861,     0,   114,   116,   350,     0,
       0,   349,   355,     0,     0,   253,     0,   256,     0,     0,
       0,     0,  1068,   238,   250,  1003,  1007,   602,   632,   632,
     602,   632,     0,  1032,     0,   797,     0,     0,     0,  1030,
       0,    16,     0,   130,   230,   244,   251,   663,   595,   632,
       0,  1056,   575,   577,   579,   917,   473,   487,     0,     0,
     485,   486,   488,   205,     0,   779,     0,   780,     0,     0,
       0,   202,     0,     0,   132,   339,     0,    28,     0,     0,
       0,     0,     0,   203,   221,     0,   249,   234,   248,   442,
     445,   222,   438,   986,     0,   933,   192,   193,   194,   195,
     196,   198,   199,   201,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   995,     0,   191,   986,   986,  1017,     0,     0,
       0,     0,     0,     0,     0,     0,   435,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     507,   509,   918,   919,     0,   932,   931,   339,   339,   986,
       0,   988,   978,  1003,     0,   222,     0,     0,   164,     0,
     915,   910,   859,     0,   487,   485,     0,  1015,     0,   600,
     858,  1006,   782,   487,   485,   486,   132,     0,   339,   464,
       0,   934,   775,     0,   140,   292,     0,   582,     0,   167,
       0,   205,   471,     0,     0,     0,     0,     0,   159,   190,
     161,  1124,  1125,  1121,  1122,     0,  1128,  1114,     0,     0,
       0,     0,    78,    43,    65,    42,  1063,   197,   200,   163,
     140,     0,   180,   189,     0,     0,     0,     0,     0,     0,
     117,     0,     0,     0,   860,   115,    18,     0,   111,     0,
     351,     0,   165,     0,     0,   166,   254,   255,  1052,     0,
       0,   487,   485,   486,   489,   488,     0,  1104,   262,     0,
    1004,     0,     0,     0,     0,   859,   859,     0,     0,     0,
       0,   168,     0,     0,   796,  1031,   850,     0,     0,  1029,
     855,  1028,   129,     5,    13,    14,     0,   260,     0,     0,
     588,     0,     0,     0,   859,     0,   770,     0,   769,   764,
     589,     0,     0,     0,     0,     0,   917,   136,     0,   861,
     916,  1133,   463,   476,   490,   950,   968,   147,   139,   143,
     144,   145,   146,   436,     0,   566,   853,   854,   127,   859,
       0,  1072,     0,     0,     0,     0,   861,   340,     0,     0,
       0,   487,   209,   210,   208,   485,   486,   205,   184,   182,
     183,   185,   571,   224,   258,     0,   985,     0,     0,   513,
     515,   514,   526,     0,     0,   546,   511,   512,   516,   518,
     517,   535,   536,   533,   534,   537,   538,   539,   540,   541,
     527,   528,   520,   521,   519,   522,   523,   525,   542,   524,
       0,     0,  1021,     0,   859,  1055,     0,  1054,  1071,   947,
     977,   240,   232,   246,     0,  1056,   236,   222,     0,   477,
     480,   482,   492,   495,   496,   497,   498,   499,   500,   501,
     502,   503,   504,   505,   506,   921,     0,   920,   923,   946,
     927,  1071,   924,     0,     0,     0,     0,     0,     0,  1049,
     475,   908,   912,   858,   914,   462,   765,     0,  1010,     0,
    1009,   258,     0,   765,   994,   993,     0,     0,   920,   923,
     992,   924,   484,   294,   296,   136,   586,   585,   469,     0,
     140,   276,   151,   472,     0,     0,     0,     0,   205,   288,
     288,   157,   859,     0,     0,  1113,     0,  1110,   859,     0,
    1084,     0,     0,     0,     0,     0,   857,     0,    37,    38,
      40,    41,    39,     0,     0,     0,   799,   803,   804,   805,
     808,   806,   807,   810,     0,   798,   134,   848,   809,  1071,
    1127,   205,     0,     0,     0,    21,     0,    22,     0,    19,
       0,   112,     0,    20,     0,     0,     0,   123,   861,     0,
     121,   116,   113,   118,     0,   348,   356,   353,     0,     0,
    1041,  1046,  1043,  1042,  1045,  1044,    12,  1102,  1103,     0,
     859,     0,     0,     0,  1003,  1000,     0,   599,     0,   613,
     858,   601,   858,   631,   616,   625,   628,   619,  1040,  1039,
    1038,     0,  1034,     0,  1035,  1037,   205,     5,     0,     0,
       0,   657,   658,   666,   665,     0,   485,     0,   858,   594,
     598,     0,   622,     0,  1057,     0,   576,     0,   205,  1091,
     917,   320,  1133,  1132,     0,     0,     0,   984,   858,  1074,
    1070,   342,   336,   337,   341,   343,   758,   860,   338,     0,
       0,     0,     0,   462,     0,     0,   490,     0,   951,   212,
     205,   142,   917,     0,     0,   260,   573,   226,   929,   930,
     545,     0,   639,   640,     0,   637,   858,  1016,     0,     0,
     339,   262,     0,   260,     0,     0,   258,     0,   995,   493,
       0,     0,   948,   949,   979,   980,     0,     0,     0,   896,
     866,   867,   868,   875,     0,    37,    38,    40,    41,    39,
       0,     0,     0,   881,   887,   888,   889,   892,   890,   891,
       0,   879,   877,   878,   902,   859,     0,   910,  1014,  1013,
       0,   260,     0,   935,     0,   781,     0,   298,     0,   205,
     148,   205,     0,   205,     0,     0,     0,     0,   268,   269,
     280,     0,   140,   278,   177,   288,     0,   288,     0,   858,
       0,     0,     0,     0,     0,   858,  1112,  1115,  1080,   859,
       0,  1075,     0,   859,   831,   832,   829,   830,   865,     0,
     859,   857,   606,   634,   634,   606,   634,   597,   634,     0,
       0,  1023,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1118,   214,     0,   217,   181,     0,     0,     0,   119,     0,
     124,   125,   117,   860,   122,     0,   352,     0,  1053,   169,
    1069,  1104,  1095,  1099,   261,   263,   362,     0,     0,  1001,
       0,   604,     0,  1033,     0,    17,   205,  1056,   259,   362,
       0,     0,   765,   591,     0,   771,  1058,     0,  1091,   580,
     135,   137,     0,     0,     0,  1133,     0,     0,   325,   323,
     923,   936,  1071,   923,   937,  1071,  1073,   986,     0,     0,
       0,   344,   133,   207,   209,   210,   486,   188,   206,   186,
     187,   211,   141,     0,   917,   257,     0,   917,     0,   544,
    1020,  1019,     0,   339,     0,     0,     0,     0,     0,     0,
     260,   228,   782,   922,   339,     0,   871,   872,   873,   874,
     882,   883,   900,     0,   859,     0,   896,   610,   636,   636,
     610,   636,     0,   870,   904,   636,     0,   858,   907,   909,
     911,     0,  1008,     0,   922,     0,     0,     0,   205,   295,
     587,   153,     0,   472,   268,   270,  1003,     0,     0,     0,
     205,     0,     0,     0,     0,     0,   282,     0,  1119,     0,
       0,  1105,     0,  1111,  1109,  1076,   858,  1082,     0,  1083,
       0,     0,   801,   858,   856,     0,     0,   859,     0,     0,
     845,   859,     0,     0,     0,     0,   859,     0,     0,   811,
     846,   847,  1027,     0,   859,   814,   816,   815,     0,     0,
     812,   813,   817,   819,   818,   835,   836,   833,   834,   837,
     838,   839,   840,   841,   826,   827,   821,   822,   820,   823,
     824,   825,   828,  1117,     0,   140,     0,     0,     0,     0,
     120,    23,   354,     0,     0,     0,  1096,  1101,     0,   436,
    1005,  1003,   478,   483,   491,     0,     0,    15,     0,   436,
     669,     0,     0,   671,   664,   667,     0,   662,     0,  1060,
       0,     0,     0,     0,   543,     0,   489,  1092,   584,  1133,
       0,   326,   327,     0,     0,   321,     0,     0,     0,   346,
     347,   345,  1091,     0,   362,     0,   917,     0,   339,     0,
     975,   362,  1056,   362,  1059,     0,     0,     0,   494,     0,
       0,   885,   858,   895,   876,     0,     0,   859,     0,     0,
     894,   859,     0,     0,     0,   869,     0,     0,   859,     0,
     880,   901,  1012,   362,     0,   140,     0,   291,   277,     0,
       0,     0,   267,   173,   281,     0,     0,   284,     0,   289,
     290,   140,   283,  1120,  1106,     0,     0,  1079,  1078,     0,
       0,  1131,   864,   863,   800,   614,   858,   605,     0,   617,
     858,   633,   626,   629,   620,     0,   858,   596,   802,   623,
       0,   638,   858,  1022,   843,     0,     0,   205,    24,    25,
      26,    27,  1098,  1093,  1094,  1097,   264,     0,     0,     0,
     443,   434,     0,     0,     0,   239,   361,   363,     0,   433,
       0,     0,     0,  1056,   436,     0,   603,  1036,   358,   245,
     660,     0,     0,   590,   578,   486,   138,   205,     0,     0,
     330,   319,     0,   322,   329,   339,   339,   335,   570,  1091,
     436,  1091,     0,  1018,     0,   974,   436,     0,   436,  1061,
     362,   917,   971,   899,   898,   884,   615,   858,   609,     0,
     618,   858,   635,   627,   630,   621,     0,   886,   858,   903,
     624,   436,   140,   205,   149,   154,   175,   271,   205,   279,
     285,   140,   287,     0,  1107,  1077,  1081,     0,     0,     0,
     608,   844,   593,     0,  1026,  1025,   842,   140,   218,  1100,
       0,     0,     0,  1064,     0,     0,     0,   265,     0,  1056,
       0,   399,   395,   401,   760,    36,     0,   389,     0,   394,
     398,   411,     0,   409,   414,     0,   413,     0,   412,     0,
     222,   365,     0,   367,     0,   368,   369,     0,     0,  1002,
       0,   661,   659,   670,   668,     0,   331,   332,     0,     0,
     317,   328,     0,     0,     0,  1091,  1085,   235,   570,  1091,
     976,   241,   358,   247,   436,     0,     0,     0,   612,   893,
     906,     0,   243,   293,   205,   205,   140,   274,   174,   286,
    1108,  1130,   862,     0,     0,     0,   205,     0,     0,   461,
       0,  1065,     0,   379,   383,   458,   459,   393,     0,     0,
       0,   374,   719,   720,   718,   721,   722,   739,   741,   740,
     710,   682,   680,   681,   700,   715,   716,   676,   687,   688,
     690,   689,   757,   709,   693,   691,   692,   694,   695,   696,
     697,   698,   699,   701,   702,   703,   704,   705,   706,   708,
     707,   677,   678,   679,   683,   684,   686,   756,   724,   725,
     729,   730,   731,   732,   733,   734,   717,   736,   726,   727,
     728,   711,   712,   713,   714,   737,   738,   742,   744,   743,
     745,   746,   723,   748,   747,   750,   752,   751,   685,   755,
     753,   754,   749,   735,   675,   406,   672,     0,   375,   427,
     428,   426,   419,     0,   420,   376,   453,     0,     0,     0,
       0,   457,     0,   222,   231,   357,     0,     0,     0,   318,
     334,   972,   973,     0,     0,     0,     0,  1091,  1085,     0,
     237,  1091,   897,     0,     0,   140,   272,   155,   176,   205,
     607,   592,  1024,   216,   377,   378,   456,   266,     0,   859,
     859,     0,   402,   390,     0,     0,     0,   408,   410,     0,
       0,   415,   422,   423,   421,     0,     0,   364,  1066,     0,
       0,     0,   460,     0,   359,     0,   333,     0,   655,   861,
     136,   861,  1087,     0,   429,   136,   225,     0,     0,   233,
       0,   611,   905,   205,     0,   178,   380,   126,     0,   381,
     382,     0,   858,     0,   858,   404,   400,   405,   673,   674,
       0,   391,   424,   425,   417,   418,   416,   454,   451,  1104,
     370,   366,   455,     0,   360,   656,   860,     0,   205,   860,
    1086,     0,  1090,   205,   136,   227,   229,     0,   275,     0,
     220,     0,   436,     0,   396,   403,   407,     0,     0,   917,
     372,     0,   653,   569,   572,  1088,  1089,   430,   205,   273,
       0,     0,   179,   387,     0,   435,   397,   452,  1067,     0,
     861,   447,   917,   654,   574,     0,   219,     0,     0,   386,
    1091,   917,   302,  1133,   450,   449,   448,  1133,   446,     0,
       0,     0,   385,  1085,   447,     0,     0,  1091,     0,   384,
       0,  1133,  1133,   308,     0,   307,   305,  1085,   140,   431,
     136,   371,     0,     0,   309,     0,     0,   303,     0,   205,
     205,   313,     0,   312,   301,     0,   304,   311,   373,   215,
     432,   314,     0,     0,   299,   310,     0,   300,   316,   315
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1764, -1764, -1764,  -574, -1764, -1764, -1764,   547,  -449,   -41,
     427, -1764,  -241,  -513, -1764, -1764,   415,   594,  1829, -1764,
    2352, -1764,  -814, -1764,  -537, -1764,  -703,     5, -1764, -1764,
   -1764, -1764, -1764, -1764, -1764, -1764,  -934, -1764, -1764,  -920,
    -356, -1764, -1764, -1764,  -412, -1764, -1764,  -167,    21,    38,
   -1764, -1764, -1764, -1764, -1764, -1764,    53, -1764, -1764, -1764,
   -1764, -1764, -1764,    56, -1764, -1764,  1111,  1117,  1109,   -88,
    -740,  -916,   570,   645,  -418,   297, -1008, -1764,  -125, -1764,
   -1764, -1764, -1764,  -779,   107, -1764, -1764, -1764, -1764,  -410,
   -1764,  -588, -1764,   377,  -451, -1764, -1764,  1006, -1764,  -104,
   -1764, -1764, -1123, -1764, -1764, -1764, -1764, -1764, -1764, -1764,
   -1764, -1764, -1764,  -139, -1764,   -45, -1764, -1764, -1764, -1764,
   -1764,  -225, -1764,    64, -1070, -1764, -1746,  -442, -1764,  -165,
      72,  -118,  -414, -1764,  -232, -1764, -1764, -1764,    73,   -91,
     -78,   -14,  -765,   -66, -1764, -1764,    40, -1764,    -9,  -373,
   -1764,    12,    -5,   -84,   -80,   -86, -1764, -1764, -1764, -1764,
   -1764, -1764, -1764, -1764, -1764,  -633,  -904, -1764, -1764, -1764,
   -1764, -1764,   507,  1254, -1764,   504, -1764,   345, -1764, -1764,
   -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764, -1764,
   -1764, -1764, -1764, -1764, -1764, -1764, -1764,   317,  -323,  -645,
   -1764, -1764, -1764, -1764, -1764,   431, -1764, -1764, -1764, -1764,
   -1764, -1764, -1764, -1764, -1059,  -380,  2800,    48, -1764,  1179,
    -417, -1764, -1764,  -488,  3752,  3612, -1764,   603, -1764, -1764,
     506,   849,  -655, -1764, -1764,   596,   357,   254, -1764,   362,
   -1764, -1764, -1764, -1764, -1764,   572, -1764, -1764, -1764,    86,
    -954,  -140,  -446,  -445, -1764,  -130,  -137, -1764, -1764,    50,
      52,   689,   -63, -1764, -1764,    46,   -77, -1764,  -367,   172,
    -154, -1764,  -436, -1764, -1764, -1764,  -423,  1274, -1764, -1764,
   -1764, -1764, -1764,   740,   731, -1764, -1764, -1764,  -365,  -702,
   -1764,  1225, -1288,  -276,   -48,  -179,   794, -1764, -1764, -1764,
   -1763, -1764,  -326, -1122, -1335,  -313,   106, -1764,   469,   550,
   -1764, -1764, -1764, -1764,   493, -1764,  3172,  -813
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   116,   977,   673,   193,   353,   788,
     373,   374,   375,   376,   928,   929,   930,   118,   119,   120,
     121,   122,   998,  1240,   433,  1030,   708,   709,   581,   269,
    1754,   587,  1658,  1755,  2010,   913,   124,   125,   729,   730,
     738,   366,   610,  1965,  1194,  1415,  2032,   455,   194,   710,
    1033,  1278,  1487,   128,   676,  1052,   711,   744,  1056,   648,
    1051,   248,   562,   712,   677,  1053,   457,   393,   415,   131,
    1035,   980,   953,  1214,  1686,  1338,  1118,  1907,  1758,   862,
    1124,   586,   871,  1126,  1531,   854,  1107,  1110,  1327,  2039,
    2040,   698,   699,  1014,   725,   726,   380,   381,   383,  1720,
    1885,  1886,  1429,  1586,  1709,  1879,  2019,  2042,  1918,  1969,
    1970,  1971,  1696,  1697,  1698,  1699,  1920,  1921,  1927,  1981,
    1702,  1703,  1707,  1872,  1873,  1874,  1956,  2081,  1587,  1588,
     195,   133,  2057,  2058,  1877,  1590,  1591,  1592,  1593,   134,
     135,   656,   583,   136,   137,   138,   139,   140,   141,   142,
     143,   262,   144,   145,   146,  1735,   147,  1032,  1277,   148,
     695,   696,   697,   266,   425,   577,   683,   684,  1376,   685,
    1377,   149,   150,   654,   655,  1366,  1367,  1496,  1497,   151,
     897,  1084,   152,   898,  1085,   153,   899,  1086,   154,   900,
    1087,   155,   901,  1088,   156,   902,  1089,   657,  1369,  1499,
     157,   903,   158,   159,  1949,   160,   678,  1722,   679,  1230,
     985,  1447,  1444,  1865,  1866,   161,   162,   163,   251,   164,
     252,   263,   436,   569,   165,  1370,  1371,   907,   908,   166,
    1149,   561,   625,  1150,  1092,  1300,  1093,  1500,  1501,  1303,
    1304,  1095,  1507,  1508,  1096,   832,   552,   207,   208,   713,
     701,   534,  1250,  1251,   820,   821,   465,   168,   254,   169,
     170,   197,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   747,   182,   258,   259,   651,   242,   243,   783,
     784,  1383,  1384,   408,   409,   971,   183,   639,   184,   694,
     185,   356,  1887,  1939,   394,   444,   719,   720,  1139,  1140,
    1896,  1951,  1952,  1244,  1426,   949,  1427,   950,   951,   877,
     878,   879,   357,   358,   910,   596,  1003,  1004
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     196,   198,   515,   200,   201,   202,   203,   205,   206,   123,
     209,   210,   211,   212,   462,   354,   232,   233,   234,   235,
     236,   237,   238,   239,   241,   126,   260,   544,   542,   430,
     267,   427,   432,   449,  1031,   535,   536,   853,   686,   268,
     450,  1111,   127,   428,  1001,   265,  1245,   276,   363,   279,
     416,   688,   364,   690,   367,   420,   421,   129,   270,   787,
     130,   451,   996,   274,  1242,   230,   230,  1142,   566,   780,
     781,  1018,   402,   911,   841,   733,   132,   462,   250,   514,
     255,   268,   256,   458,  1234,   778,   823,   824,   818,   819,
     167,  1128,  1575,  1055,   739,   740,   741,   362,  1114,   976,
    1263,  1101,  1268,   825,   429,  1772,  1439,  1334,   997,   446,
     927,   932,   430,   578,   427,   432,  1450,   846,   570,  1276,
     827,   869,   571,   631,   849,   850,   867,   615,   617,   619,
    1929,   622,   634,   -78,   -43,  1958,   -42,  1287,   -78,   -43,
     578,   -42,  1712,   555,  1714,   554,   432,  -392,  1780,  1867,
    1936,  1936,  -952,   461,   938,  1772,    14,  1930,   578,    14,
    -955,   955,  1020,   378,  1529,   955,   564,    14,   563,  1600,
     452,   955,   955,   955,   403,  1343,  1344,  1242,    14,   611,
    -641,   382,  1247,  1343,  1344,  1323,  1510,   429,  1947,  1246,
    1440,  -762, -1071,  1609,   547,   532,   533,  2074,   532,   533,
    1445,  1189,   257,  1441,   947,   948,   627,   553,   384,    14,
    -110,  -771,  2005,   443,  2006,   920,   545,   385,   429,     3,
    -109,  -652,   443,  2092,  1442,  1312,  -110,  1248,  -963, -1071,
    -649,  2021, -1071,  1948,  1446,   463,  -109,  1380,  1610,   516,
    1160,   429,  2075,   612,  -770,  -954,   199,  -951,  1050,  -953,
    -996,   406,   407,  1375,  -958,  -581,   573,   582,  -649,   573,
    -763,  -965,   975,   663,   689,   539,   268,   584,  2093,   628,
     575,  1346,  -956,  1204,   580,   405,  2022,   539,   921,  1532,
    1161,  -961,   417,  -999,  -952,   379,   230,  -998,  -649,  -960,
    1684,   870,  -955,  1313,   745,  1241,  -938,  1773,  1774,  -462,
    2070,   447,  -650,  -939,  -860,   579,   463,   595,  -860,  -962,
     642,  -324,   641,   645,  2088,   632,  1290,   868,   606,  -306,
    1249,  1931,  1452,  1113,   635,   -78,   -43,  1272,   -42,  1522,
    -860,  1611,   661,  1530,  1713,  2076,  1715,  -769,  -858,  -392,
    1781,  1868,  1937,  1991,  1575,   939,  1341,  2069,  1345,   940,
    1618,  1620,   956,  1021,   790,   203,  1066,  -324,  1626,   538,
    1628,  2094,  1430,  1657,  1719,   532,   533,   464,   735,  -764,
     731,   432,   532,   533,  1486,   640,  -964,  -954,   828,  -951,
     790,  -953,  -996,  -967,   268,   429,  -958,   538,   944,   627,
    1651,   241,   653,   268,   268,   653,   268,   540,   462,   920,
    1775,   667,   790,  1226,  -956,   354,   463,   743,  1241,   540,
    1506,  1200,  1201,   790,   268,  -999,   790,   538,   502,  -998,
     737,   205,  1012,  1013,  1880,   230,  1881,   422,  -938,   714,
     503,  -462,  1460,  -925,   230,  -939,   644,  -651,   464,   732,
     727,   230,   261,   734,  1273,  -928,   982,   214,    40,  -925,
     386,   799,   442,   230,   126,   214,    40,   264,   746,   748,
     387,  -928,   442,   416,   794,   795,   458,   532,   533,   749,
     750,   751,   752,   754,   755,   756,   757,   758,   759,   760,
     761,   762,   763,   764,   765,   766,   767,   768,   769,   770,
     771,   772,   773,   774,   775,   776,   777,  1736,   779,  1738,
     746,   746,   782,   532,   533,   132,  2001,  1744,   801,  1462,
     271,   700,   802,   803,   804,   805,   806,   807,   808,   809,
     810,   811,   812,   813,   814,  1438,   225,   225,   403,  -765,
     397,  1217,   727,   727,   746,   826,   797,   403,  1607,   423,
    1727,   802,   787,  1539,   830,   669,   424,  2084,   515,   272,
     250,   117,   255,   838,   256,   840,  1932,   800,   273,   983,
    1253,  1254,  1005,   727,  1006,   662,  -965,   856,  1519,   541,
    2101,   857, -1071,   858,   984,  1933,  -126,   112,  1934,   403,
    -126,   947,   948,   403,   365,  1340,   861,   404,  1984,   230,
    2053,   669,   388,   443,   389,  1424,  1425,  -126,  1924,  1284,
     277,   986,  1463,   352,   664,   406,   407,  1985,   686,  1049,
    1986,  1458, -1071,  1894,   406,   407,  1925,  1898,  2002,   390,
     392,   688,  -926,   690,   934,   514,   398,  2054,  2055,  2056,
    1057,   532,   533,  1728,   391,  1926,   403,   395,  -926,   396,
    2085,  1292,  1061,   414,   435,   392,  1608,  1265,   442,  1265,
     392,   392,  1673,   399,   377,   405,   406,   407,  1253,  1254,
     406,   407,   717,  2102,  1502,  1267,  1504,   400,  1269,  1270,
    1509,   213,   401,  1195,   257,  1196,   429,  1197,   392,   417,
     789,  1199,   412,   674,   675,   413,  1473,  1108,  1109,  1475,
     927,   434,   403,   171,    50,  1005,  1006,  1037,  1325,  1326,
     438,   418,  1102,  1104,   419,  1103,   822,   716,   229,   231,
     441,  1381,    55,   406,   407,   442,   658,   566,   660,  1015,
     445,   459,   187,   188,    65,    66,    67,   448,   789,   403,
    1190,   217,   218,   219,   220,   221,   691,   669,   453,   845,
     454,   403,   851,   466,  1751,  1424,  1425,   225,  1040,   669,
     499,   500,   501,   190,   502,   551,    91,  -642,   126,    93,
      94,   467,    95,   191,    97,   468,   503,   469,   230,   406,
     407,  1185,  1186,  1187,   470,  1957,   471,   686,   472,  1960,
    1627,  1048,   700,  1680,  1681,   516,   473,  1188,   108,  -643,
     688,  -644,   690,  1966,   460,   614,   616,   618,   431,   621,
    -647,   213,  2071,  -645,  1488,   670,   406,   407,  -646,   132,
     507,  1060,   117,  1342,  1343,  1344,   117,  1604,   406,   407,
     585,  1468,  1954,  1955,    50,   459,   187,   188,    65,    66,
      67,  1372,  1479,  1374,   689,  1378,   505,   790,   506,  2054,
    2055,  2056,  1106,  1489,   230,  1526,  1343,  1344,   508,   582,
    1704,   790,   790,   437,   439,   440,  2079,  2080,   268,  1982,
    1983,   217,   218,   219,   220,   221,  1978,  1979,   537,  1265,
    -959,  1112,  -648,  1123,   126,  1252,  1255,   410,  1567,  -763,
     543,   431,   550,   230,   548,   230,   225,   503,  1622,    93,
      94,  1717,    95,   191,    97,   225,   443,   556,   460,  -963,
     538,   560,   225,   559,  1031,  2049,   605,  -761,   567,   568,
     576,   230,   431,  1521,   225,   589, -1116,   600,   108,  1705,
     601,   597,   624,   633,   607,   132,   608,   626,  2063,   557,
     623,   636,   126,   637,   686,   565,   646,   459,   187,   188,
      65,    66,    67,   647,   692,  2077,   693,   688,   790,   690,
     790,   702,   703,  1221,   171,  1222,   704,   858,   171,   706,
    -131,   715,   377,   377,   377,   620,   377,   737,  1224,   496,
     497,   498,   499,   500,   501,    55,   502,   742,  1653,   831,
     117,   123,  1233,   132,   230,   833,   664,  1776,   503,   718,
     835,   836,   842,   352,  1662,   843,   859,   126,   578,   863,
     230,   230,   392,   689,   672,   866,   595,   880,  1595,  1291,
     460,  1264,  1261,  1264,   127,   734,   881,   734,   801,   126,
     912,   914,   802,   915,   937,   941,   916,  1624,   918,   129,
     917,   919,   130,   922,   952,   923,  1279,   942,   945,  1280,
     946,  1281,   957,  1745,   954,   727,   960,   962,   132,   973,
     225,   126,   959,   978,   964,   605,   392,   792,   392,   392,
     392,   392,   167,   965,  1242,  2041,   966,   979,   630,  1242,
     132,   967,   981,  1466,  -786,   987,  1467,   638,   733,   643,
     988,   817,   990,   991,   650,  1094,   700,   992,  2041,   995,
     999,  1000,  1008,  1016,  1242,  1322,   668,  2064,   739,   740,
     741,  1010,   132,   605,  1017,  1019,   250,  1022,   255,  1034,
     256,  1023,  1038,  1045,  1329,  1753,  1024,   848,   700,  1042,
    1328,  1046,   171,  1025,  1759,  1026,  1043,  1054,   117,  1683,
    1064,  1062,   126,  1063,   126,   665,  1998,   736,  1036,   671,
    1766,  2003,  -767,  1105,  1115,  1125,  1127,  1129,   909,  1133,
    1134,  1453,  1135,  1137,  1432,  1151,  1242,  1152,  1454,  1153,
     689,   230,   230,  1235,  1732,  1733,   665,  1154,   671,   665,
     671,   671,  1155,  1156,   933,   718,   686,   822,   822,  1455,
    1157,  1158,  1203,   132,  1207,   132,  1193,  1209,  1205,   688,
    2028,   690,  1210,  1211,  1216,  1220,  1229,  1223,   227,   227,
    1231,  1232,   459,    63,    64,    65,    66,    67,  1238,   970,
     972,  1433,  1243,    72,   509,  1434,  1236,   931,   931,  1909,
    1257,  1274,  1182,  1183,  1184,  1185,  1186,  1187,  1283,   225,
     257,   123,   650,  1264,  1286,  1289,  1771,   734,  1294,  1683,
    2065,  1188,  -966,  1295,  2066,  1305,  1306,   126,  1307,  1314,
    1315,  1317,   746,  1308,  1316,  1471,   511,  1309,  2082,  2083,
    1310,  1311,  1319,  1683,   127,  1683,  2090,  1331,   686,  1333,
     171,  1683,  1336,  1337,   851,   460,   851,  1339,   727,   129,
    1348,   688,   130,   690,   117,  1349,  1350,  1356,  1456,   727,
    1434, -1132,   392,  1358,  1997,  1241,  2000,  1359,   132,  1188,
    1241,  1363,  1414,  1362,   230,   225,  1428,  1416,  1417,  1418,
    1419,  1421,   167,  1431,  1448,  1050,  1461,  1464,   535,   582,
    1449,  1465,  1472,  1302,  1484,  1241,  1476,  1474,   268,  1478,
    1481,  1480,   213,  1517,   214,    40,  1483,  1514,  1528,  1490,
    1491,  1492,  1073,  1513,   225,  1515,   225,  1505,  1516,  1518,
    1527,  1523,  1533,  1536,  1540,    50,  1541,  1544,  1545,  1546,
     700,   126,  1549,   700,  1550,  1552,  1553,   230,  1963,  1554,
    1555,   430,   225,   427,   432,  1556,  1562,  1558,  1083,  1612,
    1097,  1613,   230,   230,  1598,  2052,  1637,  1241,  1561,  1559,
    1639,  1615,   217,   218,   219,   220,   221,  1566,  1597,  1568,
     117,   834,   689,  1617,  1569,  1570,  1571,  1606,  1616,  1629,
    1619,  1011,   132,  1621,  1121,   117,  1625,  1631,   815,   227,
      93,    94,  1630,    95,   191,    97,   171,  1632,  1635,  1718,
    1596,  1641,  1646,  1648,  1091,  1636,  1652,  1601,  1640,  1643,
    1644,  1602,   732,  1603,  1645,   225,   734,  1647,  1650,   108,
    1683,  1666,  1663,   816,  1654,  1655,   112,  1656,   117,  1614,
    1659,   225,   225,  1660,  1688,  1726,  1677,  1198,   718,  1716,
    1701,   462,  1623,   727,  1721,  1729,  1730,   230,   459,    63,
      64,    65,    66,    67,  1739,  1740,  1746,  1059,  1734,    72,
     509,  1742,  1764,  1761,   689,  1770,  1876,  1779,  1213,  1778,
    1882,  1589,  1875,  1888,   961,   963,  1889,  1893,   931,  1891,
     931,  1589,   931,  1895,  1903,  1594,   931,  1892,   931,   931,
    1202,  1901,  1904,   117,  1914,  1594,  1098,  1915,  1099,  1935,
     510,  1941,   511,   989,  1944,  1945,  1950,  1972,   605,  1974,
    1976,  2089,   171,  1878,  1980,   117,   512,  1988,   513,  1995,
    1996,   460,   817,   817,  1119,  1989,  1999,   171,   227,  1990,
    1302,  1498,   700,  2004,  1498,  2008,  2009,   227,  1009,  2011,
    -388,  1511,  2012,  2014,   227,  2016,  1930,   117,  2017,  2020,
    2029,  2023,  2030,  2031,  2036,  2043,   227,  2047,   126,  2038,
    2050,  2051,  2060,  2062,  2067,  2068,  2073,   687,  2078,  2086,
     171,  2087,  1725,  2091,   392,  2095,  2096,  1731,  2103,  1710,
     727,   727,  2104,  2106,  1299,  1299,  1083,  2107,  1420,  2046,
     793,  1285,   225,   225,  1228,   796,   791,  1208,   126,  2061,
    1908,  1520,  2059,  1047,  1769,  1661,  1470,   935,  1899,   132,
     290,  1923,  1928,   650,  1219,  1777,  2098,  1940,  1708,   848,
    2072,   848,  1897,  1689,   659,  1503,   117,  1365,   117,  1373,
     117,  1443,  1494,  1757,  1301,   171,   516,  1993,  1495,  1320,
     652,   728,  1091,  2025,   126,  1143,  2018,   292,  1679,   132,
    1423,  1352,  1413,   126,     0,  1354,     0,   171,     0,     0,
     213,     0,  1589,     0,     0,     0,  1138,     0,  1589,     0,
    1589,     0,     0,     0,     0,     0,  1594,   605,     0,  1266,
       0,  1266,  1594,    50,  1594,     0,  1943,   700,     0,   171,
       0,  1130,   227,  1589,  1890,   132,     0,  1136,     0,     0,
       0,     0,     0,     0,   132,     0,   909,  1594,     0,  1883,
       0,     0,     0,     0,  1633,     0,  1634,     0,     0,   591,
     217,   218,   219,   220,   221,   592,     0,     0,     0,  1906,
    1757,  1577,     0,     0,     0,   225,     0,     0,     0,     0,
       0,     0,   190,   117,   126,    91,   345,     0,    93,    94,
     126,    95,   191,    97,     0, -1133,     0,   126,     0,     0,
       0,     0,     0,     0,     0,     0,   349,   931,   171,  1212,
     171,     0,   171,    14,  1119,  1335,     0,   108,   351,     0,
       0,     0,     0,     0,     0,     0,  1589,     0,     0,     0,
       0,     0,     0,     0,     0,   132,     0,     0,   225,     0,
    1594,   132,     0,     0,     0,     0,     0,  1938,   132,     0,
       0,     0,     0,   225,   225,     0,     0,  2034,   224,   224,
       0,     0,     0,  1083,  1083,  1083,  1083,  1083,  1083,   247,
       0,     0,  1083,     0,  1083,     0,     0,     0,  1578,     0,
    1296,  1297,  1298,   213,  1579,   117,   459,  1580,   188,    65,
      66,    67,  1581,  1946,     0,   247,     0,   117,     0,     0,
       0,  1747,     0,  1748,     0,  1749,    50,  1535,     0,     0,
    1750,   227,  1938,     0,     0,     0,     0,     0,     0,  1091,
    1091,  1091,  1091,  1091,  1091,   171,     0,   462,  1091,     0,
    1091,     0,     0,     0,  1582,  1583,     0,  1584,     0,     0,
     126,  1266,     0,   217,   218,   219,   220,   221,   225,     0,
       0,     0,     0,     0,  1318,     0,     0,  1469,     0,   460,
       0,     0,     0,     0,     0,     0,     0,     0,  1585,     0,
       0,    93,    94,     0,    95,   191,    97,     0,     0,     0,
    1572,     0,     0,     0,     0,     0,     0,   227,     0,     0,
       0,   132,     0,     0,   126,     0,     0,     0,  1357,     0,
     108,     0,  1360,     0,     0,     0,     0,     0,     0,  1364,
    1902,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1512,     0,     0,     0,     0,     0,   227,   171,   227,   126,
       0,     0,     0,     0,   126,   650,  1119,     0,     0,   171,
       0,  1577,     0,     0,     0,   132,     0,  1083,     0,  1083,
       0,     0,     0,     0,   227,     0,     0,     0,     0,   126,
       0,   546,   518,   519,   520,   521,   522,   523,   524,   525,
     526,   527,   528,   529,     0,     0,     0,     0,     0,   224,
     132,     0,     0,    14,     0,   132,     0,     0,     0,     0,
       0,  2097,     0,     0,  2035,     0,     0,     0,     0,     0,
    2105,     0,     0,  1091,     0,  1091,   530,   531,  2108,     0,
     132,  2109,     0,     0,     0,   700,     0,     0,     0,     0,
     126,   126,     0,     0,   117,     0,     0,   227,     0,   247,
     650,   247,     0,     0,     0,   352,     0,     0,   700,     0,
       0,  1706,     0,   227,   227,     0,     0,   700,  1578,     0,
    1605,     0,     0,     0,  1579,     0,   459,  1580,   188,    65,
      66,    67,  1581,  1493,   117,     0,     0,  1961,  1962,     0,
       0,   132,   132,     0,  1577,     0,     0,   687,     0,     0,
       0,     0,     0,   532,   533,     0,     0,     0,     0,     0,
     247,     0,     0,     0,  1083,     0,  1083,     0,  1083,     0,
       0,     0,     0,  1083,  1582,  1583,     0,  1584,     0,     0,
     117,  1577,     0,     0,     0,   117,    14,     0,   224,   117,
       0,     0,     0,     0,     0,     0,  1547,   224,     0,   460,
    1551,     0,     0,     0,   224,  1557,     0,     0,  1599,   392,
       0,     0,   605,  1563,     0,   352,   224,   705,     0,     0,
    1091,     0,  1091,    14,  1091,  1864,     0,   224,     0,  1091,
       0,     0,  1871,     0,     0,     0,   171,     0,     0,   352,
       0,   352,     0,     0,     0,     0,     0,   352,     0,     0,
       0,  1578,   247,     0,     0,   247,     0,  1579,     0,   459,
    1580,   188,    65,    66,    67,  1581,     0,     0,     0,     0,
       0,     0,     0,  1083,   227,   227,   171,  1577,     0,     0,
     117,   117,   117,     0,     0,     0,   117,     0,  1578,     0,
       0,     0,     0,   117,  1579,     0,   459,  1580,   188,    65,
      66,    67,  1581,     0,     0,     0,     0,  1582,  1583,     0,
    1584,   247,     0,     0,     0,     0,   687,     0,     0,    14,
       0,     0,   171,     0,     0,     0,  1638,   171,     0,  1091,
    1642,   171,   460,     0,     0,     0,     0,  1649,     0,     0,
       0,  1737,     0,     0,  1582,  1583,     0,  1584,     0,     0,
       0,     0,   224,     0,     0,   546,   518,   519,   520,   521,
     522,   523,   524,   525,   526,   527,   528,   529,     0,   460,
       0,     0,     0,     0,     0,     0,     0,     0,  1741,     0,
       0,     0,     0,     0,  1578,     0,     0,     0,   355,     0,
    1579,     0,   459,  1580,   188,    65,    66,    67,  1581,     0,
     530,   531,     0,     0,   247,   605,   247,     0,     0,   896,
       0,     0,     0,     0,     0,     0,     0,   227,     0,     0,
       0,     0,   171,   171,   171,     0,   352,     0,   171,     0,
    1083,  1083,     0,  1577,     0,   171,   117,     0,     0,     0,
    1582,  1583,   896,  1584,     0,  1967,     0,     0,     0,     0,
       0,     0,  1864,  1864,     0,     0,  1871,  1871,     0,     0,
       0,     0,     0,     0,     0,   460,     0,     0,     0,     0,
     605,     0,     0,   687,  1743,    14,     0,   532,   533,     0,
     227,     0,     0,     0,     0,     0,  1091,  1091,     0,     0,
     117,     0,     0,     0,     0,   227,   227,     0,     0,     0,
       0,   247,   247,     0,     0,     0,     0,     0,     0,     0,
     247,     0,   546,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   117,     0,     0,     0,     0,
     117,   224,     0,     0,     0,     0,     0,  1577,  2033,     0,
    1578,   844,     0,     0,     0,     0,  1579,     0,   459,  1580,
     188,    65,    66,    67,  1581,   117,     0,   530,   531,     0,
       0,  2048,     0,     0,  1027,   518,   519,   520,   521,   522,
     523,   524,   525,   526,   527,   528,   529,     0,   171,    14,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     227,     0,     0,     0,     0,     0,  1582,  1583,     0,  1584,
       0,     0,     0,     0,     0,     0,     0,   224,     0,   530,
     531,     0,     0,     0,     0,     0,   117,   117,     0,     0,
       0,   460,   355,     0,   355,     0,     0,     0,     0,     0,
    1752,     0,   171,     0,   532,   533,     0,    34,    35,    36,
     247,     0,     0,     0,  1578,     0,   224,     0,   224,     0,
    1579,   215,   459,  1580,   188,    65,    66,    67,  1581,     0,
       0,     0,     0,     0,     0,     0,     0,   171,     0,     0,
       0,     0,   171,     0,   224,   896,     0,     0,     0,   213,
    1690,     0,   247,   355,     0,     0,   532,   533,     0,   247,
     247,   896,   896,   896,   896,   896,     0,   171,   943,     0,
    1582,  1583,    50,  1584,   896,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,   687,     0,   222,     0,     0,
       0,   247,     0,    89,    90,   460,     0,     0,     0,     0,
     213,     0,     0,     0,  1900,     0,     0,    99,     0,   217,
     218,   219,   220,   221,     0,     0,     0,   224,  1973,  1975,
    1028,     0,   105,    50,     0,     0,     0,     0,   171,   171,
       0,   247,     0,   224,   224,   456,     0,    93,    94,     0,
      95,   191,    97,     0,     0,   355,  1691,     0,   355,     0,
       0,     0,     0,     0,     0,     0,     0,   247,   247,  1692,
     217,   218,   219,   220,   221,  1693,   108,   224,     0,   226,
     226,     0,     0,     0,   247,     0,     0,   687,     0,     0,
     249,   247,   190,     0,     0,    91,  1694,   247,    93,    94,
       0,    95,  1695,    97,     0,     0,     0,     0,   896,  1027,
     518,   519,   520,   521,   522,   523,   524,   525,   526,   527,
     528,   529,     0,   247,     0,     0,     0,   108,     0,     0,
       0,     0,     0,     0,     0,     0,   474,   475,   476,     0,
       0,     0,     0,   247,     0,     0,     0,   247,     0,     0,
       0,     0,     0,     0,   530,   531,   477,   478,   247,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   503,     0,     0,     0,     0,
       0,     0,     0,     0,   224,   224,     0,   355,     0,   876,
       0,     0,     0,     0,     0,     0,     0,     0,   247,     0,
       0,     0,   247,     0,   247,     0,   213,   247,     0,     0,
       0,   532,   533,     0,     0,     0,     0,     0,     0,     0,
     896,   896,   896,   896,   896,   896,   224,   896,     0,    50,
     896,   896,   896,   896,   896,   896,   896,   896,   896,   896,
     896,   896,   896,   896,   896,   896,   896,   896,   896,   896,
     896,   896,   896,   896,   896,   896,   896,   896,     0,     0,
       0,     0,     0,     0,     0,     0,   217,   218,   219,   220,
     221,     0,     0,     0,     0,   705,   896,     0,     0,     0,
     226,     0,     0,     0,   355,   355,     0,     0,     0,     0,
       0,     0,  1869,   355,    93,    94,  1870,    95,   191,    97,
       0,     0,     0,     0,     0,   474,   475,   476,     0,     0,
    1436,     0,   247,     0,   247,     0,     0,     0,     0,     0,
       0,     0,     0,   108,  1705,   477,   478,   224,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,     0,     0,   247,     0,     0,   247,     0,
       0,     0,     0,     0,   503,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   247,   247,   247,   247,   247,
     247,     0,     0,   224,   247,     0,   247,     0,     0,     0,
     224, -1134, -1134, -1134, -1134, -1134,  1180,  1181,  1182,  1183,
    1184,  1185,  1186,  1187,     0,   224,   224,     0,   896,     0,
       0,     0,     0,     0,     0,     0,     0,  1188,   247,   226,
       0,     0,     0,     0,     0,   247,     0,     0,   226,     0,
     896,     0,   896,     0,     0,   226,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   226,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   896,   226,     0,
       0,     0,     0,     0,     0,  1132,     0,     0,   359,     0,
       0,     0,   355,   355,     0,     0,   213,     0,   968,     0,
     969,     0,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,     0,   247,   247,     0,   504,   247,  1039,    50,
     224,     0,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,   247,   502,
       0,     0,     0,     0,     0,     0,   217,   218,   219,   220,
     221,   503,   249,   517,   518,   519,   520,   521,   522,   523,
     524,   525,   526,   527,   528,   529,     0,     0,     0,   247,
       0,   247,     0,     0,    93,    94,     0,    95,   191,    97,
     355,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   226,     0,     0,     0,   355,   530,   531,
       0,     0,     0,   108,   355,     0,     0,     0,     0,     0,
     355,     0,     0,     0,     0,   247,   247,     0,     0,   247,
       0,     0,     0,     0,     0,   896,     0,   896,     0,   896,
       0,     0,     0,     0,   896,   224,     0,     0,     0,   896,
       0,   896,     0,     0,   896,     0,     0,     0,     0,     0,
     904,     0,     0,     0,     0,     0,   355,   247,   247,     0,
       0,   247,     0,  1162,  1163,  1164,     0,     0,   247,     0,
       0,     0,     0,     0,     0,   532,   533,     0,     0,     0,
       0,     0,     0,   904,  1165,   974,     0,  1166,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,
    1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,     0,
       0,     0,   593,     0,   594,     0,   247,     0,   247,     0,
     247,     0,  1188,     0,     0,   247,     0,   224,     0,     0,
       0,   355,     0,     0,     0,   355,     0,   876,     0,     0,
     355,     0,   247,     0,     0,     0,     0,   896,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   247,
     247,     0,     0,     0,     0,     0,     0,   247,     0,   247,
       0,     0,   226,   599,     0,     0,     0,   474,   475,   476,
       0,     0,     0,     0,     0,   873,     0,     0,     0,     0,
       0,   247,     0,   247,     0,     0,     0,   477,   478,   247,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,   247,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   213,   503,     0,     0,     0,
       0,     0,   896,   896,   896,   355,   874,   355,   226,   896,
     213,   247,     0,  1379,     0,     0,     0,   247,    50,   247,
       0,     0,     0,     0,     0,   721,     0,     0,   359,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,  1090,     0,     0,     0,     0,     0,   226,   355,   226,
       0,   355,     0,     0,     0,   217,   218,   219,   220,   221,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     217,   218,   219,   220,   221,   226,   904,   190,     0,     0,
      91,     0,     0,    93,    94,     0,    95,   191,    97,     0,
     875,     0,   904,   904,   904,   904,   904,     0,    93,    94,
       0,    95,   191,    97,     0,   904,     0,     0,     0,     0,
       0,   355,   108,     0,     0,     0,     0,     0,   355,     0,
       0,   247,  1192,     0,     0,     0,     0,   108,   742,     0,
    1007,     0,     0,     0,   247,     0,     0,     0,   247,     0,
       0,     0,   247,   247,   213,     0,     0,     0,   226,     0,
       0,     0,   873,     0,     0,     0,     0,   247,     0,     0,
       0,     0,  1215,   896,   226,   226,     0,    50,     0,     0,
       0,     0,     0,     0,   896,     0,     0,   872,     0,     0,
     896,   228,   228,     0,   896,     0,   355,   355,     0,  1215,
       0,     0,   253,     0,     0,     0,     0,     0,   226,     0,
       0,     0,   213,     0,   217,   218,   219,   220,   221,     0,
       0,   247,     0,   874,     0,     0,     0,     0,     0,     0,
       0,   355,     0,     0,     0,    50,     0,     0,     0,   904,
       0,     0,    93,    94,     0,    95,   191,    97,   247,     0,
     247,     0,     0,     0,  1275,     0,     0,     0,     0,     0,
       0,     0,     0,   213,     0,     0,   896,     0,     0,     0,
       0,   108,   217,   218,   219,   220,   221,     0,   249,   247,
       0,     0,     0,     0,   993,   994,    50,     0,     0,  1090,
       0,     0,     0,     0,   190,     0,   247,    91,     0,     0,
      93,    94,   247,    95,   191,    97,   247,  1353,   355,   355,
       0,     0,   355,     0,     0,     0,     0,     0,     0,     0,
     247,   247,     0,   217,   218,   219,   220,   221,     0,   108,
       0,     0,     0,     0,     0,   226,   226,     0,     0,     0,
       0,     0,     0,     0,     0,   190,     0,     0,    91,    92,
     355,    93,    94,     0,    95,   191,    97,     0,     0,     0,
       0,   355,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   904,   904,   904,   904,   904,   904,   226,   904,     0,
     108,   904,   904,   904,   904,   904,   904,   904,   904,   904,
     904,   904,   904,   904,   904,   904,   904,   904,   904,   904,
     904,   904,   904,   904,   904,   904,   904,   904,   904,     0,
       0,     0,   228,     0, -1134, -1134, -1134, -1134, -1134,   494,
     495,   496,   497,   498,   499,   500,   501,   904,   502,     0,
       0,     0,     0,     0,     0,   355,     0,     0,     0,     0,
     503,     0,   546,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,     0,     0,     0,     0,     0,
     355,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1141,   721,   474,   475,   476,     0,   226,     0,
       0,     0,     0,     0,   355,     0,   355,   530,   531,     0,
       0,     0,   355,     0,   477,   478,     0,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,     0,     0,     0,     0,  1090,  1090,  1090,  1090,
    1090,  1090,     0,   503,   226,  1090,     0,  1090,     0,     0,
       0,   226,     0,     0,     0,     0,     0,     0,     0,   213,
     355,   228,     0,     0,     0,     0,   226,   226,     0,   904,
     228,     0,     0,     0,   532,   533,   290,   228,     0,     0,
    1227,     0,    50,     0,     0,     0,     0,     0,     0,   228,
       0,   904,     0,   904,     0,     0,     0,  1237,     0,     0,
     253,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1256,     0,     0,   292,     0,     0,     0,     0,   904,   217,
     218,   219,   220,   221,     0,     0,   213,  1027,   518,   519,
     520,   521,   522,   523,   524,   525,   526,   527,   528,   529,
       0,     0,   906,     0,   410,     0,     0,    93,    94,    50,
      95,   191,    97,     0,     0,     0,  1288,  -435,  1576,     0,
       0,   226,     0,     0,   355,     0,   459,   187,   188,    65,
      66,    67,   530,   531,   829,   936,   108,   355,     0,     0,
     411,   355,     0,     0,   253,   591,   217,   218,   219,   220,
     221,   592,     0,     0,     0,     0,     0,     0,     0,     0,
    1968,     0,     0,     0,     0,   213,     0,     0,   190,     0,
       0,    91,   345,     0,    93,    94,     0,    95,   191,    97,
    1090,     0,  1090,     0,     0,   228,     0,     0,    50,     0,
       0,  1347,   349,     0,     0,  1351,     0,     0,     0,   460,
    1355,     0,     0,   108,   351,     0,     0,     0,     0,   532,
     533,   290,     0,     0,   355,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   217,   218,   219,   220,   221,
       0,     0,     0,     0,     0,     0,   904,     0,   904,     0,
     904,   355,   905,   355,     0,   904,   226,     0,   292,     0,
     904,     0,   904,    93,    94,   904,    95,   191,    97,     0,
       0,   213,     0,     0,     0,     0,     0,  1537,     0,  1687,
       0,     0,  1700,     0,     0,   905,     0,     0,     0,     0,
       0,     0,   108,  1036,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   355,     0,     0,     0,   355,
       0,     0,     0,     0,     0,  1457,     0,     0,     0,     0,
       0,     0,     0,   355,   355,     0,     0,     0,     0,     0,
     591,   217,   218,   219,   220,   221,   592,  1090,     0,  1090,
       0,  1090,   213,     0,     0,     0,  1090,     0,   226,     0,
       0,     0,     0,   190,     0,     0,    91,   345,  1482,    93,
      94,  1485,    95,   191,    97,    50, -1133,     0,   904,     0,
       0,     0,   290,     0,   228,     0,     0,   349,  1120,     0,
    1767,  1768,     0,     0,     0,     0,     0,     0,   108,   351,
    1700,     0,     0,     0,  1144,  1145,  1146,  1147,  1148,     0,
       0,     0,   217,   218,   219,   220,   221,  1159,     0,   292,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1534,   213,     0,     0,     0,     0,   371,  1538,     0,
      93,    94,     0,    95,   191,    97,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,  1090,     0,     0,     0,
     228,     0,     0,   598,     0,     0,     0,     0,     0,   108,
       0,     0,     0,   904,   904,   904,     0,     0,     0,     0,
     904,     0,  1917,     0,     0,     0,     0,     0,     0,     0,
    1700,   591,   217,   218,   219,   220,   221,   592,     0,   228,
       0,   228,     0,     0,     0,     0,  1573,  1574,     0,     0,
       0,     0,     0,     0,   190,     0,     0,    91,   345,     0,
      93,    94,     0,    95,   191,    97,     0,   228,   905,     0,
       0,     0,     0,     0,     0,   474,   475,   476,   349,     0,
       0,  1262,     0,     0,   905,   905,   905,   905,   905,   108,
     351,     0,     0,     0,     0,   477,   478,   905,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   503,     0,     0,     0,     0,     0,
     228,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1090,  1090,     0,   228,   228,  1664,  1665,
       0,     0,  1667,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   904,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   904,     0,     0,     0,     0,
     253,   904,     0,     0,     0,   904,     0,     0,     0,     0,
    1685,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1711,     0,  1148,  1368,     0,     0,  1368,     0,     0,
       0,   905,     0,  1382,  1385,  1386,  1387,  1389,  1390,  1391,
    1392,  1393,  1394,  1395,  1396,  1397,  1398,  1399,  1400,  1401,
    1402,  1403,  1404,  1405,  1406,  1407,  1408,  1409,  1410,  1411,
    1412,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     253,     0,     0,     0,     0,     0,     0,   904,  1039,  1422,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    2045,     0,     0,     0,     0,  1760,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1687,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1685,     0,     0,     0,     0,     0,     0,   228,   228,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1685,     0,  1685,     0,     0,     0,
       0,     0,  1685,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   905,   905,   905,   905,   905,   905,   253,
     905,     0,     0,   905,   905,   905,   905,   905,   905,   905,
     905,   905,   905,   905,   905,   905,   905,   905,   905,   905,
     905,   905,   905,   905,   905,   905,   905,   905,   905,   905,
     905,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1919,  1524,     0,     0,     0,     0,     0,     0,     0,   905,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1542,     0,  1543,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   474,   475,
     476,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1564,     0,     0,     0,     0,     0,     0,     0,   477,   478,
     228,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,     0,     0,     0,     0,
       0,     0,   213,     0,   214,    40,     0,   503,     0,     0,
       0,     0,     0,     0,  1942,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,   253,  1953,     0,     0,
       0,  1685,     0,   228,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   228,   228,
       0,   905,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   217,   218,   219,   220,   221,     0,     0,     0,
       0,     0,     0,   905,     0,   905,   280,   281,     0,   282,
     283,     0,     0,   284,   285,   286,   287,     0,   815,     0,
      93,    94,     0,    95,   191,    97,     0,     0,     0,     0,
     905,   288,   289,     0,  2013,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1669,   108,
    1670,     0,  1671,   847,     0,     0,   112,  1672,     0,     0,
     291,  1953,  1674,  2026,  1675,     0,     0,  1676,     0,     0,
       0,  1065,     0,   228,   293,   294,   295,   296,   297,   298,
     299,     0,     0,     0,   213,     0,     0,     0,     0,     0,
     300,     0,     0,     0,     0,     0,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,    50,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,     0,   336,     0,   337,   338,   339,   340,     0,
       0,     0,   341,   602,   217,   218,   219,   220,   221,   603,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     5,     6,     7,     8,     9,   604,     0,     0,     0,
    1762,    10,    93,    94,     0,    95,   191,    97,   346,     0,
     347,     0,     0,   348,     0,   360,   426,    13,   905,     0,
     905,   350,   905,     0,     0,     0,   798,   905,   253,     0,
       0,   108,   905,     0,   905,     0,     0,   905,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,  1910,  1911,  1912,     0,     0,
      55,     0,  1916,     0,     0,     0,     0,     0,     0,   186,
     187,   188,    65,    66,    67,     0,     0,    69,    70,     0,
     253,     0,     0,     0,     0,     0,     0,   189,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
     905,   213,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   190,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   191,    97,     0,    50,     0,    99,     0,     0,   100,
       0,     0,   924,   925,     0,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   109,     0,     0,
       0,     0,     0,   112,   113,   114,   115,     0,     0,     0,
       0,   217,   218,   219,   220,   221,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   280,   281,     0,   282,   283,
       0,     0,   284,   285,   286,   287,   926,     0,     0,    93,
      94,     0,    95,   191,    97,   905,   905,   905,     0,     0,
     288,   289,   905,   290,     0,     0,     0,     0,     0,     0,
       0,  1922,     0,     0,     0,     0,  1977,     0,   108,     0,
       0,     0,     0,     0,     0,     0,     0,  1987,     0,   291,
       0,     0,     0,  1992,     0,     0,     0,  1994,     0,     0,
     292,     0,     0,   293,   294,   295,   296,   297,   298,   299,
       0,     0,     0,   213,     0,     0,     0,     0,     0,   300,
       0,     0,     0,     0,     0,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,    50,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,     0,   336,     0,     0,   338,   339,   340,     0,  2037,
       0,   341,   342,   217,   218,   219,   220,   221,   343,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   344,     0,     0,    91,   345,
       0,    93,    94,     0,    95,   191,    97,   346,     0,   347,
       0,     0,   348,     0,     0,     0,   905,     0,     0,   349,
     350,     0,     5,     6,     7,     8,     9,   905,     0,     0,
     108,   351,    10,   905,     0,  1884,     0,   905,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  2015,    14,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,     0,    42,     0,   905,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,    56,    57,    58,     0,    59,  -205,    60,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,    88,    89,    90,    91,    92,     0,    93,    94,
       0,    95,    96,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,   103,     0,
     104,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,     0,     0,   112,   113,   114,   115,     5,     6,
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
       0,     0,   101,   102,   103,     0,   104,     0,   105,   106,
     107,     0,     0,   108,   109,     0,   110,   111,  1225,     0,
     112,   113,   114,   115,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,     0,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,    56,    57,    58,     0,    59,     0,
      60,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,    88,    89,    90,    91,    92,     0,
      93,    94,     0,    95,    96,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
     103,     0,   104,     0,   105,   106,   107,     0,     0,   108,
     109,     0,   110,   111,  1437,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,     0,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     190,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     191,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
     707,     0,   112,   113,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   190,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   191,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,  1029,     0,   112,   113,
     114,   115,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,     0,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,  -205,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   190,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   191,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,     0,     0,   112,   113,   114,   115,     5,     6,
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
      86,     0,     0,    87,     0,     0,     0,     0,   190,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   191,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   109,     0,   110,   111,  1191,     0,
     112,   113,   114,   115,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,     0,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   190,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   191,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     109,     0,   110,   111,  1239,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,     0,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     190,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     191,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
    1271,     0,   112,   113,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   190,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   191,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,  1330,     0,   112,   113,
     114,   115,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,     0,    42,     0,     0,
       0,    43,    44,    45,    46,  1332,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   190,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   191,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,     0,     0,   112,   113,   114,   115,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,  1525,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   190,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   191,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   109,     0,   110,   111,     0,     0,
     112,   113,   114,   115,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,     0,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   190,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   191,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     109,     0,   110,   111,  1678,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,  -297,    34,    35,    36,    37,    38,
      39,    40,     0,    41,     0,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     190,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     191,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
       0,     0,   112,   113,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   190,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   191,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,  1913,     0,   112,   113,
     114,   115,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,     0,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,  1964,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   190,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   191,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,     0,     0,   112,   113,   114,   115,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,  2007,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   190,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   191,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   109,     0,   110,   111,     0,     0,
     112,   113,   114,   115,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,     0,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   190,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   191,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     109,     0,   110,   111,  2024,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,     0,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     190,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     191,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
    2027,     0,   112,   113,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   190,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   191,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,  2044,     0,   112,   113,
     114,   115,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,     0,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   190,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   191,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,  2099,     0,   112,   113,   114,   115,     5,     6,
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
      86,     0,     0,    87,     0,     0,     0,     0,   190,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   191,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   109,     0,   110,   111,  2100,     0,
     112,   113,   114,   115,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,   574,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,     0,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
       0,    61,    62,   187,   188,    65,    66,    67,     0,    68,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   190,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   191,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     109,     0,   110,   111,     0,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,   860,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,     0,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,     0,    61,    62,   187,
     188,    65,    66,    67,     0,    68,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     190,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     191,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   109,     0,   110,   111,
       0,     0,   112,   113,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,  1122,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,     0,    61,    62,   187,   188,    65,    66,    67,
       0,    68,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   190,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   191,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,     0,     0,     0,   105,   106,   107,     0,
       0,   108,   109,     0,   110,   111,     0,     0,   112,   113,
     114,   115,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,  1756,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,     0,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,     0,    61,
      62,   187,   188,    65,    66,    67,     0,    68,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   190,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   191,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,     0,
       0,     0,   105,   106,   107,     0,     0,   108,   109,     0,
     110,   111,     0,     0,   112,   113,   114,   115,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,  1905,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,     0,    42,     0,     0,     0,    43,    44,    45,
      46,     0,    47,     0,    48,     0,    49,     0,     0,    50,
      51,     0,     0,     0,    52,    53,    54,    55,     0,    57,
      58,     0,    59,     0,     0,    61,    62,   187,   188,    65,
      66,    67,     0,    68,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   190,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   191,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   109,     0,   110,   111,     0,     0,
     112,   113,   114,   115,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,     0,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
       0,    61,    62,   187,   188,    65,    66,    67,     0,    68,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   190,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   191,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     109,     0,   110,   111,     0,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   360,     0,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,   186,   187,
     188,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   189,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     190,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     191,    97,     0,     0,     0,    99,     0,     0,   100,     5,
       6,     7,     8,     9,   101,   102,     0,     0,     0,    10,
     105,   106,   107,     0,     0,   108,   192,     0,   361,     0,
       0,     0,   112,   113,   114,   115,     0,     0,     0,     0,
       0,     0,     0,     0,   722,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,   723,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,     0,   186,   187,   188,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   189,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   190,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   191,
      97,     0,   724,     0,    99,     0,     0,   100,     5,     6,
       7,     8,     9,   101,   102,     0,     0,     0,    10,   105,
     106,   107,     0,     0,   108,   192,     0,     0,     0,     0,
       0,   112,   113,   114,   115,     0,     0,     0,     0,     0,
       0,     0,     0,  1258,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,  1259,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,     0,   186,   187,   188,    65,
      66,    67,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   189,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   190,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   191,    97,
       0,  1260,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,     0,     0,     0,   105,   106,
     107,     0,     0,   108,   192,     5,     6,     7,     8,     9,
     112,   113,   114,   115,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   360,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,     0,   186,   187,   188,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   189,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   190,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   191,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   192,     0,     0,   855,     0,     0,   112,   113,   114,
     115,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   360,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   798,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,   186,
     187,   188,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   189,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   190,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   191,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,     0,     0,
       0,   105,   106,   107,     0,     0,   108,   192,     5,     6,
       7,     8,     9,   112,   113,   114,   115,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   360,   426,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,     0,   186,   187,   188,    65,
      66,    67,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   189,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   190,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   191,    97,
       0,     0,     0,    99,     0,     0,   100,     5,     6,     7,
       8,     9,   101,   102,     0,     0,     0,    10,   105,   106,
     107,     0,     0,   108,   109,     0,     0,     0,     0,     0,
     112,   113,   114,   115,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,   204,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,     0,   186,   187,   188,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   189,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   190,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   191,    97,     0,
       0,     0,    99,     0,     0,   100,     5,     6,     7,     8,
       9,   101,   102,     0,     0,     0,    10,   105,   106,   107,
       0,     0,   108,   192,     0,     0,     0,     0,     0,   112,
     113,   114,   115,     0,     0,     0,     0,     0,     0,     0,
       0,   240,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,     0,   186,   187,   188,    65,    66,    67,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   189,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   190,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   191,    97,     0,     0,
       0,    99,     0,     0,   100,     5,     6,     7,     8,     9,
     101,   102,     0,     0,     0,    10,   105,   106,   107,     0,
       0,   108,   192,     0,     0,     0,     0,     0,   112,   113,
     114,   115,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,     0,   186,   187,   188,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   189,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   190,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   191,    97,     0,     0,     0,
      99,     0,     0,   100,     5,     6,     7,     8,     9,   101,
     102,     0,     0,     0,    10,   105,   106,   107,     0,     0,
     108,   192,     0,   275,     0,     0,     0,   112,   113,   114,
     115,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,     0,   186,   187,   188,    65,    66,    67,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     189,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   190,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   191,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   105,   106,   107,     0,     0,   108,
     192,     0,   278,     0,     0,     0,   112,   113,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   426,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,   186,   187,
     188,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   189,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     190,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     191,    97,     0,     0,     0,    99,     0,     0,   100,     5,
       6,     7,     8,     9,   101,   102,     0,     0,     0,    10,
     105,   106,   107,     0,     0,   108,   109,     0,     0,     0,
       0,     0,   112,   113,   114,   115,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,     0,   186,   187,   188,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   189,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   190,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   191,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,     0,     0,     0,   105,
     106,   107,     0,     0,   108,   192,   572,     0,     0,     0,
       0,   112,   113,   114,   115,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   360,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,     0,   186,   187,   188,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   189,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   190,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   191,    97,     0,     0,     0,
      99,     0,     0,   100,     5,     6,     7,     8,     9,   101,
     102,     0,     0,     0,    10,   105,   106,   107,     0,     0,
     108,   192,     0,     0,     0,     0,     0,   112,   113,   114,
     115,     0,     0,   753,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,     0,   186,   187,   188,    65,    66,    67,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     189,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   190,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   191,    97,     0,     0,     0,    99,
       0,     0,   100,     5,     6,     7,     8,     9,   101,   102,
       0,     0,     0,    10,   105,   106,   107,     0,     0,   108,
     192,     0,     0,     0,     0,     0,   112,   113,   114,   115,
       0,     0,     0,     0,     0,     0,     0,     0,   798,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
       0,   186,   187,   188,    65,    66,    67,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   189,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   190,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   191,    97,     0,     0,     0,    99,     0,
       0,   100,     5,     6,     7,     8,     9,   101,   102,     0,
       0,     0,    10,   105,   106,   107,     0,     0,   108,   192,
       0,     0,     0,     0,     0,   112,   113,   114,   115,     0,
       0,     0,     0,     0,     0,     0,     0,   837,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,     0,
     186,   187,   188,    65,    66,    67,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   189,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   190,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   191,    97,     0,     0,     0,    99,     0,     0,
     100,     5,     6,     7,     8,     9,   101,   102,     0,     0,
       0,    10,   105,   106,   107,     0,     0,   108,   192,     0,
       0,     0,     0,     0,   112,   113,   114,   115,     0,     0,
       0,     0,     0,     0,     0,     0,   839,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,   186,
     187,   188,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   189,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   190,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   191,    97,     0,     0,     0,    99,     0,     0,   100,
       5,     6,     7,     8,     9,   101,   102,     0,     0,     0,
      10,   105,   106,   107,     0,     0,   108,   192,     0,     0,
       0,     0,     0,   112,   113,   114,   115,     0,     0,     0,
       0,     0,     0,     0,     0,  1321,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,   186,   187,
     188,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   189,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     190,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     191,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,     0,     0,     0,
     105,   106,   107,     0,     0,   108,   192,     5,     6,     7,
       8,     9,   112,   113,   114,   115,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   360,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,     0,   186,   187,   188,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   189,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   190,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   191,    97,     0,
       0,     0,    99,     0,     0,   100,     5,     6,     7,     8,
       9,   101,   102,     0,     0,     0,    10,   105,   106,   107,
       0,     0,   108,  1451,     0,     0,     0,     0,     0,   112,
     113,   114,   115,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,     0,   186,   187,   188,    65,    66,    67,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   189,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   190,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   191,    97,     0,     0,
       0,    99,     0,     0,   100,     5,     6,     7,     8,     9,
     101,   102,     0,     0,     0,    10,   105,   106,   107,     0,
       0,   108,   192,     0,     0,     0,     0,     0,   112,   113,
     114,   115,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,   666,    39,    40,     0,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,     0,   186,   187,   188,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   189,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   190,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   191,    97,     0,   280,   281,
      99,   282,   283,   100,     0,   284,   285,   286,   287,   101,
     102,     0,     0,     0,     0,   105,   106,   107,     0,     0,
     108,   192,     0,   288,   289,     0,     0,   112,   113,   114,
     115,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,   291,   502,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   503,   293,   294,   295,   296,
     297,   298,   299,     0,     0,     0,   213,     0,   214,    40,
       0,     0,   300,     0,     0,     0,     0,     0,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,    50,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   213,   336,     0,   785,   338,   339,
     340,     0,     0,     0,   341,   602,   217,   218,   219,   220,
     221,   603,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,   280,   281,     0,   282,   283,     0,   604,   284,
     285,   286,   287,     0,    93,    94,     0,    95,   191,    97,
     346,     0,   347,     0,     0,   348,     0,   288,   289,     0,
       0,     0,     0,   350,   217,   218,   219,   220,   221,     0,
       0,     0,     0,   108,     0,     0,     0,   786,     0,     0,
     112,     0,     0,     0,     0,     0,   291,     0,     0,   926,
       0,     0,    93,    94,     0,    95,   191,    97,     0,     0,
     293,   294,   295,   296,   297,   298,   299,     0,     0,     0,
     213,     0,   214,    40,     0,     0,   300,     0,     0,     0,
       0,   108,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,    50,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,     0,   336,
       0,   337,   338,   339,   340,     0,     0,     0,   341,   602,
     217,   218,   219,   220,   221,   603,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   280,   281,     0,   282,
     283,     0,   604,   284,   285,   286,   287,     0,    93,    94,
       0,    95,   191,    97,   346,     0,   347,     0,     0,   348,
       0,   288,   289,     0,   290,     0,     0,   350,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   108,     0,     0,
       0,   786,     0,     0,   112,     0,     0,     0,     0,     0,
     291,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   292,     0,     0,   293,   294,   295,   296,   297,   298,
     299,     0,     0,     0,   213,     0,     0,     0,     0,     0,
     300,     0,     0,     0,     0,     0,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,    50,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,     0,   336,     0,     0,   338,   339,   340,     0,
       0,     0,   341,   342,   217,   218,   219,   220,   221,   343,
       0,     0,     0,     0,     0,     0,   213,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   344,  1131,     0,    91,
     345,     0,    93,    94,     0,    95,   191,    97,   346,    50,
     347,     0,     0,   348,     0,   280,   281,     0,   282,   283,
     349,   350,   284,   285,   286,   287,     0,     0,     0,     0,
       0,   108,   351,     0,     0,     0,  1959,     0,     0,     0,
     288,   289,     0,   290,     0,     0,   217,   218,   219,   220,
     221,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   190,   291,
       0,    91,     0,     0,    93,    94,     0,    95,   191,    97,
     292,     0,     0,   293,   294,   295,   296,   297,   298,   299,
       0,     0,     0,   213,     0,     0,     0,     0,     0,   300,
       0,     0,     0,   108,     0,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,    50,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,     0,   336,     0,   337,   338,   339,   340,     0,     0,
       0,   341,   342,   217,   218,   219,   220,   221,   343,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   344,     0,     0,    91,   345,
       0,    93,    94,     0,    95,   191,    97,   346,     0,   347,
       0,     0,   348,     0,   280,   281,     0,   282,   283,   349,
     350,   284,   285,   286,   287,     0,     0,     0,     0,     0,
     108,   351,     0,     0,     0,     0,     0,     0,     0,   288,
     289,     0,   290,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,   291,   502,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   292,
       0,   503,   293,   294,   295,   296,   297,   298,   299,     0,
       0,     0,   213,     0,     0,     0,     0,     0,   300,     0,
       0,     0,     0,     0,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,    50,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
       0,   336,     0,     0,   338,   339,   340,     0,     0,     0,
     341,   342,   217,   218,   219,   220,   221,   343,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   344,     0,     0,    91,   345,     0,
      93,    94,     0,    95,   191,    97,   346,     0,   347,     0,
       0,   348,     0,     0,     0,     0,     0,     0,   349,   350,
    1682,     0,     0,     0,   280,   281,     0,   282,   283,   108,
     351,   284,   285,   286,   287,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   288,
     289,     0,   290,  1165,     0,     0,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,   291,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   292,
       0,  1188,   293,   294,   295,   296,   297,   298,   299,     0,
       0,     0,   213,     0,     0,     0,     0,     0,   300,     0,
       0,     0,     0,     0,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,    50,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
       0,   336,     0,     0,   338,   339,   340,     0,     0,     0,
     341,   342,   217,   218,   219,   220,   221,   343,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   344,     0,     0,    91,   345,     0,
      93,    94,     0,    95,   191,    97,   346,     0,   347,     0,
       0,   348,     0,  1782,  1783,  1784,  1785,  1786,   349,   350,
    1787,  1788,  1789,  1790,     0,     0,     0,     0,     0,   108,
     351,     0,     0,     0,     0,     0,     0,  1791,  1792,  1793,
       0,   477,   478,     0,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,  1794,   502,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     503,  1795,  1796,  1797,  1798,  1799,  1800,  1801,     0,     0,
       0,   213,     0,     0,     0,     0,     0,  1802,     0,     0,
       0,     0,     0,  1803,  1804,  1805,  1806,  1807,  1808,  1809,
    1810,  1811,  1812,  1813,    50,  1814,  1815,  1816,  1817,  1818,
    1819,  1820,  1821,  1822,  1823,  1824,  1825,  1826,  1827,  1828,
    1829,  1830,  1831,  1832,  1833,  1834,  1835,  1836,  1837,  1838,
    1839,  1840,  1841,  1842,  1843,  1844,     0,     0,     0,  1845,
    1846,   217,   218,   219,   220,   221,     0,  1847,  1848,  1849,
    1850,  1851,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1852,  1853,  1854,     0,   213,     0,    93,
      94,     0,    95,   191,    97,  1855,     0,  1856,  1857,     0,
    1858,     0,     0,     0,     0,     0,     0,  1859,     0,  1860,
      50,  1861,     0,  1862,  1863,     0,   280,   281,   108,   282,
     283,     0,     0,   284,   285,   286,   287,     0,     0,     0,
       0,     0,     0,  1691,     0,     0,     0,     0,     0,     0,
       0,   288,   289,     0,     0,     0,  1692,   217,   218,   219,
     220,   221,  1693,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   190,
     291,     0,    91,    92,     0,    93,    94,     0,    95,  1695,
      97,     0,     0,     0,   293,   294,   295,   296,   297,   298,
     299,     0,     0,     0,   213,     0,     0,     0,     0,     0,
     300,     0,     0,     0,   108,     0,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,    50,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,     0,   336,     0,  1380,   338,   339,   340,     0,
       0,     0,   341,   602,   217,   218,   219,   220,   221,   603,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     280,   281,     0,   282,   283,     0,   604,   284,   285,   286,
     287,     0,    93,    94,     0,    95,   191,    97,   346,     0,
     347,     0,     0,   348,     0,   288,   289,     0,     0,     0,
       0,   350,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   108,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   291,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   293,   294,
     295,   296,   297,   298,   299,     0,     0,     0,   213,     0,
       0,     0,     0,     0,   300,     0,     0,     0,     0,     0,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,    50,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,     0,   336,     0,     0,
     338,   339,   340,     0,     0,     0,   341,   602,   217,   218,
     219,   220,   221,   603,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     604,     0,     0,     0,     0,     0,    93,    94,     0,    95,
     191,    97,   346,     0,   347,     0,     0,   348,   474,   475,
     476,     0,     0,     0,     0,   350,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   108,     0,     0,   477,   478,
       0,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,   474,   475,   476,     0,
       0,     0,     0,     0,     0,     0,     0,   503,     0,     0,
       0,     0,     0,     0,     0,     0,   477,   478,     0,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,   474,   475,   476,     0,     0,     0,
       0,     0,     0,     0,     0,   503,     0,     0,     0,     0,
       0,     0,     0,     0,   477,   478,     0,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,     0,   503,     0,     0,     0,     0,     0,     0,
       0,     0,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
       0,  1206,     0,   474,   475,   476,     0,     0,     0,     0,
       0,   503,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   477,   478,     0,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,  1282,
     502,   474,   475,   476,     0,     0,     0,     0,     0,     0,
       0,     0,   503,     0,     0,     0,     0,     0,     0,     0,
       0,   477,   478,     0,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,  1293,   502,   474,
     475,   476,     0,     0,     0,     0,     0,     0,     0,     0,
     503,     0,     0,     0,     0,     0,     0,     0,     0,   477,
     478,  1529,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,  1324,   502,   474,   475,   476,
       0,     0,     0,     0,     0,     0,     0,     0,   503,     0,
       0,     0,     0,     0,     0,     0,     0,   477,   478,     0,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,     0,  1723,     0,   474,   475,
     476,     0,     0,     0,     0,     0,   503,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   477,   478,
       0,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,  1724,   502,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   503,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   474,   475,
     476,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   477,   478,
    1530,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,   474,   475,   476,     0,
       0,     0,     0,     0,     0,     0,     0,   503,     0,     0,
       0,     0,     0,     0,     0,     0,   477,   478,   504,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,     0,     0,     0,   474,   475,   476,
       0,     0,     0,     0,     0,   503,     0,     0,   290,     0,
       0,     0,     0,     0,     0,     0,     0,   477,   478,   588,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,   292,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   503,     0,   213,     0,
       0,     0,     0,     0,  1002,     0,     0,   474,   475,   476,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,   477,   478,   590,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,     0,     0,   591,   217,   218,
     219,   220,   221,   592,   290,     0,   503,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   609,     0,     0,
     190,     0,     0,    91,   345,     0,    93,    94,     0,    95,
     191,    97,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   292,     0,     0,   349,     0,     0,     0,     0,     0,
       0,     0,   290,     0,   213,   108,   351,     0,     0,     0,
    1459,     0,     0,     0,     0,     0,     0,     0,   613,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   292,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   213,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   591,   217,   218,   219,   220,   221,   592,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,  1388,     0,     0,     0,   190,   852,     0,    91,
     345,     0,    93,    94,     0,    95,   191,    97,     0,     0,
     882,   883,     0,     0,     0,     0,   884,     0,   885,     0,
     349,   591,   217,   218,   219,   220,   221,   592,     0,     0,
     886,   108,   351,     0,     0,     0,     0,     0,    34,    35,
      36,   213,     0,     0,   190,     0,     0,    91,   345,     0,
      93,    94,   215,    95,   191,    97,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,   349,     0,
       0,     0,     0,     0,  1116,     0,     0,     0,     0,   108,
     351,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,
     887,   888,   889,   890,   891,   892,    29,    81,    82,    83,
      84,    85,     0,  1188,    34,    35,    36,   213,   222,   214,
      40,     0,     0,   190,    89,    90,    91,    92,   215,    93,
      94,     0,    95,   191,    97,     0,     0,     0,    99,     0,
      50,     0,     0,     0,     0,     0,     0,   893,   894,     0,
       0,     0,     0,   105,     0,     0,     0,   216,   108,   895,
       0,     0,   882,   883,     0,     0,     0,     0,   884,     0,
     885,     0,     0,     0,     0,  1117,    75,   217,   218,   219,
     220,   221,   886,    81,    82,    83,    84,    85,     0,     0,
      34,    35,    36,   213,   222,     0,     0,     0,     0,   190,
      89,    90,    91,    92,   215,    93,    94,     0,    95,   191,
      97,     0,     0,     0,    99,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   105,
       0,     0,     0,     0,   108,   223,     0,     0,  1067,  1068,
       0,   112,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   887,   888,   889,   890,   891,   892,  1069,    81,
      82,    83,    84,    85,     0,     0,  1070,  1071,  1072,   213,
     222,     0,     0,     0,     0,   190,    89,    90,    91,    92,
    1073,    93,    94,     0,    95,   191,    97,     0,     0,     0,
      99,     0,    50,     0,     0,     0,     0,     0,     0,   893,
     894,     0,     0,     0,     0,   105,     0,     0,     0,     0,
     108,   895,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1074,  1075,
    1076,  1077,  1078,  1079,    29,     0,     0,     0,     0,     0,
       0,     0,    34,    35,    36,   213,  1080,   214,    40,     0,
       0,   190,     0,     0,    91,    92,   215,    93,    94,     0,
      95,   191,    97,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,  1081,  1082,     0,     0,     0,
       0,     0,     0,     0,     0,   216,   108,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,
    1184,  1185,  1186,  1187,    75,   217,   218,   219,   220,   221,
      29,    81,    82,    83,    84,    85,     0,  1188,    34,    35,
      36,   213,   222,   214,    40,     0,     0,   190,    89,    90,
      91,    92,   215,    93,    94,     0,    95,   191,    97,     0,
       0,     0,    99,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   105,     0,     0,
       0,   216,   108,   223,     0,     0,   629,     0,     0,   112,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   649,
      75,   217,   218,   219,   220,   221,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,   222,     0,
       0,     0,     0,   190,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   191,    97,    29,  1058,     0,    99,     0,
       0,     0,     0,    34,    35,    36,   213,     0,   214,    40,
       0,     0,     0,   105,     0,     0,     0,   215,   108,   223,
       0,     0,     0,     0,     0,   112,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   216,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,
    1183,  1184,  1185,  1186,  1187,    75,   217,   218,   219,   220,
     221,    29,    81,    82,    83,    84,    85,     0,  1188,    34,
      35,    36,   213,   222,   214,    40,     0,     0,   190,    89,
      90,    91,    92,   215,    93,    94,     0,    95,   191,    97,
       0,     0,     0,    99,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   105,     0,
       0,     0,   216,   108,   223,     0,     0,     0,     0,     0,
     112,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1218,    75,   217,   218,   219,   220,   221,    29,    81,    82,
      83,    84,    85,     0,     0,    34,    35,    36,   213,   222,
     214,    40,     0,     0,   190,    89,    90,    91,    92,   215,
      93,    94,     0,    95,   191,    97,     0,     0,     0,    99,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   105,     0,     0,     0,   216,   108,
     223,     0,     0,     0,     0,     0,   112,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,   217,   218,
     219,   220,   221,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   222,     0,     0,     0,     0,
     190,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     191,    97,     0,     0,     0,    99,     0,     0,     0,     0,
     474,   475,   476,     0,     0,     0,     0,     0,     0,     0,
     105,     0,     0,     0,     0,   108,   223,     0,     0,     0,
     477,   478,   112,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,   474,   475,
     476,     0,     0,     0,     0,     0,     0,     0,     0,   503,
       0,     0,     0,     0,     0,     0,     0,     0,   477,   478,
       0,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   503,     0,   474,
     475,   476,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   549,   477,
     478,     0,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,     0,   502,   474,   475,   476,
       0,     0,     0,     0,     0,     0,     0,     0,   503,     0,
       0,     0,     0,     0,     0,     0,   558,   477,   478,     0,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   503,     0,   474,   475,
     476,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   958,   477,   478,
       0,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,   474,   475,   476,     0,
       0,     0,     0,     0,     0,     0,     0,   503,     0,     0,
       0,     0,     0,     0,     0,  1044,   477,   478,     0,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,     0,   502,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   503,     0,   474,   475,   476,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1100,   477,   478,     0,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,  1162,  1163,  1164,     0,     0,
       0,     0,     0,     0,     0,     0,   503,     0,     0,     0,
       0,     0,     0,     0,  1435,     0,  1165,     0,     0,  1166,
    1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,
    1187,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1188,  1162,  1163,  1164,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1477,  1165,     0,     0,  1166,
    1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,
    1187,  1162,  1163,  1164,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1188,     0,     0,     0,     0,     0,
       0,     0,  1165,  1361,     0,  1166,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,  1187,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1188,  1162,  1163,  1164,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1165,  1548,     0,  1166,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,  1187,  1162,  1163,  1164,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1188,     0,     0,     0,     0,     0,     0,     0,  1165,  1560,
       0,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,  1186,  1187,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1188,  1162,  1163,  1164,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1165,  1668,
       0,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,  1186,  1187,    34,    35,    36,   213,     0,   214,    40,
       0,     0,     0,     0,     0,     0,  1188,   215,     0,     0,
       0,     0,     0,     0,     0,  1763,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   244,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   245,     0,     0,
       0,     0,     0,     0,     0,     0,   217,   218,   219,   220,
     221,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,   222,     0,  1765,     0,     0,   190,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   191,    97,
       0,     0,     0,    99,     0,    34,    35,    36,   213,     0,
     214,    40,     0,     0,     0,     0,     0,     0,   105,   680,
       0,     0,     0,   108,   246,     0,     0,     0,     0,     0,
     112,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   216,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,   217,   218,
     219,   220,   221,     0,    81,    82,    83,    84,    85,   503,
       0,    34,    35,    36,   213,   222,   214,    40,     0,     0,
     190,    89,    90,    91,    92,   215,    93,    94,     0,    95,
     191,    97,     0,     0,     0,    99,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     105,     0,     0,     0,   244,   108,   681,     0,     0,     0,
       0,     0,   682,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   217,   218,   219,   220,   221,     0,
      81,    82,    83,    84,    85,   213,     0,     0,     0,     0,
       0,   222,     0,     0,     0,     0,   190,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   191,    97,    50,     0,
       0,    99,     0,     0,     0,     0,   368,   369,     0,     0,
       0,     0,     0,     0,     0,     0,   105,     0,     0,     0,
       0,   108,   246,     0,     0,     0,     0,     0,   112,     0,
       0,     0,     0,     0,     0,   217,   218,   219,   220,   221,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   370,     0,     0,
     371,     0,     0,    93,    94,     0,    95,   191,    97,     0,
     474,   475,   476,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   372,     0,     0,     0,   864,     0,     0,
     477,   478,   108,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,     0,   502,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   503,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   474,   475,   476,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   865,   477,   478,  1041,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
       0,   502,   474,   475,   476,     0,     0,     0,     0,     0,
       0,     0,     0,   503,     0,     0,     0,     0,     0,     0,
       0,     0,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,     0,   502,
    1162,  1163,  1164,     0,     0,     0,     0,     0,     0,     0,
       0,   503,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1165,  1565,     0,  1166,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,
    1182,  1183,  1184,  1185,  1186,  1187,  1162,  1163,  1164,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1188,
       0,     0,     0,     0,     0,     0,     0,  1165,     0,     0,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,
    1186,  1187,   475,   476,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1188,     0,     0,     0,     0,
       0,   477,   478,     0,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,     0,   502,  1163,
    1164,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     503,     0,     0,     0,     0,     0,     0,     0,     0,  1165,
       0,     0,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,
    1184,  1185,  1186,  1187,   476,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1188,     0,     0,
       0,     0,   477,   478,     0,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,  1164,   502,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   503,     0,     0,     0,     0,     0,  1165,     0,     0,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,
    1186,  1187,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1188,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,     0,
     502,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   503,  1166,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,
    1183,  1184,  1185,  1186,  1187,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1188,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1188,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   503,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1188,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,     0,   502,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   503,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,     0,   502,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   503, -1134, -1134, -1134,
   -1134,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,     0,   502,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   503, -1134, -1134,
   -1134, -1134,  1175,  1176,  1177,  1178,  1179,  1180,  1181,  1182,
    1183,  1184,  1185,  1186,  1187,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1188
};

static const yytype_int16 yycheck[] =
{
       5,     6,   167,     8,     9,    10,    11,    12,    13,     4,
      15,    16,    17,    18,   132,    56,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     4,    31,   194,   182,   109,
      44,   109,   109,   124,   737,   172,   173,   574,   418,    44,
     124,   855,     4,   109,   699,    33,  1000,    52,    57,    54,
      98,   418,    57,   418,    59,   103,   104,     4,    46,   508,
       4,   124,   695,    51,   998,    19,    20,   880,   247,   505,
     506,   726,    86,   610,   562,   448,     4,   195,    30,   167,
      30,    86,    30,   131,   988,   502,   537,   538,   534,   534,
       4,   870,  1427,   795,   450,   451,   452,    57,   863,   673,
    1020,   841,  1022,   539,   109,     9,  1229,  1115,   696,     9,
     623,   624,   192,     9,   192,   192,  1238,   568,   258,  1035,
     543,    32,   259,     9,   570,   570,     9,   368,   369,   370,
       9,   372,     9,     9,     9,  1898,     9,  1053,    14,    14,
       9,    14,     9,   223,     9,   223,   223,     9,     9,     9,
       9,     9,    70,   132,     9,     9,    48,    36,     9,    48,
      70,     9,     9,    83,    32,     9,   246,    48,   246,    54,
     124,     9,     9,     9,    83,   107,   108,  1111,    48,   116,
      70,    83,    38,   107,   108,  1101,    81,   192,    38,  1002,
     168,   162,   162,    38,   199,   136,   137,    38,   136,   137,
     168,   162,    30,   181,    50,    51,   103,    91,   123,    48,
     183,   162,  1958,   183,  1960,   103,   195,   132,   223,     0,
     183,    70,   183,    38,   202,    91,   199,    83,   199,   199,
      70,    38,   202,    83,   202,    70,   199,   132,    83,   167,
     162,   246,    83,   180,   162,    70,   199,    70,   199,    70,
      70,   160,   161,  1157,    70,     8,   261,   271,    70,   264,
     162,   199,   203,   403,   418,    70,   271,   272,    83,   166,
     265,   203,    70,   928,   269,   159,    83,    70,   166,   203,
     202,   199,   167,    70,   202,   205,   240,    70,    70,   199,
    1578,   202,   202,   159,   461,   998,    70,   201,   202,    70,
    2063,   201,    70,    70,   196,   201,    70,   183,   200,   199,
     390,   200,   390,   390,  2077,   201,  1056,   200,   359,   200,
     176,   200,  1242,   860,   201,   201,   201,  1030,   201,  1337,
     200,   176,   201,   201,   201,   176,   201,   162,   184,   201,
     201,   201,   201,   201,  1679,   200,  1125,   201,  1127,   200,
    1472,  1474,   200,   200,   508,   360,   200,   196,  1481,   199,
    1483,   176,   200,   200,   200,   136,   137,   202,   448,   162,
     448,   448,   136,   137,  1290,   389,   199,   202,   545,   202,
     534,   202,   202,   199,   389,   390,   202,   199,   200,   103,
    1513,   396,   397,   398,   399,   400,   401,   202,   516,   103,
    1688,   406,   556,   977,   202,   446,    70,   455,  1111,   202,
    1314,   924,   925,   567,   419,   202,   570,   199,    57,   202,
     202,   426,    83,    84,  1712,   379,  1714,    83,   202,   434,
      69,   202,  1245,   183,   388,   202,   390,    70,   202,   448,
     445,   395,   199,   448,  1032,   183,    54,    83,    84,   199,
     122,   517,   166,   407,   433,    83,    84,   199,   463,   464,
     132,   199,   166,   511,   512,   513,   514,   136,   137,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,  1619,   503,  1621,
     505,   506,   507,   136,   137,   433,    14,  1630,   517,    83,
     199,   425,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,  1227,    19,    20,    83,   162,
      70,   954,   537,   538,   539,   540,   515,    83,   202,   195,
      83,   546,   991,  1356,   549,    91,   202,    83,   713,   199,
     502,     4,   502,   558,   502,   560,    31,   517,   199,   167,
    1006,  1006,   702,   568,   704,    70,   199,   576,  1333,   205,
      83,   576,   162,   578,   182,    50,   162,   205,    53,    83,
     166,    50,    51,    83,   202,  1122,   581,    91,    31,   543,
      87,    91,   199,   183,   199,   103,   104,   183,    14,  1050,
      53,   681,   176,    56,   159,   160,   161,    50,   988,   788,
      53,  1244,   202,  1735,   160,   161,    32,  1739,  1953,   199,
      73,   988,   183,   988,   629,   713,    70,   124,   125,   126,
     797,   136,   137,   176,   199,    51,    83,   199,   199,   199,
     176,  1058,   821,    96,    91,    98,  1459,  1020,   166,  1022,
     103,   104,  1556,    70,    60,   159,   160,   161,  1104,  1104,
     160,   161,   208,   176,  1309,  1021,  1311,    70,  1024,  1025,
    1315,    81,    70,   914,   502,   916,   681,   918,   131,   167,
     508,   922,    88,   201,   202,    91,  1274,    75,    76,  1277,
    1203,   202,    83,     4,   104,   835,   836,   745,    75,    76,
      91,   199,   842,   843,   199,   842,   534,   207,    19,    20,
      32,  1160,   112,   160,   161,   166,   399,   896,   401,   724,
     199,   121,   122,   123,   124,   125,   126,   199,   556,    83,
     909,   141,   142,   143,   144,   145,   419,    91,   118,   567,
      38,    83,   570,   201,  1648,   103,   104,   240,   753,    91,
      53,    54,    55,   163,    57,   208,   166,    70,   737,   169,
     170,   201,   172,   173,   174,   201,    69,   201,   722,   160,
     161,    53,    54,    55,   201,  1897,   201,  1157,   201,  1901,
    1482,   786,   696,   134,   135,   713,   201,    69,   198,    70,
    1157,    70,  1157,   203,   194,   368,   369,   370,   109,   372,
      70,    81,    87,    70,  1292,   159,   160,   161,    70,   737,
     202,   816,   265,   106,   107,   108,   269,  1450,   160,   161,
     273,  1257,   201,   202,   104,   121,   122,   123,   124,   125,
     126,  1154,  1283,  1156,   988,  1158,    70,   991,    70,   124,
     125,   126,   847,  1294,   798,   106,   107,   108,   162,   863,
     130,  1005,  1006,   113,   114,   115,   201,   202,   863,  1929,
    1930,   141,   142,   143,   144,   145,  1925,  1926,   199,  1242,
     199,   859,    70,   868,   853,  1005,  1006,   166,  1415,   162,
     199,   192,    49,   837,   201,   839,   379,    69,  1476,   169,
     170,  1593,   172,   173,   174,   388,   183,   162,   194,   199,
     199,     9,   395,   204,  1607,   201,   359,   162,   162,   199,
       8,   865,   223,  1336,   407,   201,   162,    14,   198,   199,
     162,   199,     9,    14,   201,   853,   201,   201,  2050,   240,
     202,   132,   911,   132,  1314,   246,   200,   121,   122,   123,
     124,   125,   126,   183,    14,  2067,   103,  1314,  1102,  1314,
    1104,   200,   200,   958,   265,   960,   200,   962,   269,   200,
     199,   206,   368,   369,   370,   371,   372,   202,   973,    50,
      51,    52,    53,    54,    55,   112,    57,   199,  1515,   199,
     433,   976,   987,   911,   938,     9,   159,  1689,    69,   442,
     200,   200,   200,   446,  1531,   200,    95,   976,     9,   201,
     954,   955,   455,  1157,   410,    14,   183,   199,  1431,  1057,
     194,  1020,  1017,  1022,   976,  1020,     9,  1022,  1027,   998,
     199,   202,  1027,   201,    83,   200,   202,  1478,   202,   976,
     201,   201,   976,   202,   134,   201,  1041,   200,   200,  1044,
     201,  1046,   200,  1631,   199,  1050,     9,     9,   976,    70,
     543,  1030,   204,    32,   204,   508,   509,   510,   511,   512,
     513,   514,   976,   204,  1998,  2019,   204,   135,   379,  2003,
     998,   204,   182,  1252,   162,   138,  1255,   388,  1451,   390,
       9,   534,   200,   162,   395,   831,  1000,   200,  2042,    14,
     196,     9,     9,   200,  2028,  1100,   407,  2051,  1454,  1455,
    1456,   184,  1030,   556,     9,    14,  1058,     9,  1058,   134,
    1058,   200,   204,   203,  1109,  1652,   200,   570,  1032,   204,
    1108,     9,   433,   200,  1661,   200,   204,    14,   581,  1578,
     204,   200,  1111,   200,  1113,   404,  1950,   448,   199,   408,
    1677,  1955,   162,   200,   103,   201,   201,     9,   601,   138,
     162,  1242,     9,   200,  1220,   199,  2090,    70,  1242,    70,
    1314,  1115,  1116,   991,  1615,  1616,   435,    70,   437,   438,
     439,   440,    70,    70,   627,   628,  1556,  1005,  1006,  1242,
     199,   199,     9,  1111,    14,  1113,   202,   201,   203,  1556,
    2004,  1556,   184,     9,   202,    14,   202,   204,    19,    20,
      14,   200,   121,   122,   123,   124,   125,   126,   196,   662,
     663,  1220,    32,   132,   133,  1220,   201,   623,   624,  1756,
      70,   199,    50,    51,    52,    53,    54,    55,   199,   722,
    1058,  1226,   543,  1242,    32,    14,  1685,  1242,   199,  1688,
    2053,    69,   199,    14,  2057,    52,   199,  1226,    70,   199,
     199,     9,  1257,    70,   162,  1260,   175,    70,  2071,  2072,
      70,    70,   200,  1712,  1226,  1714,  2080,   201,  1648,   201,
     581,  1720,   199,   138,  1102,   194,  1104,    14,  1283,  1226,
     184,  1648,  1226,  1648,   737,   138,   162,     9,  1242,  1294,
    1295,   176,   745,   200,  1949,  1998,  1951,   176,  1226,    69,
    2003,     9,    83,   204,  1258,   798,     9,   203,   203,   203,
     203,   201,  1226,   199,   138,   199,    83,    14,  1455,  1333,
     201,    83,   200,  1069,   201,  2028,   199,   202,  1333,   199,
     202,   200,    81,  1328,    83,    84,   202,  1325,  1343,   138,
     204,     9,    92,   202,   837,    32,   839,   159,    77,   201,
     201,   200,   184,   138,    32,   104,   200,   200,   204,     9,
    1274,  1340,   204,  1277,     9,   204,   204,  1321,  1905,   204,
     138,  1451,   865,  1451,  1451,     9,     9,   200,   831,    14,
     833,    83,  1336,  1337,   202,  2040,     9,  2090,   203,   200,
     138,   199,   141,   142,   143,   144,   145,   200,   203,   201,
     853,   552,  1556,   204,   201,   201,   201,   201,   199,   201,
     200,   722,  1340,   200,   867,   868,   200,   199,   167,   240,
     169,   170,   202,   172,   173,   174,   737,   200,   200,  1594,
    1435,     9,   138,     9,   831,   204,    32,  1442,   204,   204,
     204,  1446,  1451,  1448,   204,   938,  1451,   200,   200,   198,
    1899,   176,   138,   202,   201,   200,   205,   200,   911,  1464,
     201,   954,   955,   201,   113,    83,   202,   920,   921,   201,
     171,  1589,  1477,  1478,   167,    14,    83,  1431,   121,   122,
     123,   124,   125,   126,   200,   200,   138,   798,   119,   132,
     133,   202,   138,   200,  1648,    14,    83,   202,   951,   183,
      14,  1429,   201,    14,   655,   656,    83,   199,   914,   200,
     916,  1439,   918,   198,   138,  1429,   922,   200,   924,   925,
     926,   200,   138,   976,   201,  1439,   837,   201,   839,    14,
     173,    14,   175,   684,   201,    14,   202,     9,   991,     9,
     203,  2078,   853,  1710,    68,   998,   189,    83,   191,    83,
       9,   194,  1005,  1006,   865,   183,     9,   868,   379,   199,
    1306,  1307,  1476,   202,  1310,   201,   116,   388,   719,   162,
     103,  1317,   103,   184,   395,   174,    36,  1030,    14,   199,
     201,   200,   199,   180,   184,    83,   407,   177,  1567,   184,
     200,     9,    83,   201,   200,   200,    83,   418,   202,    14,
     911,    83,  1607,    83,  1057,    14,    83,  1612,    14,  1588,
    1615,  1616,    83,    14,  1067,  1068,  1069,    83,  1203,  2031,
     511,  1051,  1115,  1116,   979,   514,   509,   938,  1607,  2047,
    1755,  1334,  2042,   784,  1682,  1528,  1259,   631,  1742,  1567,
      31,  1780,  1867,   954,   955,  1690,  2088,  1879,  1584,  1102,
    2064,  1104,  1738,  1580,   400,  1310,  1109,  1151,  1111,  1155,
    1113,  1230,  1305,  1658,  1068,   976,  1594,  1943,  1306,  1097,
     396,   446,  1069,  1999,  1653,   881,  1989,    68,  1572,  1607,
    1211,  1134,  1189,  1662,    -1,  1135,    -1,   998,    -1,    -1,
      81,    -1,  1620,    -1,    -1,    -1,    87,    -1,  1626,    -1,
    1628,    -1,    -1,    -1,    -1,    -1,  1620,  1160,    -1,  1020,
      -1,  1022,  1626,   104,  1628,    -1,  1883,  1631,    -1,  1030,
      -1,   872,   543,  1651,  1729,  1653,    -1,   878,    -1,    -1,
      -1,    -1,    -1,    -1,  1662,    -1,  1189,  1651,    -1,  1718,
      -1,    -1,    -1,    -1,  1490,    -1,  1492,    -1,    -1,   140,
     141,   142,   143,   144,   145,   146,    -1,    -1,    -1,  1754,
    1755,     6,    -1,    -1,    -1,  1258,    -1,    -1,    -1,    -1,
      -1,    -1,   163,  1226,  1753,   166,   167,    -1,   169,   170,
    1759,   172,   173,   174,    -1,   176,    -1,  1766,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   187,  1203,  1109,   950,
    1111,    -1,  1113,    48,  1115,  1116,    -1,   198,   199,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1744,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1753,    -1,    -1,  1321,    -1,
    1744,  1759,    -1,    -1,    -1,    -1,    -1,  1878,  1766,    -1,
      -1,    -1,    -1,  1336,  1337,    -1,    -1,  2012,    19,    20,
      -1,    -1,    -1,  1306,  1307,  1308,  1309,  1310,  1311,    30,
      -1,    -1,  1315,    -1,  1317,    -1,    -1,    -1,   113,    -1,
      78,    79,    80,    81,   119,  1328,   121,   122,   123,   124,
     125,   126,   127,  1888,    -1,    56,    -1,  1340,    -1,    -1,
      -1,  1637,    -1,  1639,    -1,  1641,   104,  1350,    -1,    -1,
    1646,   722,  1943,    -1,    -1,    -1,    -1,    -1,    -1,  1306,
    1307,  1308,  1309,  1310,  1311,  1226,    -1,  2035,  1315,    -1,
    1317,    -1,    -1,    -1,   169,   170,    -1,   172,    -1,    -1,
    1909,  1242,    -1,   141,   142,   143,   144,   145,  1431,    -1,
      -1,    -1,    -1,    -1,  1095,    -1,    -1,  1258,    -1,   194,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   203,    -1,
      -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,
    1423,    -1,    -1,    -1,    -1,    -1,    -1,   798,    -1,    -1,
      -1,  1909,    -1,    -1,  1963,    -1,    -1,    -1,  1139,    -1,
     198,    -1,  1143,    -1,    -1,    -1,    -1,    -1,    -1,  1150,
    1746,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1321,    -1,    -1,    -1,    -1,    -1,   837,  1328,   839,  1998,
      -1,    -1,    -1,    -1,  2003,  1336,  1337,    -1,    -1,  1340,
      -1,     6,    -1,    -1,    -1,  1963,    -1,  1490,    -1,  1492,
      -1,    -1,    -1,    -1,   865,    -1,    -1,    -1,    -1,  2028,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,   240,
    1998,    -1,    -1,    48,    -1,  2003,    -1,    -1,    -1,    -1,
      -1,  2086,    -1,    -1,  2012,    -1,    -1,    -1,    -1,    -1,
    2095,    -1,    -1,  1490,    -1,  1492,    59,    60,  2103,    -1,
    2028,  2106,    -1,    -1,    -1,  2019,    -1,    -1,    -1,    -1,
    2089,  2090,    -1,    -1,  1567,    -1,    -1,   938,    -1,   290,
    1431,   292,    -1,    -1,    -1,  1578,    -1,    -1,  2042,    -1,
      -1,  1584,    -1,   954,   955,    -1,    -1,  2051,   113,    -1,
    1451,    -1,    -1,    -1,   119,    -1,   121,   122,   123,   124,
     125,   126,   127,  1304,  1607,    -1,    -1,  1903,  1904,    -1,
      -1,  2089,  2090,    -1,     6,    -1,    -1,   988,    -1,    -1,
      -1,    -1,    -1,   136,   137,    -1,    -1,    -1,    -1,    -1,
     351,    -1,    -1,    -1,  1637,    -1,  1639,    -1,  1641,    -1,
      -1,    -1,    -1,  1646,   169,   170,    -1,   172,    -1,    -1,
    1653,     6,    -1,    -1,    -1,  1658,    48,    -1,   379,  1662,
      -1,    -1,    -1,    -1,    -1,    -1,  1367,   388,    -1,   194,
    1371,    -1,    -1,    -1,   395,  1376,    -1,    -1,   203,  1682,
      -1,    -1,  1685,  1384,    -1,  1688,   407,   200,    -1,    -1,
    1637,    -1,  1639,    48,  1641,  1698,    -1,   418,    -1,  1646,
      -1,    -1,  1705,    -1,    -1,    -1,  1567,    -1,    -1,  1712,
      -1,  1714,    -1,    -1,    -1,    -1,    -1,  1720,    -1,    -1,
      -1,   113,   443,    -1,    -1,   446,    -1,   119,    -1,   121,
     122,   123,   124,   125,   126,   127,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1746,  1115,  1116,  1607,     6,    -1,    -1,
    1753,  1754,  1755,    -1,    -1,    -1,  1759,    -1,   113,    -1,
      -1,    -1,    -1,  1766,   119,    -1,   121,   122,   123,   124,
     125,   126,   127,    -1,    -1,    -1,    -1,   169,   170,    -1,
     172,   502,    -1,    -1,    -1,    -1,  1157,    -1,    -1,    48,
      -1,    -1,  1653,    -1,    -1,    -1,  1497,  1658,    -1,  1746,
    1501,  1662,   194,    -1,    -1,    -1,    -1,  1508,    -1,    -1,
      -1,   203,    -1,    -1,   169,   170,    -1,   172,    -1,    -1,
      -1,    -1,   543,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,   194,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   203,    -1,
      -1,    -1,    -1,    -1,   113,    -1,    -1,    -1,    56,    -1,
     119,    -1,   121,   122,   123,   124,   125,   126,   127,    -1,
      59,    60,    -1,    -1,   595,  1878,   597,    -1,    -1,   600,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1258,    -1,    -1,
      -1,    -1,  1753,  1754,  1755,    -1,  1899,    -1,  1759,    -1,
    1903,  1904,    -1,     6,    -1,  1766,  1909,    -1,    -1,    -1,
     169,   170,   633,   172,    -1,  1918,    -1,    -1,    -1,    -1,
      -1,    -1,  1925,  1926,    -1,    -1,  1929,  1930,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   194,    -1,    -1,    -1,    -1,
    1943,    -1,    -1,  1314,   203,    48,    -1,   136,   137,    -1,
    1321,    -1,    -1,    -1,    -1,    -1,  1903,  1904,    -1,    -1,
    1963,    -1,    -1,    -1,    -1,  1336,  1337,    -1,    -1,    -1,
      -1,   692,   693,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     701,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,  1998,    -1,    -1,    -1,    -1,
    2003,   722,    -1,    -1,    -1,    -1,    -1,     6,  2011,    -1,
     113,   200,    -1,    -1,    -1,    -1,   119,    -1,   121,   122,
     123,   124,   125,   126,   127,  2028,    -1,    59,    60,    -1,
      -1,  2034,    -1,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,  1909,    48,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1431,    -1,    -1,    -1,    -1,    -1,   169,   170,    -1,   172,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   798,    -1,    59,
      60,    -1,    -1,    -1,    -1,    -1,  2089,  2090,    -1,    -1,
      -1,   194,   290,    -1,   292,    -1,    -1,    -1,    -1,    -1,
     203,    -1,  1963,    -1,   136,   137,    -1,    78,    79,    80,
     831,    -1,    -1,    -1,   113,    -1,   837,    -1,   839,    -1,
     119,    92,   121,   122,   123,   124,   125,   126,   127,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1998,    -1,    -1,
      -1,    -1,  2003,    -1,   865,   866,    -1,    -1,    -1,    81,
      31,    -1,   873,   351,    -1,    -1,   136,   137,    -1,   880,
     881,   882,   883,   884,   885,   886,    -1,  2028,   200,    -1,
     169,   170,   104,   172,   895,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,    -1,  1556,    -1,   158,    -1,    -1,
      -1,   912,    -1,   164,   165,   194,    -1,    -1,    -1,    -1,
      81,    -1,    -1,    -1,   203,    -1,    -1,   178,    -1,   141,
     142,   143,   144,   145,    -1,    -1,    -1,   938,  1919,  1920,
     200,    -1,   193,   104,    -1,    -1,    -1,    -1,  2089,  2090,
      -1,   952,    -1,   954,   955,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,   443,   127,    -1,   446,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   978,   979,   140,
     141,   142,   143,   144,   145,   146,   198,   988,    -1,    19,
      20,    -1,    -1,    -1,   995,    -1,    -1,  1648,    -1,    -1,
      30,  1002,   163,    -1,    -1,   166,   167,  1008,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    -1,    -1,  1019,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,  1034,    -1,    -1,    -1,   198,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,  1054,    -1,    -1,    -1,  1058,    -1,    -1,
      -1,    -1,    -1,    -1,    59,    60,    30,    31,  1069,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1115,  1116,    -1,   595,    -1,   597,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1129,    -1,
      -1,    -1,  1133,    -1,  1135,    -1,    81,  1138,    -1,    -1,
      -1,   136,   137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,    -1,   104,
    1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,    -1,    -1,    -1,    -1,   200,  1207,    -1,    -1,    -1,
     240,    -1,    -1,    -1,   692,   693,    -1,    -1,    -1,    -1,
      -1,    -1,   167,   701,   169,   170,   171,   172,   173,   174,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
     204,    -1,  1243,    -1,  1245,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   198,   199,    30,    31,  1258,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,  1286,    -1,    -1,  1289,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1306,  1307,  1308,  1309,  1310,
    1311,    -1,    -1,  1314,  1315,    -1,  1317,    -1,    -1,    -1,
    1321,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,  1336,  1337,    -1,  1339,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,  1349,   379,
      -1,    -1,    -1,    -1,    -1,  1356,    -1,    -1,   388,    -1,
    1361,    -1,  1363,    -1,    -1,   395,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   407,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1388,   418,    -1,
      -1,    -1,    -1,    -1,    -1,   873,    -1,    -1,    56,    -1,
      -1,    -1,   880,   881,    -1,    -1,    81,    -1,    83,    -1,
      85,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1424,  1425,    -1,   201,  1428,   203,   104,
    1431,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,  1459,    57,
      -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,    69,   502,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,    -1,    -1,  1490,
      -1,  1492,    -1,    -1,   169,   170,    -1,   172,   173,   174,
     978,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   543,    -1,    -1,    -1,   995,    59,    60,
      -1,    -1,    -1,   198,  1002,    -1,    -1,    -1,    -1,    -1,
    1008,    -1,    -1,    -1,    -1,  1536,  1537,    -1,    -1,  1540,
      -1,    -1,    -1,    -1,    -1,  1546,    -1,  1548,    -1,  1550,
      -1,    -1,    -1,    -1,  1555,  1556,    -1,    -1,    -1,  1560,
      -1,  1562,    -1,    -1,  1565,    -1,    -1,    -1,    -1,    -1,
     600,    -1,    -1,    -1,    -1,    -1,  1054,  1578,  1579,    -1,
      -1,  1582,    -1,    10,    11,    12,    -1,    -1,  1589,    -1,
      -1,    -1,    -1,    -1,    -1,   136,   137,    -1,    -1,    -1,
      -1,    -1,    -1,   633,    31,   203,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,   290,    -1,   292,    -1,  1637,    -1,  1639,    -1,
    1641,    -1,    69,    -1,    -1,  1646,    -1,  1648,    -1,    -1,
      -1,  1129,    -1,    -1,    -1,  1133,    -1,  1135,    -1,    -1,
    1138,    -1,  1663,    -1,    -1,    -1,    -1,  1668,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1680,
    1681,    -1,    -1,    -1,    -1,    -1,    -1,  1688,    -1,  1690,
      -1,    -1,   722,   351,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,
      -1,  1712,    -1,  1714,    -1,    -1,    -1,    30,    31,  1720,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,  1746,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    81,    69,    -1,    -1,    -1,
      -1,    -1,  1763,  1764,  1765,  1243,    92,  1245,   798,  1770,
      81,  1772,    -1,   200,    -1,    -1,    -1,  1778,   104,  1780,
      -1,    -1,    -1,    -1,    -1,   443,    -1,    -1,   446,    -1,
      -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   831,    -1,    -1,    -1,    -1,    -1,   837,  1286,   839,
      -1,  1289,    -1,    -1,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   865,   866,   163,    -1,    -1,
     166,    -1,    -1,   169,   170,    -1,   172,   173,   174,    -1,
     176,    -1,   882,   883,   884,   885,   886,    -1,   169,   170,
      -1,   172,   173,   174,    -1,   895,    -1,    -1,    -1,    -1,
      -1,  1349,   198,    -1,    -1,    -1,    -1,    -1,  1356,    -1,
      -1,  1882,   912,    -1,    -1,    -1,    -1,   198,   199,    -1,
     203,    -1,    -1,    -1,  1895,    -1,    -1,    -1,  1899,    -1,
      -1,    -1,  1903,  1904,    81,    -1,    -1,    -1,   938,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    -1,  1918,    -1,    -1,
      -1,    -1,   952,  1924,   954,   955,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,  1935,    -1,    -1,   595,    -1,    -1,
    1941,    19,    20,    -1,  1945,    -1,  1424,  1425,    -1,   979,
      -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,   988,    -1,
      -1,    -1,    81,    -1,   141,   142,   143,   144,   145,    -1,
      -1,  1972,    -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1459,    -1,    -1,    -1,   104,    -1,    -1,    -1,  1019,
      -1,    -1,   169,   170,    -1,   172,   173,   174,  1999,    -1,
    2001,    -1,    -1,    -1,  1034,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,  2017,    -1,    -1,    -1,
      -1,   198,   141,   142,   143,   144,   145,    -1,  1058,  2030,
      -1,    -1,    -1,    -1,   692,   693,   104,    -1,    -1,  1069,
      -1,    -1,    -1,    -1,   163,    -1,  2047,   166,    -1,    -1,
     169,   170,  2053,   172,   173,   174,  2057,   176,  1536,  1537,
      -1,    -1,  1540,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    2071,  2072,    -1,   141,   142,   143,   144,   145,    -1,   198,
      -1,    -1,    -1,    -1,    -1,  1115,  1116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,   166,   167,
    1578,   169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,
      -1,  1589,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,    -1,
     198,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,  1179,
    1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,    -1,
      -1,    -1,   240,    -1,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,  1207,    57,    -1,
      -1,    -1,    -1,    -1,    -1,  1663,    -1,    -1,    -1,    -1,
      69,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,
    1688,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   880,   881,    10,    11,    12,    -1,  1258,    -1,
      -1,    -1,    -1,    -1,  1712,    -1,  1714,    59,    60,    -1,
      -1,    -1,  1720,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,  1306,  1307,  1308,  1309,
    1310,  1311,    -1,    69,  1314,  1315,    -1,  1317,    -1,    -1,
      -1,  1321,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,
    1778,   379,    -1,    -1,    -1,    -1,  1336,  1337,    -1,  1339,
     388,    -1,    -1,    -1,   136,   137,    31,   395,    -1,    -1,
     978,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,   407,
      -1,  1361,    -1,  1363,    -1,    -1,    -1,   995,    -1,    -1,
     418,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1008,    -1,    -1,    68,    -1,    -1,    -1,    -1,  1388,   141,
     142,   143,   144,   145,    -1,    -1,    81,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,   600,    -1,   166,    -1,    -1,   169,   170,   104,
     172,   173,   174,    -1,    -1,    -1,  1054,   112,  1428,    -1,
      -1,  1431,    -1,    -1,  1882,    -1,   121,   122,   123,   124,
     125,   126,    59,    60,   200,   633,   198,  1895,    -1,    -1,
     202,  1899,    -1,    -1,   502,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1918,    -1,    -1,    -1,    -1,    81,    -1,    -1,   163,    -1,
      -1,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
    1490,    -1,  1492,    -1,    -1,   543,    -1,    -1,   104,    -1,
      -1,  1129,   187,    -1,    -1,  1133,    -1,    -1,    -1,   194,
    1138,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,   136,
     137,    31,    -1,    -1,  1972,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,    -1,    -1,    -1,  1546,    -1,  1548,    -1,
    1550,  1999,   600,  2001,    -1,  1555,  1556,    -1,    68,    -1,
    1560,    -1,  1562,   169,   170,  1565,   172,   173,   174,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    87,    -1,  1579,
      -1,    -1,  1582,    -1,    -1,   633,    -1,    -1,    -1,    -1,
      -1,    -1,   198,   199,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  2053,    -1,    -1,    -1,  2057,
      -1,    -1,    -1,    -1,    -1,  1243,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  2071,  2072,    -1,    -1,    -1,    -1,    -1,
     140,   141,   142,   143,   144,   145,   146,  1637,    -1,  1639,
      -1,  1641,    81,    -1,    -1,    -1,  1646,    -1,  1648,    -1,
      -1,    -1,    -1,   163,    -1,    -1,   166,   167,  1286,   169,
     170,  1289,   172,   173,   174,   104,   176,    -1,  1668,    -1,
      -1,    -1,    31,    -1,   722,    -1,    -1,   187,   866,    -1,
    1680,  1681,    -1,    -1,    -1,    -1,    -1,    -1,   198,   199,
    1690,    -1,    -1,    -1,   882,   883,   884,   885,   886,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   895,    -1,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1349,    81,    -1,    -1,    -1,    -1,   166,  1356,    -1,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,  1746,    -1,    -1,    -1,
     798,    -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,   198,
      -1,    -1,    -1,  1763,  1764,  1765,    -1,    -1,    -1,    -1,
    1770,    -1,  1772,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1780,   140,   141,   142,   143,   144,   145,   146,    -1,   837,
      -1,   839,    -1,    -1,    -1,    -1,  1424,  1425,    -1,    -1,
      -1,    -1,    -1,    -1,   163,    -1,    -1,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,   865,   866,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,   187,    -1,
      -1,  1019,    -1,    -1,   882,   883,   884,   885,   886,   198,
     199,    -1,    -1,    -1,    -1,    30,    31,   895,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
     938,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1903,  1904,    -1,   954,   955,  1536,  1537,
      -1,    -1,  1540,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1924,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1935,    -1,    -1,    -1,    -1,
     988,  1941,    -1,    -1,    -1,  1945,    -1,    -1,    -1,    -1,
    1578,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1589,    -1,  1151,  1152,    -1,    -1,  1155,    -1,    -1,
      -1,  1019,    -1,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,
    1188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1058,    -1,    -1,    -1,    -1,    -1,    -1,  2017,   203,  1207,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    2030,    -1,    -1,    -1,    -1,  1663,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  2047,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1688,    -1,    -1,    -1,    -1,    -1,    -1,  1115,  1116,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1712,    -1,  1714,    -1,    -1,    -1,
      -1,    -1,  1720,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1151,  1152,  1153,  1154,  1155,  1156,  1157,
    1158,    -1,    -1,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,
    1188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1778,  1339,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1207,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1361,    -1,  1363,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1388,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
    1258,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    83,    84,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,  1882,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,  1314,  1895,    -1,    -1,
      -1,  1899,    -1,  1321,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1336,  1337,
      -1,  1339,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,
      -1,    -1,    -1,  1361,    -1,  1363,     3,     4,    -1,     6,
       7,    -1,    -1,    10,    11,    12,    13,    -1,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,
    1388,    28,    29,    -1,  1972,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1546,   198,
    1548,    -1,  1550,   202,    -1,    -1,   205,  1555,    -1,    -1,
      57,  1999,  1560,  2001,  1562,    -1,    -1,  1565,    -1,    -1,
      -1,   203,    -1,  1431,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,    -1,   130,    -1,   132,   133,   134,   135,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,   146,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,     7,   163,    -1,    -1,    -1,
    1668,    13,   169,   170,    -1,   172,   173,   174,   175,    -1,
     177,    -1,    -1,   180,    -1,    27,    28,    29,  1546,    -1,
    1548,   188,  1550,    -1,    -1,    -1,    38,  1555,  1556,    -1,
      -1,   198,  1560,    -1,  1562,    -1,    -1,  1565,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,  1763,  1764,  1765,    -1,    -1,
     112,    -1,  1770,    -1,    -1,    -1,    -1,    -1,    -1,   121,
     122,   123,   124,   125,   126,    -1,    -1,   129,   130,    -1,
    1648,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
    1668,    81,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,   104,    -1,   178,    -1,    -1,   181,
      -1,    -1,   112,   113,    -1,   187,   188,    -1,    -1,    -1,
      -1,   193,   194,   195,    -1,    -1,   198,   199,    -1,    -1,
      -1,    -1,    -1,   205,   206,   207,   208,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,    -1,    10,    11,    12,    13,   166,    -1,    -1,   169,
     170,    -1,   172,   173,   174,  1763,  1764,  1765,    -1,    -1,
      28,    29,  1770,    31,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1779,    -1,    -1,    -1,    -1,  1924,    -1,   198,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1935,    -1,    57,
      -1,    -1,    -1,  1941,    -1,    -1,    -1,  1945,    -1,    -1,
      68,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,    -1,   130,    -1,    -1,   133,   134,   135,    -1,  2017,
      -1,   139,   140,   141,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,   175,    -1,   177,
      -1,    -1,   180,    -1,    -1,    -1,  1924,    -1,    -1,   187,
     188,    -1,     3,     4,     5,     6,     7,  1935,    -1,    -1,
     198,   199,    13,  1941,    -1,   203,    -1,  1945,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1974,    48,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    -1,    88,    -1,  2017,
      -1,    92,    93,    94,    95,    -1,    97,    -1,    99,    -1,
     101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,
     111,   112,   113,   114,   115,    -1,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,   129,   130,
     131,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,
     181,    -1,    -1,    -1,    -1,    -1,   187,   188,   189,    -1,
     191,    -1,   193,   194,   195,    -1,    -1,   198,   199,    -1,
     201,   202,    -1,    -1,   205,   206,   207,   208,     3,     4,
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
      -1,    -1,   187,   188,   189,    -1,   191,    -1,   193,   194,
     195,    -1,    -1,   198,   199,    -1,   201,   202,   203,    -1,
     205,   206,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,
      99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,
     109,   110,   111,   112,   113,   114,   115,    -1,   117,    -1,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
     129,   130,   131,   132,   133,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,
      -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,   175,    -1,    -1,   178,
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,
     189,    -1,   191,    -1,   193,   194,   195,    -1,    -1,   198,
     199,    -1,   201,   202,   203,    -1,   205,   206,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,    92,
      93,    94,    95,    -1,    97,    -1,    99,    -1,   101,    -1,
      -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,   112,
      -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,   129,   130,    -1,   132,
     133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,    -1,
      -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,
     193,   194,   195,    -1,    -1,   198,   199,    -1,   201,   202,
     203,    -1,   205,   206,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,    -1,
      97,    -1,    99,    -1,   101,    -1,    -1,   104,   105,    -1,
      -1,    -1,   109,   110,   111,   112,    -1,   114,   115,    -1,
     117,    -1,    -1,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,   129,   130,    -1,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,    -1,
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,   188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,
      -1,   198,   199,    -1,   201,   202,   203,    -1,   205,   206,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,
      -1,    92,    93,    94,    95,    -1,    97,    -1,    99,    -1,
     101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,
     111,   112,    -1,   114,   115,    -1,   117,   118,    -1,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,   129,   130,
      -1,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,
     181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,
      -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,    -1,
     201,   202,    -1,    -1,   205,   206,   207,   208,     3,     4,
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
      -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,
     195,    -1,    -1,   198,   199,    -1,   201,   202,   203,    -1,
     205,   206,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,
      99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,
     109,   110,   111,   112,    -1,   114,   115,    -1,   117,    -1,
      -1,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
     129,   130,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,
      -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,   175,    -1,    -1,   178,
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,
      -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,
     199,    -1,   201,   202,   203,    -1,   205,   206,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,    92,
      93,    94,    95,    -1,    97,    -1,    99,    -1,   101,    -1,
      -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,   112,
      -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,   129,   130,    -1,   132,
     133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,    -1,
      -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,
     193,   194,   195,    -1,    -1,   198,   199,    -1,   201,   202,
     203,    -1,   205,   206,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,    -1,
      97,    -1,    99,    -1,   101,    -1,    -1,   104,   105,    -1,
      -1,    -1,   109,   110,   111,   112,    -1,   114,   115,    -1,
     117,    -1,    -1,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,   129,   130,    -1,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,    -1,
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,   188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,
      -1,   198,   199,    -1,   201,   202,   203,    -1,   205,   206,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    -1,    99,    -1,
     101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,
     111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,   129,   130,
      -1,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,
     181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,
      -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,    -1,
     201,   202,    -1,    -1,   205,   206,   207,   208,     3,     4,
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
      -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,
     195,    -1,    -1,   198,   199,    -1,   201,   202,    -1,    -1,
     205,   206,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,
      99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,
     109,   110,   111,   112,    -1,   114,   115,    -1,   117,    -1,
      -1,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
     129,   130,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,
      -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,   175,    -1,    -1,   178,
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,
      -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,
     199,    -1,   201,   202,   203,    -1,   205,   206,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    77,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,    92,
      93,    94,    95,    -1,    97,    -1,    99,    -1,   101,    -1,
      -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,   112,
      -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,   129,   130,    -1,   132,
     133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,    -1,
      -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,
     193,   194,   195,    -1,    -1,   198,   199,    -1,   201,   202,
      -1,    -1,   205,   206,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,    -1,
      97,    -1,    99,    -1,   101,    -1,    -1,   104,   105,    -1,
      -1,    -1,   109,   110,   111,   112,    -1,   114,   115,    -1,
     117,    -1,    -1,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,   129,   130,    -1,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,    -1,
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,   188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,
      -1,   198,   199,    -1,   201,   202,   203,    -1,   205,   206,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,
      -1,    92,    93,    94,    95,    -1,    97,    -1,    99,   100,
     101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,
     111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,   129,   130,
      -1,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,
     181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,
      -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,    -1,
     201,   202,    -1,    -1,   205,   206,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,    94,
      95,    -1,    97,    98,    99,    -1,   101,    -1,    -1,   104,
     105,    -1,    -1,    -1,   109,   110,   111,   112,    -1,   114,
     115,    -1,   117,    -1,    -1,   120,   121,   122,   123,   124,
     125,   126,    -1,   128,   129,   130,    -1,   132,   133,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
     155,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,
     195,    -1,    -1,   198,   199,    -1,   201,   202,    -1,    -1,
     205,   206,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,
      99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,
     109,   110,   111,   112,    -1,   114,   115,    -1,   117,    -1,
      -1,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
     129,   130,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,
      -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,   175,    -1,    -1,   178,
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,
      -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,
     199,    -1,   201,   202,   203,    -1,   205,   206,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,    92,
      93,    94,    95,    -1,    97,    -1,    99,    -1,   101,    -1,
      -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,   112,
      -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,   129,   130,    -1,   132,
     133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,    -1,
      -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,
     193,   194,   195,    -1,    -1,   198,   199,    -1,   201,   202,
     203,    -1,   205,   206,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,    -1,
      97,    -1,    99,    -1,   101,    -1,    -1,   104,   105,    -1,
      -1,    -1,   109,   110,   111,   112,    -1,   114,   115,    -1,
     117,    -1,    -1,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,   129,   130,    -1,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,    -1,
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,   188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,
      -1,   198,   199,    -1,   201,   202,   203,    -1,   205,   206,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,
      -1,    92,    93,    94,    95,    -1,    97,    -1,    99,    -1,
     101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,
     111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,   129,   130,
      -1,   132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,
     181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,
      -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,    -1,
     201,   202,   203,    -1,   205,   206,   207,   208,     3,     4,
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
      -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,
     195,    -1,    -1,   198,   199,    -1,   201,   202,   203,    -1,
     205,   206,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,
      99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,
     109,   110,   111,   112,    -1,   114,   115,    -1,   117,    -1,
      -1,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
     129,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,
      -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,
      -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,
     199,    -1,   201,   202,    -1,    -1,   205,   206,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,    92,
      93,    94,    95,    -1,    97,    -1,    99,    -1,   101,    -1,
      -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,   112,
      -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,   122,
     123,   124,   125,   126,    -1,   128,   129,   130,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,   178,    -1,    -1,   181,    -1,
      -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,
     193,   194,   195,    -1,    -1,   198,   199,    -1,   201,   202,
      -1,    -1,   205,   206,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,    -1,
      97,    -1,    99,    -1,   101,    -1,    -1,   104,   105,    -1,
      -1,    -1,   109,   110,   111,   112,    -1,   114,   115,    -1,
     117,    -1,    -1,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,   129,   130,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,   188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,
      -1,   198,   199,    -1,   201,   202,    -1,    -1,   205,   206,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,
      -1,    92,    93,    94,    95,    -1,    97,    -1,    99,    -1,
     101,    -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,
     111,   112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,
     121,   122,   123,   124,   125,   126,    -1,   128,   129,   130,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,    -1,
     181,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,
      -1,    -1,   193,   194,   195,    -1,    -1,   198,   199,    -1,
     201,   202,    -1,    -1,   205,   206,   207,   208,     3,     4,
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
      -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,
     195,    -1,    -1,   198,   199,    -1,   201,   202,    -1,    -1,
     205,   206,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    -1,    -1,    92,    93,    94,    95,    -1,    97,    -1,
      99,    -1,   101,    -1,    -1,   104,   105,    -1,    -1,    -1,
     109,   110,   111,   112,    -1,   114,   115,    -1,   117,    -1,
      -1,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
     129,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    -1,   147,   148,
     149,   150,   151,    -1,    -1,    -1,   155,    -1,    -1,   158,
      -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,
      -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,
     199,    -1,   201,   202,    -1,    -1,   205,   206,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    -1,    29,    -1,    -1,    -1,
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
     173,   174,    -1,    -1,    -1,   178,    -1,    -1,   181,     3,
       4,     5,     6,     7,   187,   188,    -1,    -1,    -1,    13,
     193,   194,   195,    -1,    -1,   198,   199,    -1,   201,    -1,
      -1,    -1,   205,   206,   207,   208,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    87,    -1,    -1,    -1,    -1,    92,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,   122,   123,
     124,   125,   126,    -1,    -1,   129,   130,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,    -1,   147,   148,   149,   150,   151,    -1,    -1,
      -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,
     164,   165,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,   176,    -1,   178,    -1,    -1,   181,     3,     4,
       5,     6,     7,   187,   188,    -1,    -1,    -1,    13,   193,
     194,   195,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,
      -1,   205,   206,   207,   208,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
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
      -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,   194,
     195,    -1,    -1,   198,   199,     3,     4,     5,     6,     7,
     205,   206,   207,   208,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,
     198,   199,    -1,    -1,   202,    -1,    -1,   205,   206,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,
      -1,   193,   194,   195,    -1,    -1,   198,   199,     3,     4,
       5,     6,     7,   205,   206,   207,   208,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
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
      -1,    -1,    -1,   178,    -1,    -1,   181,     3,     4,     5,
       6,     7,   187,   188,    -1,    -1,    -1,    13,   193,   194,
     195,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,    -1,
     205,   206,   207,   208,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
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
      -1,    -1,   178,    -1,    -1,   181,     3,     4,     5,     6,
       7,   187,   188,    -1,    -1,    -1,    13,   193,   194,   195,
      -1,    -1,   198,   199,    -1,    -1,    -1,    -1,    -1,   205,
     206,   207,   208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   178,    -1,    -1,   181,     3,     4,     5,     6,     7,
     187,   188,    -1,    -1,    -1,    13,   193,   194,   195,    -1,
      -1,   198,   199,    -1,    -1,    -1,    -1,    -1,   205,   206,
     207,   208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     178,    -1,    -1,   181,     3,     4,     5,     6,     7,   187,
     188,    -1,    -1,    -1,    13,   193,   194,   195,    -1,    -1,
     198,   199,    -1,   201,    -1,    -1,    -1,   205,   206,   207,
     208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   188,
      -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,   198,
     199,    -1,   201,    -1,    -1,    -1,   205,   206,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,
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
     173,   174,    -1,    -1,    -1,   178,    -1,    -1,   181,     3,
       4,     5,     6,     7,   187,   188,    -1,    -1,    -1,    13,
     193,   194,   195,    -1,    -1,   198,   199,    -1,    -1,    -1,
      -1,    -1,   205,   206,   207,   208,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,   193,
     194,   195,    -1,    -1,   198,   199,   200,    -1,    -1,    -1,
      -1,   205,   206,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     178,    -1,    -1,   181,     3,     4,     5,     6,     7,   187,
     188,    -1,    -1,    -1,    13,   193,   194,   195,    -1,    -1,
     198,   199,    -1,    -1,    -1,    -1,    -1,   205,   206,   207,
     208,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   181,     3,     4,     5,     6,     7,   187,   188,
      -1,    -1,    -1,    13,   193,   194,   195,    -1,    -1,   198,
     199,    -1,    -1,    -1,    -1,    -1,   205,   206,   207,   208,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,
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
      -1,   181,     3,     4,     5,     6,     7,   187,   188,    -1,
      -1,    -1,    13,   193,   194,   195,    -1,    -1,   198,   199,
      -1,    -1,    -1,    -1,    -1,   205,   206,   207,   208,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
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
     181,     3,     4,     5,     6,     7,   187,   188,    -1,    -1,
      -1,    13,   193,   194,   195,    -1,    -1,   198,   199,    -1,
      -1,    -1,    -1,    -1,   205,   206,   207,   208,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,
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
       3,     4,     5,     6,     7,   187,   188,    -1,    -1,    -1,
      13,   193,   194,   195,    -1,    -1,   198,   199,    -1,    -1,
      -1,    -1,    -1,   205,   206,   207,   208,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,    -1,
     193,   194,   195,    -1,    -1,   198,   199,     3,     4,     5,
       6,     7,   205,   206,   207,   208,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   121,   122,   123,   124,   125,
     126,    -1,    -1,   129,   130,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
      -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,
      -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,    -1,
      -1,    -1,   178,    -1,    -1,   181,     3,     4,     5,     6,
       7,   187,   188,    -1,    -1,    -1,    13,   193,   194,   195,
      -1,    -1,   198,   199,    -1,    -1,    -1,    -1,    -1,   205,
     206,   207,   208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   178,    -1,    -1,   181,     3,     4,     5,     6,     7,
     187,   188,    -1,    -1,    -1,    13,   193,   194,   195,    -1,
      -1,   198,   199,    -1,    -1,    -1,    -1,    -1,   205,   206,
     207,   208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   169,   170,    -1,   172,   173,   174,    -1,     3,     4,
     178,     6,     7,   181,    -1,    10,    11,    12,    13,   187,
     188,    -1,    -1,    -1,    -1,   193,   194,   195,    -1,    -1,
     198,   199,    -1,    28,    29,    -1,    -1,   205,   206,   207,
     208,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    57,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    83,    84,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,    81,   130,    -1,   132,   133,   134,
     135,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,   163,    10,
      11,    12,    13,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,   177,    -1,    -1,   180,    -1,    28,    29,    -1,
      -1,    -1,    -1,   188,   141,   142,   143,   144,   145,    -1,
      -1,    -1,    -1,   198,    -1,    -1,    -1,   202,    -1,    -1,
     205,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,   166,
      -1,    -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    83,    84,    -1,    -1,    87,    -1,    -1,    -1,
      -1,   198,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,    -1,   130,
      -1,   132,   133,   134,   135,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,
       7,    -1,   163,    10,    11,    12,    13,    -1,   169,   170,
      -1,   172,   173,   174,   175,    -1,   177,    -1,    -1,   180,
      -1,    28,    29,    -1,    31,    -1,    -1,   188,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,    -1,    -1,
      -1,   202,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,    -1,   130,    -1,    -1,   133,   134,   135,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,   146,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   163,    92,    -1,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,   104,
     177,    -1,    -1,   180,    -1,     3,     4,    -1,     6,     7,
     187,   188,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,   198,   199,    -1,    -1,    -1,   203,    -1,    -1,    -1,
      28,    29,    -1,    31,    -1,    -1,   141,   142,   143,   144,
     145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,    57,
      -1,   166,    -1,    -1,   169,   170,    -1,   172,   173,   174,
      68,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,   198,    -1,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,    -1,   130,    -1,   132,   133,   134,   135,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,   175,    -1,   177,
      -1,    -1,   180,    -1,     3,     4,    -1,     6,     7,   187,
     188,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
     198,   199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      29,    -1,    31,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    57,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    69,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,    -1,
      -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
      -1,   130,    -1,    -1,   133,   134,   135,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,   146,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   163,    -1,    -1,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,   175,    -1,   177,    -1,
      -1,   180,    -1,    -1,    -1,    -1,    -1,    -1,   187,   188,
     189,    -1,    -1,    -1,     3,     4,    -1,     6,     7,   198,
     199,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      29,    -1,    31,    31,    -1,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    69,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,    -1,
      -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
      -1,   130,    -1,    -1,   133,   134,   135,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,   146,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   163,    -1,    -1,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,   175,    -1,   177,    -1,
      -1,   180,    -1,     3,     4,     5,     6,     7,   187,   188,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   198,
     199,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    57,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,
      -1,    -1,    -1,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   163,   164,   165,    -1,    81,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,   177,   178,    -1,
     180,    -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,   189,
     104,   191,    -1,   193,   194,    -1,     3,     4,   198,     6,
       7,    -1,    -1,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,    -1,   127,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    29,    -1,    -1,    -1,   140,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,
      57,    -1,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,   198,    -1,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,    -1,   130,    -1,   132,   133,   134,   135,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,   146,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       3,     4,    -1,     6,     7,    -1,   163,    10,    11,    12,
      13,    -1,   169,   170,    -1,   172,   173,   174,   175,    -1,
     177,    -1,    -1,   180,    -1,    28,    29,    -1,    -1,    -1,
      -1,   188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   198,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,    -1,   130,    -1,    -1,
     133,   134,   135,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     163,    -1,    -1,    -1,    -1,    -1,   169,   170,    -1,   172,
     173,   174,   175,    -1,   177,    -1,    -1,   180,    10,    11,
      12,    -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   198,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,
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
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,   203,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,   203,
      57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,   203,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,   203,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,   203,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,   203,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
     201,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   201,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   201,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    87,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    30,    31,   201,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,   140,   141,   142,
     143,   144,   145,   146,    31,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   201,    -1,    -1,
     163,    -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    81,   198,   199,    -1,    -1,    -1,
      87,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   201,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   140,   141,   142,   143,   144,   145,   146,
      -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    32,    -1,    -1,    -1,   163,   200,    -1,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    -1,
     187,   140,   141,   142,   143,   144,   145,   146,    -1,    -1,
      70,   198,   199,    -1,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    -1,    -1,   163,    -1,    -1,   166,   167,    -1,
     169,   170,    92,   172,   173,   174,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,   187,    -1,
      -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,   198,
     199,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
     140,   141,   142,   143,   144,   145,    70,   147,   148,   149,
     150,   151,    -1,    69,    78,    79,    80,    81,   158,    83,
      84,    -1,    -1,   163,   164,   165,   166,   167,    92,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,
      -1,    -1,    -1,   193,    -1,    -1,    -1,   121,   198,   199,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,    70,   147,   148,   149,   150,   151,    -1,    -1,
      78,    79,    80,    81,   158,    -1,    -1,    -1,    -1,   163,
     164,   165,   166,   167,    92,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,   178,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,
      -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    50,    51,
      -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   140,   141,   142,   143,   144,   145,    70,   147,
     148,   149,   150,   151,    -1,    -1,    78,    79,    80,    81,
     158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,
      92,   169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,
     178,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,   187,
     188,    -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,    -1,
     198,   199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,   158,    83,    84,    -1,
      -1,   163,    -1,    -1,   166,   167,    92,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,   187,   188,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   121,   198,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,   140,   141,   142,   143,   144,   145,
      70,   147,   148,   149,   150,   151,    -1,    69,    78,    79,
      80,    81,   158,    83,    84,    -1,    -1,   163,   164,   165,
     166,   167,    92,   169,   170,    -1,   172,   173,   174,    -1,
      -1,    -1,   178,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,    -1,    -1,
      -1,   121,   198,   199,    -1,    -1,   202,    -1,    -1,   205,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    70,    71,    -1,   178,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,    -1,   193,    -1,    -1,    -1,    92,   198,   199,
      -1,    -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   121,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,   140,   141,   142,   143,   144,
     145,    70,   147,   148,   149,   150,   151,    -1,    69,    78,
      79,    80,    81,   158,    83,    84,    -1,    -1,   163,   164,
     165,   166,   167,    92,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,   178,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,    -1,
      -1,    -1,   121,   198,   199,    -1,    -1,    -1,    -1,    -1,
     205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    70,   147,   148,
     149,   150,   151,    -1,    -1,    78,    79,    80,    81,   158,
      83,    84,    -1,    -1,   163,   164,   165,   166,   167,    92,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,   121,   198,
     199,    -1,    -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,   178,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     193,    -1,    -1,    -1,    -1,   198,   199,    -1,    -1,    -1,
      30,    31,   205,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
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
      -1,    -1,    -1,    -1,    -1,    -1,   138,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   138,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   138,    -1,    31,    -1,    -1,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,    31,    -1,    -1,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,   138,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,   138,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,   138,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,   138,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    92,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   121,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,    -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,
      -1,    -1,    -1,   158,    -1,   138,    -1,    -1,   163,   164,
     165,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,   178,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,   193,    92,
      -1,    -1,    -1,   198,   199,    -1,    -1,    -1,    -1,    -1,
     205,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    69,
      -1,    78,    79,    80,    81,   158,    83,    84,    -1,    -1,
     163,   164,   165,   166,   167,    92,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,   178,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     193,    -1,    -1,    -1,   121,   198,   199,    -1,    -1,    -1,
      -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    81,    -1,    -1,    -1,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   104,    -1,
      -1,   178,    -1,    -1,    -1,    -1,   112,   113,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,
      -1,   198,   199,    -1,    -1,    -1,    -1,    -1,   205,    -1,
      -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,
     166,    -1,    -1,   169,   170,    -1,   172,   173,   174,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   189,    -1,    -1,    -1,    27,    -1,    -1,
      30,    31,   198,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    30,    31,    32,    33,    34,    35,
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
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
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
      -1,    -1,    -1,    -1,    -1,    69,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   210,   211,     0,   212,     3,     4,     5,     6,     7,
      13,    27,    28,    29,    48,    50,    51,    56,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    70,
      71,    72,    73,    74,    78,    79,    80,    81,    82,    83,
      84,    86,    88,    92,    93,    94,    95,    97,    99,   101,
     104,   105,   109,   110,   111,   112,   113,   114,   115,   117,
     119,   120,   121,   122,   123,   124,   125,   126,   128,   129,
     130,   131,   132,   133,   139,   140,   141,   142,   143,   144,
     145,   147,   148,   149,   150,   151,   155,   158,   163,   164,
     165,   166,   167,   169,   170,   172,   173,   174,   175,   178,
     181,   187,   188,   189,   191,   193,   194,   195,   198,   199,
     201,   202,   205,   206,   207,   208,   213,   216,   226,   227,
     228,   229,   230,   236,   245,   246,   257,   258,   262,   265,
     272,   278,   339,   340,   348,   349,   352,   353,   354,   355,
     356,   357,   358,   359,   361,   362,   363,   365,   368,   380,
     381,   388,   391,   394,   397,   400,   403,   409,   411,   412,
     414,   424,   425,   426,   428,   433,   438,   458,   466,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,   482,   495,   497,   499,   121,   122,   123,   139,
     163,   173,   199,   216,   257,   339,   361,   470,   361,   199,
     361,   361,   361,   361,   109,   361,   361,   456,   457,   361,
     361,   361,   361,    81,    83,    92,   121,   141,   142,   143,
     144,   145,   158,   199,   227,   381,   425,   428,   433,   470,
     474,   470,   361,   361,   361,   361,   361,   361,   361,   361,
      38,   361,   486,   487,   121,   132,   199,   227,   270,   425,
     426,   427,   429,   433,   467,   468,   469,   478,   483,   484,
     361,   199,   360,   430,   199,   360,   372,   350,   361,   238,
     360,   199,   199,   199,   360,   201,   361,   216,   201,   361,
       3,     4,     6,     7,    10,    11,    12,    13,    28,    29,
      31,    57,    68,    71,    72,    73,    74,    75,    76,    77,
      87,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   130,   132,   133,   134,
     135,   139,   140,   146,   163,   167,   175,   177,   180,   187,
     188,   199,   216,   217,   218,   229,   500,   521,   522,   525,
      27,   201,   355,   357,   361,   202,   250,   361,   112,   113,
     163,   166,   189,   219,   220,   221,   222,   226,    83,   205,
     305,   306,    83,   307,   123,   132,   122,   132,   199,   199,
     199,   199,   216,   276,   503,   199,   199,    70,    70,    70,
      70,    70,   350,    83,    91,   159,   160,   161,   492,   493,
     166,   202,   226,   226,   216,   277,   503,   167,   199,   199,
     503,   503,    83,   195,   202,   373,    28,   349,   352,   361,
     363,   470,   475,   233,   202,    91,   431,   492,    91,   492,
     492,    32,   166,   183,   504,   199,     9,   201,   199,   348,
     362,   471,   474,   118,    38,   256,   167,   275,   503,   121,
     194,   257,   340,    70,   202,   465,   201,   201,   201,   201,
     201,   201,   201,   201,    10,    11,    12,    30,    31,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    57,    69,   201,    70,    70,   202,   162,   133,
     173,   175,   189,   191,   278,   338,   339,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      59,    60,   136,   137,   460,   465,   465,   199,   199,    70,
     202,   205,   479,   199,   256,   257,    14,   361,   201,   138,
      49,   216,   455,    91,   349,   363,   162,   470,   138,   204,
       9,   440,   271,   349,   363,   470,   504,   162,   199,   432,
     460,   465,   200,   361,    32,   236,     8,   374,     9,   201,
     236,   237,   350,   351,   361,   216,   290,   240,   201,   201,
     201,   140,   146,   525,   525,   183,   524,   199,   112,   525,
      14,   162,   140,   146,   163,   216,   218,   201,   201,   201,
     251,   116,   180,   201,   219,   221,   219,   221,   219,   221,
     226,   219,   221,   202,     9,   441,   201,   103,   166,   202,
     470,     9,   201,    14,     9,   201,   132,   132,   470,   496,
     350,   349,   363,   470,   474,   475,   200,   183,   268,   139,
     470,   485,   486,   361,   382,   383,   350,   406,   406,   382,
     406,   201,    70,   460,   159,   493,    82,   361,   470,    91,
     159,   493,   226,   215,   201,   202,   263,   273,   415,   417,
      92,   199,   205,   375,   376,   378,   424,   428,   477,   479,
     497,   406,    14,   103,   498,   369,   370,   371,   300,   301,
     458,   459,   200,   200,   200,   200,   200,   203,   235,   236,
     258,   265,   272,   458,   361,   206,   207,   208,   216,   505,
     506,   525,    38,    87,   176,   303,   304,   361,   500,   247,
     248,   349,   357,   358,   361,   363,   470,   202,   249,   249,
     249,   249,   199,   503,   266,   256,   361,   481,   361,   361,
     361,   361,   361,    32,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   361,   429,   361,
     481,   481,   361,   488,   489,   132,   202,   217,   218,   478,
     479,   276,   216,   277,   503,   503,   275,   257,    38,   352,
     355,   357,   361,   361,   361,   361,   361,   361,   361,   361,
     361,   361,   361,   361,   361,   167,   202,   216,   461,   462,
     463,   464,   478,   303,   303,   481,   361,   485,   256,   200,
     361,   199,   454,     9,   440,   200,   200,    38,   361,    38,
     361,   432,   200,   200,   200,   478,   303,   202,   216,   461,
     462,   478,   200,   233,   294,   202,   357,   361,   361,    95,
      32,   236,   288,   201,    27,   103,    14,     9,   200,    32,
     202,   291,   525,    31,    92,   176,   229,   518,   519,   520,
     199,     9,    50,    51,    56,    58,    70,   140,   141,   142,
     143,   144,   145,   187,   188,   199,   227,   389,   392,   395,
     398,   401,   404,   410,   425,   433,   434,   436,   437,   216,
     523,   233,   199,   244,   202,   201,   202,   201,   202,   201,
     103,   166,   202,   201,   112,   113,   166,   222,   223,   224,
     225,   226,   222,   216,   361,   306,   434,    83,     9,   200,
     200,   200,   200,   200,   200,   200,   201,    50,    51,   514,
     516,   517,   134,   281,   199,     9,   200,   200,   138,   204,
       9,   440,     9,   440,   204,   204,   204,   204,    83,    85,
     216,   494,   216,    70,   203,   203,   212,   214,    32,   135,
     280,   182,    54,   167,   182,   419,   363,   138,     9,   440,
     200,   162,   200,   525,   525,    14,   374,   300,   231,   196,
       9,   441,    87,   525,   526,   460,   460,   203,     9,   440,
     184,   470,    83,    84,   302,   361,   200,     9,   441,    14,
       9,   200,     9,   200,   200,   200,   200,    14,   200,   203,
     234,   235,   366,   259,   134,   279,   199,   503,   204,   203,
     361,    32,   204,   204,   138,   203,     9,   440,   361,   504,
     199,   269,   264,   274,    14,   498,   267,   256,    71,   470,
     361,   504,   200,   200,   204,   203,   200,    50,    51,    70,
      78,    79,    80,    92,   140,   141,   142,   143,   144,   145,
     158,   187,   188,   216,   390,   393,   396,   399,   402,   405,
     425,   436,   443,   445,   446,   450,   453,   216,   470,   470,
     138,   279,   460,   465,   460,   200,   361,   295,    75,    76,
     296,   231,   360,   233,   351,   103,    38,   139,   285,   470,
     434,   216,    32,   236,   289,   201,   292,   201,   292,     9,
     440,    92,   229,   138,   162,     9,   440,   200,    87,   507,
     508,   525,   526,   505,   434,   434,   434,   434,   434,   439,
     442,   199,    70,    70,    70,    70,    70,   199,   199,   434,
     162,   202,    10,    11,    12,    31,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    69,   162,
     504,   203,   425,   202,   253,   221,   221,   221,   216,   221,
     222,   222,   226,     9,   441,   203,   203,    14,   470,   201,
     184,     9,   440,   216,   282,   425,   202,   485,   139,   470,
      14,   361,   361,   204,   361,   203,   212,   525,   282,   202,
     418,    14,   200,   361,   375,   478,   201,   525,   196,   203,
     232,   235,   245,    32,   512,   459,   526,    38,    83,   176,
     461,   462,   464,   461,   462,   464,   525,    70,    38,    87,
     176,   361,   434,   248,   357,   358,   470,   249,   248,   249,
     249,   203,   235,   300,   199,   425,   280,   367,   260,   361,
     361,   361,   203,   199,   303,   281,    32,   280,   525,    14,
     279,   503,   429,   203,   199,    14,    78,    79,    80,   216,
     444,   444,   446,   448,   449,    52,   199,    70,    70,    70,
      70,    70,    91,   159,   199,   199,   162,     9,   440,   200,
     454,    38,   361,   280,   203,    75,    76,   297,   360,   236,
     203,   201,    96,   201,   285,   470,   199,   138,   284,    14,
     233,   292,   106,   107,   108,   292,   203,   525,   184,   138,
     162,   525,   216,   176,   518,   525,     9,   440,   200,   176,
     440,   138,   204,     9,   440,   439,   384,   385,   434,   407,
     434,   435,   407,   384,   407,   375,   377,   379,   407,   200,
     132,   217,   434,   490,   491,   434,   434,   434,    32,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   523,    83,   254,   203,   203,   203,   203,
     225,   201,   434,   517,   103,   104,   513,   515,     9,   311,
     200,   199,   352,   357,   361,   138,   204,   203,   498,   311,
     168,   181,   202,   414,   421,   168,   202,   420,   138,   201,
     512,   199,   248,   348,   362,   471,   474,   525,   374,    87,
     526,    83,    83,   176,    14,    83,   504,   504,   481,   470,
     302,   361,   200,   300,   202,   300,   199,   138,   199,   303,
     200,   202,   525,   202,   201,   525,   280,   261,   432,   303,
     138,   204,     9,   440,   445,   448,   386,   387,   446,   408,
     446,   447,   408,   386,   408,   159,   375,   451,   452,   408,
      81,   446,   470,   202,   360,    32,    77,   236,   201,   351,
     284,   485,   285,   200,   434,   102,   106,   201,   361,    32,
     201,   293,   203,   184,   525,   216,   138,    87,   525,   526,
      32,   200,   434,   434,   200,   204,     9,   440,   138,   204,
       9,   440,   204,   204,   204,   138,     9,   440,   200,   200,
     138,   203,     9,   440,   434,    32,   200,   233,   201,   201,
     201,   201,   216,   525,   525,   513,   425,     6,   113,   119,
     122,   127,   169,   170,   172,   203,   312,   337,   338,   339,
     344,   345,   346,   347,   458,   485,   361,   203,   202,   203,
      54,   361,   361,   361,   374,   470,   201,   202,   526,    38,
      83,   176,    14,    83,   361,   199,   199,   204,   512,   200,
     311,   200,   300,   361,   303,   200,   311,   498,   311,   201,
     202,   199,   200,   446,   446,   200,   204,     9,   440,   138,
     204,     9,   440,   204,   204,   204,   138,   200,     9,   440,
     200,   311,    32,   233,   201,   200,   200,   200,   241,   201,
     201,   293,   233,   138,   525,   525,   176,   525,   138,   434,
     434,   434,   434,   375,   434,   434,   434,   202,   203,   515,
     134,   135,   189,   217,   501,   525,   283,   425,   113,   347,
      31,   127,   140,   146,   167,   173,   321,   322,   323,   324,
     425,   171,   329,   330,   130,   199,   216,   331,   332,   313,
     257,   525,     9,   201,     9,   201,   201,   498,   338,   200,
     308,   167,   416,   203,   203,   361,    83,    83,   176,    14,
      83,   361,   303,   303,   119,   364,   512,   203,   512,   200,
     200,   203,   202,   203,   311,   300,   138,   446,   446,   446,
     446,   375,   203,   233,   239,   242,    32,   236,   287,   233,
     525,   200,   434,   138,   138,   138,   233,   425,   425,   503,
      14,   217,     9,   201,   202,   501,   498,   324,   183,   202,
       9,   201,     3,     4,     5,     6,     7,    10,    11,    12,
      13,    27,    28,    29,    57,    71,    72,    73,    74,    75,
      76,    77,    87,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   139,   140,   147,   148,   149,
     150,   151,   163,   164,   165,   175,   177,   178,   180,   187,
     189,   191,   193,   194,   216,   422,   423,     9,   201,   167,
     171,   216,   332,   333,   334,   201,    83,   343,   256,   314,
     501,   501,    14,   257,   203,   309,   310,   501,    14,    83,
     361,   200,   200,   199,   512,   198,   509,   364,   512,   308,
     203,   200,   446,   138,   138,    32,   236,   286,   287,   233,
     434,   434,   434,   203,   201,   201,   434,   425,   317,   525,
     325,   326,   433,   322,    14,    32,    51,   327,   330,     9,
      36,   200,    31,    50,    53,    14,     9,   201,   218,   502,
     343,    14,   525,   256,   201,    14,   361,    38,    83,   413,
     202,   510,   511,   525,   201,   202,   335,   512,   509,   203,
     512,   446,   446,   233,   100,   252,   203,   216,   229,   318,
     319,   320,     9,   440,     9,   440,   203,   434,   423,   423,
      68,   328,   333,   333,    31,    50,    53,   434,    83,   183,
     199,   201,   434,   502,   434,    83,     9,   441,   231,     9,
     441,    14,   513,   231,   202,   335,   335,    98,   201,   116,
     243,   162,   103,   525,   184,   433,   174,    14,   514,   315,
     199,    38,    83,   200,   203,   511,   525,   203,   231,   201,
     199,   180,   255,   216,   338,   339,   184,   434,   184,   298,
     299,   459,   316,    83,   203,   425,   253,   177,   216,   201,
     200,     9,   441,    87,   124,   125,   126,   341,   342,   298,
      83,   283,   201,   512,   459,   526,   526,   200,   200,   201,
     509,    87,   341,    83,    38,    83,   176,   512,   202,   201,
     202,   336,   526,   526,    83,   176,    14,    83,   509,   233,
     231,    83,    38,    83,   176,    14,    83,   361,   336,   203,
     203,    83,   176,    14,    83,   361,    14,    83,   361,   361
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   209,   211,   210,   212,   212,   213,   213,   213,   213,
     213,   213,   213,   213,   214,   213,   215,   213,   213,   213,
     213,   213,   213,   213,   213,   213,   213,   213,   213,   216,
     216,   216,   216,   216,   216,   216,   216,   216,   216,   216,
     216,   216,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   217,   217,   217,   217,   218,
     218,   219,   219,   220,   220,   221,   222,   222,   222,   222,
     223,   223,   224,   225,   225,   225,   226,   226,   227,   227,
     227,   228,   229,   230,   230,   231,   231,   232,   232,   233,
     233,   234,   234,   235,   235,   235,   235,   236,   236,   236,
     237,   236,   238,   236,   239,   236,   240,   236,   236,   236,
     236,   236,   236,   236,   236,   236,   236,   236,   236,   236,
     236,   236,   236,   241,   236,   242,   236,   236,   243,   236,
     244,   236,   236,   236,   236,   236,   236,   236,   236,   236,
     236,   236,   236,   236,   236,   236,   236,   236,   236,   236,
     236,   236,   236,   245,   246,   246,   247,   247,   248,   248,
     248,   249,   249,   251,   250,   252,   252,   254,   253,   255,
     255,   256,   256,   257,   259,   258,   260,   258,   261,   258,
     263,   262,   264,   262,   266,   265,   267,   265,   268,   265,
     269,   265,   271,   270,   273,   272,   274,   272,   275,   275,
     276,   277,   278,   278,   278,   278,   278,   279,   279,   280,
     280,   281,   281,   282,   282,   283,   283,   284,   284,   285,
     285,   285,   286,   286,   287,   287,   288,   288,   289,   289,
     290,   290,   291,   291,   291,   291,   292,   292,   292,   293,
     293,   294,   294,   295,   295,   296,   296,   297,   297,   298,
     298,   298,   298,   298,   298,   298,   298,   299,   299,   299,
     299,   299,   299,   299,   299,   299,   299,   300,   300,   300,
     300,   300,   300,   300,   300,   301,   301,   301,   301,   301,
     301,   301,   301,   301,   301,   302,   302,   302,   303,   303,
     304,   304,   304,   304,   304,   304,   304,   304,   305,   305,
     306,   306,   306,   307,   307,   307,   307,   308,   308,   309,
     310,   311,   311,   313,   312,   314,   312,   312,   312,   312,
     315,   312,   316,   312,   312,   312,   312,   312,   312,   312,
     312,   317,   317,   317,   318,   319,   319,   320,   320,   321,
     321,   322,   322,   323,   323,   324,   324,   324,   324,   324,
     324,   324,   325,   325,   326,   327,   327,   328,   328,   329,
     329,   330,   331,   331,   331,   332,   332,   332,   332,   333,
     333,   333,   333,   333,   333,   333,   334,   334,   334,   335,
     335,   336,   336,   337,   337,   338,   338,   339,   339,   340,
     340,   340,   340,   340,   340,   340,   341,   341,   342,   342,
     342,   343,   343,   343,   343,   344,   344,   345,   345,   346,
     346,   347,   348,   349,   349,   349,   349,   349,   349,   350,
     350,   351,   351,   352,   352,   352,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   361,   361,   361,   361,
     362,   363,   363,   363,   363,   363,   363,   363,   363,   363,
     363,   363,   363,   363,   363,   363,   363,   363,   363,   363,
     363,   363,   363,   363,   363,   363,   363,   363,   363,   363,
     363,   363,   363,   363,   363,   363,   363,   363,   363,   363,
     363,   363,   363,   363,   363,   363,   363,   363,   363,   363,
     363,   363,   363,   363,   363,   363,   363,   363,   363,   363,
     363,   363,   363,   363,   363,   363,   363,   363,   363,   363,
     363,   363,   363,   363,   363,   363,   363,   363,   363,   364,
     364,   366,   365,   367,   365,   369,   368,   370,   368,   371,
     368,   372,   368,   373,   368,   374,   374,   374,   375,   375,
     376,   376,   377,   377,   378,   378,   379,   379,   380,   381,
     381,   382,   382,   383,   383,   384,   384,   385,   385,   386,
     386,   387,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   406,   407,   407,   408,   408,   409,   410,   411,
     411,   412,   412,   412,   412,   412,   412,   412,   412,   412,
     412,   412,   412,   413,   413,   413,   413,   414,   415,   415,
     416,   416,   417,   417,   418,   418,   419,   420,   420,   421,
     421,   421,   422,   422,   422,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   423,   423,
     423,   423,   423,   423,   423,   423,   423,   423,   424,   425,
     425,   426,   426,   426,   426,   426,   427,   427,   428,   428,
     428,   428,   429,   429,   429,   430,   430,   430,   431,   431,
     431,   432,   432,   433,   433,   433,   433,   433,   433,   433,
     433,   433,   433,   433,   433,   433,   433,   433,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   434,   434,   434,
     434,   434,   434,   434,   435,   435,   436,   437,   437,   438,
     438,   438,   438,   438,   438,   438,   439,   439,   440,   440,
     441,   441,   442,   442,   442,   442,   443,   443,   443,   443,
     443,   444,   444,   444,   444,   445,   445,   446,   446,   446,
     446,   446,   446,   446,   446,   446,   446,   446,   446,   446,
     446,   446,   446,   447,   447,   448,   448,   449,   449,   449,
     449,   450,   450,   451,   451,   452,   452,   453,   453,   454,
     454,   455,   455,   457,   456,   458,   459,   459,   460,   460,
     461,   461,   461,   462,   462,   463,   463,   464,   464,   465,
     465,   466,   466,   466,   467,   467,   468,   468,   469,   469,
     470,   470,   470,   470,   470,   470,   470,   470,   470,   470,
     470,   471,   472,   472,   472,   472,   472,   472,   472,   473,
     473,   473,   473,   473,   473,   473,   473,   473,   474,   475,
     475,   476,   476,   476,   477,   477,   477,   478,   478,   479,
     479,   479,   480,   480,   480,   481,   481,   482,   482,   483,
     483,   483,   483,   483,   483,   484,   484,   484,   484,   484,
     485,   485,   485,   485,   485,   485,   486,   486,   487,   487,
     487,   487,   487,   487,   487,   487,   488,   488,   489,   489,
     489,   489,   490,   490,   491,   491,   491,   491,   492,   492,
     492,   492,   493,   493,   493,   493,   493,   493,   494,   494,
     494,   495,   495,   495,   495,   495,   495,   495,   495,   495,
     495,   495,   496,   496,   497,   497,   498,   498,   499,   499,
     499,   499,   500,   500,   501,   501,   502,   502,   503,   503,
     504,   504,   505,   505,   506,   507,   507,   507,   507,   507,
     507,   508,   508,   508,   508,   509,   509,   510,   510,   511,
     511,   512,   512,   513,   513,   514,   515,   515,   516,   516,
     516,   516,   517,   517,   517,   518,   518,   518,   518,   519,
     519,   520,   520,   520,   520,   521,   522,   523,   523,   524,
     524,   525,   525,   525,   525,   525,   525,   525,   525,   525,
     525,   525,   526,   526
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
       1,     2,     3,     3,     1,     2,     1,     2,     3,     4,
       3,     1,     2,     1,     2,     2,     1,     3,     1,     3,
       2,     2,     2,     5,     4,     2,     0,     1,     3,     2,
       0,     2,     1,     1,     1,     1,     1,     3,     5,     8,
       0,     4,     0,     6,     0,    10,     0,     4,     2,     3,
       2,     3,     2,     3,     3,     3,     3,     3,     3,     5,
       1,     1,     1,     0,     9,     0,    10,     5,     0,    13,
       0,     5,     3,     3,     3,     3,     5,     5,     5,     3,
       3,     2,     2,     2,     2,     2,     2,     3,     2,     2,
       3,     2,     2,     2,     1,     0,     3,     3,     1,     1,
       1,     3,     2,     0,     4,     9,     0,     0,     4,     2,
       0,     1,     0,     1,     0,    10,     0,    11,     0,    11,
       0,     9,     0,    10,     0,     8,     0,     9,     0,     7,
       0,     8,     0,     8,     0,     7,     0,     8,     1,     1,
       1,     1,     1,     2,     3,     3,     2,     2,     0,     2,
       0,     2,     0,     1,     3,     1,     3,     2,     0,     1,
       2,     4,     1,     4,     1,     4,     1,     4,     1,     4,
       3,     5,     3,     4,     4,     5,     5,     4,     0,     1,
       1,     4,     0,     5,     0,     2,     0,     3,     0,     7,
       8,     6,     2,     5,     6,     4,     0,     4,     4,     5,
       7,     6,     6,     6,     7,     9,     8,     6,     7,     5,
       2,     4,     5,     3,     0,     3,     4,     4,     6,     5,
       5,     6,     6,     8,     7,     4,     1,     1,     2,     0,
       1,     2,     2,     2,     3,     4,     4,     4,     3,     1,
       1,     2,     4,     3,     5,     1,     3,     2,     0,     2,
       3,     2,     0,     0,     4,     0,     5,     2,     2,     2,
       0,    11,     0,    12,     3,     3,     3,     4,     4,     3,
       5,     2,     2,     0,     6,     5,     4,     3,     1,     1,
       3,     4,     1,     2,     1,     1,     5,     6,     1,     1,
       4,     1,     1,     3,     2,     2,     0,     2,     0,     1,
       3,     1,     1,     1,     1,     3,     4,     4,     4,     1,
       1,     2,     2,     2,     3,     3,     1,     1,     1,     1,
       3,     1,     3,     1,     1,     1,     0,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     1,     1,
       1,     3,     5,     1,     3,     5,     4,     3,     3,     3,
       4,     3,     3,     3,     3,     2,     2,     1,     1,     3,
       1,     1,     0,     1,     2,     4,     3,     3,     6,     2,
       3,     2,     3,     6,     3,     1,     1,     1,     1,     1,
       3,     6,     3,     4,     6,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     5,     4,     3,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     2,     1,     5,
       0,     0,    12,     0,    13,     0,     4,     0,     7,     0,
       5,     0,     3,     0,     6,     2,     2,     4,     1,     1,
       5,     3,     5,     3,     2,     0,     2,     0,     4,     4,
       3,     2,     0,     5,     3,     2,     0,     5,     3,     2,
       0,     5,     3,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     2,     0,     2,     0,     2,     0,     4,     4,     4,
       4,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     1,     3,     4,     1,     2,     4,     2,     6,
       0,     1,     4,     0,     2,     0,     1,     1,     3,     1,
       3,     1,     1,     3,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     4,     1,
       1,     1,     1,     1,     1,     3,     1,     3,     1,     1,
       1,     3,     1,     1,     1,     2,     1,     0,     0,     1,
       1,     3,     0,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     2,     1,     1,
       4,     3,     4,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     5,     4,     3,     1,     3,     3,     1,     1,
       1,     1,     1,     3,     3,     3,     2,     0,     1,     0,
       1,     0,     5,     3,     3,     1,     1,     1,     1,     3,
       2,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       3,     1,     2,     2,     4,     3,     4,     1,     1,     1,
       1,     1,     1,     3,     1,     2,     0,     5,     3,     3,
       1,     3,     1,     2,     0,     5,     3,     2,     0,     3,
       0,     4,     2,     0,     3,     3,     1,     0,     1,     1,
       1,     1,     3,     1,     1,     1,     3,     1,     1,     3,
       3,     2,     2,     2,     2,     4,     5,     5,     5,     5,
       1,     1,     1,     1,     1,     1,     3,     3,     4,     4,
       3,     3,     1,     1,     1,     1,     3,     1,     4,     1,
       1,     1,     1,     1,     3,     3,     4,     4,     3,     1,
       1,     7,     9,     9,     7,     6,     8,     1,     2,     4,
       4,     1,     1,     1,     4,     1,     0,     1,     2,     1,
       1,     1,     3,     3,     3,     0,     1,     1,     3,     3,
       2,     3,     6,     0,     1,     4,     2,     0,     5,     3,
       3,     1,     6,     4,     4,     2,     2,     0,     5,     3,
       3,     1,     2,     0,     5,     3,     3,     1,     2,     2,
       1,     2,     1,     4,     3,     3,     6,     3,     1,     1,
       1,     4,     4,     4,     4,     4,     4,     2,     2,     4,
       2,     2,     1,     3,     3,     3,     0,     2,     5,     6,
       6,     7,     1,     2,     1,     2,     1,     4,     1,     4,
       3,     0,     1,     3,     2,     1,     2,     4,     3,     3,
       1,     4,     2,     2,     0,     0,     3,     1,     3,     3,
       2,     0,     2,     2,     2,     2,     1,     2,     4,     2,
       5,     3,     1,     1,     0,     3,     4,     5,     6,     3,
       1,     3,     2,     1,     0,     4,     1,     3,     2,     4,
       5,     2,     2,     1,     1,     1,     1,     3,     2,     1,
       8,     6,     1,     0
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
#line 758 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
#line 7349 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 761 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 7357 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 768 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 7363 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 769 "hphp.y" /* yacc.c:1646  */
    { }
#line 7369 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 772 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 7375 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 773 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7381 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 774 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7387 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 775 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7393 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 776 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7399 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 777 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 7405 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 778 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 7413 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 781 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 7420 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 783 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 7426 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 784 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7432 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 785 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 7438 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 786 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7444 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 787 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7452 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 791 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7461 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 796 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7470 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 801 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7477 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 804 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7484 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 807 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7492 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 811 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7500 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 815 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7508 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 819 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7516 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 823 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7524 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 826 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7531 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 831 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7537 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7543 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7549 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 834 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7555 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7561 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7567 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7573 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 838 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7579 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 839 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7585 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 840 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7591 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 841 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7597 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 842 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7603 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 843 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7609 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 925 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7615 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 927 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7621 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 932 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7627 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 933 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7634 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 939 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7640 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 943 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7646 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 944 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7652 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 946 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7658 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 948 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7664 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 953 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7670 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 954 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7677 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 960 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7683 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 964 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7690 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 966 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7697 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 968 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7704 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 973 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7710 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 975 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7716 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 978 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7722 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 980 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7728 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 981 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7734 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 986 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7743 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 993 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7752 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 1001 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7759 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1004 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7766 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 1010 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 7772 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1011 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval)); }
#line 7778 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1016 "hphp.y" /* yacc.c:1646  */
    {
                                         _p->onUsing((yyval), (yyvsp[-2]), true, (yyvsp[-1]), nullptr);
                                       }
#line 7786 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1023 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7792 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1024 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7798 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1029 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 7804 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1030 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval), (yyval), (yyvsp[0])); }
#line 7811 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1035 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7817 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1036 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7823 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1037 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7829 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1038 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7835 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1041 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7841 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1045 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7847 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1050 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7853 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1051 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7860 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1053 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7868 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1057 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7875 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1060 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7883 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1064 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7890 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1066 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7898 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1069 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7905 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1071 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7913 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1074 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7919 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1075 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7925 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1076 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7931 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1077 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7937 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1078 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7943 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1079 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7949 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1080 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7955 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1081 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7961 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1082 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7967 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1083 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7973 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1084 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7979 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1085 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 7985 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1086 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 7991 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1087 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 7997 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1088 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 8004 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1092 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 8011 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1094 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 8019 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1099 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 8026 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1101 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 8034 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1105 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 8042 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1114 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 8048 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1115 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8054 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1118 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false); }
#line 8060 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1119 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 8066 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1121 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8074 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1125 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8082 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1129 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8090 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1133 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8098 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1137 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8106 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1141 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8114 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1146 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8122 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1149 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 8128 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1150 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 8137 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1154 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8143 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1155 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8149 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1156 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8155 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1157 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8161 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1158 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8167 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1159 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8173 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1160 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 8179 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1161 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8185 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1162 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8191 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1163 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 8197 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1164 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8203 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1165 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 8213 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1187 "hphp.y" /* yacc.c:1646  */
    { _p->pushLabelScope(LS::Using);
                                         _p->onNewLabelScope(false);
                                         (yyval) = (yyvsp[-1]); }
#line 8221 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1193 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1; }
#line 8227 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1194 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8233 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 1203 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), nullptr, (yyvsp[-2]));
                                         _p->onExprListElem((yyval), &(yyval), (yyvsp[0])); }
#line 8240 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 1205 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0])); }
#line 8246 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1215 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8252 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1216 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8258 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1220 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false); }
#line 8264 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1221 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8270 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1230 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 8276 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1231 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8282 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1235 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::Finally);}
#line 8289 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1237 "hphp.y" /* yacc.c:1646  */
    { _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->popLabelScope();
                                         _p->onCompleteLabelScope(false);}
#line 8297 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1243 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8303 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1244 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8309 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1248 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 8315 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1249 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8321 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1253 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 8327 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1259 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8336 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1266 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8346 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1274 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8355 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1281 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8365 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1289 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8374 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1295 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8384 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1304 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8391 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1308 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 8397 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1312 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8404 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1316 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 8410 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1322 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8417 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 1325 "hphp.y" /* yacc.c:1646  */
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
#line 8435 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1340 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8442 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 1343 "hphp.y" /* yacc.c:1646  */
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
#line 8460 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1357 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8467 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1360 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8475 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1365 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8482 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1368 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8490 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1374 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 8496 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1377 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 8502 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1381 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8509 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1384 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8520 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1392 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8527 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1395 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8538 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1403 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8544 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1404 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 8551 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1408 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8557 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1411 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8563 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1414 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 8569 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1415 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 8575 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1416 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 8583 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1419 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 8589 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1420 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 8595 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1424 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8601 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1425 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8607 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1428 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8613 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1429 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8619 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1432 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8625 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1433 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8631 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1436 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 8637 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1438 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8643 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1441 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 8649 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1443 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8655 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1447 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8661 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1448 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8667 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1451 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8673 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1452 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8679 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1453 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8685 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1457 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8691 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8697 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1462 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8703 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1464 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8709 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1467 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8715 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1469 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8721 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1472 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8727 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1474 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8733 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1478 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8739 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1480 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8746 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1485 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8752 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1486 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8758 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1487 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8764 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1488 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8770 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1493 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8776 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1495 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8782 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1496 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8788 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1499 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8794 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1500 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8800 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1505 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8806 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1506 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8812 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1511 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8818 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1512 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8824 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1515 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8830 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1516 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8836 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1519 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8842 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1520 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8848 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1528 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8855 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1534 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8862 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1540 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8870 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1544 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8876 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1548 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8883 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1553 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8890 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1558 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8898 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1561 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8904 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1567 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8912 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1572 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8920 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1577 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8928 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1583 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8936 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1589 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8944 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1595 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8952 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1601 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8960 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1607 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8968 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1614 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8976 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1621 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8984 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1630 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 8991 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1635 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 8998 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1640 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 9006 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1644 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9012 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1647 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 9019 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1651 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 9026 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1655 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 9034 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1658 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9040 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1663 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 9048 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1667 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9056 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1671 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9064 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1676 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9072 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1681 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9080 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1686 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 9088 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1691 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9096 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1696 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9104 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1702 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9112 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1708 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9120 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1714 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 9126 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1715 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 9132 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1716 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 9138 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1721 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9144 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1722 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9150 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1725 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,false);}
#line 9157 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1727 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::InOut,false);}
#line 9164 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1729 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::Ref,false);}
#line 9171 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1731 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,true);}
#line 9178 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1734 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::In,false);}
#line 9185 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1737 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::In,true);}
#line 9192 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1740 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::Ref,false);}
#line 9199 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1743 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::InOut,false);}
#line 9206 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1748 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9212 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1749 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 9218 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1752 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9224 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1753 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 9230 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1754 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 9236 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1758 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9242 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1760 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9248 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1761 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 9254 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1762 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9260 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1767 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9266 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1768 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9272 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 359:
#line 1771 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9279 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1776 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9285 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1782 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9291 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1783 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9297 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1786 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[0]));}
#line 9303 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1787 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 9310 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1790 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[-1]));}
#line 9316 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1791 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-3]));}
#line 9323 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1793 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9330 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1796 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL, true);}
#line 9337 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1798 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9343 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1801 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9351 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1808 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9361 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1816 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9369 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1823 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9379 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1829 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 9385 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1831 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9391 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1833 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9397 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1835 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 9403 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1837 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 9409 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1838 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 9416 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1841 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 9422 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1844 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9428 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1845 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9434 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 1846 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 9440 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1852 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 9446 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1857 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 9453 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1860 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 9461 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1867 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 9467 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1868 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 9474 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1873 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 9481 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1876 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 9487 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1883 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 9494 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 1885 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 9500 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1889 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9506 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1894 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9512 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1896 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9518 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1898 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9524 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1899 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 9535 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1905 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 9541 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1907 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 9547 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1908 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 9553 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1912 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,0,(yyvsp[0]),0);}
#line 9559 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1914 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),0,(yyvsp[0]),0);}
#line 9565 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1919 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9571 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1922 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9577 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1923 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 9583 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1927 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 9589 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1928 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 9595 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1932 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 9602 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1935 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 9609 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1940 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 9616 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1945 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 9622 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1946 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 9629 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1948 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 9635 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1952 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 9641 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1953 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 9647 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1954 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 9653 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1955 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 9659 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1959 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9665 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1960 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 9671 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1961 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 9677 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1962 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 9683 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 9689 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1965 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 9695 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1967 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 9701 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1971 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 9709 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1974 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 9715 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1975 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 9721 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1979 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9727 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1980 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9733 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1984 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9739 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1985 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9745 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1988 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9751 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 1989 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9757 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 1992 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9763 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 1993 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9769 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 1996 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9775 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 1998 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9781 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 2001 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9787 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 2002 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9793 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 2003 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9799 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 2004 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9805 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 2005 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9811 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 2006 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9817 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 2007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9823 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 2011 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9829 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 2012 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9835 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 2015 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9841 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 2016 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9847 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9853 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 2021 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9859 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 2023 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9865 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 2024 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9871 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 2025 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9877 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 2029 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9883 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 2031 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9889 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 2035 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9895 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 2037 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9901 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 2041 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9909 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 2045 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9916 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 2049 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9922 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 2053 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9928 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 2057 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9934 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 2059 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9940 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 2060 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9946 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 2061 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9952 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 2062 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9958 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 2063 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9964 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 2066 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9970 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 2067 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9976 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 2071 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9982 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 2072 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9988 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 2076 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 9994 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 2077 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 10000 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 2078 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 10006 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 2079 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10012 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 2083 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10018 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 2088 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 10024 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 2092 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 10030 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 2096 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10036 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 2100 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 10042 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 2104 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 10048 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 2109 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 10054 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 2113 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10060 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 2117 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10066 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 2118 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10072 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 2119 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10078 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 2120 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10084 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 2121 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10090 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 2125 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10096 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 2130 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 10102 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 2131 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 10108 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 2132 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 10114 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 2135 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 10120 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 2136 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 10126 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 2137 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 10132 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 2138 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 10138 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 2139 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 10144 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 2140 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 10150 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 2141 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 10156 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 2142 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 10162 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 2143 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 10168 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2144 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 10174 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2145 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 10180 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2146 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 10186 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2147 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 10192 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2148 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 10198 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2149 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 10204 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2150 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 10210 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2151 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 10216 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2152 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 10222 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2153 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 10228 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2154 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 10234 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2155 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 10240 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2156 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 10246 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2157 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 10252 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2158 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 10258 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2159 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 10264 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2160 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 10270 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2161 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 10276 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2162 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 10282 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2163 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 10288 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2164 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 10294 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2165 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 10300 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2166 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 10306 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2167 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 10312 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2168 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 10318 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2169 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 10324 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2170 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 10330 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2171 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 10336 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2172 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 10342 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2173 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 10348 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2174 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 10354 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2175 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 10360 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2176 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 10366 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2177 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 10372 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2178 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 10378 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2179 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 10385 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2181 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 10391 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2182 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 10398 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2184 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 10404 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2186 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 10410 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2187 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10416 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2188 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 10422 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2189 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 10428 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2190 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 10434 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2191 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10440 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2192 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 10446 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2193 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 10452 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2194 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 10458 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2195 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 10464 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2196 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 10470 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2197 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 10476 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2198 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 10482 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2199 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 10488 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2200 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 10494 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2201 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10500 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2202 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10506 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2203 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10512 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2204 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10518 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2205 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10524 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2206 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10530 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2207 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10536 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2208 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10542 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2209 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10548 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2210 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 10554 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2211 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 10560 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2212 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10566 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2219 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 10572 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 570:
#line 2220 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10578 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2225 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10587 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 572:
#line 2231 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10599 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2240 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10608 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2246 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[-12]),
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10620 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2257 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,NULL,NULL);}
#line 10634 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
#line 2266 "hphp.y" /* yacc.c:1646  */
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
#line 10649 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2277 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10659 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
#line 2285 "hphp.y" /* yacc.c:1646  */
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
#line 10674 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
#line 2296 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10684 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
#line 2302 "hphp.y" /* yacc.c:1646  */
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
#line 10701 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2314 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,NULL,NULL);}
#line 10715 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
#line 2323 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10728 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2331 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10738 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2339 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10751 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2350 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10757 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2351 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10763 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2353 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10769 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2357 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10776 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2359 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10782 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2366 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10788 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2369 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10794 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2376 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10800 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2379 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10806 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2384 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10812 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2385 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10818 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2390 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10824 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2391 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10830 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2395 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval), (yyvsp[-1]), T_DARRAY);}
#line 10836 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2399 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10842 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2400 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10848 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2405 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10854 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2406 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10860 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2411 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10866 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2412 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10872 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2417 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10878 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2418 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10884 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2424 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10890 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2426 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10896 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2431 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10902 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2432 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10908 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2438 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10914 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2440 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10920 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2444 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10926 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2448 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10932 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2452 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10938 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2456 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10944 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2460 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10950 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2464 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10956 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2468 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10962 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2472 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10968 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2476 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10974 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2480 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10980 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2484 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10986 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2488 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10992 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2492 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10998 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2496 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11004 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2500 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 11010 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2504 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11016 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2508 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11022 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2512 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 11028 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2517 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11034 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2518 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11040 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2523 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11046 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2524 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11052 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2529 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11058 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2530 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11064 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2535 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 11072 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2542 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 11080 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2549 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 11086 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2551 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 11092 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2555 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11098 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2556 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11104 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2557 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11110 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2558 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11116 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2559 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11122 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2560 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11128 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2561 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11134 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2562 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11140 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2563 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11146 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2564 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 11153 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2566 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11159 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2567 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11165 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 653:
#line 2571 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11171 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2572 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 11177 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2573 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 11183 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2574 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 11189 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2581 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 11195 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2584 "hphp.y" /* yacc.c:1646  */
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
#line 11213 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2599 "hphp.y" /* yacc.c:1646  */
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
#line 11231 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2614 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 11237 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2615 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 11243 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2620 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11249 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2621 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11255 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2624 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-1]),0,(yyvsp[0]),0);}
#line 11261 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2625 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11267 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2628 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11274 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2632 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11282 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2635 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11288 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2638 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         if ((yyvsp[0]).htmlTrim()) {
                                           (yyvsp[0]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));
                                         }
                                       }
#line 11300 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2645 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11306 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2646 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11312 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2650 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11318 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2652 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 11324 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2654 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 11330 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2658 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11336 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2659 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11342 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2660 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11348 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2661 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11354 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2662 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11360 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2663 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11366 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2664 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11372 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2665 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11378 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2666 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11384 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2667 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11390 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2668 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11396 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2669 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11402 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2670 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11408 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2671 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11414 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2672 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11420 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2673 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11426 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2674 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11432 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2675 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11438 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2676 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11444 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2677 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11450 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2678 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11456 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2679 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11462 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2680 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11468 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2681 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11474 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2682 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11480 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2683 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11486 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2684 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11492 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2685 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11498 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2686 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11504 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2687 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11510 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2688 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11516 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2689 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11522 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2690 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11528 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2691 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11534 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2692 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11540 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2693 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11546 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2694 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11552 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2695 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11558 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2696 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11564 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2697 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11570 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2698 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11576 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2699 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11582 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2700 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11588 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2701 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11594 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2702 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11600 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2703 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11606 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2704 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11612 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2705 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11618 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2706 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11624 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2707 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11630 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2708 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11636 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2709 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11642 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2710 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11648 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2711 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11654 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2712 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11660 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2713 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11666 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2714 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11672 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2715 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11678 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2716 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11684 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2717 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11690 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2718 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11696 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2719 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11702 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2720 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11708 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2721 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11714 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2722 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11720 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2723 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11726 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2724 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11732 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2725 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11738 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2726 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11744 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2727 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11750 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2728 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11756 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2729 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11762 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2730 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11768 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2731 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11774 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2732 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11780 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2733 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11786 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2734 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11792 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2735 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11798 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2736 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11804 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2737 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11810 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2738 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11816 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2739 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11822 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2740 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11828 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2745 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11834 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2749 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11840 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2750 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11846 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2754 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11852 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2755 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11858 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2756 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11864 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2757 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11871 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2759 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11878 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2763 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11884 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2772 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11890 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2775 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11896 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2776 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11903 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2778 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11910 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2788 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11916 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2792 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11922 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2793 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11928 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2794 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11934 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2798 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11940 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2799 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11946 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2800 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11952 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2804 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11958 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2805 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11964 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2806 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11970 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2810 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11976 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2811 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11982 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2815 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11988 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2816 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11994 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2817 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 12000 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2818 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 12007 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2820 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 12013 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2821 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 12019 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2822 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 12025 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2823 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 12031 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2824 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 12037 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2825 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 12043 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2826 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 12049 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2827 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 12055 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2828 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 12061 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2831 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12067 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2833 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12073 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12079 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2838 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12085 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2840 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12091 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2841 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12097 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2843 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_DARRAY);}
#line 12103 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2844 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12109 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2845 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12115 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2846 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12121 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2847 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12127 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2848 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12133 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2849 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12139 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2850 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12145 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2851 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12151 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2852 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12157 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2854 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 12163 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2856 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 12169 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2858 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 12175 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2860 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 12181 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2862 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 12187 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2863 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 12193 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2864 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 12199 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2865 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 12205 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2866 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 12211 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2867 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 12217 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2868 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 12223 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2869 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 12229 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2870 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 12235 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2871 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 12241 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2872 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 12247 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2873 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 12253 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2874 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 12259 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2875 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 12265 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2876 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 12271 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2877 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12277 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 832:
#line 2878 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12283 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2880 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 12289 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2882 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 12295 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2884 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 12301 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2886 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 12307 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2887 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 12313 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2889 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 12320 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2891 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 12326 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2894 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 12333 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2898 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 12339 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2901 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 12345 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2902 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 12351 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2906 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12357 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2907 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12363 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2913 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12369 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2919 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12375 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2920 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12381 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2924 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12387 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2925 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12393 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2926 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12399 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2927 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12405 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2928 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 12411 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2929 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 12417 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2931 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 12424 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2936 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12430 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2937 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12436 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2941 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12442 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 2942 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12448 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2945 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 12454 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2946 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12460 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2952 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12466 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2954 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12472 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2956 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12478 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2957 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12484 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2961 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 12490 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2962 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 12496 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2963 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 12502 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2966 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12508 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 2968 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12514 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2971 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 12520 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2972 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 12526 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2973 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 12532 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2974 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12538 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2978 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 12545 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 2981 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 12553 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 2988 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12559 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 2989 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12565 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 2992 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 12573 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 2995 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12579 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 2996 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12585 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 2997 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12591 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 2999 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12597 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 3000 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12603 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
#line 3002 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_DARRAY);}
#line 12609 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 3003 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12615 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 3004 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12621 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 3005 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12627 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 3006 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12633 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 3007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12639 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 3008 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12645 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 3013 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12651 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 3014 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12657 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 3019 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12663 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 3020 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12669 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 3025 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12675 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 3027 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12681 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 3029 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12687 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 3030 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12693 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3034 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12699 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3035 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12705 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3040 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12711 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3041 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12717 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3046 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12723 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3049 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12729 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3054 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12735 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3055 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12741 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3058 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12747 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3059 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 12754 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3066 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 12760 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3068 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 12766 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3071 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 12772 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3073 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12778 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3076 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12784 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3079 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12790 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3080 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12796 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3084 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 12802 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3085 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 12808 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3089 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12814 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3090 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 12820 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3091 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 12826 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3095 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12832 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3097 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12838 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3105 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12844 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3106 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12850 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3110 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12856 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3112 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12862 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3120 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12868 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3121 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12874 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3126 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12880 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3127 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12886 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3129 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12892 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3134 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12898 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3136 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12904 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3142 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12918 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
#line 3153 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12932 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
#line 3168 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12946 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3180 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12960 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
#line 3192 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12966 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
#line 3193 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12972 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3194 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12978 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3195 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12984 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
#line 3196 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12990 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3197 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12996 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
#line 3199 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13010 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
#line 3216 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13016 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
#line 3218 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13022 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3220 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13028 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3221 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13034 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3225 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13040 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3229 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13046 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3230 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13052 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3231 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13058 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3232 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13064 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3240 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13078 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3249 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13084 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3251 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13090 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3260 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13096 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3261 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13102 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3262 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13108 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3263 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13114 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3264 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13120 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3265 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13126 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3266 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13132 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3268 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13138 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3270 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13144 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3274 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13150 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3278 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13156 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3279 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13162 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3285 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13168 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3289 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13174 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3293 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13180 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3300 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 13186 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3309 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 13192 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3313 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 13198 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3317 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13204 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3320 "hphp.y" /* yacc.c:1646  */
    { _p->onIndirectRef((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 13210 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3326 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13216 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3327 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13222 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3328 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13228 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3332 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13234 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3333 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 13240 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3334 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 13246 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3341 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13252 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3342 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13258 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3347 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 13264 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3348 "hphp.y" /* yacc.c:1646  */
    { (yyval)++;}
#line 13270 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3353 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13276 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3354 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13282 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3355 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13288 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3358 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13302 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3369 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13308 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3370 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13314 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3374 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13320 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3375 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13326 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
#line 3378 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13340 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3387 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13346 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3391 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 13352 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3392 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 13358 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3394 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 13364 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3395 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 13370 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1004:
#line 3396 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 13376 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1005:
#line 3397 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 13382 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1006:
#line 3402 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13388 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1007:
#line 3403 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13394 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1008:
#line 3407 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13400 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3408 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13406 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3409 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13412 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3410 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13418 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3413 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13424 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3415 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 13430 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3416 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13436 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3417 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 13442 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3422 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13448 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3423 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13454 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3427 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13460 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3428 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13466 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3429 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13472 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3430 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13478 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3435 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13484 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3436 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13490 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3441 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13496 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3443 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13502 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3445 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13508 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1027:
#line 3446 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13514 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1028:
#line 3450 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 13520 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1029:
#line 3452 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 13526 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1030:
#line 3453 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 13532 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1031:
#line 3455 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 13539 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1032:
#line 3460 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13545 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1033:
#line 3462 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13551 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1034:
#line 3464 "hphp.y" /* yacc.c:1646  */
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[-2]),
                                           !(yyvsp[-1]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[0])
                                         );
                                       }
#line 13565 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1035:
#line 3474 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 13571 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1036:
#line 3476 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 13577 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1037:
#line 3477 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13583 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1038:
#line 3480 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 13589 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1039:
#line 3481 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 13595 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1040:
#line 3482 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 13601 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1041:
#line 3486 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 13607 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1042:
#line 3487 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 13613 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3488 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13619 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3489 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13625 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3490 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13631 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3491 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13637 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3492 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 13643 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3493 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 13649 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3494 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 13655 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3495 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 13661 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3496 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 13667 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3500 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 13673 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3501 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 13679 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3506 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13685 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1055:
#line 3508 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13691 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1058:
#line 3522 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13699 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1059:
#line 3527 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 13707 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1060:
#line 3531 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13715 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1061:
#line 3536 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 13723 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1062:
#line 3542 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13729 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3543 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13735 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3547 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13741 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3548 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13747 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3554 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13753 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3558 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 13759 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3564 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13765 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1069:
#line 3568 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 13772 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3575 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13778 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3576 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13784 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3580 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 13792 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3583 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 13799 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3589 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13805 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3593 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13813 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1076:
#line 3596 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13821 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1077:
#line 3599 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-3]); }
#line 13828 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1078:
#line 3601 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13835 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1079:
#line 3603 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13842 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1080:
#line 3605 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13848 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1081:
#line 3610 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-3]); }
#line 13854 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1082:
#line 3611 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13860 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1083:
#line 3612 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13866 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1084:
#line 3613 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13872 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1091:
#line 3634 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13878 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1092:
#line 3635 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13884 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1095:
#line 3644 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13890 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1098:
#line 3655 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13896 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1099:
#line 3657 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13902 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1100:
#line 3661 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13908 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1101:
#line 3664 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13914 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1102:
#line 3668 "hphp.y" /* yacc.c:1646  */
    {}
#line 13920 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1103:
#line 3669 "hphp.y" /* yacc.c:1646  */
    {}
#line 13926 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1104:
#line 3670 "hphp.y" /* yacc.c:1646  */
    {}
#line 13932 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1105:
#line 3676 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13939 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1106:
#line 3681 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13949 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1107:
#line 3690 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13955 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1108:
#line 3696 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13964 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1109:
#line 3704 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 13970 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1110:
#line 3705 "hphp.y" /* yacc.c:1646  */
    { }
#line 13976 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1111:
#line 3711 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 13982 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1112:
#line 3713 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 13988 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1113:
#line 3714 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 13998 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1114:
#line 3719 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 14005 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1115:
#line 3725 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("HH\\darray"); }
#line 14012 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1116:
#line 3730 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14018 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1117:
#line 3735 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 14026 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1118:
#line 3739 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 14032 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1119:
#line 3744 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 14038 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1120:
#line 3746 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 14044 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1121:
#line 3752 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 14051 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1122:
#line 3754 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 14059 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1123:
#line 3757 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14065 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1124:
#line 3758 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 14073 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1125:
#line 3761 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 14081 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1126:
#line 3764 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14087 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1127:
#line 3767 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 14095 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1128:
#line 3770 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 14102 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1129:
#line 3772 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 14111 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1130:
#line 3778 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 14120 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1131:
#line 3784 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("HH\\varray");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 14130 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1132:
#line 3792 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14136 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1133:
#line 3793 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 14142 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;


#line 14146 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 3796 "hphp.y" /* yacc.c:1906  */

/* !PHP5_ONLY*/
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}
