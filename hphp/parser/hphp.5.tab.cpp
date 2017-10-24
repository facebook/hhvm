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



int Compiler5parse (HPHP::HPHP_PARSER_NS::Parser *_p);

#endif /* !YY_YY_HPHP_Y_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 903 "hphp.5.tab.cpp" /* yacc.c:358  */

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
#define YYLAST   19931

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  208
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  315
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1125
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  2094

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
    3066,  3067,  3071,  3072,  3073,  3077,  3079,  3087,  3088,  3092,
    3094,  3102,  3103,  3107,  3109,  3110,  3115,  3117,  3122,  3133,
    3147,  3159,  3174,  3175,  3176,  3177,  3178,  3179,  3180,  3190,
    3199,  3201,  3203,  3207,  3211,  3212,  3213,  3214,  3215,  3231,
    3232,  3242,  3243,  3244,  3245,  3246,  3247,  3248,  3249,  3251,
    3256,  3260,  3261,  3265,  3268,  3272,  3279,  3283,  3292,  3299,
    3301,  3307,  3309,  3310,  3314,  3315,  3316,  3323,  3324,  3329,
    3330,  3335,  3336,  3337,  3338,  3349,  3352,  3355,  3356,  3357,
    3358,  3369,  3373,  3374,  3375,  3377,  3378,  3379,  3383,  3385,
    3388,  3390,  3391,  3392,  3393,  3396,  3398,  3399,  3403,  3405,
    3408,  3410,  3411,  3412,  3416,  3418,  3421,  3424,  3426,  3428,
    3432,  3433,  3435,  3436,  3442,  3443,  3445,  3455,  3457,  3459,
    3462,  3463,  3464,  3468,  3469,  3470,  3471,  3472,  3473,  3474,
    3475,  3476,  3477,  3478,  3482,  3483,  3487,  3489,  3497,  3499,
    3503,  3507,  3512,  3516,  3524,  3525,  3529,  3530,  3536,  3537,
    3546,  3547,  3555,  3558,  3562,  3565,  3570,  3575,  3578,  3581,
    3583,  3585,  3587,  3591,  3593,  3594,  3595,  3598,  3600,  3606,
    3607,  3611,  3612,  3616,  3617,  3621,  3622,  3625,  3630,  3631,
    3635,  3638,  3640,  3644,  3650,  3651,  3652,  3656,  3660,  3668,
    3673,  3685,  3687,  3691,  3694,  3696,  3701,  3706,  3712,  3715,
    3720,  3725,  3727,  3734,  3736,  3739,  3740,  3743,  3746,  3747,
    3752,  3754,  3758,  3764,  3774,  3775
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
     426,   427,   428,   429,   430,   431,   432,   433,    40,    41,
      59,   123,   125,    93,    36,    96,    34,    39
};
# endif

#define YYPACT_NINF -1761

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1761)))

#define YYTABLE_NINF -1126

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1126)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1761,   126, -1761, -1761,  5543, 15248, 15248,    11, 15248, 15248,
   15248, 15248, 12673, 15248, -1761, 15248, 15248, 15248, 15248, 18569,
   18569, 15248, 15248, 15248, 15248, 15248, 15248, 15248, 15248, 12869,
   19366, 15248,    31,    58, -1761, -1761, -1761,   209, -1761,   254,
   -1761, -1761, -1761,   226, 15248, -1761,    58,   220,   233,   247,
   -1761,    58, 13065,  5363, 13270, -1761, 16005, 11479,   117, 15248,
    4488,    81,   162,   565,   471, -1761, -1761, -1761,   265,   355,
     379,   397, -1761,  5363,   404,   421,   577,   601,   608,   621,
     624, -1761, -1761, -1761, -1761, -1761, 15248,   626,  2045, -1761,
   -1761,  5363, -1761, -1761, -1761, -1761,  5363, -1761,  5363, -1761,
     548,   527,  5363,  5363, -1761,   464, -1761, -1761, 13475, -1761,
   -1761,   528,   572,   654,   654, -1761,   766,   639,   720,   656,
   -1761,    92, -1761,   670,   777,   848, -1761, -1761, -1761, -1761,
    2791,   649, -1761,    99, -1761,   700,   705,   714,   725,   729,
     736,   739,   743, 17259, -1761, -1761, -1761, -1761, -1761,   140,
     850,   864,   875,   877,   879, -1761,   881,   883, -1761,   343,
     763, -1761,   831,    70, -1761,  1736,   199, -1761, -1761,  2523,
      99,    99,   770,   148, -1761,   103,    43,   799,   388, -1761,
     175, -1761,   936, -1761,   849, -1761, -1761,   818,   857, -1761,
   15248, -1761,   848,   649, 19862,  2887, 19862, 15248, 19862, 19862,
   16541, 16541,   824, 18741, 19862,   977,  5363,   958,   958,   661,
     958, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761,
     108, 15248,   846, -1761, -1761,   868,   842,    67,   843,    67,
     958,   958,   958,   958,   958,   958,   958,   958, 18569, 18789,
     829,  1033,   849, -1761, 15248,   846, -1761,   884, -1761,   887,
     852, -1761,   202, -1761, -1761, -1761,    67,    99, -1761, 13671,
   -1761, -1761, 15248, 10053,  1043,   102, 19862, 11078, -1761, 15248,
   15248,  5363, -1761, -1761, 17307,   853, -1761, 17355, -1761, -1761,
   -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, 17732, -1761,
   17732, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761,
   -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761,
   -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761,
   -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761,
   -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761,
     105,   116,   857, -1761, -1761, -1761, -1761,   856, 17587,   119,
   -1761, -1761,   894,  1045, -1761,   895, 16732, 15248, -1761,   860,
     861, 17403, -1761,    75, 17451, 15700, 15700, 15700,  5363, 15700,
     865,  1053,   869, -1761,   133, -1761, 18156,   120, -1761,  1051,
     125,   942, -1761,   944, -1761, 18569, 15248, 15248,   882,   897,
   -1761, -1761, 18257, 12869, 15248, 15248, 15248, 15248, 15248,   128,
     628,   485, -1761, 15444, 18569,   630, -1761,  5363, -1761,   612,
     639, -1761, -1761, -1761, -1761, 19467,  1068,   983, -1761, -1761,
   -1761,    94, 15248,   888,   891, 19862,   892,   734,   893,  6158,
   15248,    88,   898,   686,    88,   532,   516, -1761,  5363, 17732,
     901, 11684, 16005, -1761, 13876,   907,   907,   907,   907, -1761,
   -1761,  3548, -1761, -1761, -1761, -1761, -1761,   848, -1761, 15248,
   15248, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761,
   15248, 15248, 15248, 15248, 14072, 15248, 15248, 15248, 15248, 15248,
   15248, 15248, 15248, 15248, 15248, 15248, 15248, 15248, 15248, 15248,
   15248, 15248, 15248, 15248, 15248, 15248, 15248, 15248, 19568, 15248,
   -1761, 15248, 15248, 15248,  4969,  5363,  5363,  5363,  5363,  5363,
    2791,   994,   709, 11283, 15248, 15248, 15248, 15248, 15248, 15248,
   15248, 15248, 15248, 15248, 15248, 15248, -1761, -1761, -1761, -1761,
     768, -1761, -1761, 11684, 11684, 15248, 15248,   528,   208, 18257,
     912,   848, 14268, 17499, -1761, 15248, -1761,   917,  1107,   959,
     918,   921,  5313,    67, 14464, -1761, 14660, -1761,   852,   926,
     928,  1935, -1761,   420, 11684, -1761,  2834, -1761, -1761, 17547,
   -1761, -1761, 12076, -1761, 15248, -1761,  1036, 10258,  1123,   933,
   19740,  1120,   144,    80, -1761, -1761, -1761,   953, -1761, -1761,
   -1761, 17732, -1761,  3393,   939,  1130, 17982,  5363, -1761, -1761,
   -1761, -1761, -1761, -1761, -1761, -1761, -1761,   943, -1761, -1761,
     941,   940,   946,   948,   949,   952,   488,   966,   954,  3836,
   15878, -1761, -1761,  5363,  5363, 15248,    67,    81, -1761, 17982,
    1061, -1761, -1761, -1761,    67,   152,   156,   960,   961,  2305,
     387,   970,   976,   510,  1047,   972,    67,   165,   980, 18850,
     979,  1176,  1177,   984,   992,   997,   998, -1761,  5385,  5363,
   -1761, -1761,  1127,  2816,    61, -1761, -1761, -1761,   639, -1761,
   -1761, -1761,  1174,  1073,  1027,   517,  1048, 15248,   528,  1074,
    1205,  1016, -1761,  1055, -1761,   208, -1761, 17732, 17732,  1208,
    1043,    94, -1761,  1024,  1215, -1761, 17635,   114, -1761,   569,
     225, -1761, -1761, -1761, -1761, -1761, -1761, -1761,  1673,  3031,
   -1761, -1761, -1761, -1761,  1216,  1046, -1761, 18569,   129, 15248,
    1028,  1220, 19862,  1217,   166,  1225,  1040,  1041,  1044, 19862,
    1049,  2435,  6363, -1761, -1761, -1761, -1761, -1761, -1761,  1101,
    3654, 19862,  1042,  3211, 13659, 15432, 16541, 16184, 15248, 19814,
   16363, 11859, 14246,  4804,  4724, 14831, 19550, 19550, 19550, 19550,
    4067,  4067,  4067,  4067,  4067,  1252,  1252,   905,   905,   905,
     661,   661,   661, -1761,   958,  1054,  1059, 18898,  1056,  1237,
      55, 15248,   236,   846,   297,   208, -1761, -1761, -1761,  1254,
     983, -1761,   848, 18367, -1761, -1761, -1761, 16541, 16541, 16541,
   16541, 16541, 16541, 16541, 16541, 16541, 16541, 16541, 16541, 16541,
   -1761, 15248,   506,   374, -1761, -1761,   846,   658,  1067,  1070,
    1069,  3825,   167,  1075, -1761, 19862, 18058, -1761,  5363, -1761,
      67,   610, 18569, 19862, 18569, 18959,  1101,   363,    67,   380,
    1113,  1077, 15248, -1761,   381, -1761, -1761, -1761,  6568,   735,
   -1761, -1761, 19862, 19862,    58, -1761, -1761, -1761, 15248,  1178,
   17830, 17982,  5363, 10463,  1080,  1084, -1761,  1268,  5056,  1148,
   -1761,  1125, -1761,  1280,  1091,  4412, 17732, 17982, 17982, 17982,
   17982, 17982,  1093,  1222,  1223,  1229,  1231,  1238,  1112, 17982,
     723, -1761, -1761, -1761, -1761, -1761, -1761,    10, -1761, 13048,
   -1761, -1761,   499, -1761,  6773,  5071,  1110, 15878, -1761, 15878,
   -1761, 15878, -1761,  5363,  5363, 15878, -1761, 15878, 15878,  5363,
   -1761,  1303,  1115, -1761,   494, -1761, -1761,  4026, -1761, 13048,
    1300, 18569,  1118, -1761, -1761, -1761, -1761, -1761, -1761, -1761,
   -1761, -1761,  1131,  1314,  5363,  5071,  1124, 18257, 18468,  1310,
   -1761, 15248, -1761, 15248, -1761, 15248, -1761, -1761, -1761, -1761,
   -1761, -1761, -1761, -1761,  1126, -1761, 15248, -1761, -1761,  5748,
   -1761, 17732,  5071,  1129, -1761, -1761, -1761, -1761,  1313,  1132,
   15248, 19467, -1761, -1761,  4969,  1128, -1761, 17732, -1761,  1138,
    6978,  1302,   146, -1761, 17732, -1761,    98,   768,   768, -1761,
   17732, -1761, -1761,    67, -1761, -1761,  1265, 19862, -1761, 11880,
   -1761, 17982, 13876,   907, 13876, -1761,   907,   907, -1761, 12281,
   -1761, -1761,  7183, -1761,   132,  1139,  5071,  1073, -1761, -1761,
   -1761, -1761, 16363, 15248, -1761, -1761, 15248, -1761, 15248, -1761,
    4153,  1141, 11684,  1047,  1308,  1073, 17732,  1327,  1101,  5363,
   19568,    67,  4280,  1145,   365,  1151, -1761, -1761,  1332,  3821,
    3821, 18058, -1761, -1761, -1761,  1298,  1154,  1283,  1284,  1286,
    1288,  1292,   489,  1168,   618, -1761, -1761, -1761, -1761, -1761,
    1206, -1761, -1761, -1761, -1761,  1358,  1170,   917,    67,    67,
   14856,  1073,  2834, -1761,  2834, -1761,  4663,   815,    58, 11078,
   -1761,  7388,  1172,  7593,  1173, 17830, 18569,  1180,  1243,    67,
   13048,  1365, -1761, -1761, -1761, -1761,   696, -1761,   304, 17732,
    1204,  1253,  1232, 17732,  5363, 15531, -1761, -1761, 17732,  1387,
    1199,  1240,  1246,  1216,   862,   862,  1336,  1336, 19116,  1218,
    1399, 17982, 17982, 17982, 17982, 17982, 17982, 19467,  3319, 16886,
   17982, 17982, 17982, 17982, 17906, 17982, 17982, 17982, 17982, 17982,
   17982, 17982, 17982, 17982, 17982, 17982, 17982, 17982, 17982, 17982,
   17982, 17982, 17982, 17982, 17982, 17982, 17982, 17982,  5363, -1761,
   -1761,  1340, -1761, -1761,  1234,  1235,  1242, -1761,  1247, -1761,
   -1761,   524,  3836, -1761,  1219, -1761, 17982,    67, -1761, -1761,
     145, -1761,   631,  1418, -1761, -1761,   170,  1233,    67, 12477,
   19862, 19007, -1761,  2378, -1761,  5953,   983,  1418, -1761,   659,
      51, -1761, 19862,  1305,  1249, -1761,  1248,  1302, -1761, -1761,
   -1761, 15052, 17732,  1043, 17684,  1367,   110,  1440,  1372,   383,
   -1761,   846,   386, -1761,   846, -1761, 15248, 18569,   129, 15248,
   19862, 13048, -1761, -1761, -1761,  3095, -1761, -1761, -1761, -1761,
   -1761, -1761,  1258,   132, -1761,  1257,   132,  1263, 16363, 19862,
   19068,  1266, 11684,  1267,  1264, 17732,  1285,  1287, 17732,  1073,
   -1761,   852,   680, 11684, 15248, -1761, -1761, -1761, -1761, -1761,
   -1761,  1346,  1294,  1480,  1398, 18058, 18058, 18058, 18058, 18058,
   18058,  1347, -1761, 19467,   525, 18058, -1761, -1761, -1761, 18569,
   19862,  1304, -1761,    58,  1463,  1422, 11078, -1761, -1761, -1761,
    1307, 15248,  1243,    67, 18257, 17830,  1311, 17982,  7798,   878,
    1309, 15248,    60,   449, -1761,  1328, -1761, 17732,  5363, -1761,
    1373, -1761, -1761, -1761, 17538, -1761,  1481, -1761,  1316, 17982,
   -1761, 17982, -1761,  1317,  1323,  1508, 19176,  1331, 13048,  1521,
    1333,  1334,  1335,  1397,  1530,  1341, -1761, -1761, -1761, 19222,
    1339,  1535, 15231, 16728,  5314, 17982, 12656, 12848, 14442, 14637,
   14048, 18142, 18239, 18239, 18239, 18239,  3659,  3659,  3659,  3659,
    3659,  1427,  1427,   862,   862,   862,  1336,  1336,  1336,  1336,
   -1761,  1350, -1761,  1348,  1352,  1354,  1355, -1761, -1761, 13048,
    5363, 17732, 17732, -1761,   631,  5071,   101, -1761, 18257, -1761,
   -1761, 16541, 15248,  1356, -1761,  1361,   235, -1761,   154, 15248,
   -1761, -1761, -1761, 15248, -1761, 15248, -1761,  1043, 13876,  1357,
     452,   907,   452,   378, -1761, -1761, 17732,   150, -1761,  1536,
    1477, 15248, -1761,  1368,  1369,  1362,    67,  1265, 19862,  1302,
    1371, -1761,  1379,   132, 15248, 11684,  1380, -1761, -1761,   983,
   -1761, -1761,  1382,  1383,  1370, -1761,  1381, 18058, -1761, 18058,
   -1761, -1761,  1386,  1385,  1577,  1451,  1390, -1761,  1585,  1394,
    1395,  1400, -1761,  1462,  1403,  1595, -1761, -1761,    67, -1761,
    1574, -1761,  1411, -1761, -1761,  1413,  1414,   178, -1761, -1761,
   13048,  1423,  1424, -1761, 17211, -1761, -1761, -1761, -1761, -1761,
   -1761,  1495, 17732, 17732,  1240,  1458, 17732, -1761, 13048, 19282,
   -1761, -1761, 17982, -1761, 17982, -1761, 17982, -1761, -1761, -1761,
   -1761, 17982, 19467, -1761, -1761, 17982, -1761, 17982, -1761, 11664,
   17982,  1434,  8003, -1761, -1761, -1761, -1761,   631, -1761, -1761,
   -1761, -1761,   822, 16183,  5071,  1523, -1761,  4864,  1468,  1241,
   -1761, -1761, -1761,   994,  4756,   130,   134,  1438,   983,   709,
     181, 19862, -1761, -1761, -1761,  1473,  5240, 13256, 19862, -1761,
    2541, -1761,  6363,  1558,   164,  1628,  1561, 15248, -1761, 19862,
   11684, 11684, -1761,  1527,  1302,  1349,  1302,  1448, 19862,  1449,
   -1761,  1450,  1452,  1775, -1761, -1761,   132, -1761, -1761,  1516,
   -1761, -1761, 18058, -1761, 18058, -1761, 18058, -1761, -1761, -1761,
   -1761, 18058, -1761, 19467, -1761,  2033, -1761,  8208, -1761, -1761,
   -1761, -1761, 10668, -1761, -1761, -1761,  6568, 17732, -1761, -1761,
   -1761,  1456, 17982, 19328, 13048, 13048, 13048,  1519, 13048, 19388,
   11664, -1761, -1761,   631,  5071,  5071,  5363, -1761,  1644, 17040,
     112, -1761, 16183,   983,  4284, -1761,  1478, -1761,   136,  1459,
     139, -1761, 16540, -1761, -1761, -1761,   142, -1761, -1761,  3308,
   -1761,  1466, -1761,  1579,   848, -1761, 16362, -1761, 16362, -1761,
   -1761,  1649,   994, -1761, 15649, -1761, -1761, -1761, -1761,  2674,
   -1761,  1654,  1586, 15248, -1761, 19862,  1471,  1472,  1479,  1302,
    1482, -1761,  1527,  1302, -1761, -1761, -1761, -1761,  2191,  1484,
   18058,  1538, -1761, -1761, -1761,  1540, -1761,  6568, 10873, 10668,
   -1761, -1761, -1761,  6568, -1761, -1761, 13048, 17982, 17982, 17982,
    8413,  1485,  1487, -1761, 17982, -1761,  5071, -1761, -1761, -1761,
   -1761, -1761, 17732,  1467,  4864, -1761, -1761, -1761, -1761, -1761,
   -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761,
   -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761,
   -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761,
   -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761,
   -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761,
   -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761,
   -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761,
   -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761,   671,
   -1761,  1468, -1761, -1761, -1761, -1761, -1761,   123,   776, -1761,
    1666,   147, 16732,  1579,  1667, -1761, 17732,   848, -1761, -1761,
    1488,  1670, 15248, -1761, 19862, -1761, -1761,   163,  1489, 17732,
      42,  1302,  1482, 15827, -1761,  1302, -1761, 18058, 18058, -1761,
   -1761, -1761, -1761,  8618, 13048, 13048, 13048, -1761, -1761, -1761,
   13048, -1761,  1359,  1682,  1683,  1491, -1761, -1761, 17982, 16540,
   16540,  1626, -1761,  3308,  3308,   834, -1761, -1761, -1761, 17982,
    1612, -1761,  1514,  1500,   149, 17982, -1761, 16732, -1761, 17982,
   19862,  1620, -1761,  1695, -1761,  1696, -1761,   616, -1761, -1761,
   -1761,  1505,    42, -1761,    42, -1761, -1761,  8823,  1507,  1593,
   -1761,  1607,  1549, -1761, -1761,  1609, 17732,  1532,  1467, -1761,
   -1761, 13048, -1761, -1761,  1546, -1761,  1678, -1761, -1761, -1761,
   -1761, 13048,  1708,   510, -1761, -1761, 13048,  1529, 13048, -1761,
     193,  1534,  9028, 17732, -1761, 17732, -1761,  9233, -1761, -1761,
   -1761,  1531, -1761,  1539,  1554,  5363,   709,  1545, -1761, -1761,
   -1761, 17982,  1552,   138, -1761,  1656, -1761, -1761, -1761, -1761,
   -1761, -1761,  9438, -1761,  5071,  1110, -1761,  1565,  5363,   699,
   -1761, 13048, -1761,  1548,  1735,   657,   138, -1761, -1761,  1662,
   -1761,  5071,  1553, -1761,  1302,   173, -1761, 17732, -1761, -1761,
   -1761, 17732, -1761,  1550,  1555,   153, -1761,  1482,   751,  1665,
     192,  1302,  1551, -1761,   795, 17732, 17732, -1761,   331,  1741,
    1674,  1482, -1761, -1761, -1761, -1761,  1675,   197,  1742,  1677,
   15248, -1761,   795,  9643,  9848, -1761,   442,  1750,  1684, 15248,
   -1761, 19862, -1761, -1761, -1761,  1754,  1686, 15248, -1761, 19862,
   15248, -1761, 19862, 19862
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   203,   472,     0,   905,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   999,
     987,     0,   771,     0,   777,   778,   779,    29,   843,   974,
     975,   170,   171,   780,     0,   151,     0,     0,     0,     0,
      30,     0,     0,     0,     0,   222,     0,     0,     0,     0,
       0,     0,   441,   442,   443,   440,   439,   438,     0,     0,
       0,     0,   251,     0,     0,     0,    37,    38,    40,    41,
      39,   784,   786,   787,   781,   782,     0,     0,     0,   788,
     783,     0,   754,    32,    33,    34,    36,    35,     0,   785,
       0,     0,     0,     0,   789,   444,   581,    31,     0,   169,
     139,   979,   772,     0,     0,     4,   125,   127,   842,     0,
     753,     0,     6,     0,     0,   221,     7,     9,     8,    10,
       0,     0,   436,     0,   486,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   542,   484,   961,   962,   563,   557,
     558,   559,   560,   561,   562,   467,   566,     0,   466,   933,
     755,   762,     0,   845,   556,   435,   936,   937,   949,   485,
       0,     0,     0,   488,   487,   934,   935,   932,   969,   973,
       0,   546,   844,    11,   441,   442,   443,     0,     0,    36,
       0,   125,   221,     0,  1039,   485,  1040,     0,  1042,  1043,
     565,   480,     0,   473,   478,     0,     0,   528,   529,   530,
     531,    29,   974,   780,   757,    37,    38,    40,    41,    39,
       0,     0,  1063,   954,   755,     0,   756,   507,     0,   509,
     547,   548,   549,   550,   551,   552,   553,   555,     0,  1003,
       0,   852,   767,   241,     0,  1063,   464,   766,   760,     0,
     776,   756,   982,   983,   989,   981,   768,     0,   465,     0,
     770,   554,     0,   204,     0,     0,   469,   204,   149,   471,
       0,     0,   155,   157,     0,     0,   159,     0,    75,    76,
      81,    82,    67,    68,    59,    79,    90,    91,     0,    62,
       0,    66,    74,    72,    93,    85,    84,    57,   107,    80,
     100,   101,    58,    96,    55,    97,    56,    98,    54,   102,
      89,    94,    99,    86,    87,    61,    88,    92,    53,    83,
      69,   103,    77,   105,    70,    60,    47,    48,    49,    50,
      51,    52,    71,   106,   104,   109,    64,    45,    46,    73,
    1116,  1117,    65,  1121,    44,    63,    95,     0,     0,   125,
     108,  1054,  1115,     0,  1118,     0,     0,     0,   161,     0,
       0,     0,   212,     0,     0,     0,     0,     0,     0,     0,
       0,   854,     0,   113,   115,   349,     0,     0,   348,   354,
       0,     0,   252,     0,   255,     0,     0,     0,     0,  1060,
     237,   249,   995,   999,   600,   627,   627,   600,   627,     0,
    1024,     0,   791,     0,     0,     0,  1022,     0,    16,     0,
     129,   229,   243,   250,   657,   593,     0,  1048,   573,   575,
     577,   909,   472,   486,     0,     0,   484,   485,   487,   204,
       0,   773,     0,   774,     0,     0,     0,   201,     0,     0,
     131,   338,     0,    28,     0,     0,     0,     0,     0,   202,
     220,     0,   248,   233,   247,   441,   444,   221,   437,   978,
       0,   925,   191,   192,   193,   194,   195,   197,   198,   200,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   987,     0,
     190,   978,   978,  1009,     0,     0,     0,     0,     0,     0,
       0,     0,   434,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   506,   508,   910,   911,
       0,   924,   923,   338,   338,   978,     0,   980,   970,   995,
       0,   221,     0,     0,   163,     0,   907,   902,   852,     0,
     486,   484,     0,  1007,     0,   598,   851,   998,   776,   486,
     484,   485,   131,     0,   338,   463,     0,   926,   769,     0,
     139,   291,     0,   580,     0,   166,     0,   204,   470,     0,
       0,     0,     0,     0,   158,   189,   160,  1116,  1117,  1113,
    1114,     0,  1120,  1106,     0,     0,     0,     0,    78,    43,
      65,    42,  1055,   196,   199,   162,   139,     0,   179,   188,
       0,     0,     0,     0,     0,     0,   116,     0,     0,     0,
     853,   114,    18,     0,   110,     0,   350,     0,   164,     0,
       0,   165,   253,   254,  1044,     0,     0,   486,   484,   485,
     488,   487,     0,  1096,   261,     0,   996,     0,     0,     0,
       0,   852,   852,     0,     0,     0,     0,   167,     0,     0,
     790,  1023,   843,     0,     0,  1021,   848,  1020,   128,     5,
      13,    14,     0,   259,     0,     0,   586,     0,     0,     0,
     852,     0,   764,     0,   763,   758,   587,     0,     0,     0,
       0,   909,   135,     0,   854,   908,  1125,   462,   475,   489,
     942,   960,   146,   138,   142,   143,   144,   145,   435,     0,
     564,   846,   847,   126,   852,     0,  1064,     0,     0,     0,
       0,   854,   339,     0,     0,     0,   486,   208,   209,   207,
     484,   485,   204,   183,   181,   182,   184,   569,   223,   257,
       0,   977,     0,     0,   512,   514,   513,   525,     0,     0,
     545,   510,   511,   515,   517,   516,   534,   535,   532,   533,
     536,   537,   538,   539,   540,   526,   527,   519,   520,   518,
     521,   522,   524,   541,   523,     0,     0,  1013,     0,   852,
    1047,     0,  1046,  1063,   939,   969,   239,   231,   245,     0,
    1048,   235,   221,     0,   476,   479,   481,   491,   494,   495,
     496,   497,   498,   499,   500,   501,   502,   503,   504,   505,
     913,     0,   912,   915,   938,   919,  1063,   916,     0,     0,
       0,     0,     0,     0,  1041,   474,   900,   904,   851,   906,
     461,   759,     0,  1002,     0,  1001,   257,     0,   759,   986,
     985,     0,     0,   912,   915,   984,   916,   483,   293,   295,
     135,   584,   583,   468,     0,   139,   275,   150,   471,     0,
       0,     0,     0,   204,   287,   287,   156,   852,     0,     0,
    1105,     0,  1102,   852,     0,  1076,     0,     0,     0,     0,
       0,   850,     0,    37,    38,    40,    41,    39,     0,     0,
     793,   797,   798,   799,   800,   801,   803,     0,   792,   133,
     841,   802,  1063,  1119,   204,     0,     0,     0,    21,     0,
      22,     0,    19,     0,   111,     0,    20,     0,     0,     0,
     122,   854,     0,   120,   115,   112,   117,     0,   347,   355,
     352,     0,     0,  1033,  1038,  1035,  1034,  1037,  1036,    12,
    1094,  1095,     0,   852,     0,     0,     0,   995,   992,     0,
     597,     0,   611,   851,   599,   851,   626,   614,   620,   623,
     617,  1032,  1031,  1030,     0,  1026,     0,  1027,  1029,   204,
       5,     0,     0,     0,   651,   652,   660,   659,     0,   484,
       0,   851,   592,   596,     0,     0,  1049,     0,   574,     0,
     204,  1083,   909,   319,  1125,  1124,     0,     0,     0,   976,
     851,  1066,  1062,   341,   335,   336,   340,   342,   752,   853,
     337,     0,     0,     0,     0,   461,     0,     0,   489,     0,
     943,   211,   204,   141,   909,     0,     0,   259,   571,   225,
     921,   922,   544,     0,   634,   635,     0,   632,   851,  1008,
       0,     0,   338,   261,     0,   259,     0,     0,   257,     0,
     987,   492,     0,     0,   940,   941,   971,   972,     0,     0,
       0,   888,   859,   860,   861,   868,     0,    37,    38,    40,
      41,    39,     0,     0,   874,   880,   881,   882,   883,   884,
       0,   872,   870,   871,   894,   852,     0,   902,  1006,  1005,
       0,   259,     0,   927,     0,   775,     0,   297,     0,   204,
     147,   204,     0,   204,     0,     0,     0,     0,   267,   268,
     279,     0,   139,   277,   176,   287,     0,   287,     0,   851,
       0,     0,     0,     0,     0,   851,  1104,  1107,  1072,   852,
       0,  1067,     0,   852,   824,   825,   822,   823,   858,     0,
     852,   850,   604,   629,   629,   604,   629,   595,     0,     0,
    1015,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1110,
     213,     0,   216,   180,     0,     0,     0,   118,     0,   123,
     124,   116,   853,   121,     0,   351,     0,  1045,   168,  1061,
    1096,  1087,  1091,   260,   262,   361,     0,     0,   993,     0,
     602,     0,  1025,     0,    17,   204,  1048,   258,   361,     0,
       0,   759,   589,     0,   765,  1050,     0,  1083,   578,   134,
     136,     0,     0,     0,  1125,     0,     0,   324,   322,   915,
     928,  1063,   915,   929,  1063,  1065,   978,     0,     0,     0,
     343,   132,   206,   208,   209,   485,   187,   205,   185,   186,
     210,   140,     0,   909,   256,     0,   909,     0,   543,  1012,
    1011,     0,   338,     0,     0,     0,     0,     0,     0,   259,
     227,   776,   914,   338,     0,   864,   865,   866,   867,   875,
     876,   892,     0,   852,     0,   888,   608,   631,   631,   608,
     631,     0,   863,   896,     0,   851,   899,   901,   903,     0,
    1000,     0,   914,     0,     0,     0,   204,   294,   585,   152,
       0,   471,   267,   269,   995,     0,     0,     0,   204,     0,
       0,     0,     0,     0,   281,     0,  1111,     0,     0,  1097,
       0,  1103,  1101,  1068,   851,  1074,     0,  1075,     0,     0,
     795,   851,   849,     0,     0,   852,     0,     0,   838,   852,
       0,     0,     0,     0,   852,     0,   804,   839,   840,  1019,
       0,   852,   807,   809,   808,     0,     0,   805,   806,   810,
     812,   811,   828,   829,   826,   827,   830,   831,   832,   833,
     834,   819,   820,   814,   815,   813,   816,   817,   818,   821,
    1109,     0,   139,     0,     0,     0,     0,   119,    23,   353,
       0,     0,     0,  1088,  1093,     0,   435,   997,   995,   477,
     482,   490,     0,     0,    15,     0,   435,   663,     0,     0,
     665,   658,   661,     0,   656,     0,  1052,     0,     0,     0,
       0,   542,     0,   488,  1084,   582,  1125,     0,   325,   326,
       0,     0,   320,     0,     0,     0,   345,   346,   344,  1083,
       0,   361,     0,   909,     0,   338,     0,   967,   361,  1048,
     361,  1051,     0,     0,     0,   493,     0,     0,   878,   851,
     887,   869,     0,     0,   852,     0,     0,   886,   852,     0,
       0,     0,   862,     0,     0,   852,   873,   893,  1004,   361,
       0,   139,     0,   290,   276,     0,     0,     0,   266,   172,
     280,     0,     0,   283,     0,   288,   289,   139,   282,  1112,
    1098,     0,     0,  1071,  1070,     0,     0,  1123,   857,   856,
     794,   612,   851,   603,     0,   615,   851,   628,   621,   624,
     618,     0,   851,   594,   796,     0,   633,   851,  1014,   836,
       0,     0,   204,    24,    25,    26,    27,  1090,  1085,  1086,
    1089,   263,     0,     0,     0,   442,   433,     0,     0,     0,
     238,   360,   362,     0,   432,     0,     0,     0,  1048,   435,
       0,   601,  1028,   357,   244,   654,     0,     0,   588,   576,
     485,   137,   204,     0,     0,   329,   318,     0,   321,   328,
     338,   338,   334,   568,  1083,   435,  1083,     0,  1010,     0,
     966,   435,     0,   435,  1053,   361,   909,   963,   891,   890,
     877,   613,   851,   607,     0,   616,   851,   630,   622,   625,
     619,     0,   879,   851,   895,   435,   139,   204,   148,   153,
     174,   270,   204,   278,   284,   139,   286,     0,  1099,  1069,
    1073,     0,     0,     0,   606,   837,   591,     0,  1018,  1017,
     835,   139,   217,  1092,     0,     0,     0,  1056,     0,     0,
       0,   264,     0,  1048,     0,   398,   394,   400,   754,    36,
       0,   388,     0,   393,   397,   410,     0,   408,   413,     0,
     412,     0,   411,     0,   221,   364,     0,   366,     0,   367,
     368,     0,     0,   994,     0,   655,   653,   664,   662,     0,
     330,   331,     0,     0,   316,   327,     0,     0,     0,  1083,
    1077,   234,   568,  1083,   968,   240,   357,   246,   435,     0,
       0,     0,   610,   885,   898,     0,   242,   292,   204,   204,
     139,   273,   173,   285,  1100,  1122,   855,     0,     0,     0,
     204,     0,     0,   460,     0,  1057,     0,   378,   382,   457,
     458,   392,     0,     0,     0,   373,   713,   714,   712,   715,
     716,   733,   735,   734,   704,   676,   674,   675,   694,   709,
     710,   670,   681,   682,   684,   683,   751,   703,   687,   685,
     686,   688,   689,   690,   691,   692,   693,   695,   696,   697,
     698,   699,   700,   702,   701,   671,   672,   673,   677,   678,
     680,   750,   718,   719,   723,   724,   725,   726,   727,   728,
     711,   730,   720,   721,   722,   705,   706,   707,   708,   731,
     732,   736,   738,   737,   739,   740,   717,   742,   741,   744,
     746,   745,   679,   749,   747,   748,   743,   729,   669,   405,
     666,     0,   374,   426,   427,   425,   418,     0,   419,   375,
     452,     0,     0,     0,     0,   456,     0,   221,   230,   356,
       0,     0,     0,   317,   333,   964,   965,     0,     0,     0,
       0,  1083,  1077,     0,   236,  1083,   889,     0,     0,   139,
     271,   154,   175,   204,   605,   590,  1016,   215,   376,   377,
     455,   265,     0,   852,   852,     0,   401,   389,     0,     0,
       0,   407,   409,     0,     0,   414,   421,   422,   420,     0,
       0,   363,  1058,     0,     0,     0,   459,     0,   358,     0,
     332,     0,   649,   854,   135,   854,  1079,     0,   428,   135,
     224,     0,     0,   232,     0,   609,   897,   204,     0,   177,
     379,   125,     0,   380,   381,     0,   851,     0,   851,   403,
     399,   404,   667,   668,     0,   390,   423,   424,   416,   417,
     415,   453,   450,  1096,   369,   365,   454,     0,   359,   650,
     853,     0,   204,   853,  1078,     0,  1082,   204,   135,   226,
     228,     0,   274,     0,   219,     0,   435,     0,   395,   402,
     406,     0,     0,   909,   371,     0,   647,   567,   570,  1080,
    1081,   429,   204,   272,     0,     0,   178,   386,     0,   434,
     396,   451,  1059,     0,   854,   446,   909,   648,   572,     0,
     218,     0,     0,   385,  1083,   909,   301,  1125,   449,   448,
     447,  1125,   445,     0,     0,     0,   384,  1077,   446,     0,
       0,  1083,     0,   383,     0,  1125,  1125,   307,     0,   306,
     304,  1077,   139,   430,   135,   370,     0,     0,   308,     0,
       0,   302,     0,   204,   204,   312,     0,   311,   300,     0,
     303,   310,   372,   214,   431,   313,     0,     0,   298,   309,
       0,   299,   315,   314
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1761, -1761, -1761,  -584, -1761, -1761, -1761,   104,  -449,   -47,
     611, -1761,  -199,  -496, -1761, -1761,   579,   806,  1645, -1761,
    2608, -1761,  -804, -1761,  -538, -1761,  -699,    44, -1761, -1761,
   -1761, -1761, -1761, -1761, -1761, -1761,  -919, -1761, -1761,  -917,
    -330, -1761, -1761, -1761,  -243, -1761, -1761,  -167,   390,    41,
   -1761, -1761, -1761, -1761, -1761, -1761,    46, -1761, -1761, -1761,
   -1761, -1761, -1761,    56, -1761, -1761,  1269,  1270,  1271,   -66,
    -750,  -947,   731,   805,  -251,   463, -1017, -1761,    48, -1761,
   -1761, -1761, -1761,  -782,   274, -1761, -1761, -1761, -1761,  -237,
   -1761,  -604, -1761,   542,  -444, -1761, -1761,  1164, -1761,    77,
   -1761, -1761, -1083, -1761, -1761, -1761, -1761, -1761, -1761, -1761,
   -1761, -1761, -1761,    28, -1761,   135, -1761, -1761, -1761, -1761,
   -1761,   -50, -1761,   241,  -913, -1761, -1203,  -268, -1761,  -150,
     273,  -129,  -236, -1761,   -52, -1761, -1761, -1761,   249,   -93,
     -94,    66,  -767,   -64, -1761, -1761,    25, -1761,   -20,  -383,
   -1761,     7,    -5,   -85,   -81,    95, -1761, -1761, -1761, -1761,
   -1761, -1761, -1761, -1761, -1761,  -628,  -907, -1761, -1761, -1761,
   -1761, -1761,  1245,  1419, -1761,   673, -1761,   520, -1761, -1761,
   -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761, -1761,
   -1761, -1761, -1761, -1761,   399,  -301,  -372, -1761, -1761, -1761,
   -1761, -1761,   602, -1761, -1761, -1761, -1761, -1761, -1761, -1761,
   -1761,  -890,  -381,  2610,    29, -1761,   504,  -422, -1761, -1761,
    -489,  3584,  3644, -1761,   641, -1761, -1761,   679,  -497,  -637,
   -1761, -1761,   762,   530,  -261, -1761,   531, -1761, -1761, -1761,
   -1761, -1761,   740, -1761, -1761, -1761,   127,  -926,  -163,  -428,
    -425, -1761,     5,  -142, -1761, -1761,    45,    47,   660,   -82,
   -1761, -1761,    23,   -43, -1761,  -359,    74,    86, -1761,  -429,
   -1761, -1761, -1761,  -445,  1437, -1761, -1761, -1761, -1761, -1761,
     876,   757, -1761, -1761, -1761,  -348,  -690, -1761,  1389, -1266,
     -95,   -67,  -166,   957, -1761, -1761, -1761, -1760, -1761,  -148,
   -1012, -1336,  -137,   280, -1761,   638,   715, -1761, -1761, -1761,
   -1761,   666, -1761,  2697,  -805
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   115,   970,   669,   191,   350,   783,
     370,   371,   372,   373,   921,   922,   923,   117,   118,   119,
     120,   121,   990,  1229,   429,  1022,   703,   704,   577,   267,
    1738,   583,  1642,  1739,  1994,   906,   123,   124,   724,   725,
     733,   363,   606,  1949,  1183,  1402,  2016,   451,   192,   705,
    1025,  1267,  1474,   127,   672,  1044,   706,   739,  1048,   644,
    1043,   246,   558,   707,   673,  1045,   453,   390,   412,   130,
    1027,   973,   946,  1203,  1670,  1326,  1108,  1891,  1742,   857,
    1114,   582,   866,  1116,  1517,   849,  1097,  1100,  1315,  2023,
    2024,   693,   694,  1006,   720,   721,   377,   378,   380,  1704,
    1869,  1870,  1416,  1571,  1693,  1863,  2003,  2026,  1902,  1953,
    1954,  1955,  1680,  1681,  1682,  1683,  1904,  1905,  1911,  1965,
    1686,  1687,  1691,  1856,  1857,  1858,  1940,  2065,  1572,  1573,
     193,   132,  2041,  2042,  1861,  1575,  1576,  1577,  1578,   133,
     134,   578,   579,   135,   136,   137,   138,   139,   140,   141,
     142,   260,   143,   144,   145,  1719,   146,  1024,  1266,   147,
     690,   691,   692,   264,   421,   573,   679,   680,  1364,   681,
    1365,   148,   149,   650,   651,  1354,  1355,  1483,  1484,   150,
     891,  1075,   151,   892,  1076,   152,   893,  1077,   153,   894,
    1078,   154,   895,  1079,   653,  1357,  1486,   155,   896,   156,
     157,  1933,   158,   674,  1706,   675,  1219,   978,  1434,  1431,
    1849,  1850,   159,   160,   161,   249,   162,   250,   261,   432,
     565,   163,  1358,  1359,   900,   901,   164,  1139,   557,   621,
    1140,  1082,  1289,  1083,  1487,  1488,  1292,  1293,  1085,  1494,
    1495,  1086,   827,   548,   205,   206,   708,   696,   530,  1239,
    1240,   815,   816,   461,   166,   252,   167,   168,   195,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   742,
     180,   256,   257,   647,   240,   241,   778,   779,  1370,  1371,
     405,   406,   964,   181,   635,   182,   689,   183,   353,  1871,
    1923,   391,   440,   714,   715,  1129,  1130,  1880,  1935,  1936,
    1233,  1413,   942,  1414,   943,   944,   872,   873,   874,   354,
     355,   903,   592,   995,   996
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     194,   196,   458,   198,   199,   200,   201,   203,   204,   351,
     207,   208,   209,   210,   423,   511,   230,   231,   232,   233,
     234,   235,   236,   237,   239,   540,   258,   426,   531,   532,
     445,   413,   848,  1023,   682,   416,   417,   360,   446,   266,
     263,   447,   228,   228,   424,   126,  1101,   274,   122,   277,
     128,   829,   361,   268,   364,   782,   684,   993,   272,   248,
     129,   728,   988,   454,   458,   428,  1234,   686,   904,   836,
    1132,  1231,   775,   776,  1223,   253,   773,   254,  1560,   562,
    1265,   266,   359,  1118,  1010,   969,  1091,   989,  1322,   818,
     819,  1104,  1515,   566,   822,  1252,   423,  1257,  1276,   510,
    1047,   442,   813,   425,   255,   814,   820,  1562,   116,   426,
     265,   574,   864,  -946,   -78,   567,   734,   735,   736,   -78,
     841,  1756,  1942,   920,   925,   -43,     3,   550,   -42,   627,
     -43,   165,  1913,   -42,   630,  1426,  1236,   574,   844,  1696,
     551,   845,    14,  1698,  1311,  -391,   448,   428,  1764,    14,
     559,  1851,   399,   862,   954,   956,  1920,   275,  1920,  1914,
     349,   931,  1756,   560,   375,   574,   611,   613,   615,   459,
     618,   400,  1149,  -945,   948,  1012,   948,   389,   428,   948,
      14,  1237,  1231,   982,  -461,   425,    14,   948,  1594,  1235,
     948,   607,   543,  1449,    14,   940,   941,   528,   529,   549,
     411,  1931,   389,   528,   529,  -763,   389,   389,  1585,   197,
    -636,  1150,  1004,  1005,  1563,  1437,   425,  1001,  -643,  1432,
    1564,    14,   455,  1565,   186,    65,    66,    67,  1566,   259,
    2058,  2005,  -756,  1595,   389,  2076,   623,   659,  -109,   425,
    1363,  1562,  1938,  1939,  -946,   379,  1932,  1711,   403,   404,
     528,   529,  1433,  -109,   569,   608,   262,   569,   212,    40,
    1516,   228,  -579,   968,   266,   580,   538,   402,  -955,  -947,
    1567,  1568,  -988,  1569,  1238,  2059,  2006,   131,   535,  -646,
    2077,   865,  1039,    14,  1193,   376,  1450,  2054,   591,  -323,
     740,  1230,   443,   637,   456,  -943,  -644,  1668,  1279,   624,
     460,  2072,   575,  1570,  -945,   -78,   638,   571,  1508,   602,
     547,   576,  1757,  1758,  1439,  -461,   -43,  1103,   362,   -42,
     628,   414,  1915,  1261,  -757,   631,  1596,  1560,   657,  -851,
    1697,  -323,  1473,  1329,  1699,  1333,  -391,  -305,  -954,  1765,
    1712,  -853,  1852,   863,   641,  -853,   534,  1921,  1563,  1975,
     726,   932,   201,  2053,  1564,   933,   455,  1565,   186,    65,
      66,    67,  1566,   730,   949,  1013,  1058,   116,  2060,  1417,
    1120,   116,  -853,  2078,   823,   581,  1126,  1641,  1605,   537,
    1703,   266,   425,   458,   738,  1611,  1215,  1613,   239,   649,
     266,   266,   649,   266,   125,   351,  1493,  -952,   663,   228,
    -947,   428,  1230,  -988,  1567,  1568,  1759,  1569,   228,   536,
     640,  1331,  1332,  -944,  2068,   228,  1635,   203,   269,  -108,
    1262,  1189,  1190,  -956,   727,   709,  -943,   228,   456,  1447,
    1864,   270,  1865,   459,  -108,  -950,   722,  1584,   512,   729,
     413,   789,   790,   454,  -948,   271,  1201,  1603,  -643,   794,
    -991,  -990,   636,  -930,   741,   743,  -931,  -643,   535,  -765,
     601,   652,   652,   385,   652,   744,   745,   746,   747,   749,
     750,   751,   752,   753,   754,   755,   756,   757,   758,   759,
     760,   761,   762,   763,   764,   765,   766,   767,   768,   769,
     770,   771,   772,   796,   774,  1042,   741,   741,   777,   528,
     529,   685,  1206,   212,    40,  -764,  1334,  2069,   797,   798,
     799,   800,   801,   802,   803,   804,   805,   806,   807,   808,
     809,   457,   459,   225,   225,  2085,  1425,   248,   722,   722,
     741,   821,  1728,   116,   997,   782,   998,   797,   795,  1525,
     825,  -953,   713,   253,  -944,   254,   349,   418,   695,   833,
    -758,   835,   851,   386,  1505,   389,  1331,  1332,   511,   722,
     940,   941,   228,  -959,   460,  1084,  -950,   852,   400,   853,
    1242,   975,   255,  1243,  1328,  -948,   534,   387,   784,   732,
    1301,  -991,  -990,   541,  -930,   534,   937,  -931,  1306,   536,
     785,   913,  1720,   383,  1722,   388,   979,   623,  1273,   400,
     682,  1986,   392,   384,   817,  1445,  1496,   665,   601,   389,
     787,   389,   389,   389,   389,   400,   785,  1041,  2086,   393,
     927,   856,   684,   665,   111,  1049,   784,   913,  1281,  1254,
    1985,  1254,  1345,   686,   812,  1657,  1348,   840,   785,  -645,
     846,  1593,   510,  1352,   660,   403,   404,   394,  1302,   785,
    1053,  1518,   785,  1592,   914,   400,   601,  1367,   419,  1460,
     438,  1178,  1462,   431,   169,   420,  1242,   997,   998,  1243,
     843,   395,   425,  1029,  1092,  1094,   403,   404,   396,   227,
     229,   116,   439,  1256,   976,  1908,  1258,  1259,   381,  -917,
     438,   397,   403,   404,   398,  1093,   920,   382,   658,   977,
    1368,   902,   131,  1909,  -917,   528,   529,  1878,  1184,   400,
    1185,  1882,  1186,   400,  1007,   414,  1188,   401,   498,  1411,
    1412,   665,  1910,   712,   562,   415,  1735,   926,   713,   430,
     499,  -759,   403,   404,  1411,  1412,  1179,   400,   711,  1989,
     228,  1990,   225,  1032,  2037,   434,   528,   529,   542,   514,
     515,   516,   517,   518,   519,   520,   521,   522,   523,   524,
     525,    55,   963,   965,   528,   529,   682,  -957,   427,   400,
     455,   185,   186,    65,    66,    67,  1040,   665,   438,  1612,
    -125,  2038,  2039,  2040,  -125,   402,   403,   404,   684,   666,
     403,   404,  1475,   526,   527,   654,  1480,   656,   437,   686,
    1291,  -125,  1330,  1331,  1332,   438,  1052,  1916,  -957,  1589,
    1098,  1099,   670,   671,   403,   404,   228,  1455,   695,   125,
     455,   185,   186,    65,    66,    67,  1917,  1427,  1466,  1918,
     455,   185,   186,    65,    66,    67,   116,  1096,  2055,  1476,
    1428,  -920,   456,  1360,   389,  1362,   403,   404,  1254,   211,
     427,   212,    40,   266,   441,   228,  -920,   228,  1533,  1607,
    1429,  1102,  1537,  -918,  1552,  1968,   374,  1543,   444,  1941,
     528,   529,    50,  1944,  1548,  2038,  2039,  2040,  -918,  1507,
     225,   427, -1063,   228,  1969, -1063,   450,  1970,  1701,   225,
    1313,  1314,   456,  1023,   409,   449,   225,   410,   553,  2033,
     462,   792,   456,   439,   561,   463,   439,  1113,   225,   215,
     216,   217,   218,   219,   464,  1174,  1175,  1176, -1063,   683,
    -637, -1063,   682,   169, -1063,   465,  1489,   169,  1491,   466,
    1074,  1177,  1087,   700,  -638,   810,   467,    93,    94,   468,
      95,   189,    97,   469,   684,  -639,  1210,  -640,  1211,  -641,
     853,   501,   116,   502,   228,   686,  1664,  1665,   495,   496,
     497,  1213,   498,  1637,   503,   107,  1111,   116,   533,   811,
     228,   228,   111,  1580,   499,  1222,   610,   612,   614,  1646,
     617,   512,  1280,  1760,  1512,  1331,  1332,  1623,   433,   435,
     436,  1627,  1253,   504,  1253,  2063,  2064,  -951,  1634,   796,
    1966,  1967,  1241,  1244,  1250,   131,  -642,   729,   116,   729,
     126,  -757,  1729,   122,   797,   128,   539,  1187,   713,  1962,
    1963,  1609,  2047,   407,   544,   129,   546,   499,  1268,   439,
     552,  1269,   555,  1270,  1291,  1485,   626,   722,  1485,  2061,
    -955,   534,   556,   225,  1497,   634,  -755,   639,  1202,   563,
     564,   572,   646,   585,   593,   728, -1108,   597,  1224,   596,
     603,   604,   620,  1231,   664,   629,   619,   685,  1231,   622,
     785,   817,   817,   116,   632,  1453,   633,  2025,  1454,   248,
     643,   642,   687,   785,   785,  1310,   688,   697,   601,   169,
     698,   699,   701,  1231,   116,   253,   165,   254,  1737,  -130,
    2025,   812,   812,   710,   731,  1316,    55,  1743,   732,  2048,
     737,   734,   735,   736,  1667,   826,   828,   830,   660,   695,
     831,   131,   125,  1750,   255,   837,   116,   838,   228,   228,
    1982,   854,   574,   858,   861,  1987,   591,   875,  1440,   876,
     908,   905,   907,  1317,   930,  1419,  1441,   909,   910,  1442,
     911,   695,   912,   389,   916,  1231,  1716,  1717,   661,   934,
     935,   682,   667,  1288,  1288,  1074,   846,   915,   846,   938,
     947,   374,   374,   374,   616,   374,   939,   131,   785,   950,
     785,   945,   952,   684,  2012,   953,   955,   957,   661,  1420,
     667,   661,   667,   667,   686,   958,   843,   966,   843,   646,
     959,   960,  1893,   116,  1421,   116,   971,   116,   972,   974,
    -780,  1253,   980,   668,   981,   983,  1618,   984,  1619,   991,
    1755,   225,   987,  1667,   992,  1000,   729,  1008,  1340,  1009,
    1002,  1011,  2049,   685,  1014,  1026,  2050,   169,   125,  1015,
    1016,   741,   131,  1017,  1458,  1030,  1038,  1667,  1018,  1667,
    2066,  2067,   682,   601,  1443,  1667,   126,  1034,  1037,   122,
    2074,   128,  1035,   131,   223,   223,  1054,   722,  1046,  1055,
     228,   129,  1056,  1028,   684,  -761,  1095,  1119,   722,  1421,
    1115,  1105,   902,  1230,  1117,   686,  1123,  1124,  1230,  1125,
    1127,  1141,  1142,  1143,   125,   131,  1981,   225,  1984,  1144,
     531,  1145,   492,   493,   494,   495,   496,   497,  1146,   498,
    1147,  1182,  1192,  1230,  1196,  1199,   266,  1194,  1198,   116,
    1500,   499,   211,  1200,  1209,  1205,  1514,  1220,  1225,  1212,
    1218,  1221,   228,  1227,  1232,  1246,   225,  1263,   225,  1272,
    1275,  1278,   165,  1283,   423,    50,  1284,   228,   228,  -958,
    1294,  1947,  1295,  1296,  1297,  1562,  1298,   426,  1299,   125,
    1503,  1731,  1300,  1732,   225,  1733,  1303,  1305,  1304,  1307,
    1734,  1688,  1319,  1321,   131,  1230,   131,  1003,  1324,  1327,
     125,  1325,   215,   216,   217,   218,   219,  2036,  1336,   685,
     695,  1337,   169,   695,  1338,   428,  1344,    14,  1346,  1074,
    1074,  1074,  1074,  1074,  1074,  1177,  1957,  1959,  1351,  1074,
      93,    94,   125,    95,   189,    97, -1124,  1581,   727,  1408,
     116,  1350,  1347,  1401,  1586,   924,   924,  1415,  1587,  1702,
    1588,  1418,   116,   729,  1667,   225,  1403,  1404,   107,  1689,
     211,   228,  1521,  1435,  1405,   458,  1599,  1042,  1436,  1406,
    1448,   225,   225,  1051,  1451,  1452,  1562,  1459,  1461,  1608,
     722,  1463,  1563,    50,  1465,  1468,  1467,  1081,  1564,  1886,
     455,  1565,   186,    65,    66,    67,  1566,  1171,  1172,  1173,
    1174,  1175,  1176,   223,  1477,   683,  1470,  1471,   131,  1479,
    1065,   125,  1088,   125,  1089,  1501,  1177,  1478,    14,  1502,
     215,   216,   217,   218,   219,  1499,  1492,  1504,   169,  1513,
    1509,  1522,  1519,  1526,  1557,  1527,  1530,  1532,  1567,  1568,
    1109,  1569,   188,   169,  2073,    91,  1531,  1862,    93,    94,
    1536,    95,   189,    97,  1535,  1541,  1538,  1539,  1540,  1542,
    1544,  1546,   456,  1579,  1547,    34,    35,    36,  1553,  1551,
    1597,  1721,  1554,  1579,  1555,  1556,   107,  1591,  1582,   213,
    1598,  1950,  1583,  1563,   169,  1602,  1600,  1601,  1616,  1564,
    1604,   455,  1565,   186,    65,    66,    67,  1566,  1606,  1610,
    1617,  1074,  1614,  1074,  1615,  1620,  1622,  1709,  1621,  1624,
     695,  1197,  1715,  1625,  1626,   722,   722,  1628,  1629,  1753,
    1631,   131,  1632,  1630,  1633,   125,  1636,   646,  1208,   225,
     225,  1638,  1639,  1640,    81,    82,    83,    84,    85,  1567,
    1568,   223,  1569,  1643,  1644,   220,  1945,  1946,   685,   169,
     223,    89,    90,  1647,  1650,  1661,  1672,   223,  1700,  1685,
    1705,  1710,  1713,   456,  1714,    99,  1718,  1723,  1724,   223,
     169,   683,  1725,  1726,  1730,  1745,   116,  1748,  1754,   104,
    1763,  1762,  1860,  1866,   222,   222,  1859,   349,  1872,  1873,
    1875,  1876,  1255,  1690,  1255,   245,  1887,  1877,  1888,  1879,
    1919,  1925,   169,  1885,  1929,  1898,  1741,  1899,  1928,  1574,
    1934,  1956,  1958,  1960,  1964,  1972,   116,  1973,  1974,  1574,
    1927,   245,  1081,  1979,  1980,  1983,  1988,  1992,  1874,  1993,
    -387,  1995,  1996,   924,  1914,   924,  1998,   924,   125,   685,
    2000,   924,  2001,   924,   924,  1191,  1074,  2004,  1074,  2020,
    1074,  2013,  1579,  2007,  2015,  1074,  2022,  2014,  1579,  2027,
    1579,   116,  2031,   695,  2035,  2044,   116,  2034,  2057,  2051,
     116,   225,  2062,  2046,  2052,  2070,  2079,  2071,  2075,   169,
    2080,   169,  1579,   169,  2087,  1109,  1323,  2088,  2090,  2091,
     389,  1407,  2030,   601,  1274,   786,   349,  1217,   788,   791,
    2045,  1562,  1890,  1741,   223,  1506,  1848,  1892,  1645,  2043,
    1457,   928,  1907,  1855,   455,    63,    64,    65,    66,    67,
     349,  1912,   349,  1883,  2082,    72,   505,   683,   349,  1761,
    1692,  1924,  2056,   225,  1673,  1922,   655,  1881,  1361,  1490,
    1353,  1430,  1290,    14,  1481,   131,  1482,  1308,   225,   225,
     648,   723,  1977,  1133,  1074,  2009,  2002,  1663,  1410,     0,
    1342,   116,   116,   116,  1400,     0,  2018,   116,   507,     0,
       0,     0,   512,     0,   116,  1579,     0,   455,    63,    64,
      65,    66,    67,     0,     0,   131,   456,  1930,    72,   505,
       0,     0,     0,     0,     0,   169,     0,     0,  1574,     0,
    1922,     0,     0,   222,  1574,     0,  1574,     0,  1563,     0,
     458,  1255,     0,     0,  1564,     0,   455,  1565,   186,    65,
      66,    67,  1566,     0,     0,     0,     0,  1456,  1574,   506,
     131,   507,     0,     0,     0,     0,     0,     0,     0,   131,
       0,     0,   225,     0,   508,     0,   509,     0,     0,   456,
       0,     0,     0,   245,     0,   245,  1081,  1081,  1081,  1081,
    1081,  1081,   125,     0,  1567,  1568,  1081,  1569,     0,   542,
     514,   515,   516,   517,   518,   519,   520,   521,   522,   523,
     524,   525,   223,  1694,     0,     0,   601,     0,   456,  1498,
       0,     0,     0,     0,     0,     0,   169,  1727,     0,     0,
       0,     0,   125,     0,   646,  1109,     0,   349,   169,     0,
       0,  1074,  1074,   245,   526,   527,     0,   116,   924,     0,
       0,  1574,     0,     0,     0,     0,  1951,     0,     0,     0,
     131,     0,     0,  1848,  1848,     0,   131,  1855,  1855,     0,
       0,   222,     0,   131,     0,     0,     0,   125,     0,     0,
     222,   601,     0,     0,     0,     0,   125,   222,   223,  1562,
       0,     0,     0,     0,     0,     0,   683,     0,     0,   222,
       0,   116,     0,     0,     0,     0,     0,     0,     0,     0,
     222,     0,     0,     0,     0,  2081,     0,     0,     0,     0,
       0,   528,   529,     0,  2089,     0,     0,   223,   646,   223,
       0,    14,  2092,     0,   245,  2093,   116,   245,     0,     0,
       0,   116,  1867,     0,     0,     0,     0,     0,  1590,  2017,
       0,     0,     0,     0,     0,   223,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   116,     0,  1081,     0,
    1081,     0,  2032,     0,     0,     0,   211,   125,     0,     0,
     695,     0,     0,   125,   839,     0,     0,   683,     0,     0,
     125,     0,     0,   245,     0,     0,  1563,     0,     0,    50,
       0,     0,  1564,   695,   455,  1565,   186,    65,    66,    67,
    1566,     0,   695,     0,     0,     0,   131,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   223,   116,   116,     0,
       0,     0,     0,     0,   222,     0,   215,   216,   217,   218,
     219,     0,   223,   223,     0,     0,     0,  1562,     0,     0,
       0,     0,  1567,  1568,     0,  1569,     0,     0,     0,     0,
       0,   407,   169,     0,    93,    94,     0,    95,   189,    97,
     131,     0,     0,     0,     0,     0,   456,     0,     0,     0,
       0,     0,     0,     0,     0,  1736,   245,     0,   245,    14,
       0,   890,   107,     0,     0,     0,   408,     0,     0,     0,
       0,     0,   169,     0,     0,   131,     0,     0,     0,     0,
     131,     0,     0,  1081,     0,  1081,     0,  1081,     0,  2019,
       0,     0,  1081,     0,   890,     0,     0,     0,     0,     0,
       0,     0,     0,   125,     0,   131,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   169,     0,     0,
       0,     0,   169,     0,  1563,     0,   169,     0,     0,     0,
    1564,     0,   455,  1565,   186,    65,    66,    67,  1566,   542,
     514,   515,   516,   517,   518,   519,   520,   521,   522,   523,
     524,   525,   245,   245,     0,     0,     0,   125,     0,     0,
       0,   245,     0,     0,     0,     0,   131,   131,     0,     0,
     223,   223,     0,     0,     0,     0,     0,     0,     0,     0,
    1567,  1568,   222,  1569,   526,   527,     0,     0,     0,     0,
       0,  1081,   125,     0,     0,     0,     0,   125,     0,     0,
       0,     0,     0,     0,   456,     0,     0,     0,   470,   471,
     472,     0,     0,  1884,     0,     0,     0,   169,   169,   169,
       0,     0,   125,   169,     0,     0,     0,     0,   473,   474,
     169,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,     0,   498,     0,     0,   222,     0,
       0,   528,   529,     0,     0,     0,     0,   499,     0,  1019,
     514,   515,   516,   517,   518,   519,   520,   521,   522,   523,
     524,   525,     0,   125,   125,     0,     0,     0,     0,     0,
       0,   245,     0,     0,     0,     0,     0,   222,     0,   222,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   223,     0,   526,   527,     0,     0,     0,     0,
       0,     0,     0,     0,   936,   222,   890,     0,     0,     0,
       0,     0,     0,   245,     0,     0,     0,     0,     0,     0,
     245,   245,   890,   890,   890,   890,   890,     0,  1081,  1081,
       0,     0,     0,     0,   890,     0,     0,   513,   514,   515,
     516,   517,   518,   519,   520,   521,   522,   523,   524,   525,
     245,     0,     0,   169,   223,  1019,   514,   515,   516,   517,
     518,   519,   520,   521,   522,   523,   524,   525,     0,   223,
     223,   528,   529,     0,     0,     0,   222,     0,     0,     0,
       0,  1423,   526,   527,     0,     0,     0,     0,     0,     0,
     245,     0,   222,   222,     0,     0,     0,     0,     0,     0,
     526,   527,     0,     0,     0,     0,     0,   169,     0,     0,
       0,     0,     0,     0,     0,     0,   245,   245,     0,     0,
       0,     0,     0,     0,     0,     0,   222,     0,     0,   224,
     224,     0,   245,     0,  1020,     0,     0,     0,     0,   245,
     247,     0,   169,     0,     0,   245,     0,   169,     0,     0,
       0,     0,     0,     0,     0,     0,   890,     0,     0,   528,
     529,     0,     0,   223,   352,     0,     0,     0,     0,     0,
       0,   245,   169,     0,     0,     0,     0,   528,   529,     0,
       0,     0,     0,     0,   470,   471,   472,     0,     0,     0,
       0,   245,     0,     0,     0,   245,     0,     0,     0,     0,
       0,     0,     0,     0,   473,   474,   245,   475,   476,   477,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
       0,   498,     0,   169,   169,     0,     0,     0,     0,     0,
     700,     0,     0,   499,     0,     0,     0,     0,     0,     0,
     222,   222,     0,   356,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   245,     0,     0,     0,   245,     0,
     245,     0,     0,   245,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   890,   890,   890,   890,
     890,   890,   222,     0,     0,   890,   890,   890,   890,   890,
     890,   890,   890,   890,   890,   890,   890,   890,   890,   890,
     890,   890,   890,   890,   890,   890,   890,   890,   890,   890,
     890,   890,   890,     0,     0,     0,   470,   471,   472,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   890,     0,     0,     0,     0,   473,   474,   224,   475,
     476,   477,   478,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   211,   498,   500,     0,  1031,   245,     0,   245,
       0,     0,     0,     0,     0,   499,     0,     0,     0,     0,
       0,     0,   222,     0,     0,    50,   352,     0,   352,     0,
       0,   542,   514,   515,   516,   517,   518,   519,   520,   521,
     522,   523,   524,   525,     0,   211,     0,   212,    40,     0,
     245,     0,     0,   245,     0,     0,     0,     0,     0,     0,
       0,     0,   215,   216,   217,   218,   219,     0,    50,     0,
     245,   245,   245,   245,   245,   245,   526,   527,   222,     0,
     245,     0,     0,     0,   222,     0,   352,     0,   452,     0,
      93,    94,     0,    95,   189,    97,     0,     0,     0,   222,
     222,     0,   890,     0,     0,   215,   216,   217,   218,   219,
       0,     0,   245,     0,     0,   589,   224,   590,   107,   245,
       0,     0,     0,     0,   890,   224,   890,     0,     0,     0,
       0,   810,   224,    93,    94,     0,    95,   189,    97,     0,
       0,     0,     0,     0,   224,     0,     0,     0,   967,     0,
     890,     0,     0,   528,   529,   224,     0,     0,     0,     0,
       0,   107,     0,     0,     0,   842,     0,     0,   111,     0,
       0,   470,   471,   472,     0,   595,     0,   352,     0,     0,
     352,     0,     0,     0,     0,     0,   245,   245,     0,     0,
     245,   473,   474,   222,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,     0,   498,     0,
       0,   245,     0,     0,     0,     0,     0,     0,     0,     0,
     499,     0,     0,     0,     0,     0,     0,     0,   247,  1019,
     514,   515,   516,   517,   518,   519,   520,   521,   522,   523,
     524,   525,   245,     0,   245,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   716,     0,     0,   356,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   224,
       0,     0,     0,     0,   526,   527,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   245,   245,     0,
       0,   245,     0,     0,     0,     0,     0,   890,     0,   890,
       0,   890,     0,     0,     0,     0,   890,   222,     0,     0,
     890,     0,   890,     0,     0,   890,     0,     0,     0,   352,
       0,   871,     0,     0,     0,     0,   897,     0,   245,   245,
       0,     0,   245,     0,     0,     0,     0,     0,     0,   245,
       0,   470,   471,   472,     0,     0,     0,     0,     0,     0,
       0,   528,   529,   999,     0,     0,     0,     0,     0,   897,
       0,   473,   474,     0,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   245,   498,   245,
       0,   245,     0,     0,     0,     0,   245,     0,   222,     0,
     499,     0,     0,     0,     0,     0,     0,     0,   867,     0,
       0,     0,   245,     0,     0,   352,   352,   890,     0,     0,
       0,     0,     0,     0,   352,     0,     0,     0,     0,   245,
     245,     0,     0,     0,     0,     0,     0,   245,     0,   245,
       0,     0,     0,     0,     0,     0,     0,   224,     0,  1151,
    1152,  1153,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   245,     0,   245,     0,     0,     0,     0,     0,   245,
    1154,     0,     0,  1155,  1156,  1157,  1158,  1159,  1160,  1161,
    1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,  1175,  1176,   245,     0,     0,     0,     0,
       0,     0,     0,     0,   985,   986,     0,     0,  1177,   211,
       0,     0,   890,   890,   890,     0,     0,     0,     0,   890,
       0,   245,     0,   224,     0,     0,     0,   245,     0,   245,
       0,     0,    50,  1031,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   868,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1080,     0,     0,     0,
       0,     0,   224,     0,   224,     0,     0,     0,     0,   215,
     216,   217,   218,   219,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     224,   897,     0,     0,   211,  1853,  1122,    93,    94,  1854,
      95,   189,    97,   352,   352,   869,     0,   897,   897,   897,
     897,   897,     0,     0,     0,     0,     0,    50,     0,   897,
       0,     0,     0,     0,     0,   107,  1689,     0,     0,     0,
       0,   245,     0,     0,     0,  1181,     0,     0,  1366,     0,
       0,     0,     0,     0,   245,     0,     0,     0,   245,     0,
       0,     0,   245,   245,   215,   216,   217,   218,   219,     0,
       0,   224,     0,     0,     0,     0,     0,   245,     0,     0,
       0,     0,     0,   890,     0,  1204,   188,   224,   224,    91,
       0,     0,    93,    94,   890,    95,   189,    97,     0,   870,
     890,     0,  1131,   716,   890,     0,     0,     0,     0,   352,
       0,     0,  1204,     0,     0,     0,     0,     0,     0,     0,
     107,   224,     0,     0,     0,   352,     0,     0,     0,     0,
       0,   245,   352,   226,   226,     0,     0,     0,   352,     0,
       0,     0,     0,     0,   251,     0,     0,     0,     0,     0,
       0,   897,     0,     0,     0,     0,     0,     0,   245,   211,
     245,     0,     0,     0,     0,     0,  1264,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   890,     0,     0,     0,
       0,     0,    50,     0,   352,     0,     0,     0,     0,   245,
     247,     0,     0,     0,     0,     0,     0,     0,  1216,     0,
       0,  1080,     0,     0,     0,     0,   245,     0,     0,     0,
       0,     0,   245,     0,  1226,     0,   245,     0,     0,   215,
     216,   217,   218,   219,     0,     0,     0,  1245,     0,     0,
     245,   245, -1126, -1126, -1126, -1126, -1126,  1169,  1170,  1171,
    1172,  1173,  1174,  1175,  1176,   224,   224,    93,    94,     0,
      95,   189,    97,     0,     0,     0,     0,   352,  1177,     0,
       0,   352,     0,   871,     0,   211,   352,     0,     0,     0,
       0,     0,     0,  1277,     0,   107,   737,     0,     0,     0,
       0,   897,   897,   897,   897,   897,   897,   224,    50,     0,
     897,   897,   897,   897,   897,   897,   897,   897,   897,   897,
     897,   897,   897,   897,   897,   897,   897,   897,   897,   897,
     897,   897,   897,   897,   897,   897,   897,   897,     0,     0,
       0,     0,     0,     0,     0,   215,   216,   217,   218,   219,
       0,     0,     0,     0,     0,     0,   897,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1335,     0,     0,     0,
    1339,     0,   226,    93,    94,  1343,    95,   189,    97,     0,
       0,     0,     0,     0,     0,   470,   471,   472,     0,     0,
     352,     0,   352,     0,     0,     0,     0,     0,     0,     0,
       0,   107,  1028,     0,     0,   473,   474,   224,   475,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,     0,   498,   352,     0,     0,   352,     0,     0,     0,
       0,     0,     0,     0,   499,     0,     0,     0,     0,  1285,
    1286,  1287,   211,     0,     0,  1080,  1080,  1080,  1080,  1080,
    1080,     0,     0,   224,     0,  1080,     0,   211,     0,   224,
       0,     0,     0,     0,     0,    50,     0,     0,     0,  1444,
       0,     0,     0,     0,   224,   224,     0,   897,     0,     0,
      50,     0,     0,     0,     0,   352,     0,     0,   917,   918,
       0,     0,   352,     0,     0,     0,     0,     0,     0,   897,
     226,   897,   215,   216,   217,   218,   219,     0,     0,   226,
       0,     0,  1469,     0,     0,  1472,   226,   215,   216,   217,
     218,   219,     0,     0,     0,   897,     0,     0,   226,     0,
      93,    94,     0,    95,   189,    97,     0,     0,     0,   251,
       0,     0,   919,     0,     0,    93,    94,     0,    95,   189,
      97,     0,     0,     0,     0,     0,     0,     0,   107,   352,
     352,     0,     0,     0,     0,  1561,     0,  1057,   224,     0,
       0,     0,     0,   107,  1520,     0,   470,   471,   472,     0,
       0,  1524,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   352,     0,   473,   474,     0,   475,
     476,   477,   478,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   251,   498,     0,     0,     0,  1080,     0,  1080,
       0,     0,     0,     0,     0,   499,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1558,  1559,
   -1126, -1126, -1126, -1126, -1126,   490,   491,   492,   493,   494,
     495,   496,   497,   226,   498,     0,     0,     0,     0,     0,
     352,   352,     0,     0,   352,     0,   499,     0,     0,     0,
       0,     0,   897,     0,   897,     0,   897,     0,     0,     0,
       0,   897,   224,     0,     0,   897,     0,   897,     0,     0,
     897,     0,     0,   470,   471,   472,     0,     0,     0,     0,
       0,   352,     0,     0,  1671,     0,     0,  1684,     0,     0,
     898,     0,   352,   473,   474,     0,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,     0,
     498,     0,     0,   898,     0,     0,     0,     0,     0,  1648,
    1649,     0,   499,  1651,     0,     0,     0,     0,  1195,     0,
       0,     0,  1080,     0,  1080,     0,  1080,     0,     0,     0,
     899,  1080,     0,   224,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   352,     0,     0,     0,     0,
    1669,     0,   897,     0,     0,     0,     0,     0,     0,     0,
       0,  1695,     0,   929,  1751,  1752,     0,     0,     0,     0,
     352,     0,     0,     0,  1684,     0,     0,     0,     0,     0,
     470,   471,   472,     0,     0,     0,     0,     0,     0,     0,
       0,   226,     0,     0,   352,     0,   352,     0,     0,     0,
     473,   474,   352,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,     0,   498,     0,     0,
    1080,     0,     0,     0,  1744,     0,     0,     0,     0,   499,
       0,     0,     0,     0,     0,  1271,     0,   897,   897,   897,
       0,     0,     0,     0,   897,   211,  1901,     0,     0,  1669,
     352,     0,     0,     0,  1684,     0,     0,   226,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,  1669,     0,  1669,     0,     0,     0,     0,
       0,  1669,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1675,     0,     0,     0,     0,   226,     0,   226,     0,
       0,     0,     0,     0,  1676,   215,   216,   217,   218,   219,
    1677,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   288,   226,   898,     0,   188,     0,     0,
      91,    92,     0,    93,    94,     0,    95,  1679,    97,  1903,
       0,   898,   898,   898,   898,   898,     0,     0,     0,     0,
       0,     0,     0,   898,   352,     0,     0,     0,     0,     0,
     290,   107,  1282,     0,     0,     0,     0,   352,     0,     0,
       0,   352,     0,   211,     0,     0,     0,  1080,  1080,  1128,
       0,     0,     0,     0,     0,  1110,     0,     0,     0,     0,
    1952,     0,     0,     0,     0,   226,    50,     0,   897,     0,
       0,  1134,  1135,  1136,  1137,  1138,     0,     0,     0,   897,
       0,   226,   226,  1148,     0,   897,     0,     0,     0,   897,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   587,   215,   216,   217,   218,   219,   588,     0,
       0,     0,     0,  1926,   352,   251,     0,     0,     0,   211,
       0,     0,     0,     0,     0,   188,  1937,     0,    91,   343,
    1669,    93,    94,     0,    95,   189,    97,     0, -1125,     0,
       0,   352,    50,   352,     0,   898,     0,     0,     0,   347,
     365,   366,     0,     0,     0,     0,     0,     0,     0,   107,
     348,   897,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  2029,     0,     0,     0,     0,   215,
     216,   217,   218,   219,   251,     0,     0,     0,     0,     0,
       0,  1671,     0,     0,     0,   352,     0,     0,     0,   352,
       0,   367,     0,  1997,   368,  1251,     0,    93,    94,     0,
      95,   189,    97,   352,   352,     0,     0,     0,     0,     0,
       0,     0,     0,   470,   471,   472,   369,     0,     0,     0,
    1937,     0,  2010,     0,     0,   107,     0,     0,     0,   226,
     226,     0,     0,   473,   474,     0,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,     0,
     498,     0,     0,     0,     0,   898,   898,   898,   898,   898,
     898,   251,   499,     0,   898,   898,   898,   898,   898,   898,
     898,   898,   898,   898,   898,   898,   898,   898,   898,   898,
     898,   898,   898,   898,   898,   898,   898,   898,   898,   898,
     898,   898,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     898,   498,     0,     0,     0,  1138,  1356,   288,     0,  1356,
       0,     0,     0,   499,  1369,  1372,  1373,  1374,  1376,  1377,
    1378,  1379,  1380,  1381,  1382,  1383,  1384,  1385,  1386,  1387,
    1388,  1389,  1390,  1391,  1392,  1393,  1394,  1395,  1396,  1397,
    1398,  1399,     0,     0,   290,     0,     0,     0,     0,     0,
       0,   226,     0,     0,     0,     0,     0,   211,     0,     0,
    1409,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
      50,   498,     0,     0,     0,  1312,     0,     0,  -434,     0,
       0,     0,     0,   499,     0,     0,     0,   455,   185,   186,
      65,    66,    67,     0,     0,     0,     0,   251,     0,     0,
       0,     0,     0,   226,     0,  1674,   587,   215,   216,   217,
     218,   219,   588,     0,     0,     0,     0,     0,   226,   226,
       0,   898,     0,     0,     0,     0,     0,     0,     0,   188,
       0,     0,    91,   343,     0,    93,    94,     0,    95,   189,
      97,     0,     0,   898,     0,   898,     0,     0,     0,     0,
       0,     0,     0,   347,     0,   211,     0,     0,     0,   456,
       0,     0,     0,   107,   348,     0,     0,     0,     0,   898,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,  1510,   278,   279,     0,   280,   281,     0,     0,   282,
     283,   284,   285,     0,     0,     0,     0,     0,     0,     0,
       0,  1675,     0,  1528,     0,  1529,     0,   286,   287,     0,
       0,     0,   226,     0,  1676,   215,   216,   217,   218,   219,
    1677,     0,     0,     0,     0,     0,     0,     0,     0,  1549,
       0,     0,     0,     0,     0,     0,   289,   188,     0,     0,
      91,  1678,     0,    93,    94,     0,    95,  1679,    97,     0,
     291,   292,   293,   294,   295,   296,   297,     0,     0,     0,
     211,     0,   212,    40,     0,     0,   298,     0,     0,     0,
       0,   107,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,    50,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,     0,   334,
       0,   780,   336,   337,   338,     0,     0,     0,   339,   598,
     215,   216,   217,   218,   219,   599,   898,     0,   898,     0,
     898,     0,     0,     0,     0,   898,   251,     0,     0,   898,
       0,   898,   600,     0,   898,     0,     0,   211,    93,    94,
       0,    95,   189,    97,   344,     0,   345,     0,  1121,   346,
       0,     0,   211,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,   107,     0,     0,     0,
     781,     0,     0,   111,     0,    50,  1653,     0,  1654,     0,
    1655,     0,     0,     0,     0,  1656,     0,     0,     0,  1658,
       0,  1659,     0,     0,  1660,     0,     0,   215,   216,   217,
     218,   219,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   215,   216,   217,   218,   219,   251,     0,   188,
       0,     0,    91,     0,     0,    93,    94,     0,    95,   189,
      97,     0,     0,     0,   188,     0,   898,    91,    92,     0,
      93,    94,     0,    95,   189,    97,     0,     0,     0,     0,
     470,   471,   472,   107,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   107,     0,
     473,   474,     0,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,  1746,   498,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   499,
       0,     0,     0,     0,     0,     0,   278,   279,     0,   280,
     281,     0,     0,   282,   283,   284,   285,     0,     0,     0,
       0,   898,   898,   898,     0,     0,     0,     0,   898,     0,
       0,   286,   287,     0,     0,  1154,     0,  1906,  1155,  1156,
    1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
    1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
     289,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1177,   291,   292,   293,   294,   295,   296,
     297,  1894,  1895,  1896,   211,     0,   212,    40,  1900,     0,
     298,     0,     0,     0,     0,     0,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,    50,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,  1707,   334,   211,   335,   336,   337,   338,     0,
       0,     0,   339,   598,   215,   216,   217,   218,   219,   599,
       0,     0,     0,     0,     0,     0,   211,    50,   961,     0,
     962,     0,     0,     0,     0,     0,   600,     0,     0,     0,
       0,     0,    93,    94,     0,    95,   189,    97,   344,    50,
     345,     0,   898,   346,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   898,   215,   216,   217,   218,   219,   898,
     107,     0,     0,   898,   781,     0,     0,   111,     0,     0,
       0,     0,     0,     0,     0,     0,   215,   216,   217,   218,
     219,     0,    93,    94,     0,    95,   189,    97,     0,     0,
       0,     0,  1999,     0,     0,     0,     5,     6,     7,     8,
       9,     0,  1961,     0,    93,    94,    10,    95,   189,    97,
     107,     0,     0,  1971,     0,     0,     0,     0,     0,  1976,
      11,    12,    13,  1978,     0,     0,     0,     0,     0,     0,
       0,     0,   107,     0,     0,   898,     0,     0,     0,     0,
       0,    14,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,  2021,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,    56,    57,    58,     0,
      59,  -204,    60,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,    88,    89,    90,    91,
      92,     0,    93,    94,     0,    95,    96,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,   103,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,     0,     0,   111,   112,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,    56,    57,    58,     0,    59,     0,    60,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,    88,    89,    90,    91,    92,     0,    93,    94,     0,
      95,    96,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,   103,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
    1214,     0,   111,   112,   113,   114,     5,     6,     7,     8,
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
       0,     0,    52,    53,    54,    55,    56,    57,    58,     0,
      59,     0,    60,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,    88,    89,    90,    91,
      92,     0,    93,    94,     0,    95,    96,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,   102,     0,   103,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,  1424,     0,   111,   112,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
     702,     0,   111,   112,   113,   114,     5,     6,     7,     8,
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
       0,    87,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,  1021,     0,   111,   112,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,  -204,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
       0,     0,   111,   112,   113,   114,     5,     6,     7,     8,
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
       0,    87,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,  1180,     0,   111,   112,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
    1228,     0,   111,   112,   113,   114,     5,     6,     7,     8,
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
       0,    87,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,  1260,     0,   111,   112,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
    1318,     0,   111,   112,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
       0,    42,     0,     0,     0,    43,    44,    45,    46,  1320,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,     0,     0,   111,   112,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
    1511,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
       0,     0,   111,   112,   113,   114,     5,     6,     7,     8,
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
       0,    87,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,  1662,     0,   111,   112,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,  -296,    34,    35,    36,    37,
      38,    39,    40,     0,    41,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
       0,     0,   111,   112,   113,   114,     5,     6,     7,     8,
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
       0,    87,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,  1897,     0,   111,   112,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,  1948,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
       0,     0,   111,   112,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,  1991,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,     0,     0,   111,   112,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
    2008,     0,   111,   112,   113,   114,     5,     6,     7,     8,
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
       0,    87,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,  2011,     0,   111,   112,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
    2028,     0,   111,   112,   113,   114,     5,     6,     7,     8,
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
       0,    87,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,  2083,     0,   111,   112,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
    2084,     0,   111,   112,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,   570,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,     0,    61,    62,   185,   186,    65,    66,    67,
       0,    68,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,     0,     0,   111,   112,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
     855,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
     185,   186,    65,    66,    67,     0,    68,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
       0,     0,   111,   112,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,  1112,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,     0,    61,    62,   185,   186,    65,    66,    67,
       0,    68,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,     0,     0,   111,   112,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
    1740,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
     185,   186,    65,    66,    67,     0,    68,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
       0,     0,   111,   112,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,  1889,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,     0,    61,    62,   185,   186,    65,    66,    67,
       0,    68,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,     0,     0,   111,   112,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
     185,   186,    65,    66,    67,     0,    68,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
       0,     0,   111,   112,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     357,   422,    13,     0,     0,     0,     0,     0,     0,     0,
       0,   793,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,     0,   184,   185,   186,    65,    66,    67,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   187,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     5,     6,     7,     8,     9,   111,   112,   113,
     114,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   357,     0,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,     0,
     184,   185,   186,    65,    66,    67,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   187,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   188,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   189,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   190,     0,   358,
       0,     0,     0,   111,   112,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,  1155,  1156,
    1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
    1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
       0,     0,   717,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1177,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,   718,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,     0,   184,   185,   186,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   187,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   188,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   189,    97,     0,
     719,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   190,     5,     6,     7,     8,     9,   111,   112,
     113,   114,     0,    10,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,     0,   498,     0,  1247,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   499,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,  1248,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
       0,   184,   185,   186,    65,    66,    67,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   187,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   188,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   189,    97,     0,  1249,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   190,     5,
       6,     7,     8,     9,   111,   112,   113,   114,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   357,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,     0,   184,   185,   186,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   187,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   188,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   189,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   190,     0,     0,   850,     0,     0,
     111,   112,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   357,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   793,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,     0,   184,   185,   186,    65,    66,    67,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     187,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   188,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   189,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   190,
       5,     6,     7,     8,     9,   111,   112,   113,   114,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   357,   422,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,   184,   185,
     186,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   187,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     188,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     189,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,  1151,  1152,  1153,   104,
     105,   106,     0,     0,   107,   108,     5,     6,     7,     8,
       9,   111,   112,   113,   114,     0,    10,  1154,  1550,     0,
    1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,  1177,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,   202,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,     0,   184,   185,   186,    65,    66,    67,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   187,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   190,     5,     6,     7,     8,     9,   111,   112,   113,
     114,     0,    10,  1156,  1157,  1158,  1159,  1160,  1161,  1162,
    1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,     0,     0,     0,   238,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1177,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,     0,
     184,   185,   186,    65,    66,    67,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   187,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   188,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   189,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,  1151,  1152,
    1153,   104,   105,   106,     0,     0,   107,   190,     5,     6,
       7,     8,     9,   111,   112,   113,   114,     0,    10,  1154,
       0,     0,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,
    1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,  1177,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,     0,   184,   185,   186,    65,
      66,    67,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   187,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   188,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   189,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   190,     0,   273,   470,   471,   472,   111,
     112,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,   473,   474,     0,   475,
     476,   477,   478,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,     0,   498,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,   499,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
       0,   184,   185,   186,    65,    66,    67,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   187,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   188,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   189,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,  1708,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   190,     0,
     276,     0,     0,     0,   111,   112,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   422,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,     0,   184,   185,   186,    65,
      66,    67,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   187,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   188,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   189,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
     471,   472,   107,   108,     5,     6,     7,     8,     9,   111,
     112,   113,   114,     0,    10,     0,     0,     0,     0,   473,
     474,     0,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,     0,   498,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,   499,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,     0,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,     0,   184,   185,   186,    65,    66,    67,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     187,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   188,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   189,    97,     0,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   190,
     568,     0,     0,     0,     0,   111,   112,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   357,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,     0,   184,   185,   186,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   187,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   188,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   189,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   190,     5,     6,     7,     8,     9,
     111,   112,   113,   114,     0,    10,  1159,  1160,  1161,  1162,
    1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,   748,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1177,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,     0,   184,   185,   186,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   187,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   189,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     190,     5,     6,     7,     8,     9,   111,   112,   113,   114,
       0,    10,   478,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,     0,   498,     0,     0,   793,     0,     0,     0,
       0,     0,     0,     0,     0,   499,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,   184,
     185,   186,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   187,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   190,     5,     6,     7,
       8,     9,   111,   112,   113,   114,     0,    10,  1157,  1158,
    1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,     0,     0,
       0,     0,   832,     0,     0,     0,     0,     0,     0,     0,
       0,  1177,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,     0,   184,   185,   186,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   187,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   188,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   189,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   190,     5,     6,     7,     8,     9,   111,   112,
     113,   114,     0,    10,  1158,  1159,  1160,  1161,  1162,  1163,
    1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,
    1174,  1175,  1176,     0,     0,     0,     0,     0,   834,     0,
       0,     0,     0,     0,     0,     0,  1177,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
       0,   184,   185,   186,    65,    66,    67,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   187,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   188,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   189,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   190,     5,
       6,     7,     8,     9,   111,   112,   113,   114,     0,    10,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,     0,   498,     0,
       0,     0,     0,     0,  1309,     0,     0,     0,     0,     0,
     499,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,     0,   184,   185,   186,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   187,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   188,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   189,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   190,     5,     6,     7,     8,     9,
     111,   112,   113,   114,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   357,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,     0,   184,   185,   186,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   187,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   189,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,  1152,  1153,   104,   105,   106,     0,     0,   107,
    1438,     5,     6,     7,     8,     9,   111,   112,   113,   114,
       0,    10,  1154,     0,     0,  1155,  1156,  1157,  1158,  1159,
    1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
    1177,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,   184,
     185,   186,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   187,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,   472,   107,   190,     5,     6,     7,
       8,     9,   111,   112,   113,   114,     0,    10,     0,     0,
       0,     0,   473,   474,     0,   475,   476,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,     0,   498,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,   499,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,   662,    39,    40,     0,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,   868,     0,     0,   184,   185,   186,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   187,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   188,    89,    90,
      91,    92,   211,    93,    94,     0,    95,   189,    97,     0,
       0,     0,    99,   869,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,    50,   104,   105,   106,     0,
       0,   107,   190,     0,     0,     0,     0,     0,   111,   112,
     113,   114,   278,   279,     0,   280,   281,     0,     0,   282,
     283,   284,   285,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   215,   216,   217,   218,   219,   286,   287,     0,
     288,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   188,     0,     0,    91,     0,     0,
      93,    94,     0,    95,   189,    97,   289,  1341,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   290,     0,     0,
     291,   292,   293,   294,   295,   296,   297,     0,   107,     0,
     211,     0,     0,     0,     0,     0,   298,     0,     0,     0,
       0,     0,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,    50,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,     0,   334,
       0,   211,   336,   337,   338,     0,     0,     0,   339,   340,
     215,   216,   217,   218,   219,   341,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,   342,     0,     0,    91,   343,     0,    93,    94,
       0,    95,   189,    97,   344,     0,   345,     0,     0,   346,
     278,   279,     0,   280,   281,     0,   347,   282,   283,   284,
     285,   215,   216,   217,   218,   219,   107,   348,     0,     0,
       0,  1868,     0,     0,     0,   286,   287,     0,   288,     0,
       0,     0,     0,     0,     0,     0,   368,     0,     0,    93,
      94,     0,    95,   189,    97,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   289,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   290,     0,   107,   291,   292,
     293,   294,   295,   296,   297,     0,     0,     0,   211,     0,
       0,     0,     0,     0,   298,     0,     0,     0,     0,     0,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,    50,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,     0,   334,     0,   211,
     336,   337,   338,     0,     0,     0,   339,   340,   215,   216,
     217,   218,   219,   341,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
     342,     0,     0,    91,   343,     0,    93,    94,     0,    95,
     189,    97,   344,     0,   345,     0,     0,   346,   278,   279,
       0,   280,   281,     0,   347,   282,   283,   284,   285,   215,
     216,   217,   218,   219,   107,   348,     0,     0,     0,  1943,
       0,     0,     0,   286,   287,     0,   288,     0,     0,     0,
       0,     0,     0,     0,   919,     0,     0,    93,    94,     0,
      95,   189,    97,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   289,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   290,     0,   107,   291,   292,   293,   294,
     295,   296,   297,     0,     0,     0,   211,     0,     0,     0,
       0,     0,   298,     0,     0,     0,     0,     0,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,    50,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,     0,   334,     0,   335,   336,   337,
     338,     0,     0,     0,   339,   340,   215,   216,   217,   218,
     219,   341,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   342,     0,
       0,    91,   343,     0,    93,    94,     0,    95,   189,    97,
     344,     0,   345,     0,     0,   346,   278,   279,     0,   280,
     281,     0,   347,   282,   283,   284,   285,     0,     0,     0,
       0,     0,   107,   348,     0,     0,     0,     0,     0,     0,
       0,   286,   287,     0,   288,   474,     0,   475,   476,   477,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     289,   498,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   290,     0,   499,   291,   292,   293,   294,   295,   296,
     297,     0,     0,     0,   211,     0,     0,     0,     0,     0,
     298,     0,     0,     0,     0,     0,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,    50,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,     0,   334,     0,     0,   336,   337,   338,     0,
       0,     0,   339,   340,   215,   216,   217,   218,   219,   341,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   342,     0,     0,    91,
     343,     0,    93,    94,     0,    95,   189,    97,   344,     0,
     345,     0,     0,   346,     0,   278,   279,     0,   280,   281,
     347,  1666,   282,   283,   284,   285,     0,     0,     0,     0,
     107,   348,     0,     0,     0,     0,     0,     0,     0,     0,
     286,   287,     0,   288,     0,     0,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   289,
     498,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     290,     0,   499,   291,   292,   293,   294,   295,   296,   297,
       0,     0,     0,   211,     0,     0,     0,     0,     0,   298,
       0,     0,     0,     0,     0,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,    50,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,     0,   334,     0,     0,   336,   337,   338,     0,     0,
       0,   339,   340,   215,   216,   217,   218,   219,   341,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   342,     0,     0,    91,   343,
       0,    93,    94,     0,    95,   189,    97,   344,     0,   345,
       0,     0,   346,  1766,  1767,  1768,  1769,  1770,     0,   347,
    1771,  1772,  1773,  1774,     0,     0,     0,     0,     0,   107,
     348,     0,     0,     0,     0,     0,     0,  1775,  1776,  1777,
       0,   473,   474,     0,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,  1778,   498,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     499,  1779,  1780,  1781,  1782,  1783,  1784,  1785,     0,     0,
       0,   211,     0,     0,     0,     0,     0,  1786,     0,     0,
       0,     0,     0,  1787,  1788,  1789,  1790,  1791,  1792,  1793,
    1794,  1795,  1796,  1797,    50,  1798,  1799,  1800,  1801,  1802,
    1803,  1804,  1805,  1806,  1807,  1808,  1809,  1810,  1811,  1812,
    1813,  1814,  1815,  1816,  1817,  1818,  1819,  1820,  1821,  1822,
    1823,  1824,  1825,  1826,  1827,  1828,     0,     0,     0,  1829,
    1830,   215,   216,   217,   218,   219,     0,  1831,  1832,  1833,
    1834,  1835,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1836,  1837,  1838,     0,     0,     0,    93,
      94,     0,    95,   189,    97,  1839,     0,  1840,  1841,     0,
    1842,     0,     0,     0,     0,     0,     0,  1843,  1844,     0,
    1845,     0,  1846,  1847,     0,   278,   279,   107,   280,   281,
    1153,     0,   282,   283,   284,   285,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1154,
     286,   287,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,
    1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,     0,     0,     0,     0,     0,   289,
       0,     0,     0,     0,     0,     0,     0,  1177,     0,     0,
       0,     0,     0,   291,   292,   293,   294,   295,   296,   297,
       0,     0,     0,   211,     0,     0,     0,     0,     0,   298,
       0,     0,     0,     0,     0,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,    50,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,     0,   334,     0,   335,   336,   337,   338,     0,     0,
       0,   339,   598,   215,   216,   217,   218,   219,   599,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   278,
     279,     0,   280,   281,     0,   600,   282,   283,   284,   285,
       0,    93,    94,     0,    95,   189,    97,   344,     0,   345,
       0,     0,   346,     0,   286,   287,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   107,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   289,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   291,   292,   293,
     294,   295,   296,   297,     0,     0,     0,   211,     0,     0,
       0,     0,     0,   298,     0,     0,     0,     0,     0,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
      50,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,     0,   334,     0,  1367,   336,
     337,   338,     0,     0,     0,   339,   598,   215,   216,   217,
     218,   219,   599,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   278,   279,     0,   280,   281,     0,   600,
     282,   283,   284,   285,     0,    93,    94,     0,    95,   189,
      97,   344,     0,   345,     0,     0,   346,     0,   286,   287,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   107,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   289,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   291,   292,   293,   294,   295,   296,   297,     0,     0,
       0,   211,     0,     0,     0,     0,     0,   298,     0,     0,
       0,     0,     0,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,    50,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,     0,
     334,     0,     0,   336,   337,   338,     0,     0,     0,   339,
     598,   215,   216,   217,   218,   219,   599,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   600,     0,     0,     0,     0,     0,    93,
      94,     0,    95,   189,    97,   344,     0,   345,     0,     0,
     346,   470,   471,   472,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   107,     0,     0,
       0,   473,   474,  1515,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,     0,   498,   470,
     471,   472,     0,     0,     0,     0,     0,     0,     0,     0,
     499,     0,     0,     0,     0,     0,     0,     0,     0,   473,
     474,     0,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,     0,   498,   470,   471,   472,
       0,     0,     0,     0,     0,     0,     0,     0,   499,     0,
       0,     0,     0,     0,     0,     0,     0,   473,   474,     0,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,     0,   498,   470,   471,   472,     0,     0,
       0,     0,     0,     0,     0,     0,   499,     0,     0,     0,
       0,     0,     0,     0,     0,   473,   474,     0,   475,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,  1516,   498,   470,   471,   472,     0,     0,     0,     0,
       0,     0,     0,     0,   499,     0,     0,     0,     0,     0,
       0,     0,     0,   473,   474,     0,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   500,
     498,   470,   471,   472,     0,     0,     0,     0,     0,     0,
       0,     0,   499,     0,     0,     0,     0,     0,     0,     0,
       0,   473,   474,     0,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   584,   498,   470,
     471,   472,     0,     0,     0,     0,     0,     0,     0,     0,
     499,     0,     0,     0,     0,     0,     0,     0,     0,   473,
     474,     0,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   586,   498,   470,   471,   472,
       0,     0,     0,     0,     0,     0,     0,     0,   499,   288,
       0,     0,     0,     0,     0,     0,     0,   473,   474,     0,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   605,   498,     0,   290,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   499,     0,   288,   211,
       0,     0,     0,     0,     0,  1523,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,   609,     0,     0,     0,   290,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   288,     0,   211,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   587,   215,
     216,   217,   218,   219,   588,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,   824,   594,
       0,   188,     0,   290,    91,   343,     0,    93,    94,     0,
      95,   189,    97,     0, -1125,   288,   211,     0,     0,     0,
       0,     0,   994,     0,     0,   347,     0,   587,   215,   216,
     217,   218,   219,   588,     0,   107,   348,     0,     0,    50,
       0,     0,     0,     0,     0,     0,   847,     0,     0,     0,
     188,     0,   290,    91,   343,     0,    93,    94,     0,    95,
     189,    97,     0,   288,     0,   211,     0,     0,     0,     0,
       0,  1446,     0,     0,   347,   587,   215,   216,   217,   218,
     219,   588,     0,     0,   107,   348,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   188,     0,
     290,    91,   343,     0,    93,    94,     0,    95,   189,    97,
       0,     0,     0,   211,     0,     0,     0,     0,     0,     0,
       0,     0,   347,     0,   587,   215,   216,   217,   218,   219,
     588,     0,   107,   348,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   188,     0,     0,
      91,   343,     0,    93,    94,     0,    95,   189,    97,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1106,     0,
       0,   347,   587,   215,   216,   217,   218,   219,   588,     0,
       0,   107,   348,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   188,     0,     0,    91,   343,
      29,    93,    94,     0,    95,   189,    97,     0,    34,    35,
      36,   211,     0,   212,    40,     0,     0,     0,     0,   347,
       0,     0,   213,     0,     0,     0,     0,     0,     0,   107,
     348,     0,     0,     0,    50,     0,     0,     0,  1375,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   214,     0,     0,     0,     0,   877,   878,     0,     0,
       0,     0,   879,     0,   880,     0,     0,     0,     0,  1107,
      75,   215,   216,   217,   218,   219,   881,    81,    82,    83,
      84,    85,     0,     0,    34,    35,    36,   211,   220,     0,
       0,     0,     0,   188,    89,    90,    91,    92,   213,    93,
      94,     0,    95,   189,    97,     0,     0,     0,    99,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   104,     0,     0,     0,     0,   107,   221,     0,
       0,     0,   877,   878,   111,     0,     0,     0,   879,     0,
     880,     0,     0,     0,     0,     0,   882,   883,   884,   885,
     886,   887,   881,    81,    82,    83,    84,    85,     0,     0,
      34,    35,    36,   211,   220,     0,     0,     0,     0,   188,
      89,    90,    91,    92,   213,    93,    94,     0,    95,   189,
      97,     0,     0,     0,    99,     0,    50,     0,     0,     0,
       0,     0,     0,   888,     0,     0,     0,     0,   104,     0,
       0,     0,     0,   107,   889,     0,     0,     0,  1059,  1060,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   882,   883,   884,   885,   886,   887,  1061,    81,
      82,    83,    84,    85,     0,     0,  1062,  1063,  1064,   211,
     220,     0,     0,     0,     0,   188,    89,    90,    91,    92,
    1065,    93,    94,     0,    95,   189,    97,     0,     0,     0,
      99,     0,    50,     0,     0,     0,     0,     0,     0,   888,
       0,     0,     0,     0,   104,     0,     0,     0,     0,   107,
     889,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1066,  1067,
    1068,  1069,  1070,  1071,     0,     0,     0,     0,     0,     0,
       0,  1177,     0,     0,     0,     0,  1072,     0,     0,     0,
       0,   188,     0,     0,    91,    92,    29,    93,    94,     0,
      95,   189,    97,     0,    34,    35,    36,   211,     0,   212,
      40,     0,     0,     0,     0,  1073,     0,     0,   213,     0,
       0,     0,     0,     0,     0,   107,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   214, -1126, -1126,
   -1126, -1126,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,
    1172,  1173,  1174,  1175,  1176,     0,    75,   215,   216,   217,
     218,   219,     0,    81,    82,    83,    84,    85,  1177,     0,
       0,     0,     0,     0,   220,     0,     0,     0,     0,   188,
      89,    90,    91,    92,     0,    93,    94,    29,    95,   189,
      97,     0,     0,     0,    99,    34,    35,    36,   211,     0,
     212,    40,     0,     0,     0,     0,     0,     0,   104,   213,
       0,     0,     0,   107,   221,     0,     0,   625,     0,     0,
     111,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   214,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   645,    75,   215,   216,
     217,   218,   219,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   220,     0,     0,     0,     0,
     188,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     189,    97,     0,     0,     0,    99,     0,    29,  1050,     0,
       0,     0,     0,     0,     0,    34,    35,    36,   211,   104,
     212,    40,     0,     0,   107,   221,     0,     0,     0,   213,
       0,   111,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   214,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,   215,   216,
     217,   218,   219,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   220,     0,     0,     0,     0,
     188,    89,    90,    91,    92,     0,    93,    94,    29,    95,
     189,    97,     0,     0,     0,    99,    34,    35,    36,   211,
       0,   212,    40,     0,     0,     0,     0,     0,     0,   104,
     213,     0,     0,     0,   107,   221,     0,     0,     0,     0,
       0,   111,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   214,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1207,    75,   215,
     216,   217,   218,   219,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,   220,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,    29,
      95,   189,    97,     0,     0,     0,    99,    34,    35,    36,
     211,     0,   212,    40,     0,     0,     0,     0,     0,     0,
     104,   213,     0,     0,     0,   107,   221,     0,     0,     0,
       0,     0,   111,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     214,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
     215,   216,   217,   218,   219,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,   220,     0,     0,
       0,     0,   188,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   189,    97,     0,     0,     0,    99,     0,     0,
       0,   470,   471,   472,     0,     0,     0,     0,     0,     0,
       0,   104,     0,     0,     0,     0,   107,   221,     0,     0,
       0,   473,   474,   111,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,     0,   498,   470,
     471,   472,     0,     0,     0,     0,     0,     0,     0,     0,
     499,     0,     0,     0,     0,     0,     0,     0,     0,   473,
     474,     0,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,     0,   498,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   499,     0,
     470,   471,   472,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   545,
     473,   474,     0,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,     0,   498,   470,   471,
     472,     0,     0,     0,     0,     0,     0,     0,     0,   499,
       0,     0,     0,     0,     0,     0,     0,   554,   473,   474,
       0,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,     0,   498,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   499,     0,   470,
     471,   472,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   951,   473,
     474,     0,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,     0,   498,   470,   471,   472,
       0,     0,     0,     0,     0,     0,     0,     0,   499,     0,
       0,     0,     0,     0,     0,     0,  1036,   473,   474,     0,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,     0,   498,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   499,     0,   470,   471,
     472,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1090,   473,   474,
       0,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,     0,   498,  1151,  1152,  1153,     0,
       0,     0,     0,     0,     0,     0,     0,   499,     0,     0,
       0,     0,     0,     0,     0,  1422,     0,  1154,     0,     0,
    1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1177,  1151,  1152,  1153,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1464,  1154,     0,     0,
    1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1151,  1152,  1153,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1177,     0,     0,     0,     0,
       0,     0,     0,  1154,  1349,     0,  1155,  1156,  1157,  1158,
    1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1177,  1151,  1152,  1153,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1154,  1534,     0,  1155,  1156,  1157,  1158,
    1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1151,  1152,
    1153,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1177,     0,     0,     0,     0,     0,     0,     0,  1154,
    1545,     0,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,
    1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1177,  1151,  1152,
    1153,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1154,
    1652,     0,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,
    1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,
    1173,  1174,  1175,  1176,    34,    35,    36,   211,     0,   212,
      40,     0,     0,     0,     0,     0,     0,  1177,   213,     0,
       0,     0,     0,     0,     0,     0,  1747,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   242,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   243,     0,
       0,     0,     0,     0,     0,     0,     0,   215,   216,   217,
     218,   219,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,   220,     0,  1749,     0,     0,   188,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   189,
      97,     0,     0,     0,    99,    34,    35,    36,   211,     0,
     212,    40,     0,     0,     0,     0,     0,     0,   104,   676,
       0,     0,     0,   107,   244,     0,     0,     0,     0,     0,
     111,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   214, -1126,
   -1126, -1126, -1126,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,     0,   498,   215,   216,
     217,   218,   219,     0,    81,    82,    83,    84,    85,   499,
       0,     0,     0,     0,     0,   220,     0,     0,     0,     0,
     188,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     189,    97,     0,     0,     0,    99,    34,    35,    36,   211,
       0,   212,    40,     0,     0,     0,     0,     0,     0,   104,
     213,     0,     0,     0,   107,   677,     0,     0,     0,     0,
       0,   678,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   242,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   215,
     216,   217,   218,   219,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,   220,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,     0,     0,     0,    99,     0,     0,     0,
     470,   471,   472,     0,     0,     0,     0,     0,     0,     0,
     104,     0,     0,     0,     0,   107,   244,   859,     0,     0,
     473,   474,   111,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,     0,   498,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   499,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   470,   471,   472,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   860,   473,   474,  1033,   475,   476,   477,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
       0,   498,   470,   471,   472,     0,     0,     0,     0,     0,
       0,     0,     0,   499,     0,     0,     0,     0,     0,     0,
       0,     0,   473,   474,     0,   475,   476,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,     0,   498,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   499
};

static const yytype_int16 yycheck[] =
{
       5,     6,   131,     8,     9,    10,    11,    12,    13,    56,
      15,    16,    17,    18,   108,   165,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   192,    31,   108,   170,   171,
     123,    98,   570,   732,   415,   102,   103,    57,   123,    44,
      33,   123,    19,    20,   108,     4,   850,    52,     4,    54,
       4,   548,    57,    46,    59,   504,   415,   694,    51,    30,
       4,   444,   690,   130,   193,   108,   992,   415,   606,   558,
     875,   990,   501,   502,   981,    30,   498,    30,  1414,   245,
    1027,    86,    57,   865,   721,   669,   836,   691,  1105,   533,
     534,   858,    32,   256,   539,  1012,   190,  1014,  1045,   165,
     790,     9,   530,   108,    30,   530,   535,     6,     4,   190,
      44,     9,    32,    70,     9,   257,   446,   447,   448,    14,
     564,     9,  1882,   619,   620,     9,     0,   221,     9,     9,
      14,     4,     9,    14,     9,  1218,    38,     9,   566,     9,
     221,   566,    48,     9,  1091,     9,   123,   190,     9,    48,
     244,     9,    86,     9,   651,   652,     9,    53,     9,    36,
      56,     9,     9,   244,    83,     9,   365,   366,   367,    70,
     369,    83,   162,    70,     9,     9,     9,    73,   221,     9,
      48,    83,  1101,   680,    70,   190,    48,     9,    38,   994,
       9,   116,   197,    83,    48,    50,    51,   136,   137,    91,
      96,    38,    98,   136,   137,   162,   102,   103,    54,   198,
      70,   201,    83,    84,   113,  1227,   221,   714,    70,   168,
     119,    48,   121,   122,   123,   124,   125,   126,   127,   198,
      38,    38,   162,    83,   130,    38,   103,   400,   183,   244,
    1147,     6,   200,   201,   201,    83,    83,    83,   160,   161,
     136,   137,   201,   198,   259,   180,   198,   262,    83,    84,
     200,   238,     8,   202,   269,   270,   180,   159,   198,    70,
     169,   170,    70,   172,   176,    83,    83,     4,    70,    70,
      83,   201,   779,    48,   921,   204,   176,  2047,   183,   195,
     457,   990,   200,   387,   193,    70,    70,  1563,  1048,   166,
     201,  2061,   200,   202,   201,   200,   387,   263,  1325,   356,
     206,   267,   200,   201,  1231,   201,   200,   855,   201,   200,
     200,   167,   199,  1022,   162,   200,   176,  1663,   200,   184,
     200,   199,  1279,  1115,   200,  1117,   200,   199,   198,   200,
     176,   195,   200,   199,   387,   199,   198,   200,   113,   200,
     444,   199,   357,   200,   119,   199,   121,   122,   123,   124,
     125,   126,   127,   444,   199,   199,   199,   263,   176,   199,
     867,   267,   199,   176,   541,   271,   873,   199,  1461,   204,
     199,   386,   387,   512,   451,  1468,   970,  1470,   393,   394,
     395,   396,   397,   398,     4,   442,  1303,   198,   403,   376,
     201,   444,  1101,   201,   169,   170,  1672,   172,   385,   201,
     387,   107,   108,    70,    83,   392,  1499,   422,   198,   183,
    1024,   917,   918,   198,   444,   430,   201,   404,   193,  1234,
    1696,   198,  1698,    70,   198,    70,   441,   202,   165,   444,
     507,   508,   509,   510,    70,   198,   943,  1459,    70,   513,
      70,    70,   386,    70,   459,   460,    70,    70,    70,   162,
     356,   395,   396,   198,   398,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   513,   499,   198,   501,   502,   503,   136,
     137,   415,   947,    83,    84,   162,   202,   176,   513,   514,
     515,   516,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   131,    70,    19,    20,    83,  1216,   498,   533,   534,
     535,   536,  1615,   429,   697,   984,   699,   542,   513,  1344,
     545,   198,   438,   498,   201,   498,   442,    83,   421,   554,
     162,   556,   572,   198,  1321,   451,   107,   108,   708,   564,
      50,    51,   539,   198,   201,   826,   201,   572,    83,   574,
     998,    54,   498,   998,  1112,   201,   198,   198,   504,   201,
      91,   201,   201,   193,   201,   198,   199,   201,  1085,   201,
     504,   103,  1604,   122,  1606,   198,   677,   103,  1042,    83,
     981,  1937,   198,   132,   530,  1233,    81,    91,   504,   505,
     506,   507,   508,   509,   510,    83,   530,   783,   176,   198,
     625,   577,   981,    91,   204,   792,   552,   103,  1050,  1012,
      14,  1014,  1129,   981,   530,  1542,  1133,   563,   552,    70,
     566,  1446,   708,  1140,   159,   160,   161,    70,   159,   563,
     816,   202,   566,   201,   166,    83,   552,   132,   194,  1263,
     166,   162,  1266,    91,     4,   201,  1094,   830,   831,  1094,
     566,    70,   677,   740,   837,   838,   160,   161,    70,    19,
      20,   577,   183,  1013,   167,    14,  1016,  1017,   123,   183,
     166,    70,   160,   161,    70,   837,  1192,   132,    70,   182,
    1149,   597,   429,    32,   198,   136,   137,  1719,   907,    83,
     909,  1723,   911,    83,   719,   167,   915,    91,    57,   103,
     104,    91,    51,   207,   890,   198,  1633,   623,   624,   201,
      69,   162,   160,   161,   103,   104,   902,    83,   206,  1942,
     717,  1944,   238,   748,    87,    91,   136,   137,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,   112,   658,   659,   136,   137,  1147,   198,   108,    83,
     121,   122,   123,   124,   125,   126,   781,    91,   166,  1469,
     162,   124,   125,   126,   166,   159,   160,   161,  1147,   159,
     160,   161,  1281,    59,    60,   396,  1293,   398,    32,  1147,
    1061,   183,   106,   107,   108,   166,   811,    31,   198,  1437,
      75,    76,   200,   201,   160,   161,   793,  1246,   691,   429,
     121,   122,   123,   124,   125,   126,    50,   168,  1272,    53,
     121,   122,   123,   124,   125,   126,   732,   842,    87,  1283,
     181,   183,   193,  1144,   740,  1146,   160,   161,  1231,    81,
     190,    83,    84,   858,   198,   832,   198,   834,  1355,  1463,
     201,   854,  1359,   183,  1402,    31,    60,  1364,   198,  1881,
     136,   137,   104,  1885,  1371,   124,   125,   126,   198,  1324,
     376,   221,   162,   860,    50,   162,    38,    53,  1578,   385,
      75,    76,   193,  1592,    88,   118,   392,    91,   238,   200,
     200,   511,   193,   183,   244,   200,   183,   863,   404,   141,
     142,   143,   144,   145,   200,    53,    54,    55,   198,   415,
      70,   201,  1303,   263,   201,   200,  1298,   267,  1300,   200,
     826,    69,   828,   199,    70,   167,   200,   169,   170,   200,
     172,   173,   174,   200,  1303,    70,   951,    70,   953,    70,
     955,    70,   848,    70,   931,  1303,   134,   135,    53,    54,
      55,   966,    57,  1501,   201,   197,   862,   863,   198,   201,
     947,   948,   204,  1418,    69,   980,   365,   366,   367,  1517,
     369,   708,  1049,  1673,   106,   107,   108,  1484,   112,   113,
     114,  1488,  1012,   162,  1014,   200,   201,   198,  1495,  1019,
    1913,  1914,   997,   998,  1009,   732,    70,  1012,   904,  1014,
     969,   162,  1616,   969,  1019,   969,   198,   913,   914,  1909,
    1910,  1465,  2034,   166,   200,   969,    49,    69,  1033,   183,
     162,  1036,   203,  1038,  1295,  1296,   376,  1042,  1299,  2051,
     198,   198,     9,   539,  1305,   385,   162,   387,   944,   162,
     198,     8,   392,   200,   198,  1438,   162,   162,   984,    14,
     200,   200,     9,  1982,   404,    14,   201,   981,  1987,   200,
     984,   997,   998,   969,   132,  1241,   132,  2003,  1244,  1050,
     183,   199,    14,   997,   998,  1090,   103,   199,   984,   429,
     199,   199,   199,  2012,   990,  1050,   969,  1050,  1636,   198,
    2026,   997,   998,   205,   444,  1098,   112,  1645,   201,  2035,
     198,  1441,  1442,  1443,  1563,   198,     9,   199,   159,   992,
     199,   848,   732,  1661,  1050,   199,  1022,   199,  1105,  1106,
    1934,    95,     9,   200,    14,  1939,   183,   198,  1231,     9,
     200,   198,   201,  1099,    83,  1209,  1231,   201,   200,  1231,
     201,  1024,   200,  1049,   200,  2074,  1600,  1601,   401,   199,
     199,  1542,   405,  1059,  1060,  1061,  1092,   201,  1094,   199,
     198,   365,   366,   367,   368,   369,   200,   904,  1092,   199,
    1094,   134,   203,  1542,  1988,     9,     9,   203,   431,  1209,
     433,   434,   435,   436,  1542,   203,  1092,    70,  1094,   539,
     203,   203,  1740,  1099,  1209,  1101,    32,  1103,   135,   182,
     162,  1231,   138,   407,     9,   199,  1477,   162,  1479,   195,
    1669,   717,    14,  1672,     9,     9,  1231,   199,  1124,     9,
     184,    14,  2037,  1147,     9,   134,  2041,   577,   848,   199,
     199,  1246,   969,   199,  1249,   203,     9,  1696,   199,  1698,
    2055,  2056,  1633,  1149,  1231,  1704,  1215,   203,   202,  1215,
    2064,  1215,   203,   990,    19,    20,   199,  1272,    14,   199,
    1247,  1215,   203,   198,  1633,   162,   199,     9,  1283,  1284,
     200,   103,  1178,  1982,   200,  1633,   138,   162,  1987,     9,
     199,   198,    70,    70,   904,  1022,  1933,   793,  1935,    70,
    1442,    70,    50,    51,    52,    53,    54,    55,    70,    57,
     198,   201,     9,  2012,    14,   184,  1321,   202,   200,  1215,
    1313,    69,    81,     9,    14,   201,  1331,    14,   200,   203,
     201,   199,  1309,   195,    32,    70,   832,   198,   834,   198,
      32,    14,  1215,   198,  1438,   104,    14,  1324,  1325,   198,
      52,  1889,   198,    70,    70,     6,    70,  1438,    70,   969,
    1316,  1622,    70,  1624,   860,  1626,   198,     9,   162,   199,
    1631,   130,   200,   200,  1101,  2074,  1103,   717,   198,    14,
     990,   138,   141,   142,   143,   144,   145,  2024,   184,  1303,
    1263,   138,   732,  1266,   162,  1438,     9,    48,   199,  1295,
    1296,  1297,  1298,  1299,  1300,    69,  1903,  1904,     9,  1305,
     169,   170,  1022,   172,   173,   174,   176,  1422,  1438,   200,
    1316,   203,   176,    83,  1429,   619,   620,     9,  1433,  1579,
    1435,   198,  1328,  1438,  1883,   931,   202,   202,   197,   198,
      81,  1418,  1338,   138,   202,  1574,  1451,   198,   200,   202,
      83,   947,   948,   793,    14,    83,     6,   199,   201,  1464,
    1465,   198,   113,   104,   198,   201,   199,   826,   119,  1730,
     121,   122,   123,   124,   125,   126,   127,    50,    51,    52,
      53,    54,    55,   238,   138,   981,   201,   200,  1215,     9,
      92,  1101,   832,  1103,   834,    32,    69,   203,    48,    77,
     141,   142,   143,   144,   145,   201,   159,   200,   848,   200,
     199,   138,   184,    32,  1410,   199,   199,     9,   169,   170,
     860,   172,   163,   863,  2062,   166,   203,  1694,   169,   170,
       9,   172,   173,   174,   203,   138,   203,   203,   203,     9,
     199,   202,   193,  1416,     9,    78,    79,    80,   200,   199,
      14,   202,   200,  1426,   200,   200,   197,   200,   202,    92,
      83,   202,   201,   113,   904,   203,   198,   198,   198,   119,
     199,   121,   122,   123,   124,   125,   126,   127,   199,   199,
     199,  1477,   200,  1479,   201,   199,     9,  1592,   203,   138,
    1463,   931,  1597,   203,     9,  1600,  1601,   203,   203,  1666,
     138,  1328,   199,   203,     9,  1215,    32,   947,   948,  1105,
    1106,   200,   199,   199,   147,   148,   149,   150,   151,   169,
     170,   376,   172,   200,   200,   158,  1887,  1888,  1542,   969,
     385,   164,   165,   138,   176,   201,   113,   392,   200,   171,
     167,    83,    14,   193,    83,   178,   119,   199,   199,   404,
     990,  1147,   202,   201,   138,   199,  1552,   138,    14,   192,
     201,   183,    83,    14,    19,    20,   200,  1563,    14,    83,
     199,   199,  1012,  1569,  1014,    30,   138,   198,   138,   197,
      14,    14,  1022,   199,    14,   200,  1642,   200,   200,  1416,
     201,     9,     9,   202,    68,    83,  1592,   183,   198,  1426,
    1867,    56,  1061,    83,     9,     9,   201,   200,  1713,   116,
     103,   162,   103,   907,    36,   909,   184,   911,  1328,  1633,
     174,   915,    14,   917,   918,   919,  1622,   198,  1624,   184,
    1626,   200,  1605,   199,   180,  1631,   184,   198,  1611,    83,
    1613,  1637,   177,  1616,     9,    83,  1642,   199,    83,   199,
    1646,  1247,   201,   200,   199,    14,    14,    83,    83,  1099,
      83,  1101,  1635,  1103,    14,  1105,  1106,    83,    14,    83,
    1666,  1192,  2015,  1669,  1043,   505,  1672,   972,   507,   510,
    2031,     6,  1738,  1739,   539,  1322,  1682,  1739,  1514,  2026,
    1248,   627,  1764,  1689,   121,   122,   123,   124,   125,   126,
    1696,  1851,  1698,  1726,  2072,   132,   133,  1303,  1704,  1674,
    1569,  1863,  2048,  1309,  1565,  1862,   397,  1722,  1145,  1299,
    1141,  1219,  1060,    48,  1294,  1552,  1295,  1087,  1324,  1325,
     393,   442,  1927,   876,  1730,  1983,  1973,  1557,  1200,    -1,
    1125,  1737,  1738,  1739,  1178,    -1,  1996,  1743,   175,    -1,
      -1,    -1,  1579,    -1,  1750,  1728,    -1,   121,   122,   123,
     124,   125,   126,    -1,    -1,  1592,   193,  1872,   132,   133,
      -1,    -1,    -1,    -1,    -1,  1215,    -1,    -1,  1605,    -1,
    1927,    -1,    -1,   238,  1611,    -1,  1613,    -1,   113,    -1,
    2019,  1231,    -1,    -1,   119,    -1,   121,   122,   123,   124,
     125,   126,   127,    -1,    -1,    -1,    -1,  1247,  1635,   173,
    1637,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1646,
      -1,    -1,  1418,    -1,   188,    -1,   190,    -1,    -1,   193,
      -1,    -1,    -1,   288,    -1,   290,  1295,  1296,  1297,  1298,
    1299,  1300,  1552,    -1,   169,   170,  1305,   172,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,   717,  1573,    -1,    -1,  1862,    -1,   193,  1309,
      -1,    -1,    -1,    -1,    -1,    -1,  1316,   202,    -1,    -1,
      -1,    -1,  1592,    -1,  1324,  1325,    -1,  1883,  1328,    -1,
      -1,  1887,  1888,   348,    59,    60,    -1,  1893,  1192,    -1,
      -1,  1728,    -1,    -1,    -1,    -1,  1902,    -1,    -1,    -1,
    1737,    -1,    -1,  1909,  1910,    -1,  1743,  1913,  1914,    -1,
      -1,   376,    -1,  1750,    -1,    -1,    -1,  1637,    -1,    -1,
     385,  1927,    -1,    -1,    -1,    -1,  1646,   392,   793,     6,
      -1,    -1,    -1,    -1,    -1,    -1,  1542,    -1,    -1,   404,
      -1,  1947,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     415,    -1,    -1,    -1,    -1,  2070,    -1,    -1,    -1,    -1,
      -1,   136,   137,    -1,  2079,    -1,    -1,   832,  1418,   834,
      -1,    48,  2087,    -1,   439,  2090,  1982,   442,    -1,    -1,
      -1,  1987,  1702,    -1,    -1,    -1,    -1,    -1,  1438,  1995,
      -1,    -1,    -1,    -1,    -1,   860,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  2012,    -1,  1477,    -1,
    1479,    -1,  2018,    -1,    -1,    -1,    81,  1737,    -1,    -1,
    2003,    -1,    -1,  1743,   199,    -1,    -1,  1633,    -1,    -1,
    1750,    -1,    -1,   498,    -1,    -1,   113,    -1,    -1,   104,
      -1,    -1,   119,  2026,   121,   122,   123,   124,   125,   126,
     127,    -1,  2035,    -1,    -1,    -1,  1893,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   931,  2073,  2074,    -1,
      -1,    -1,    -1,    -1,   539,    -1,   141,   142,   143,   144,
     145,    -1,   947,   948,    -1,    -1,    -1,     6,    -1,    -1,
      -1,    -1,   169,   170,    -1,   172,    -1,    -1,    -1,    -1,
      -1,   166,  1552,    -1,   169,   170,    -1,   172,   173,   174,
    1947,    -1,    -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   202,   591,    -1,   593,    48,
      -1,   596,   197,    -1,    -1,    -1,   201,    -1,    -1,    -1,
      -1,    -1,  1592,    -1,    -1,  1982,    -1,    -1,    -1,    -1,
    1987,    -1,    -1,  1622,    -1,  1624,    -1,  1626,    -1,  1996,
      -1,    -1,  1631,    -1,   629,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1893,    -1,  2012,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1637,    -1,    -1,
      -1,    -1,  1642,    -1,   113,    -1,  1646,    -1,    -1,    -1,
     119,    -1,   121,   122,   123,   124,   125,   126,   127,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,   687,   688,    -1,    -1,    -1,  1947,    -1,    -1,
      -1,   696,    -1,    -1,    -1,    -1,  2073,  2074,    -1,    -1,
    1105,  1106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     169,   170,   717,   172,    59,    60,    -1,    -1,    -1,    -1,
      -1,  1730,  1982,    -1,    -1,    -1,    -1,  1987,    -1,    -1,
      -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,   202,    -1,    -1,    -1,  1737,  1738,  1739,
      -1,    -1,  2012,  1743,    -1,    -1,    -1,    -1,    30,    31,
    1750,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,   793,    -1,
      -1,   136,   137,    -1,    -1,    -1,    -1,    69,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,  2073,  2074,    -1,    -1,    -1,    -1,    -1,
      -1,   826,    -1,    -1,    -1,    -1,    -1,   832,    -1,   834,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1247,    -1,    59,    60,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   199,   860,   861,    -1,    -1,    -1,
      -1,    -1,    -1,   868,    -1,    -1,    -1,    -1,    -1,    -1,
     875,   876,   877,   878,   879,   880,   881,    -1,  1887,  1888,
      -1,    -1,    -1,    -1,   889,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     905,    -1,    -1,  1893,  1309,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,  1324,
    1325,   136,   137,    -1,    -1,    -1,   931,    -1,    -1,    -1,
      -1,   203,    59,    60,    -1,    -1,    -1,    -1,    -1,    -1,
     945,    -1,   947,   948,    -1,    -1,    -1,    -1,    -1,    -1,
      59,    60,    -1,    -1,    -1,    -1,    -1,  1947,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   971,   972,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   981,    -1,    -1,    19,
      20,    -1,   987,    -1,   199,    -1,    -1,    -1,    -1,   994,
      30,    -1,  1982,    -1,    -1,  1000,    -1,  1987,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1011,    -1,    -1,   136,
     137,    -1,    -1,  1418,    56,    -1,    -1,    -1,    -1,    -1,
      -1,  1026,  2012,    -1,    -1,    -1,    -1,   136,   137,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,  1046,    -1,    -1,    -1,  1050,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,  1061,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,  2073,  2074,    -1,    -1,    -1,    -1,    -1,
     199,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
    1105,  1106,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1119,    -1,    -1,    -1,  1123,    -1,
    1125,    -1,    -1,  1128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1141,  1142,  1143,  1144,
    1145,  1146,  1147,    -1,    -1,  1150,  1151,  1152,  1153,  1154,
    1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,
    1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1196,    -1,    -1,    -1,    -1,    30,    31,   238,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    81,    57,   200,    -1,   202,  1232,    -1,  1234,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,  1247,    -1,    -1,   104,   288,    -1,   290,    -1,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    81,    -1,    83,    84,    -1,
    1275,    -1,    -1,  1278,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,    -1,   104,    -1,
    1295,  1296,  1297,  1298,  1299,  1300,    59,    60,  1303,    -1,
    1305,    -1,    -1,    -1,  1309,    -1,   348,    -1,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,  1324,
    1325,    -1,  1327,    -1,    -1,   141,   142,   143,   144,   145,
      -1,    -1,  1337,    -1,    -1,   288,   376,   290,   197,  1344,
      -1,    -1,    -1,    -1,  1349,   385,  1351,    -1,    -1,    -1,
      -1,   167,   392,   169,   170,    -1,   172,   173,   174,    -1,
      -1,    -1,    -1,    -1,   404,    -1,    -1,    -1,   202,    -1,
    1375,    -1,    -1,   136,   137,   415,    -1,    -1,    -1,    -1,
      -1,   197,    -1,    -1,    -1,   201,    -1,    -1,   204,    -1,
      -1,    10,    11,    12,    -1,   348,    -1,   439,    -1,    -1,
     442,    -1,    -1,    -1,    -1,    -1,  1411,  1412,    -1,    -1,
    1415,    30,    31,  1418,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,  1446,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   498,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,  1477,    -1,  1479,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   439,    -1,    -1,   442,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   539,
      -1,    -1,    -1,    -1,    59,    60,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1522,  1523,    -1,
      -1,  1526,    -1,    -1,    -1,    -1,    -1,  1532,    -1,  1534,
      -1,  1536,    -1,    -1,    -1,    -1,  1541,  1542,    -1,    -1,
    1545,    -1,  1547,    -1,    -1,  1550,    -1,    -1,    -1,   591,
      -1,   593,    -1,    -1,    -1,    -1,   596,    -1,  1563,  1564,
      -1,    -1,  1567,    -1,    -1,    -1,    -1,    -1,    -1,  1574,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   136,   137,   202,    -1,    -1,    -1,    -1,    -1,   629,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,  1622,    57,  1624,
      -1,  1626,    -1,    -1,    -1,    -1,  1631,    -1,  1633,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   591,    -1,
      -1,    -1,  1647,    -1,    -1,   687,   688,  1652,    -1,    -1,
      -1,    -1,    -1,    -1,   696,    -1,    -1,    -1,    -1,  1664,
    1665,    -1,    -1,    -1,    -1,    -1,    -1,  1672,    -1,  1674,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   717,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1696,    -1,  1698,    -1,    -1,    -1,    -1,    -1,  1704,
      31,    -1,    -1,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,  1730,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   687,   688,    -1,    -1,    69,    81,
      -1,    -1,  1747,  1748,  1749,    -1,    -1,    -1,    -1,  1754,
      -1,  1756,    -1,   793,    -1,    -1,    -1,  1762,    -1,  1764,
      -1,    -1,   104,   202,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   826,    -1,    -1,    -1,
      -1,    -1,   832,    -1,   834,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     860,   861,    -1,    -1,    81,   167,   868,   169,   170,   171,
     172,   173,   174,   875,   876,    92,    -1,   877,   878,   879,
     880,   881,    -1,    -1,    -1,    -1,    -1,   104,    -1,   889,
      -1,    -1,    -1,    -1,    -1,   197,   198,    -1,    -1,    -1,
      -1,  1866,    -1,    -1,    -1,   905,    -1,    -1,   199,    -1,
      -1,    -1,    -1,    -1,  1879,    -1,    -1,    -1,  1883,    -1,
      -1,    -1,  1887,  1888,   141,   142,   143,   144,   145,    -1,
      -1,   931,    -1,    -1,    -1,    -1,    -1,  1902,    -1,    -1,
      -1,    -1,    -1,  1908,    -1,   945,   163,   947,   948,   166,
      -1,    -1,   169,   170,  1919,   172,   173,   174,    -1,   176,
    1925,    -1,   875,   876,  1929,    -1,    -1,    -1,    -1,   971,
      -1,    -1,   972,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     197,   981,    -1,    -1,    -1,   987,    -1,    -1,    -1,    -1,
      -1,  1956,   994,    19,    20,    -1,    -1,    -1,  1000,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,
      -1,  1011,    -1,    -1,    -1,    -1,    -1,    -1,  1983,    81,
    1985,    -1,    -1,    -1,    -1,    -1,  1026,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  2001,    -1,    -1,    -1,
      -1,    -1,   104,    -1,  1046,    -1,    -1,    -1,    -1,  2014,
    1050,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   971,    -1,
      -1,  1061,    -1,    -1,    -1,    -1,  2031,    -1,    -1,    -1,
      -1,    -1,  2037,    -1,   987,    -1,  2041,    -1,    -1,   141,
     142,   143,   144,   145,    -1,    -1,    -1,  1000,    -1,    -1,
    2055,  2056,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,  1105,  1106,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,    -1,  1119,    69,    -1,
      -1,  1123,    -1,  1125,    -1,    81,  1128,    -1,    -1,    -1,
      -1,    -1,    -1,  1046,    -1,   197,   198,    -1,    -1,    -1,
      -1,  1141,  1142,  1143,  1144,  1145,  1146,  1147,   104,    -1,
    1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,
    1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
      -1,    -1,    -1,    -1,    -1,    -1,  1196,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1119,    -1,    -1,    -1,
    1123,    -1,   238,   169,   170,  1128,   172,   173,   174,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
    1232,    -1,  1234,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   197,   198,    -1,    -1,    30,    31,  1247,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,  1275,    -1,    -1,  1278,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    -1,    -1,  1295,  1296,  1297,  1298,  1299,
    1300,    -1,    -1,  1303,    -1,  1305,    -1,    81,    -1,  1309,
      -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,  1232,
      -1,    -1,    -1,    -1,  1324,  1325,    -1,  1327,    -1,    -1,
     104,    -1,    -1,    -1,    -1,  1337,    -1,    -1,   112,   113,
      -1,    -1,  1344,    -1,    -1,    -1,    -1,    -1,    -1,  1349,
     376,  1351,   141,   142,   143,   144,   145,    -1,    -1,   385,
      -1,    -1,  1275,    -1,    -1,  1278,   392,   141,   142,   143,
     144,   145,    -1,    -1,    -1,  1375,    -1,    -1,   404,    -1,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,   415,
      -1,    -1,   166,    -1,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   197,  1411,
    1412,    -1,    -1,    -1,    -1,  1415,    -1,   202,  1418,    -1,
      -1,    -1,    -1,   197,  1337,    -1,    10,    11,    12,    -1,
      -1,  1344,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1446,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,   498,    57,    -1,    -1,    -1,  1477,    -1,  1479,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1411,  1412,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,   539,    57,    -1,    -1,    -1,    -1,    -1,
    1522,  1523,    -1,    -1,  1526,    -1,    69,    -1,    -1,    -1,
      -1,    -1,  1532,    -1,  1534,    -1,  1536,    -1,    -1,    -1,
      -1,  1541,  1542,    -1,    -1,  1545,    -1,  1547,    -1,    -1,
    1550,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,  1563,    -1,    -1,  1564,    -1,    -1,  1567,    -1,    -1,
     596,    -1,  1574,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,   629,    -1,    -1,    -1,    -1,    -1,  1522,
    1523,    -1,    69,  1526,    -1,    -1,    -1,    -1,   202,    -1,
      -1,    -1,  1622,    -1,  1624,    -1,  1626,    -1,    -1,    -1,
     596,  1631,    -1,  1633,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1647,    -1,    -1,    -1,    -1,
    1563,    -1,  1652,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1574,    -1,   629,  1664,  1665,    -1,    -1,    -1,    -1,
    1672,    -1,    -1,    -1,  1674,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   717,    -1,    -1,  1696,    -1,  1698,    -1,    -1,    -1,
      30,    31,  1704,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
    1730,    -1,    -1,    -1,  1647,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,   202,    -1,  1747,  1748,  1749,
      -1,    -1,    -1,    -1,  1754,    81,  1756,    -1,    -1,  1672,
    1762,    -1,    -1,    -1,  1764,    -1,    -1,   793,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,  1696,    -1,  1698,    -1,    -1,    -1,    -1,
      -1,  1704,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   127,    -1,    -1,    -1,    -1,   832,    -1,   834,    -1,
      -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,   145,
     146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,   860,   861,    -1,   163,    -1,    -1,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,  1762,
      -1,   877,   878,   879,   880,   881,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   889,  1866,    -1,    -1,    -1,    -1,    -1,
      68,   197,   202,    -1,    -1,    -1,    -1,  1879,    -1,    -1,
      -1,  1883,    -1,    81,    -1,    -1,    -1,  1887,  1888,    87,
      -1,    -1,    -1,    -1,    -1,   861,    -1,    -1,    -1,    -1,
    1902,    -1,    -1,    -1,    -1,   931,   104,    -1,  1908,    -1,
      -1,   877,   878,   879,   880,   881,    -1,    -1,    -1,  1919,
      -1,   947,   948,   889,    -1,  1925,    -1,    -1,    -1,  1929,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   140,   141,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,  1866,  1956,   981,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,   163,  1879,    -1,   166,   167,
    1883,   169,   170,    -1,   172,   173,   174,    -1,   176,    -1,
      -1,  1983,   104,  1985,    -1,  1011,    -1,    -1,    -1,   187,
     112,   113,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   197,
     198,  2001,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  2014,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,  1050,    -1,    -1,    -1,    -1,    -1,
      -1,  2031,    -1,    -1,    -1,  2037,    -1,    -1,    -1,  2041,
      -1,   163,    -1,  1956,   166,  1011,    -1,   169,   170,    -1,
     172,   173,   174,  2055,  2056,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,   188,    -1,    -1,    -1,
    1983,    -1,  1985,    -1,    -1,   197,    -1,    -1,    -1,  1105,
    1106,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,  1141,  1142,  1143,  1144,  1145,
    1146,  1147,    69,    -1,  1150,  1151,  1152,  1153,  1154,  1155,
    1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    1196,    57,    -1,    -1,    -1,  1141,  1142,    31,    -1,  1145,
      -1,    -1,    -1,    69,  1150,  1151,  1152,  1153,  1154,  1155,
    1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,
    1166,  1167,  1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,
    1176,  1177,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,
      -1,  1247,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,
    1196,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
     104,    57,    -1,    -1,    -1,   202,    -1,    -1,   112,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,   121,   122,   123,
     124,   125,   126,    -1,    -1,    -1,    -1,  1303,    -1,    -1,
      -1,    -1,    -1,  1309,    -1,    31,   140,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,    -1,  1324,  1325,
      -1,  1327,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,
      -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,  1349,    -1,  1351,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   187,    -1,    81,    -1,    -1,    -1,   193,
      -1,    -1,    -1,   197,   198,    -1,    -1,    -1,    -1,  1375,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,  1327,     3,     4,    -1,     6,     7,    -1,    -1,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   127,    -1,  1349,    -1,  1351,    -1,    28,    29,    -1,
      -1,    -1,  1418,    -1,   140,   141,   142,   143,   144,   145,
     146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1375,
      -1,    -1,    -1,    -1,    -1,    -1,    57,   163,    -1,    -1,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    83,    84,    -1,    -1,    87,    -1,    -1,    -1,
      -1,   197,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,    -1,   130,
      -1,   132,   133,   134,   135,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,   146,  1532,    -1,  1534,    -1,
    1536,    -1,    -1,    -1,    -1,  1541,  1542,    -1,    -1,  1545,
      -1,  1547,   163,    -1,  1550,    -1,    -1,    81,   169,   170,
      -1,   172,   173,   174,   175,    -1,   177,    -1,    92,   180,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,   197,    -1,    -1,    -1,
     201,    -1,    -1,   204,    -1,   104,  1532,    -1,  1534,    -1,
    1536,    -1,    -1,    -1,    -1,  1541,    -1,    -1,    -1,  1545,
      -1,  1547,    -1,    -1,  1550,    -1,    -1,   141,   142,   143,
     144,   145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,  1633,    -1,   163,
      -1,    -1,   166,    -1,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,   163,    -1,  1652,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,
      10,    11,    12,   197,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   197,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,  1652,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,
       7,    -1,    -1,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,  1747,  1748,  1749,    -1,    -1,    -1,    -1,  1754,    -1,
      -1,    28,    29,    -1,    -1,    31,    -1,  1763,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    71,    72,    73,    74,    75,    76,
      77,  1747,  1748,  1749,    81,    -1,    83,    84,  1754,    -1,
      87,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   202,   130,    81,   132,   133,   134,   135,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,   146,
      -1,    -1,    -1,    -1,    -1,    -1,    81,   104,    83,    -1,
      85,    -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,    -1,
      -1,    -1,   169,   170,    -1,   172,   173,   174,   175,   104,
     177,    -1,  1908,   180,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1919,   141,   142,   143,   144,   145,  1925,
     197,    -1,    -1,  1929,   201,    -1,    -1,   204,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,    -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,    -1,  1958,    -1,    -1,    -1,     3,     4,     5,     6,
       7,    -1,  1908,    -1,   169,   170,    13,   172,   173,   174,
     197,    -1,    -1,  1919,    -1,    -1,    -1,    -1,    -1,  1925,
      27,    28,    29,  1929,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   197,    -1,    -1,  2001,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,    -1,
      97,    -1,    99,    -1,   101,  2001,    -1,   104,   105,    -1,
      -1,    -1,   109,   110,   111,   112,   113,   114,   115,    -1,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,   129,   130,   131,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,    -1,
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,   188,    -1,   190,    -1,   192,   193,   194,    -1,    -1,
     197,   198,    -1,   200,   201,    -1,    -1,   204,   205,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,
      92,    93,    94,    95,    -1,    97,    -1,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,
     112,   113,   114,   115,    -1,   117,    -1,   119,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,   129,   130,   131,
     132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,   188,    -1,   190,    -1,
     192,   193,   194,    -1,    -1,   197,   198,    -1,   200,   201,
     202,    -1,   204,   205,   206,   207,     3,     4,     5,     6,
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
      -1,    -1,   109,   110,   111,   112,   113,   114,   115,    -1,
     117,    -1,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,   129,   130,   131,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,    -1,
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,   188,    -1,   190,    -1,   192,   193,   194,    -1,    -1,
     197,   198,    -1,   200,   201,   202,    -1,   204,   205,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,
      92,    93,    94,    95,    -1,    97,    -1,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,
     112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,   129,   130,    -1,
     132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,
     192,   193,   194,    -1,    -1,   197,   198,    -1,   200,   201,
     202,    -1,   204,   205,   206,   207,     3,     4,     5,     6,
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
     187,    -1,    -1,    -1,    -1,   192,   193,   194,    -1,    -1,
     197,   198,    -1,   200,   201,   202,    -1,   204,   205,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,
      92,    93,    94,    95,    -1,    97,    -1,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,
     112,    -1,   114,   115,    -1,   117,   118,    -1,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,   129,   130,    -1,
     132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,
     192,   193,   194,    -1,    -1,   197,   198,    -1,   200,   201,
      -1,    -1,   204,   205,   206,   207,     3,     4,     5,     6,
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
     187,    -1,    -1,    -1,    -1,   192,   193,   194,    -1,    -1,
     197,   198,    -1,   200,   201,   202,    -1,   204,   205,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,
      92,    93,    94,    95,    -1,    97,    -1,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,
     112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,   129,   130,    -1,
     132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,
     192,   193,   194,    -1,    -1,   197,   198,    -1,   200,   201,
     202,    -1,   204,   205,   206,   207,     3,     4,     5,     6,
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
     187,    -1,    -1,    -1,    -1,   192,   193,   194,    -1,    -1,
     197,   198,    -1,   200,   201,   202,    -1,   204,   205,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,
      92,    93,    94,    95,    -1,    97,    -1,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,
     112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,   129,   130,    -1,
     132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,
     192,   193,   194,    -1,    -1,   197,   198,    -1,   200,   201,
     202,    -1,   204,   205,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,    96,
      97,    -1,    99,    -1,   101,    -1,    -1,   104,   105,    -1,
      -1,    -1,   109,   110,   111,   112,    -1,   114,   115,    -1,
     117,    -1,    -1,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,   129,   130,    -1,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,    -1,
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,    -1,    -1,    -1,    -1,   192,   193,   194,    -1,    -1,
     197,   198,    -1,   200,   201,    -1,    -1,   204,   205,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,
      92,    93,    94,    95,    -1,    97,    -1,    99,    -1,   101,
     102,    -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,
     112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,   129,   130,    -1,
     132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,
     192,   193,   194,    -1,    -1,   197,   198,    -1,   200,   201,
      -1,    -1,   204,   205,   206,   207,     3,     4,     5,     6,
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
     187,    -1,    -1,    -1,    -1,   192,   193,   194,    -1,    -1,
     197,   198,    -1,   200,   201,   202,    -1,   204,   205,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    77,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,
      92,    93,    94,    95,    -1,    97,    -1,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,
     112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,   129,   130,    -1,
     132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,
     192,   193,   194,    -1,    -1,   197,   198,    -1,   200,   201,
      -1,    -1,   204,   205,   206,   207,     3,     4,     5,     6,
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
     187,    -1,    -1,    -1,    -1,   192,   193,   194,    -1,    -1,
     197,   198,    -1,   200,   201,   202,    -1,   204,   205,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,
      92,    93,    94,    95,    -1,    97,    -1,    99,   100,   101,
      -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,
     112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,   129,   130,    -1,
     132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,
     192,   193,   194,    -1,    -1,   197,   198,    -1,   200,   201,
      -1,    -1,   204,   205,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    -1,    -1,    92,    93,    94,    95,    -1,
      97,    98,    99,    -1,   101,    -1,    -1,   104,   105,    -1,
      -1,    -1,   109,   110,   111,   112,    -1,   114,   115,    -1,
     117,    -1,    -1,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,   129,   130,    -1,   132,   133,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,   155,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,    -1,
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,    -1,    -1,    -1,    -1,   192,   193,   194,    -1,    -1,
     197,   198,    -1,   200,   201,    -1,    -1,   204,   205,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,
      92,    93,    94,    95,    -1,    97,    -1,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,
     112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,   129,   130,    -1,
     132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,
     192,   193,   194,    -1,    -1,   197,   198,    -1,   200,   201,
     202,    -1,   204,   205,   206,   207,     3,     4,     5,     6,
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
     187,    -1,    -1,    -1,    -1,   192,   193,   194,    -1,    -1,
     197,   198,    -1,   200,   201,   202,    -1,   204,   205,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,
      92,    93,    94,    95,    -1,    97,    -1,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,
     112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,   129,   130,    -1,
     132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,
     192,   193,   194,    -1,    -1,   197,   198,    -1,   200,   201,
     202,    -1,   204,   205,   206,   207,     3,     4,     5,     6,
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
     187,    -1,    -1,    -1,    -1,   192,   193,   194,    -1,    -1,
     197,   198,    -1,   200,   201,   202,    -1,   204,   205,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,
      92,    93,    94,    95,    -1,    97,    -1,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,
     112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,   129,   130,    -1,
     132,   133,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,
     192,   193,   194,    -1,    -1,   197,   198,    -1,   200,   201,
     202,    -1,   204,   205,   206,   207,     3,     4,     5,     6,
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
     187,    -1,    -1,    -1,    -1,   192,   193,   194,    -1,    -1,
     197,   198,    -1,   200,   201,    -1,    -1,   204,   205,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,
      92,    93,    94,    95,    -1,    97,    -1,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,
     112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,   129,   130,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,
     192,   193,   194,    -1,    -1,   197,   198,    -1,   200,   201,
      -1,    -1,   204,   205,   206,   207,     3,     4,     5,     6,
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
     187,    -1,    -1,    -1,    -1,   192,   193,   194,    -1,    -1,
     197,   198,    -1,   200,   201,    -1,    -1,   204,   205,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,
      92,    93,    94,    95,    -1,    97,    -1,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,
     112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,   129,   130,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,
     192,   193,   194,    -1,    -1,   197,   198,    -1,   200,   201,
      -1,    -1,   204,   205,   206,   207,     3,     4,     5,     6,
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
     187,    -1,    -1,    -1,    -1,   192,   193,   194,    -1,    -1,
     197,   198,    -1,   200,   201,    -1,    -1,   204,   205,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,    -1,
      92,    93,    94,    95,    -1,    97,    -1,    99,    -1,   101,
      -1,    -1,   104,   105,    -1,    -1,    -1,   109,   110,   111,
     112,    -1,   114,   115,    -1,   117,    -1,    -1,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,   129,   130,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,   155,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,   178,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,
     192,   193,   194,    -1,    -1,   197,   198,    -1,   200,   201,
      -1,    -1,   204,   205,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,    -1,    -1,    -1,    -1,   192,   193,   194,    -1,    -1,
     197,   198,     3,     4,     5,     6,     7,   204,   205,   206,
     207,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     181,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,
      -1,   192,   193,   194,    -1,    -1,   197,   198,    -1,   200,
      -1,    -1,    -1,   204,   205,   206,   207,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    87,    -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   121,   122,   123,   124,   125,
     126,    -1,    -1,   129,   130,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
      -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,
      -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,    -1,
     176,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,
      -1,   187,    -1,    -1,    -1,    -1,   192,   193,   194,    -1,
      -1,   197,   198,     3,     4,     5,     6,     7,   204,   205,
     206,   207,    -1,    13,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    87,    -1,    -1,
      -1,    -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   121,   122,   123,   124,   125,   126,    -1,    -1,   129,
     130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   147,   148,   149,
     150,   151,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,    -1,   176,    -1,   178,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,
      -1,    -1,   192,   193,   194,    -1,    -1,   197,   198,     3,
       4,     5,     6,     7,   204,   205,   206,   207,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,
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
     194,    -1,    -1,   197,   198,    -1,    -1,   201,    -1,    -1,
     204,   205,   206,   207,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,
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
      -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,   187,    -1,
      -1,    -1,    -1,   192,   193,   194,    -1,    -1,   197,   198,
       3,     4,     5,     6,     7,   204,   205,   206,   207,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,   187,    -1,    10,    11,    12,   192,
     193,   194,    -1,    -1,   197,   198,     3,     4,     5,     6,
       7,   204,   205,   206,   207,    -1,    13,    31,    32,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    69,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      -1,    -1,   109,    -1,    -1,   112,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   121,   122,   123,   124,   125,   126,
      -1,    -1,   129,   130,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,    -1,
     147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,    -1,
      -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,    -1,    -1,
      -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
     187,    -1,    -1,    -1,    -1,   192,   193,   194,    -1,    -1,
     197,   198,     3,     4,     5,     6,     7,   204,   205,   206,
     207,    -1,    13,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    50,
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
     181,    -1,    -1,    -1,    -1,    -1,   187,    -1,    10,    11,
      12,   192,   193,   194,    -1,    -1,   197,   198,     3,     4,
       5,     6,     7,   204,   205,   206,   207,    -1,    13,    31,
      -1,    -1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    69,    -1,    -1,
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
      -1,    -1,   197,   198,    -1,   200,    10,    11,    12,   204,
     205,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    69,    56,    -1,    58,    59,
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
      -1,   181,    -1,    -1,    -1,    -1,    -1,   187,   202,    -1,
      -1,    -1,   192,   193,   194,    -1,    -1,   197,   198,    -1,
     200,    -1,    -1,    -1,   204,   205,   206,   207,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,   187,    -1,    -1,    -1,    -1,   192,   193,   194,
      11,    12,   197,   198,     3,     4,     5,     6,     7,   204,
     205,   206,   207,    -1,    13,    -1,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    69,    58,
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
     199,    -1,    -1,    -1,    -1,   204,   205,   206,   207,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,
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
     194,    -1,    -1,   197,   198,     3,     4,     5,     6,     7,
     204,   205,   206,   207,    -1,    13,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
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
      -1,    13,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    50,    51,
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
      -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,
     192,   193,   194,    -1,    -1,   197,   198,     3,     4,     5,
       6,     7,   204,   205,   206,   207,    -1,    13,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
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
      -1,    -1,   178,    -1,    -1,   181,    -1,    -1,    -1,    -1,
      -1,   187,    -1,    -1,    -1,    -1,   192,   193,   194,    -1,
      -1,   197,   198,     3,     4,     5,     6,     7,   204,   205,
     206,   207,    -1,    13,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    -1,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
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
      -1,    -1,   192,   193,   194,    -1,    -1,   197,   198,     3,
       4,     5,     6,     7,   204,   205,   206,   207,    -1,    13,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
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
     194,    -1,    -1,   197,   198,     3,     4,     5,     6,     7,
     204,   205,   206,   207,    -1,    13,    -1,    -1,    -1,    -1,
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
      -1,    -1,    11,    12,   192,   193,   194,    -1,    -1,   197,
     198,     3,     4,     5,     6,     7,   204,   205,   206,   207,
      -1,    13,    31,    -1,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      69,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
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
      -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,
     192,   193,   194,    -1,    12,   197,   198,     3,     4,     5,
       6,     7,   204,   205,   206,   207,    -1,    13,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    69,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,   121,   122,   123,   124,   125,
     126,    -1,    -1,   129,   130,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,   145,
      -1,   147,   148,   149,   150,   151,    -1,    -1,    -1,    -1,
      -1,    -1,   158,    -1,    -1,    -1,    -1,   163,   164,   165,
     166,   167,    81,   169,   170,    -1,   172,   173,   174,    -1,
      -1,    -1,   178,    92,    -1,   181,    -1,    -1,    -1,    -1,
      -1,   187,    -1,    -1,    -1,   104,   192,   193,   194,    -1,
      -1,   197,   198,    -1,    -1,    -1,    -1,    -1,   204,   205,
     206,   207,     3,     4,    -1,     6,     7,    -1,    -1,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,    28,    29,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   163,    -1,    -1,   166,    -1,    -1,
     169,   170,    -1,   172,   173,   174,    57,   176,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,   197,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,
      -1,    -1,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,    -1,   130,
      -1,    81,   133,   134,   135,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   163,    -1,    -1,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,   175,    -1,   177,    -1,    -1,   180,
       3,     4,    -1,     6,     7,    -1,   187,    10,    11,    12,
      13,   141,   142,   143,   144,   145,   197,   198,    -1,    -1,
      -1,   202,    -1,    -1,    -1,    28,    29,    -1,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    -1,   197,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,    -1,   130,    -1,    81,
     133,   134,   135,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     163,    -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,   175,    -1,   177,    -1,    -1,   180,     3,     4,
      -1,     6,     7,    -1,   187,    10,    11,    12,    13,   141,
     142,   143,   144,   145,   197,   198,    -1,    -1,    -1,   202,
      -1,    -1,    -1,    28,    29,    -1,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,
     172,   173,   174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    -1,   197,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,    -1,   130,    -1,   132,   133,   134,
     135,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,
      -1,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
     175,    -1,   177,    -1,    -1,   180,     3,     4,    -1,     6,
       7,    -1,   187,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,   197,   198,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    29,    -1,    31,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      57,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    69,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      87,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,    -1,   130,    -1,    -1,   133,   134,   135,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,   145,   146,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,    -1,
     177,    -1,    -1,   180,    -1,     3,     4,    -1,     6,     7,
     187,   188,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
     197,   198,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    29,    -1,    31,    -1,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    57,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    69,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,    -1,   130,    -1,    -1,   133,   134,   135,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,   166,   167,
      -1,   169,   170,    -1,   172,   173,   174,   175,    -1,   177,
      -1,    -1,   180,     3,     4,     5,     6,     7,    -1,   187,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   197,
     198,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
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
      -1,    -1,    -1,   163,   164,   165,    -1,    -1,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,   177,   178,    -1,
     180,    -1,    -1,    -1,    -1,    -1,    -1,   187,   188,    -1,
     190,    -1,   192,   193,    -1,     3,     4,   197,     6,     7,
      12,    -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      28,    29,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    87,
      -1,    -1,    -1,    -1,    -1,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,    -1,   130,    -1,   132,   133,   134,   135,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,    -1,     6,     7,    -1,   163,    10,    11,    12,    13,
      -1,   169,   170,    -1,   172,   173,   174,   175,    -1,   177,
      -1,    -1,   180,    -1,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   197,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    -1,   130,    -1,   132,   133,
     134,   135,    -1,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,   146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,   163,
      10,    11,    12,    13,    -1,   169,   170,    -1,   172,   173,
     174,   175,    -1,   177,    -1,    -1,   180,    -1,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   197,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,
      -1,    -1,    -1,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,    -1,
     130,    -1,    -1,   133,   134,   135,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,   146,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,
     170,    -1,   172,   173,   174,   175,    -1,   177,    -1,    -1,
     180,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   197,    -1,    -1,
      -1,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
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
      55,   200,    57,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,   200,
      57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,   200,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,   200,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,   200,    57,    -1,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    31,    81,
      -1,    -1,    -1,    -1,    -1,    87,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   200,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,   145,   146,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,   199,   112,
      -1,   163,    -1,    68,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,    -1,   176,    31,    81,    -1,    -1,    -1,
      -1,    -1,    87,    -1,    -1,   187,    -1,   140,   141,   142,
     143,   144,   145,   146,    -1,   197,   198,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,   199,    -1,    -1,    -1,
     163,    -1,    68,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,    -1,    31,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    87,    -1,    -1,   187,   140,   141,   142,   143,   144,
     145,   146,    -1,    -1,   197,   198,    -1,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,
      68,   166,   167,    -1,   169,   170,    -1,   172,   173,   174,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   187,    -1,   140,   141,   142,   143,   144,   145,
     146,    -1,   197,   198,    -1,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,
     166,   167,    -1,   169,   170,    -1,   172,   173,   174,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,
      -1,   187,   140,   141,   142,   143,   144,   145,   146,    -1,
      -1,   197,   198,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,   166,   167,
      70,   169,   170,    -1,   172,   173,   174,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,   187,
      -1,    -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,   197,
     198,    -1,    -1,    -1,   104,    -1,    -1,    -1,    32,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   121,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    70,   147,   148,   149,
     150,   151,    -1,    -1,    78,    79,    80,    81,   158,    -1,
      -1,    -1,    -1,   163,   164,   165,   166,   167,    92,   169,
     170,    -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   192,    -1,    -1,    -1,    -1,   197,   198,    -1,
      -1,    -1,    50,    51,   204,    -1,    -1,    -1,    56,    -1,
      58,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,
     144,   145,    70,   147,   148,   149,   150,   151,    -1,    -1,
      78,    79,    80,    81,   158,    -1,    -1,    -1,    -1,   163,
     164,   165,   166,   167,    92,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,   178,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,   192,    -1,
      -1,    -1,    -1,   197,   198,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   140,   141,   142,   143,   144,   145,    70,   147,
     148,   149,   150,   151,    -1,    -1,    78,    79,    80,    81,
     158,    -1,    -1,    -1,    -1,   163,   164,   165,   166,   167,
      92,   169,   170,    -1,   172,   173,   174,    -1,    -1,    -1,
     178,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,   187,
      -1,    -1,    -1,    -1,   192,    -1,    -1,    -1,    -1,   197,
     198,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,   140,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,    -1,    -1,   166,   167,    70,   169,   170,    -1,
     172,   173,   174,    -1,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,    -1,    -1,   187,    -1,    -1,    92,    -1,
      -1,    -1,    -1,    -1,    -1,   197,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,   140,   141,   142,   143,
     144,   145,    -1,   147,   148,   149,   150,   151,    69,    -1,
      -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,   163,
     164,   165,   166,   167,    -1,   169,   170,    70,   172,   173,
     174,    -1,    -1,    -1,   178,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,   192,    92,
      -1,    -1,    -1,   197,   198,    -1,    -1,   201,    -1,    -1,
     204,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    -1,   172,
     173,   174,    -1,    -1,    -1,   178,    -1,    70,    71,    -1,
      -1,    -1,    -1,    -1,    -1,    78,    79,    80,    81,   192,
      83,    84,    -1,    -1,   197,   198,    -1,    -1,    -1,    92,
      -1,   204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    -1,
      -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,
     163,   164,   165,   166,   167,    -1,   169,   170,    70,   172,
     173,   174,    -1,    -1,    -1,   178,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,   192,
      92,    -1,    -1,    -1,   197,   198,    -1,    -1,    -1,    -1,
      -1,   204,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,   147,   148,   149,   150,   151,
      -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,
      -1,   163,   164,   165,   166,   167,    -1,   169,   170,    70,
     172,   173,   174,    -1,    -1,    -1,   178,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
     192,    92,    -1,    -1,    -1,   197,   198,    -1,    -1,    -1,
      -1,    -1,   204,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,
     141,   142,   143,   144,   145,    -1,   147,   148,   149,   150,
     151,    -1,    -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,
      -1,    -1,   163,   164,   165,   166,   167,    -1,   169,   170,
      -1,   172,   173,   174,    -1,    -1,    -1,   178,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   192,    -1,    -1,    -1,    -1,   197,   198,    -1,    -1,
      -1,    30,    31,   204,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
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
      -1,    -1,    -1,    -1,    -1,   138,    -1,    31,    -1,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   138,    31,    -1,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,   138,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,   138,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
     138,    -1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
     138,    -1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    69,    92,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,    -1,   147,   148,   149,   150,   151,    -1,    -1,
      -1,    -1,    -1,    -1,   158,    -1,   138,    -1,    -1,   163,
     164,   165,   166,   167,    -1,   169,   170,    -1,   172,   173,
     174,    -1,    -1,    -1,   178,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,   192,    92,
      -1,    -1,    -1,   197,   198,    -1,    -1,    -1,    -1,    -1,
     204,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,   141,   142,
     143,   144,   145,    -1,   147,   148,   149,   150,   151,    69,
      -1,    -1,    -1,    -1,    -1,   158,    -1,    -1,    -1,    -1,
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
     172,   173,   174,    -1,    -1,    -1,   178,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     192,    -1,    -1,    -1,    -1,   197,   198,    27,    -1,    -1,
      30,    31,   204,    33,    34,    35,    36,    37,    38,    39,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69
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
     478,   491,   493,   495,   121,   122,   123,   139,   163,   173,
     198,   215,   256,   338,   360,   466,   360,   198,   360,   360,
     360,   360,   109,   360,   360,   452,   453,   360,   360,   360,
     360,    81,    83,    92,   121,   141,   142,   143,   144,   145,
     158,   198,   226,   380,   421,   424,   429,   466,   470,   466,
     360,   360,   360,   360,   360,   360,   360,   360,    38,   360,
     482,   483,   121,   132,   198,   226,   269,   421,   422,   423,
     425,   429,   463,   464,   465,   474,   479,   480,   360,   198,
     359,   426,   198,   359,   371,   349,   360,   237,   359,   198,
     198,   198,   359,   200,   360,   215,   200,   360,     3,     4,
       6,     7,    10,    11,    12,    13,    28,    29,    31,    57,
      68,    71,    72,    73,    74,    75,    76,    77,    87,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   130,   132,   133,   134,   135,   139,
     140,   146,   163,   167,   175,   177,   180,   187,   198,   215,
     216,   217,   228,   496,   517,   518,   521,    27,   200,   354,
     356,   360,   201,   249,   360,   112,   113,   163,   166,   188,
     218,   219,   220,   221,   225,    83,   204,   304,   305,    83,
     306,   123,   132,   122,   132,   198,   198,   198,   198,   215,
     275,   499,   198,   198,    70,    70,    70,    70,    70,   349,
      83,    91,   159,   160,   161,   488,   489,   166,   201,   225,
     225,   215,   276,   499,   167,   198,   499,   499,    83,   194,
     201,   372,    28,   348,   351,   360,   362,   466,   471,   232,
     201,    91,   427,   488,    91,   488,   488,    32,   166,   183,
     500,   198,     9,   200,   198,   347,   361,   467,   470,   118,
      38,   255,   167,   274,   499,   121,   193,   256,   339,    70,
     201,   461,   200,   200,   200,   200,   200,   200,   200,   200,
      10,    11,    12,    30,    31,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    57,    69,
     200,    70,    70,   201,   162,   133,   173,   175,   188,   190,
     277,   337,   338,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    59,    60,   136,   137,
     456,   461,   461,   198,   198,    70,   201,   204,   475,   198,
     255,   256,    14,   360,   200,   138,    49,   215,   451,    91,
     348,   362,   162,   466,   138,   203,     9,   436,   270,   348,
     362,   466,   500,   162,   198,   428,   456,   461,   199,   360,
      32,   235,     8,   373,     9,   200,   235,   236,   349,   350,
     360,   215,   289,   239,   200,   200,   200,   140,   146,   521,
     521,   183,   520,   198,   112,   521,    14,   162,   140,   146,
     163,   215,   217,   200,   200,   200,   250,   116,   180,   200,
     218,   220,   218,   220,   218,   220,   225,   218,   220,   201,
       9,   437,   200,   103,   166,   201,   466,     9,   200,    14,
       9,   200,   132,   132,   466,   492,   349,   348,   362,   466,
     470,   471,   199,   183,   267,   139,   466,   481,   482,   360,
     381,   382,   349,   402,   402,   381,   402,   200,    70,   456,
     159,   489,    82,   360,   466,    91,   159,   489,   225,   214,
     200,   201,   262,   272,   411,   413,    92,   198,   204,   374,
     375,   377,   420,   424,   473,   475,   493,    14,   103,   494,
     368,   369,   370,   299,   300,   454,   455,   199,   199,   199,
     199,   199,   202,   234,   235,   257,   264,   271,   454,   360,
     205,   206,   207,   215,   501,   502,   521,    38,    87,   176,
     302,   303,   360,   496,   246,   247,   348,   356,   357,   360,
     362,   466,   201,   248,   248,   248,   248,   198,   499,   265,
     255,   360,   477,   360,   360,   360,   360,   360,    32,   360,
     360,   360,   360,   360,   360,   360,   360,   360,   360,   360,
     360,   360,   360,   360,   360,   360,   360,   360,   360,   360,
     360,   360,   360,   425,   360,   477,   477,   360,   484,   485,
     132,   201,   216,   217,   474,   475,   275,   215,   276,   499,
     499,   274,   256,    38,   351,   354,   356,   360,   360,   360,
     360,   360,   360,   360,   360,   360,   360,   360,   360,   360,
     167,   201,   215,   457,   458,   459,   460,   474,   302,   302,
     477,   360,   481,   255,   199,   360,   198,   450,     9,   436,
     199,   199,    38,   360,    38,   360,   428,   199,   199,   199,
     474,   302,   201,   215,   457,   458,   474,   199,   232,   293,
     201,   356,   360,   360,    95,    32,   235,   287,   200,    27,
     103,    14,     9,   199,    32,   201,   290,   521,    31,    92,
     176,   228,   514,   515,   516,   198,     9,    50,    51,    56,
      58,    70,   140,   141,   142,   143,   144,   145,   187,   198,
     226,   388,   391,   394,   397,   400,   406,   421,   429,   430,
     432,   433,   215,   519,   232,   198,   243,   201,   200,   201,
     200,   201,   200,   103,   166,   201,   200,   112,   113,   166,
     221,   222,   223,   224,   225,   221,   215,   360,   305,   430,
      83,     9,   199,   199,   199,   199,   199,   199,   199,   200,
      50,    51,   510,   512,   513,   134,   280,   198,     9,   199,
     199,   138,   203,     9,   436,     9,   436,   203,   203,   203,
     203,    83,    85,   215,   490,   215,    70,   202,   202,   211,
     213,    32,   135,   279,   182,    54,   167,   182,   415,   362,
     138,     9,   436,   199,   162,   521,   521,    14,   373,   299,
     230,   195,     9,   437,    87,   521,   522,   456,   456,   202,
       9,   436,   184,   466,    83,    84,   301,   360,   199,     9,
     437,    14,     9,   199,     9,   199,   199,   199,   199,    14,
     199,   202,   233,   234,   365,   258,   134,   278,   198,   499,
     203,   202,   360,    32,   203,   203,   138,   202,     9,   436,
     360,   500,   198,   268,   263,   273,    14,   494,   266,   255,
      71,   466,   360,   500,   199,   199,   203,   202,   199,    50,
      51,    70,    78,    79,    80,    92,   140,   141,   142,   143,
     144,   145,   158,   187,   215,   389,   392,   395,   398,   401,
     421,   432,   439,   441,   442,   446,   449,   215,   466,   466,
     138,   278,   456,   461,   456,   199,   360,   294,    75,    76,
     295,   230,   359,   232,   350,   103,    38,   139,   284,   466,
     430,   215,    32,   235,   288,   200,   291,   200,   291,     9,
     436,    92,   228,   138,   162,     9,   436,   199,    87,   503,
     504,   521,   522,   501,   430,   430,   430,   430,   430,   435,
     438,   198,    70,    70,    70,    70,    70,   198,   430,   162,
     201,    10,    11,    12,    31,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    69,   162,   500,
     202,   421,   201,   252,   220,   220,   220,   215,   220,   221,
     221,   225,     9,   437,   202,   202,    14,   466,   200,   184,
       9,   436,   215,   281,   421,   201,   481,   139,   466,    14,
     360,   360,   203,   360,   202,   211,   521,   281,   201,   414,
      14,   199,   360,   374,   474,   200,   521,   195,   202,   231,
     234,   244,    32,   508,   455,   522,    38,    83,   176,   457,
     458,   460,   457,   458,   460,   521,    70,    38,    87,   176,
     360,   430,   247,   356,   357,   466,   248,   247,   248,   248,
     202,   234,   299,   198,   421,   279,   366,   259,   360,   360,
     360,   202,   198,   302,   280,    32,   279,   521,    14,   278,
     499,   425,   202,   198,    14,    78,    79,    80,   215,   440,
     440,   442,   444,   445,    52,   198,    70,    70,    70,    70,
      70,    91,   159,   198,   162,     9,   436,   199,   450,    38,
     360,   279,   202,    75,    76,   296,   359,   235,   202,   200,
      96,   200,   284,   466,   198,   138,   283,    14,   232,   291,
     106,   107,   108,   291,   202,   521,   184,   138,   162,   521,
     215,   176,   514,   521,     9,   436,   199,   176,   436,   138,
     203,     9,   436,   435,   383,   384,   430,   403,   430,   431,
     403,   383,   403,   374,   376,   378,   199,   132,   216,   430,
     486,   487,   430,   430,   430,    32,   430,   430,   430,   430,
     430,   430,   430,   430,   430,   430,   430,   430,   430,   430,
     430,   430,   430,   430,   430,   430,   430,   430,   430,   430,
     519,    83,   253,   202,   202,   202,   202,   224,   200,   430,
     513,   103,   104,   509,   511,     9,   310,   199,   198,   351,
     356,   360,   138,   203,   202,   494,   310,   168,   181,   201,
     410,   417,   168,   201,   416,   138,   200,   508,   198,   247,
     347,   361,   467,   470,   521,   373,    87,   522,    83,    83,
     176,    14,    83,   500,   500,   477,   466,   301,   360,   199,
     299,   201,   299,   198,   138,   198,   302,   199,   201,   521,
     201,   200,   521,   279,   260,   428,   302,   138,   203,     9,
     436,   441,   444,   385,   386,   442,   404,   442,   443,   404,
     385,   404,   159,   374,   447,   448,    81,   442,   466,   201,
     359,    32,    77,   235,   200,   350,   283,   481,   284,   199,
     430,   102,   106,   200,   360,    32,   200,   292,   202,   184,
     521,   215,   138,    87,   521,   522,    32,   199,   430,   430,
     199,   203,     9,   436,   138,   203,     9,   436,   203,   203,
     203,   138,     9,   436,   199,   138,   202,     9,   436,   430,
      32,   199,   232,   200,   200,   200,   200,   215,   521,   521,
     509,   421,     6,   113,   119,   122,   127,   169,   170,   172,
     202,   311,   336,   337,   338,   343,   344,   345,   346,   454,
     481,   360,   202,   201,   202,    54,   360,   360,   360,   373,
     466,   200,   201,   522,    38,    83,   176,    14,    83,   360,
     198,   198,   203,   508,   199,   310,   199,   299,   360,   302,
     199,   310,   494,   310,   200,   201,   198,   199,   442,   442,
     199,   203,     9,   436,   138,   203,     9,   436,   203,   203,
     203,   138,   199,     9,   436,   310,    32,   232,   200,   199,
     199,   199,   240,   200,   200,   292,   232,   138,   521,   521,
     176,   521,   138,   430,   430,   430,   430,   374,   430,   430,
     430,   201,   202,   511,   134,   135,   188,   216,   497,   521,
     282,   421,   113,   346,    31,   127,   140,   146,   167,   173,
     320,   321,   322,   323,   421,   171,   328,   329,   130,   198,
     215,   330,   331,   312,   256,   521,     9,   200,     9,   200,
     200,   494,   337,   199,   307,   167,   412,   202,   202,   360,
      83,    83,   176,    14,    83,   360,   302,   302,   119,   363,
     508,   202,   508,   199,   199,   202,   201,   202,   310,   299,
     138,   442,   442,   442,   442,   374,   202,   232,   238,   241,
      32,   235,   286,   232,   521,   199,   430,   138,   138,   138,
     232,   421,   421,   499,    14,   216,     9,   200,   201,   497,
     494,   323,   183,   201,     9,   200,     3,     4,     5,     6,
       7,    10,    11,    12,    13,    27,    28,    29,    57,    71,
      72,    73,    74,    75,    76,    77,    87,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   139,
     140,   147,   148,   149,   150,   151,   163,   164,   165,   175,
     177,   178,   180,   187,   188,   190,   192,   193,   215,   418,
     419,     9,   200,   167,   171,   215,   331,   332,   333,   200,
      83,   342,   255,   313,   497,   497,    14,   256,   202,   308,
     309,   497,    14,    83,   360,   199,   199,   198,   508,   197,
     505,   363,   508,   307,   202,   199,   442,   138,   138,    32,
     235,   285,   286,   232,   430,   430,   430,   202,   200,   200,
     430,   421,   316,   521,   324,   325,   429,   321,    14,    32,
      51,   326,   329,     9,    36,   199,    31,    50,    53,    14,
       9,   200,   217,   498,   342,    14,   521,   255,   200,    14,
     360,    38,    83,   409,   201,   506,   507,   521,   200,   201,
     334,   508,   505,   202,   508,   442,   442,   232,   100,   251,
     202,   215,   228,   317,   318,   319,     9,   436,     9,   436,
     202,   430,   419,   419,    68,   327,   332,   332,    31,    50,
      53,   430,    83,   183,   198,   200,   430,   498,   430,    83,
       9,   437,   230,     9,   437,    14,   509,   230,   201,   334,
     334,    98,   200,   116,   242,   162,   103,   521,   184,   429,
     174,    14,   510,   314,   198,    38,    83,   199,   202,   507,
     521,   202,   230,   200,   198,   180,   254,   215,   337,   338,
     184,   430,   184,   297,   298,   455,   315,    83,   202,   421,
     252,   177,   215,   200,   199,     9,   437,    87,   124,   125,
     126,   340,   341,   297,    83,   282,   200,   508,   455,   522,
     522,   199,   199,   200,   505,    87,   340,    83,    38,    83,
     176,   508,   201,   200,   201,   335,   522,   522,    83,   176,
      14,    83,   505,   232,   230,    83,    38,    83,   176,    14,
      83,   360,   335,   202,   202,    83,   176,    14,    83,   360,
      14,    83,   360,   360
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
     468,   469,   469,   469,   469,   469,   469,   469,   469,   469,
     470,   471,   471,   472,   472,   472,   473,   473,   473,   474,
     474,   475,   475,   475,   476,   476,   476,   477,   477,   478,
     478,   479,   479,   479,   479,   479,   479,   480,   480,   480,
     480,   480,   481,   481,   481,   481,   481,   481,   482,   482,
     483,   483,   483,   483,   483,   483,   483,   483,   484,   484,
     485,   485,   485,   485,   486,   486,   487,   487,   487,   487,
     488,   488,   488,   488,   489,   489,   489,   489,   489,   489,
     490,   490,   490,   491,   491,   491,   491,   491,   491,   491,
     491,   491,   491,   491,   492,   492,   493,   493,   494,   494,
     495,   495,   495,   495,   496,   496,   497,   497,   498,   498,
     499,   499,   500,   500,   501,   501,   502,   503,   503,   503,
     503,   503,   503,   504,   504,   504,   504,   505,   505,   506,
     506,   507,   507,   508,   508,   509,   509,   510,   511,   511,
     512,   512,   512,   512,   513,   513,   513,   514,   514,   514,
     514,   515,   515,   516,   516,   516,   516,   517,   518,   519,
     519,   520,   520,   521,   521,   521,   521,   521,   521,   521,
     521,   521,   521,   521,   522,   522
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
       4,     1,     1,     1,     1,     1,     3,     3,     4,     4,
       3,     1,     1,     7,     9,     9,     7,     6,     8,     1,
       2,     4,     4,     1,     1,     1,     4,     1,     0,     1,
       2,     1,     1,     1,     3,     3,     3,     0,     1,     1,
       3,     3,     2,     3,     6,     0,     1,     4,     2,     0,
       5,     3,     3,     1,     6,     4,     4,     2,     2,     0,
       5,     3,     3,     1,     2,     0,     5,     3,     3,     1,
       2,     2,     1,     2,     1,     4,     3,     3,     6,     3,
       1,     1,     1,     4,     4,     4,     4,     4,     4,     2,
       2,     4,     2,     2,     1,     3,     3,     3,     0,     2,
       5,     6,     6,     7,     1,     2,     1,     2,     1,     4,
       1,     4,     3,     0,     1,     3,     2,     1,     2,     4,
       3,     3,     1,     4,     2,     2,     0,     0,     3,     1,
       3,     3,     2,     0,     2,     2,     2,     2,     1,     2,
       4,     2,     5,     3,     1,     1,     0,     3,     4,     5,
       6,     3,     1,     3,     2,     1,     0,     4,     1,     3,
       2,     4,     5,     2,     2,     1,     1,     1,     1,     3,
       2,     1,     8,     6,     1,     0
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
#line 7269 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 760 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 7277 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 767 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 7283 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 768 "hphp.y" /* yacc.c:1646  */
    { }
#line 7289 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 771 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 7295 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 772 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7301 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 773 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7307 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 774 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7313 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 775 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7319 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 776 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 7325 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 7333 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 780 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 7340 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 782 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 7346 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 783 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7352 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 784 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 7358 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 785 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7364 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 786 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7372 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 790 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7381 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 795 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7390 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 800 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7397 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 803 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7404 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 806 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7412 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 810 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7420 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 814 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7428 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 818 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7436 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 822 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7444 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 825 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7451 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 830 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7457 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 831 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7463 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7469 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7475 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 834 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7481 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7487 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7493 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7499 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 838 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7505 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 839 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7511 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 840 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7517 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 841 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7523 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 842 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7529 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 923 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7535 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 925 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7541 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 930 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7547 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 931 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7554 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 937 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7560 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 941 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7566 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 942 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7572 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 944 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7578 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 946 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7584 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 951 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7590 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 952 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7597 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 958 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7603 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 962 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7610 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 964 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7617 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 966 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7624 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 971 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7630 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 973 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7636 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 976 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7642 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 978 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7648 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 979 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7654 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 984 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7663 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 991 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7672 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 999 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7679 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 1002 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7686 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1008 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 7692 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 1009 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval)); }
#line 7698 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 1014 "hphp.y" /* yacc.c:1646  */
    {
                                         _p->onUsing((yyval), (yyvsp[-2]), true, (yyvsp[-1]), nullptr);
                                       }
#line 7706 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1021 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7712 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1022 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7718 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1027 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 7724 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1028 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval), (yyval), (yyvsp[0])); }
#line 7731 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1033 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7737 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1034 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7743 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1035 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7749 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1036 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7755 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1039 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7761 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1043 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7767 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1048 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7773 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1049 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7780 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1051 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7788 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1055 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7795 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1058 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7803 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1062 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7810 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1064 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7818 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1067 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7825 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1069 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7833 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1072 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7839 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1073 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7845 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1074 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7851 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1075 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7857 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1076 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7863 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1077 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7869 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1078 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7875 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1079 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7881 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1080 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7887 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1081 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7893 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1082 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7899 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1083 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 7905 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1084 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 7911 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1085 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 7917 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1086 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 7924 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1090 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7931 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1092 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 7939 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1097 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7946 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1099 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 7954 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1103 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 7962 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1112 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7968 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1113 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 7974 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1116 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false); }
#line 7980 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1117 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 7986 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1119 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7994 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1123 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8002 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1127 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8010 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1131 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8018 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1135 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8026 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1139 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8034 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1144 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 8042 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1147 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 8048 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1148 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 8057 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1152 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8063 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1153 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8069 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1154 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8075 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1155 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8081 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1156 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8087 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1157 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8093 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1158 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 8099 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1159 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8105 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1160 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8111 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1161 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 8117 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1162 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8123 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1163 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 8133 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1185 "hphp.y" /* yacc.c:1646  */
    { _p->pushLabelScope(LS::Using);
                                         _p->onNewLabelScope(false);
                                         (yyval) = (yyvsp[-1]); }
#line 8141 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1191 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1; }
#line 8147 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1192 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8153 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1201 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), nullptr, (yyvsp[-2]));
                                         _p->onExprListElem((yyval), &(yyval), (yyvsp[0])); }
#line 8160 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 1203 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0])); }
#line 8166 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 1213 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8172 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1214 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8178 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1218 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false); }
#line 8184 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1219 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8190 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1228 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 8196 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1229 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8202 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1233 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::Finally);}
#line 8209 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1235 "hphp.y" /* yacc.c:1646  */
    { _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->popLabelScope();
                                         _p->onCompleteLabelScope(false);}
#line 8217 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1241 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8223 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1242 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8229 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1246 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 8235 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1247 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8241 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1251 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 8247 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1257 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8256 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1264 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8266 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1272 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8275 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1279 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8285 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1287 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8294 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1293 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8304 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1302 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8311 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1306 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 8317 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1310 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8324 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1314 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 8330 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1320 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8337 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 8355 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 1338 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8362 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 8380 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 1355 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8387 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1358 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8395 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1363 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8402 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1366 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8410 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1372 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 8416 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1375 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 8422 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1379 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8429 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1382 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8440 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1390 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8447 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1393 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8458 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1401 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8464 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1402 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 8471 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1406 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8477 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1409 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8483 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1412 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 8489 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1413 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 8495 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1414 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 8503 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1417 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 8509 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1418 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 8515 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1422 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8521 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1423 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8527 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1426 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8533 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1427 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8539 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1430 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8545 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1431 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8551 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1434 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 8557 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1436 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8563 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1439 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 8569 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1441 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8575 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1445 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8581 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1446 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8587 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1449 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8593 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1450 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8599 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1451 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8605 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1455 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8611 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1457 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8617 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1460 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8623 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1462 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8629 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1465 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8635 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1467 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8641 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1470 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8647 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1472 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8653 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1476 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8659 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1478 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8666 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8672 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1484 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8678 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1485 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8684 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1486 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8690 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1491 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8696 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1493 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8702 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1494 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8708 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1497 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8714 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1498 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8720 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1503 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8726 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1504 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8732 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1509 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8738 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1510 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8744 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1513 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8750 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1514 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8756 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1517 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8762 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1518 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8768 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1526 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8775 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1532 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8782 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1538 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8790 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1542 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8796 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1546 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8803 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1551 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8810 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1556 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8818 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1559 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8824 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1565 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8832 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1570 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8840 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1575 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8848 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1581 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8856 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1587 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8864 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1593 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8872 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1599 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL);}
#line 8880 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1605 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8888 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1612 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8896 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1619 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8904 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1628 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 8911 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1633 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 8918 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1638 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 8926 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1642 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8932 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1645 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 8939 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1649 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 8946 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1653 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 8954 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1656 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8960 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1661 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8968 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1665 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8976 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1669 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8984 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1674 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8992 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1679 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9000 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1684 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::In,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 9008 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1689 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),
                                                     ParamMode::InOut,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9016 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1694 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::Ref,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 9024 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1700 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),
                                                     ParamMode::Ref,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 9032 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1706 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),
                                                     ParamMode::In,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 9040 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1712 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 9046 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1713 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 9052 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1714 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 9058 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1719 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9064 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1720 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9070 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1723 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,false);}
#line 9077 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1725 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::InOut,false);}
#line 9084 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1727 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::Ref,false);}
#line 9091 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1729 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),
                                                       ParamMode::In,true);}
#line 9098 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1732 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),
                                                     ParamMode::In,false);}
#line 9105 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1735 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::In,true);}
#line 9112 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1738 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::Ref,false);}
#line 9119 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1741 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),
                                                     ParamMode::InOut,false);}
#line 9126 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1746 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9132 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1747 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 9138 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1750 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9144 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1751 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 9150 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1752 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 9156 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1756 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9162 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1758 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9168 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1759 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 9174 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1760 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9180 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1765 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9186 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1766 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9192 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1769 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9199 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 359:
#line 1774 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9205 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1780 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9211 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1781 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9217 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1784 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[0]));}
#line 9223 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1785 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 9230 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1788 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[-1]));}
#line 9236 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1789 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-3]));}
#line 9243 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1791 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9250 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1794 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL, true);}
#line 9257 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1796 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9263 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1799 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9271 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1806 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9281 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1814 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9289 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1821 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9299 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1827 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 9305 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1829 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9311 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1831 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9317 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1833 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 9323 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1835 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 9329 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1836 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 9336 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1839 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 9342 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1842 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9348 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1843 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9354 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1844 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 9360 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 1850 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 9366 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1855 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 9373 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1858 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 9381 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1865 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 9387 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1866 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 9394 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1871 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 9401 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1874 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 9407 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1881 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 9414 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1883 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 9420 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 1887 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9426 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 394:
#line 1892 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9432 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1894 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9438 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1896 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9444 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1897 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 9455 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1903 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 9461 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1905 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 9467 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1906 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 9473 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1910 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,0,(yyvsp[0]),0);}
#line 9479 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1912 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),0,(yyvsp[0]),0);}
#line 9485 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1917 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9491 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1920 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9497 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1921 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 9503 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1925 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 9509 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1926 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 9515 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1930 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 9522 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1933 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 9529 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1938 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 9536 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1943 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 9542 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1944 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 9549 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1946 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 9555 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1950 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 9561 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1951 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 9567 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1952 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 9573 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1953 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 9579 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1957 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9585 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1958 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 9591 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1959 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 9597 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1960 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 9603 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1961 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 9609 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 9615 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1965 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 9621 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1969 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 9629 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1972 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 9635 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1973 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 9641 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1977 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9647 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1978 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9653 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1982 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9659 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1983 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9665 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1986 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9671 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1987 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9677 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 1990 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9683 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 1991 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9689 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 1994 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9695 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 1996 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9701 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 1999 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9707 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9713 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 2001 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9719 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 2002 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9725 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 2003 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9731 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 2004 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9737 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 2005 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9743 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 2009 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9749 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 2010 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9755 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 2013 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9761 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 2014 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9767 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 2015 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9773 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 2019 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9779 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 2021 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9785 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 2022 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9791 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 2023 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9797 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 2027 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9803 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 2029 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9809 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 2033 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9815 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 2035 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9821 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 2039 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9829 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 2043 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9836 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 2047 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9842 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 2051 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9848 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 2055 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9854 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 2057 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9860 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 2058 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9866 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 2059 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9872 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 2060 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9878 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 2061 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9884 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 2064 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9890 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 2065 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9896 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 2069 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9902 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 2070 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9908 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 2074 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 9914 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 2075 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 9920 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 2076 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 9926 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 2077 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9932 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 2081 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9938 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 2086 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9944 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 2090 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 9950 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 2094 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9956 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 2098 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 9962 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 2102 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9968 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 2107 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9974 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 2111 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9980 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 2115 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9986 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 2116 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9992 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 2117 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9998 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 2118 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10004 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 2119 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10010 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 2123 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10016 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 2128 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 10022 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 2129 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 10028 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 2130 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 10034 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 2133 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 10040 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 2134 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 10046 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 2135 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 10052 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 2136 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 10058 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 2137 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 10064 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 2138 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 10070 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 2139 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 10076 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 2140 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 10082 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 2141 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 10088 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 2142 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 10094 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2143 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 10100 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2144 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 10106 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2145 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 10112 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2146 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 10118 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2147 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 10124 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2148 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 10130 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2149 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 10136 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2150 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 10142 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2151 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 10148 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2152 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 10154 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2153 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 10160 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2154 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 10166 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2155 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 10172 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2156 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 10178 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2157 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 10184 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2158 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 10190 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2159 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 10196 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2160 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 10202 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2161 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 10208 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2162 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 10214 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2163 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 10220 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2164 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 10226 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2165 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 10232 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2166 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 10238 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2167 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 10244 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2168 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 10250 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2169 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 10256 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2170 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 10262 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2171 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 10268 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2172 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 10274 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2173 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 10280 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2174 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 10286 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2175 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 10292 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2176 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 10298 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2177 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 10305 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2179 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 10311 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2180 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 10318 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2182 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 10324 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2184 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 10330 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2185 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10336 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2186 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 10342 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2187 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 10348 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2188 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 10354 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2189 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10360 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2190 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 10366 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2191 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 10372 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2192 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 10378 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2193 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 10384 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2194 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 10390 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2195 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 10396 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2196 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 10402 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2197 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 10408 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2198 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 10414 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2199 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10420 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2200 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10426 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2201 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10432 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2202 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10438 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2203 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10444 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2204 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10450 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2205 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10456 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2206 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10462 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2207 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 10468 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2208 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 10474 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2209 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10480 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2216 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 10486 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2217 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10492 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2222 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10501 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 10513 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2237 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10522 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 10534 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 10548 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 10563 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2274 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10573 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 10588 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2293 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10598 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 10615 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 10629 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 10642 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2328 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10652 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 10665 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2347 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10671 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2348 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10677 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2350 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10683 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2354 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10690 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2356 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10696 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2363 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10702 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2366 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10708 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2373 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10714 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2376 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10720 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2381 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10726 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2382 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10732 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2387 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10738 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2388 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10744 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2392 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval), (yyvsp[-1]), T_ARRAY);}
#line 10750 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2396 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10756 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2397 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10762 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2402 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10768 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2403 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10774 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2408 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10780 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2409 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10786 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2414 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10792 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2415 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10798 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2421 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10804 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2423 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10810 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2428 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10816 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2429 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10822 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2435 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10828 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2437 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10834 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2441 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10840 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2445 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10846 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2449 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10852 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2453 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10858 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2457 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10864 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2461 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10870 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2465 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10876 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2469 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10882 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2473 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10888 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2477 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10894 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2481 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10900 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2485 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10906 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2489 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10912 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2493 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10918 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2497 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10924 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2502 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10930 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2503 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10936 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2508 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10942 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2509 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10948 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2514 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10954 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2515 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10960 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2520 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10968 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2527 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10976 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2534 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10982 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2536 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10988 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2540 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10994 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2541 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11000 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2542 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11006 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2543 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11012 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2544 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11018 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2545 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11024 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2546 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11030 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2547 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11036 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2548 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 11043 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2550 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11049 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2551 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11055 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2555 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11061 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2556 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 11067 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2557 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 11073 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2558 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 11079 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2565 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 11085 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 11103 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 11121 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2598 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 11127 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2599 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 11133 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2604 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11139 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2605 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11145 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2608 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-1]),0,(yyvsp[0]),0);}
#line 11151 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2609 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11157 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2612 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11164 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2616 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11172 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2619 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11178 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 11190 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2629 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11196 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2630 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11202 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2634 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11208 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2636 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 11214 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2638 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 11220 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2642 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11226 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2643 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11232 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2644 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11238 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2645 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11244 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2646 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11250 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2647 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11256 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2648 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11262 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2649 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11268 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2650 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11274 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2651 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11280 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2652 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11286 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2653 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11292 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2654 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11298 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2655 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11304 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2656 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11310 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2657 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11316 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2658 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11322 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2659 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11328 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2660 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11334 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2661 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11340 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2662 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11346 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2663 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11352 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2664 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11358 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2665 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11364 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2666 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11370 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2667 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11376 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2668 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11382 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2669 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11388 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2670 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11394 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2671 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11400 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2672 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11406 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2673 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11412 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2674 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11418 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2675 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11424 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2676 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11430 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2677 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11436 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2678 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11442 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2679 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11448 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2680 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11454 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2681 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11460 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2682 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11466 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2683 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11472 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2684 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11478 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2685 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11484 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2686 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11490 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2687 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11496 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2688 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11502 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2689 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11508 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2690 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11514 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2691 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11520 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2692 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11526 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2693 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11532 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2694 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11538 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2695 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11544 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2696 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11550 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2697 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11556 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2698 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11562 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2699 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11568 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2700 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11574 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2701 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11580 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2702 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11586 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2703 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11592 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2704 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11598 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2705 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11604 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2706 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11610 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2707 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11616 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2708 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11622 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2709 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11628 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2710 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11634 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2711 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11640 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2712 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11646 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2713 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11652 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2714 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11658 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2715 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11664 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2716 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11670 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2717 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11676 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2718 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11682 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2719 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11688 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2720 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11694 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2721 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11700 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2722 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11706 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2723 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11712 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2724 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11718 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2729 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11724 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2733 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11730 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2734 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11736 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2738 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11742 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2739 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11748 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2740 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11754 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2741 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11761 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2743 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11768 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2747 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11774 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2756 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11780 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2759 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11786 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2760 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11793 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2762 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11800 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2772 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11806 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2776 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11812 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2777 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11818 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2778 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11824 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2782 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11830 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2783 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11836 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2784 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11842 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2788 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11848 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2789 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11854 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2790 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11860 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2794 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11866 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2795 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11872 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2799 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11878 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2800 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11884 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2801 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11890 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2802 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 11897 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2804 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 11903 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2805 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 11909 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2806 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 11915 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2807 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 11921 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2808 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 11927 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2809 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 11933 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2810 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 11939 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2811 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 11945 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2812 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 11951 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2815 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11957 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2817 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11963 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2821 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11969 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2822 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11975 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2824 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11981 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2825 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11987 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2827 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11993 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2828 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11999 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2829 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12005 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2830 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12011 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2831 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12017 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12023 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12029 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2834 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12035 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12041 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2837 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 12047 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2839 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 12053 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2841 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 12059 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2843 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 12065 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2845 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 12071 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2846 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 12077 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2847 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 12083 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2848 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 12089 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2849 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 12095 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2850 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 12101 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2851 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 12107 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2852 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 12113 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2853 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 12119 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2854 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 12125 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2855 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 12131 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2856 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 12137 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2857 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 12143 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2858 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 12149 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2859 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 12155 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2860 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12161 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2861 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12167 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2863 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 12173 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2865 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 12179 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2867 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 12185 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2869 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 12191 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2870 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 12197 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2872 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 12204 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 832:
#line 2874 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 12210 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2877 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 12217 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2881 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 12223 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2884 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 12229 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2885 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 12235 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2889 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12241 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2890 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12247 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2896 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12253 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2902 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12259 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2903 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12265 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2907 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12271 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2908 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12277 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2909 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12283 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2910 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12289 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2911 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 12295 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2912 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 12301 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2914 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 12308 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2919 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12314 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2920 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12320 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2924 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12326 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2925 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12332 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2928 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 12338 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2929 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12344 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2935 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12350 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2937 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12356 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2939 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12362 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2940 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12368 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 2944 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 12374 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2945 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 12380 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2946 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 12386 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2949 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12392 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2951 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12398 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2954 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 12404 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2955 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 12410 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2956 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 12416 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2957 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12422 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2961 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 12429 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2964 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 12437 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2971 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12443 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2972 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12449 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2975 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 12457 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2978 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12463 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2979 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12469 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 2980 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12475 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 877:
#line 2982 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12481 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 2983 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12487 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 2985 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12493 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 2986 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12499 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 2987 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12505 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 2988 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12511 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 2989 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12517 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 2990 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12523 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 2995 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12529 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
#line 2996 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12535 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 3001 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12541 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 3002 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12547 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 3007 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12553 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 3009 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12559 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 3011 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12565 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 3012 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12571 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 3016 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12577 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 3017 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12583 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 3022 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12589 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 3023 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12595 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 3028 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12601 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 3031 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12607 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 3036 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12613 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 3037 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12619 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3040 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12625 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3041 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 12632 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3048 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 12638 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3050 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 12644 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3053 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 12650 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3055 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12656 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3058 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12662 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3061 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12668 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3062 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12674 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3066 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 12680 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3067 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 12686 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3071 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12692 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3072 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 12698 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3073 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 12704 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3077 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12710 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3079 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12716 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3087 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12722 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3088 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12728 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3092 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12734 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3094 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12740 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3102 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12746 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3103 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12752 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3108 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12758 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3109 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12764 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3111 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12770 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3116 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12776 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3118 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12782 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 12796 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 12810 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 12824 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 12838 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3174 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12844 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3175 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12850 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3176 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12856 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3177 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12862 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3178 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12868 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
#line 3179 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12874 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 12888 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3198 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12894 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
#line 3200 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12900 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
#line 3202 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12906 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3203 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12912 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3207 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12918 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
#line 3211 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12924 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3212 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12930 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
#line 3213 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12936 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
#line 3214 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12942 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 12956 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3231 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12962 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3233 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12968 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3242 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12974 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3243 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12980 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3244 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12986 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3245 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12992 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3246 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12998 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3247 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13004 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3248 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13010 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3250 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13016 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3252 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 13022 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3256 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13028 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3260 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13034 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3261 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13040 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3267 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13046 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3271 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13052 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3275 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 13058 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3282 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 13064 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3291 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 13070 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3295 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 13076 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3299 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13082 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3302 "hphp.y" /* yacc.c:1646  */
    { _p->onIndirectRef((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 13088 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3308 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13094 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3309 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13100 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3310 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13106 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3314 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13112 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3315 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 13118 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3316 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 13124 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3323 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13130 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3324 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13136 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3329 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 13142 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3330 "hphp.y" /* yacc.c:1646  */
    { (yyval)++;}
#line 13148 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3335 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13154 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3336 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13160 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3337 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13166 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
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
#line 13180 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3351 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13186 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3352 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13192 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3356 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13198 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3357 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13204 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
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
#line 13218 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3369 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13224 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3373 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 13230 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3374 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 13236 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3376 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 13242 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 995:
#line 3377 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 13248 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3378 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 13254 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3379 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 13260 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
#line 3384 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13266 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3385 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13272 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3389 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13278 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3390 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13284 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3391 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13290 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3392 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13296 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1004:
#line 3395 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13302 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1005:
#line 3397 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 13308 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1006:
#line 3398 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13314 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1007:
#line 3399 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 13320 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1008:
#line 3404 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13326 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3405 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13332 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3409 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13338 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3410 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13344 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3411 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13350 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3412 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13356 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3417 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13362 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3418 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13368 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3423 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13374 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3425 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13380 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3427 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13386 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3428 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13392 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3432 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 13398 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3434 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 13404 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3435 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 13410 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3437 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 13417 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3442 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13423 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3444 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13429 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
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
#line 13443 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1027:
#line 3456 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 13449 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1028:
#line 3458 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 13455 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1029:
#line 3459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13461 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1030:
#line 3462 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 13467 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1031:
#line 3463 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 13473 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1032:
#line 3464 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 13479 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1033:
#line 3468 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 13485 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1034:
#line 3469 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 13491 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1035:
#line 3470 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13497 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1036:
#line 3471 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13503 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1037:
#line 3472 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13509 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1038:
#line 3473 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13515 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1039:
#line 3474 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 13521 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1040:
#line 3475 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 13527 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1041:
#line 3476 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 13533 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1042:
#line 3477 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 13539 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3478 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 13545 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3482 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 13551 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3483 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 13557 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3488 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13563 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3490 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13569 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3504 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13577 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3509 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 13585 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3513 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13593 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3518 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 13601 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3524 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13607 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1055:
#line 3525 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13613 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1056:
#line 3529 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13619 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1057:
#line 3530 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13625 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1058:
#line 3536 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13631 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1059:
#line 3540 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 13637 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1060:
#line 3546 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13643 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1061:
#line 3550 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 13650 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1062:
#line 3557 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13656 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3558 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13662 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3562 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 13670 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3565 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 13677 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3571 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13683 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3575 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13691 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3578 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                                        _p->onTypeList((yyvsp[0]), t);
                                                        (yyval) = (yyvsp[0]); }
#line 13699 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1069:
#line 3581 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-3]); }
#line 13706 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3583 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13713 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3585 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                                        (yyval) = (yyvsp[-2]); }
#line 13720 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3587 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13726 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3592 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-3]); }
#line 13732 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3593 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13738 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3594 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13744 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1076:
#line 3595 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13750 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1083:
#line 3616 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13756 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1084:
#line 3617 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13762 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1087:
#line 3626 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13768 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1090:
#line 3637 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13774 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1091:
#line 3639 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13780 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1092:
#line 3643 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13786 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1093:
#line 3646 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13792 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1094:
#line 3650 "hphp.y" /* yacc.c:1646  */
    {}
#line 13798 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1095:
#line 3651 "hphp.y" /* yacc.c:1646  */
    {}
#line 13804 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1096:
#line 3652 "hphp.y" /* yacc.c:1646  */
    {}
#line 13810 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1097:
#line 3658 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13817 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1098:
#line 3663 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13827 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1099:
#line 3672 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13833 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1100:
#line 3678 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13842 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1101:
#line 3686 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 13848 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1102:
#line 3687 "hphp.y" /* yacc.c:1646  */
    { }
#line 13854 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1103:
#line 3693 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 13860 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1104:
#line 3695 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 13866 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1105:
#line 3696 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 13876 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1106:
#line 3701 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 13883 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1107:
#line 3707 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("array"); }
#line 13890 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1108:
#line 3712 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13896 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1109:
#line 3717 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 13904 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1110:
#line 3721 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13910 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1111:
#line 3726 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 13916 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1112:
#line 3728 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 13922 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1113:
#line 3734 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 13929 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1114:
#line 3736 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 13937 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1115:
#line 3739 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13943 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1116:
#line 3740 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13951 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1117:
#line 3743 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13959 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1118:
#line 3746 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13965 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1119:
#line 3749 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 13973 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1120:
#line 3752 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13980 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1121:
#line 3754 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 13989 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1122:
#line 3760 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 13998 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1123:
#line 3766 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 14008 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1124:
#line 3774 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 14014 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1125:
#line 3775 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 14020 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;


#line 14024 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}
