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

#line 656 "hphp.5.tab.cpp" /* yacc.c:339  */

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
    T_USING = 353,
    T_USE = 354,
    T_GLOBAL = 355,
    T_STATIC = 356,
    T_ABSTRACT = 357,
    T_FINAL = 358,
    T_PRIVATE = 359,
    T_PROTECTED = 360,
    T_PUBLIC = 361,
    T_VAR = 362,
    T_UNSET = 363,
    T_ISSET = 364,
    T_EMPTY = 365,
    T_HALT_COMPILER = 366,
    T_CLASS = 367,
    T_INTERFACE = 368,
    T_EXTENDS = 369,
    T_IMPLEMENTS = 370,
    T_OBJECT_OPERATOR = 371,
    T_NULLSAFE_OBJECT_OPERATOR = 372,
    T_DOUBLE_ARROW = 373,
    T_LIST = 374,
    T_ARRAY = 375,
    T_DICT = 376,
    T_VEC = 377,
    T_VARRAY = 378,
    T_DARRAY = 379,
    T_KEYSET = 380,
    T_CALLABLE = 381,
    T_CLASS_C = 382,
    T_METHOD_C = 383,
    T_FUNC_C = 384,
    T_LINE = 385,
    T_FILE = 386,
    T_COMMENT = 387,
    T_DOC_COMMENT = 388,
    T_OPEN_TAG = 389,
    T_OPEN_TAG_WITH_ECHO = 390,
    T_CLOSE_TAG = 391,
    T_WHITESPACE = 392,
    T_START_HEREDOC = 393,
    T_END_HEREDOC = 394,
    T_DOLLAR_OPEN_CURLY_BRACES = 395,
    T_CURLY_OPEN = 396,
    T_DOUBLE_COLON = 397,
    T_NAMESPACE = 398,
    T_NS_C = 399,
    T_DIR = 400,
    T_NS_SEPARATOR = 401,
    T_XHP_LABEL = 402,
    T_XHP_TEXT = 403,
    T_XHP_ATTRIBUTE = 404,
    T_XHP_CATEGORY = 405,
    T_XHP_CATEGORY_LABEL = 406,
    T_XHP_CHILDREN = 407,
    T_ENUM = 408,
    T_XHP_REQUIRED = 409,
    T_TRAIT = 410,
    T_ELLIPSIS = 411,
    T_INSTEADOF = 412,
    T_TRAIT_C = 413,
    T_HH_ERROR = 414,
    T_FINALLY = 415,
    T_XHP_TAG_LT = 416,
    T_XHP_TAG_GT = 417,
    T_TYPELIST_LT = 418,
    T_TYPELIST_GT = 419,
    T_UNRESOLVED_LT = 420,
    T_COLLECTION = 421,
    T_SHAPE = 422,
    T_TYPE = 423,
    T_UNRESOLVED_TYPE = 424,
    T_NEWTYPE = 425,
    T_UNRESOLVED_NEWTYPE = 426,
    T_COMPILER_HALT_OFFSET = 427,
    T_ASYNC = 428,
    T_LAMBDA_OP = 429,
    T_LAMBDA_CP = 430,
    T_UNRESOLVED_OP = 431,
    T_WHERE = 432
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

#line 898 "hphp.5.tab.cpp" /* yacc.c:358  */

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
#define YYLAST   19687

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  207
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  313
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1108
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  2067

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   432

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    56,   205,     2,   202,    55,    38,   206,
     197,   198,    53,    50,     9,    51,    52,    54,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    32,   199,
      43,    14,    45,    31,    68,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    70,     2,   204,    37,     2,   203,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   200,    36,   201,    58,     2,     2,     2,
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
     194,   195,   196
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   752,   752,   752,   761,   763,   766,   767,   768,   769,
     770,   771,   772,   775,   777,   777,   779,   779,   781,   784,
     789,   794,   797,   800,   804,   808,   812,   816,   820,   825,
     826,   827,   828,   829,   830,   831,   832,   833,   834,   835,
     836,   837,   841,   842,   843,   844,   845,   846,   847,   848,
     849,   850,   851,   852,   853,   854,   855,   856,   857,   858,
     859,   860,   861,   862,   863,   864,   865,   866,   867,   868,
     869,   870,   871,   872,   873,   874,   875,   876,   877,   878,
     879,   880,   881,   882,   883,   884,   885,   886,   887,   888,
     889,   890,   891,   892,   893,   894,   895,   896,   897,   898,
     899,   900,   901,   902,   903,   904,   908,   912,   913,   917,
     918,   923,   925,   930,   935,   936,   937,   939,   944,   946,
     951,   956,   958,   960,   965,   966,   970,   971,   973,   977,
     984,   991,   995,  1001,  1003,  1007,  1008,  1014,  1016,  1020,
    1022,  1027,  1028,  1029,  1030,  1033,  1034,  1038,  1043,  1043,
    1049,  1049,  1056,  1055,  1061,  1061,  1066,  1067,  1068,  1069,
    1070,  1071,  1072,  1073,  1074,  1075,  1076,  1077,  1078,  1079,
    1080,  1084,  1082,  1091,  1089,  1096,  1106,  1100,  1110,  1108,
    1112,  1116,  1120,  1124,  1128,  1132,  1136,  1141,  1142,  1146,
    1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,
    1157,  1179,  1185,  1186,  1195,  1197,  1201,  1202,  1203,  1207,
    1208,  1212,  1212,  1217,  1223,  1227,  1227,  1235,  1236,  1240,
    1241,  1245,  1251,  1249,  1266,  1263,  1281,  1278,  1296,  1295,
    1304,  1302,  1314,  1313,  1332,  1330,  1349,  1348,  1357,  1355,
    1366,  1366,  1373,  1372,  1384,  1382,  1395,  1396,  1400,  1403,
    1406,  1407,  1408,  1411,  1412,  1415,  1417,  1420,  1421,  1424,
    1425,  1428,  1429,  1433,  1434,  1439,  1440,  1443,  1444,  1445,
    1449,  1450,  1454,  1455,  1459,  1460,  1464,  1465,  1470,  1471,
    1477,  1478,  1479,  1480,  1483,  1486,  1488,  1491,  1492,  1496,
    1498,  1501,  1504,  1507,  1508,  1511,  1512,  1516,  1522,  1528,
    1535,  1537,  1542,  1547,  1553,  1557,  1561,  1565,  1570,  1575,
    1580,  1585,  1591,  1600,  1605,  1610,  1616,  1618,  1622,  1626,
    1631,  1635,  1638,  1641,  1645,  1649,  1653,  1657,  1662,  1670,
    1672,  1675,  1676,  1677,  1678,  1680,  1682,  1687,  1688,  1691,
    1692,  1693,  1697,  1698,  1700,  1701,  1705,  1707,  1710,  1714,
    1720,  1722,  1725,  1725,  1729,  1728,  1732,  1734,  1737,  1740,
    1738,  1755,  1752,  1767,  1769,  1771,  1773,  1775,  1777,  1779,
    1783,  1784,  1785,  1788,  1794,  1798,  1804,  1807,  1812,  1814,
    1819,  1824,  1828,  1829,  1833,  1834,  1836,  1838,  1844,  1845,
    1847,  1851,  1852,  1857,  1861,  1862,  1866,  1867,  1871,  1873,
    1879,  1884,  1885,  1887,  1891,  1892,  1893,  1894,  1898,  1899,
    1900,  1901,  1902,  1903,  1905,  1910,  1913,  1914,  1918,  1919,
    1923,  1924,  1927,  1928,  1931,  1932,  1935,  1936,  1940,  1941,
    1942,  1943,  1944,  1945,  1946,  1950,  1951,  1954,  1955,  1956,
    1959,  1961,  1963,  1964,  1967,  1969,  1973,  1975,  1979,  1983,
    1987,  1992,  1996,  1997,  1999,  2000,  2001,  2002,  2005,  2006,
    2010,  2011,  2015,  2016,  2017,  2018,  2022,  2026,  2031,  2035,
    2039,  2043,  2047,  2052,  2056,  2057,  2058,  2059,  2060,  2064,
    2068,  2070,  2071,  2072,  2075,  2076,  2077,  2078,  2079,  2080,
    2081,  2082,  2083,  2084,  2085,  2086,  2087,  2088,  2089,  2090,
    2091,  2092,  2093,  2094,  2095,  2096,  2097,  2098,  2099,  2100,
    2101,  2102,  2103,  2104,  2105,  2106,  2107,  2108,  2109,  2110,
    2111,  2112,  2113,  2114,  2115,  2116,  2117,  2118,  2120,  2121,
    2123,  2124,  2126,  2127,  2128,  2129,  2130,  2131,  2132,  2133,
    2134,  2135,  2136,  2137,  2138,  2139,  2140,  2141,  2142,  2143,
    2144,  2145,  2146,  2147,  2148,  2149,  2150,  2154,  2158,  2163,
    2162,  2178,  2176,  2195,  2194,  2214,  2213,  2233,  2232,  2251,
    2251,  2267,  2267,  2286,  2287,  2288,  2293,  2295,  2299,  2303,
    2309,  2313,  2319,  2321,  2325,  2327,  2331,  2335,  2336,  2340,
    2342,  2346,  2348,  2352,  2354,  2358,  2361,  2366,  2368,  2372,
    2375,  2380,  2384,  2388,  2392,  2396,  2400,  2404,  2408,  2412,
    2416,  2420,  2424,  2428,  2432,  2436,  2440,  2442,  2446,  2448,
    2452,  2454,  2458,  2465,  2472,  2474,  2479,  2480,  2481,  2482,
    2483,  2484,  2485,  2486,  2487,  2489,  2490,  2494,  2495,  2496,
    2497,  2501,  2507,  2516,  2529,  2530,  2533,  2536,  2539,  2540,
    2543,  2547,  2550,  2553,  2560,  2561,  2565,  2566,  2568,  2573,
    2574,  2575,  2576,  2577,  2578,  2579,  2580,  2581,  2582,  2583,
    2584,  2585,  2586,  2587,  2588,  2589,  2590,  2591,  2592,  2593,
    2594,  2595,  2596,  2597,  2598,  2599,  2600,  2601,  2602,  2603,
    2604,  2605,  2606,  2607,  2608,  2609,  2610,  2611,  2612,  2613,
    2614,  2615,  2616,  2617,  2618,  2619,  2620,  2621,  2622,  2623,
    2624,  2625,  2626,  2627,  2628,  2629,  2630,  2631,  2632,  2633,
    2634,  2635,  2636,  2637,  2638,  2639,  2640,  2641,  2642,  2643,
    2644,  2645,  2646,  2647,  2648,  2649,  2650,  2651,  2652,  2653,
    2654,  2658,  2663,  2664,  2668,  2669,  2670,  2671,  2673,  2677,
    2678,  2689,  2690,  2692,  2694,  2706,  2707,  2708,  2712,  2713,
    2714,  2718,  2719,  2720,  2723,  2725,  2729,  2730,  2731,  2732,
    2734,  2735,  2736,  2737,  2738,  2739,  2740,  2741,  2742,  2743,
    2746,  2751,  2752,  2753,  2755,  2756,  2758,  2759,  2760,  2761,
    2762,  2763,  2764,  2765,  2766,  2768,  2770,  2772,  2774,  2776,
    2777,  2778,  2779,  2780,  2781,  2782,  2783,  2784,  2785,  2786,
    2787,  2788,  2789,  2790,  2791,  2792,  2794,  2796,  2798,  2800,
    2801,  2804,  2805,  2809,  2813,  2815,  2819,  2820,  2824,  2830,
    2833,  2837,  2838,  2839,  2840,  2841,  2842,  2843,  2848,  2850,
    2854,  2855,  2858,  2859,  2863,  2866,  2868,  2870,  2874,  2875,
    2876,  2877,  2880,  2884,  2885,  2886,  2887,  2891,  2893,  2900,
    2901,  2902,  2903,  2908,  2909,  2910,  2911,  2913,  2914,  2916,
    2917,  2918,  2919,  2920,  2924,  2926,  2930,  2932,  2935,  2938,
    2940,  2942,  2945,  2947,  2951,  2953,  2956,  2959,  2965,  2967,
    2970,  2971,  2976,  2979,  2983,  2983,  2988,  2991,  2992,  2996,
    2997,  3001,  3002,  3003,  3007,  3009,  3017,  3018,  3022,  3024,
    3032,  3033,  3037,  3039,  3040,  3045,  3047,  3052,  3063,  3077,
    3089,  3104,  3105,  3106,  3107,  3108,  3109,  3110,  3120,  3129,
    3131,  3133,  3137,  3141,  3142,  3143,  3144,  3145,  3161,  3162,
    3172,  3173,  3174,  3175,  3176,  3177,  3178,  3179,  3181,  3186,
    3190,  3191,  3195,  3198,  3202,  3209,  3213,  3222,  3229,  3231,
    3237,  3239,  3240,  3244,  3245,  3246,  3253,  3254,  3259,  3260,
    3265,  3266,  3267,  3268,  3279,  3282,  3285,  3286,  3287,  3288,
    3299,  3303,  3304,  3305,  3307,  3308,  3309,  3313,  3315,  3318,
    3320,  3321,  3322,  3323,  3326,  3328,  3329,  3333,  3335,  3338,
    3340,  3341,  3342,  3346,  3348,  3351,  3354,  3356,  3358,  3362,
    3363,  3365,  3366,  3372,  3373,  3375,  3385,  3387,  3389,  3392,
    3393,  3394,  3398,  3399,  3400,  3401,  3402,  3403,  3404,  3405,
    3406,  3407,  3408,  3412,  3413,  3417,  3419,  3427,  3429,  3433,
    3437,  3442,  3446,  3454,  3455,  3459,  3460,  3466,  3467,  3476,
    3477,  3485,  3488,  3492,  3495,  3500,  3505,  3507,  3508,  3509,
    3512,  3514,  3520,  3521,  3525,  3526,  3530,  3531,  3535,  3536,
    3539,  3544,  3545,  3549,  3552,  3554,  3558,  3564,  3565,  3566,
    3570,  3574,  3582,  3587,  3599,  3601,  3605,  3608,  3610,  3615,
    3620,  3626,  3629,  3634,  3639,  3641,  3648,  3650,  3653,  3654,
    3657,  3660,  3661,  3666,  3668,  3672,  3678,  3688,  3689
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
  "T_UNRESOLVED_OP", "T_WHERE", "'('", "')'", "';'", "'{'", "'}'", "'$'",
  "'`'", "']'", "'\"'", "'\\''", "$accept", "start", "$@1",
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
     426,   427,   428,   429,   430,   431,   432,    40,    41,    59,
     123,   125,    36,    96,    93,    34,    39
};
# endif

#define YYPACT_NINF -1678

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1678)))

#define YYTABLE_NINF -1109

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1109)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1678,   163, -1678, -1678,  5587, 15175, 15175,   -29, 15175, 15175,
   15175, 15175, 12523, 15175, -1678, 15175, 15175, 15175, 15175, 18218,
   18218, 15175, 15175, 15175, 15175, 15175, 15175, 15175, 15175, 12727,
   18895, 15175,    23,    69, -1678, -1678, -1678,   188, -1678,   274,
   -1678, -1678, -1678,   341, 15175, -1678,    69,   242,   260,   366,
   -1678,    69, 12931, 16981, 13135, -1678, 16058, 11503,   379, 15175,
   19139,   192,   417,   635,   474, -1678, -1678, -1678,   395,   409,
     416,   426, -1678, 16981,   480,   492,   630,   637,   648,   698,
     703, -1678, -1678, -1678, -1678, -1678, 15175,   555,  3603, -1678,
   -1678, 16981, -1678, -1678, -1678, -1678, 16981, -1678, 16981, -1678,
     571,   550, 16981, 16981, -1678,   331, -1678, -1678, 13339, -1678,
   -1678,   579,   504,   584,   584, -1678,   750,   623,   527,   606,
   -1678,    91, -1678,   611,   708,   789, -1678, -1678, -1678, -1678,
    2122,   670, -1678,   149, -1678,   641,   645,   656,   661,   664,
     674,   676,   682,  2595, -1678, -1678, -1678, -1678, -1678,   329,
     795,   816,   818,   824,   826, -1678,   828,   840, -1678,   170,
     711, -1678,   752,    62, -1678,  1794,   171, -1678, -1678,  2449,
     149,   149,   717,   356, -1678,   156,    76,   718,   175, -1678,
     360, -1678,   847, -1678,   758, -1678, -1678,   723,   756, -1678,
   15175, -1678,   789,   670, 19354,  3951, 19354, 15175, 19354, 19354,
   19618, 19618,   724,  5108, 19354,   873, 16981,   855,   855,   368,
     855, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678,
     346, 15175,   744, -1678, -1678,   766,   731,   439,   733,   439,
     855,   855,   855,   855,   855,   855,   855,   855, 18218, 17739,
     728,   928,   758, -1678, 15175,   744, -1678,   786, -1678,   792,
     748, -1678,   161, -1678, -1678, -1678,   439,   149, -1678, 13543,
   -1678, -1678, 15175, 10075,   942,    92, 19354, 11095, -1678, 15175,
   15175, 16981, -1678, -1678, 17257,   759, -1678, 17305, -1678, -1678,
   -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678,  3639, -1678,
    3639, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678,
   -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678,
   -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678,
   -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678,
   -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678,    85,
      88,   756, -1678, -1678, -1678, -1678,   760,  2737,   105, -1678,
   -1678,   800,   948, -1678,   802, 16781, 15175, -1678,   768,   769,
   17353, -1678,    71, 17424, 16675, 16675, 16675, 16981, 16675,   771,
     960,   777, -1678,   478, -1678, 17802,   102, -1678,   963,   106,
     849, -1678,   850, -1678, 18218, 15175, 15175,   785,   803, -1678,
   -1678, 17906, 12727, 15175, 15175, 15175, 15175, 15175,   111,    89,
     482, -1678, 15379, 18218,   590, -1678, 16981, -1678,   522,   623,
   -1678, -1678, -1678, -1678, 18996,   973,   888, -1678, -1678, -1678,
      87, 15175,   804,   806, 19354,   808,  2549,   809,  6199, 15175,
     411,   810,   603,   411,   477,   498, -1678, 16981,  3639,   812,
    5362, 16058, -1678, 13747,   814,   814,   814,   814, -1678, -1678,
    4870, -1678, -1678, -1678, -1678, -1678,   789, -1678, 15175, 15175,
   -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, 15175,
   15175, 15175, 15175, 13951, 15175, 15175, 15175, 15175, 15175, 15175,
   15175, 15175, 15175, 15175, 15175, 15175, 15175, 15175, 15175, 15175,
   15175, 15175, 15175, 15175, 15175, 15175, 15175, 19097, 15175, -1678,
   15175, 15175, 15175,  5208, 16981, 16981, 16981, 16981, 16981,  2122,
     901,   690, 11299, 15175, 15175, 15175, 15175, 15175, 15175, 15175,
   15175, 15175, 15175, 15175, 15175, -1678, -1678, -1678, -1678,  2610,
   -1678, -1678,  5362,  5362, 15175, 15175,   579,   178, 17906,   819,
     789, 14155, 17472, -1678, 15175, -1678,   821,  1006,   861,   822,
     823, 15551,   439, 14359, -1678, 14563, -1678,   748,   825,   830,
    2652, -1678,   418,  5362, -1678,  4863, -1678, -1678, 17520, -1678,
   -1678, 11707, -1678, 15175, -1678,   932, 10279,  1021,   838, 15363,
    1024,   100,   147, -1678, -1678, -1678,   859, -1678, -1678, -1678,
    3639, -1678,  2230,   848,  1035, 17663, 16981, -1678, -1678, -1678,
   -1678, -1678, -1678, -1678, -1678, -1678,   851, -1678, -1678,   852,
     856,   854,   858,   863,   860,   481,   867,   862,  5433, 16828,
   -1678, -1678, 16981, 16981, 15175,   439,   192, -1678, 17663,   966,
   -1678, -1678, -1678,   439,   123,   136,   872,   877,  2703,   325,
     879,   881,   710,   925,   874,   439,   139,   880, 18388,   878,
    1078,  1079,   889,   890,   897,   898, -1678,  5091, 16981, -1678,
   -1678,  1033,  3589,    42, -1678, -1678, -1678,   623, -1678, -1678,
   -1678,  1072,   972,   926,   374,   951, 15175,   579,   976,  1100,
     917, -1678,   955, -1678,   178, -1678,  3639,  3639,  1104,   942,
      87, -1678,   929,  1110, -1678,  3639,    86, -1678,   463,   172,
   -1678, -1678, -1678, -1678, -1678, -1678, -1678,  1056,  3880, -1678,
   -1678, -1678, -1678,  1111,   941, -1678, 18218, 15175,   931,  1117,
   19354,  1113,   140,  1121,   934,   936,   938, 19354,   940,  3159,
    6403, -1678, -1678, -1678, -1678, -1678, -1678,  1007,  4878, 19354,
     943,  4093, 13120, 19537, 19618, 15882, 15175, 19306, 16059, 14134,
   14541, 14744,  4796, 16228, 16586, 16586, 16586, 16586,  1503,  1503,
    1503,  1503,  1503,   767,   767,   886,   886,   886,   368,   368,
     368, -1678,   855,   947,   949, 18436,   953,  1148,   235, 15175,
     266,   744,   388,   178, -1678, -1678, -1678,  1145,   888, -1678,
     789, 18010, -1678, -1678, -1678, 19618, 19618, 19618, 19618, 19618,
   19618, 19618, 19618, 19618, 19618, 19618, 19618, 19618, -1678, 15175,
     407,   183, -1678, -1678,   744,   433,   962,   971,   957,  4370,
     143,   967, -1678, 19354, 17767, -1678, 16981, -1678,   439,    52,
   18218, 19354, 18218, 18496,  1007,   204,   439,   201,  1010,   974,
   15175, -1678,   207, -1678, -1678, -1678,  6607,   559, -1678, -1678,
   19354, 19354,    69, -1678, -1678, -1678, 15175,  1071, 17538, 17663,
   16981, 10483,   975,   983, -1678,  1166, 16480,  1049, -1678,  1028,
   -1678,  1182,   996,  3350,  3639, 17663, 17663, 17663, 17663, 17663,
     998,  1126,  1130,  1131,  1132,  1134,  1008, 17663,   247, -1678,
   -1678, -1678, -1678, -1678, -1678,   259, -1678, 19448, -1678, -1678,
      74, -1678,  6811, 15598,  1009, 16828, -1678, 16828, -1678, 16828,
   -1678, 16981, 16981, 16828, -1678, 16828, 16828, 16981, -1678,  1198,
    1011, -1678,   500, -1678, -1678,  4430, -1678, 19448,  1196, 18218,
    1012, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678,
    1030,  1207, 16981, 15598,  1017, 17906, 18114,  1208, -1678, 15175,
   -1678, 15175, -1678, 15175, -1678, -1678, -1678, -1678, -1678, -1678,
   -1678, -1678,  1019, -1678, 15175, -1678, -1678,  5791, -1678,  3639,
   15598,  1025, -1678, -1678, -1678, -1678,  1210,  1029, 15175, 18996,
   -1678, -1678,  5208,  1032, -1678,  3639, -1678,  1034,  7015,  1197,
      95, -1678, -1678,   179,  2610,  2610, -1678,  3639, -1678, -1678,
     439, 19354, -1678, 11911, -1678, 17663, 13747,   814, 13747, -1678,
     814,   814, -1678, 12115, -1678, -1678,  7219, -1678,   109,  1037,
   15598,   972, -1678, -1678, -1678, -1678, 16059, 15175, -1678, -1678,
   15175, -1678, 15175, -1678,  4884,  1039,  5362,   925,  1205,   972,
    3639,  1224,  1007, 16981, 19097,   439,  5060,  1043,   213,  1046,
   -1678, -1678,  1230,  2993,  2993, 17767, -1678, -1678, -1678,  1194,
    1053,  1189,  1190,  1204,  1206,  1209,   453,  1070,   574, -1678,
   -1678, -1678, -1678, -1678,  1114, -1678, -1678, -1678, -1678,  1269,
    1082,   821,   439,   439, 14767,   972,  4863, -1678,  4863, -1678,
   12507,   709,    69, 11095, -1678,  7423,  1084,  7627,  1085, 17538,
   18218,  1089,  1150,   439, 19448,  1275, -1678, -1678, -1678, -1678,
     665, -1678,    59,  3639,  1108,  1156,  1135,  3639, 16981,  3224,
   -1678, -1678,  1290, -1678,  1103,  1128,  1129,  1111,   784,   784,
    1237,  1237, 18652,  1105,  1301, 17663, 17663, 17663, 17663, 17663,
   17663, 18996,   160, 16934, 17663, 17663, 17663, 17663,  4139, 17663,
   17663, 17663, 17663, 17663, 17663, 17663, 17663, 17663, 17663, 17663,
   17663, 17663, 17663, 17663, 17663, 17663, 17663, 17663, 17663, 17663,
   17663, 17663, 16981, -1678, -1678,  1228, -1678, -1678,  1115,  1119,
    1124, -1678,  1125, -1678, -1678,   525,  5433, -1678,  1116, -1678,
   17663,   439, -1678, -1678,   125, -1678,   730,  1309, -1678, -1678,
     144,  1133,   439, 12319, 19354, 18544, -1678,  2996, -1678,  5995,
     888,  1309, -1678,   587,   508, -1678, 19354,  1185,  1140, -1678,
    1139,  1197, -1678, -1678, -1678, 14971,  3639,   942,  3639,    77,
    1315,  1249,   218, -1678,   744,   222, -1678,   744, -1678, 18218,
   15175, 19354, 19448, -1678, -1678, -1678,  3971, -1678, -1678, -1678,
   -1678, -1678, -1678,  1142,   109, -1678,  1141,   109,  1146, 16059,
   19354, 18604,  1149,  5362,  1144,  1151,  3639,  1152,  1155,  3639,
     972, -1678,   748,   514,  5362, 15175, -1678, -1678, -1678, -1678,
   -1678, -1678,  1218,  1160,  1336,  1265, 17767, 17767, 17767, 17767,
   17767, 17767,  1211, -1678, 18996,   519, 17767, -1678, -1678, -1678,
   18218, 19354,  1165, -1678,    69,  1335,  1291, 11095, -1678, -1678,
   -1678,  1171, 15175,  1150,   439, 17906, 17538,  1173, 17663,  7831,
     700,  1178, 15175,    98,    67, -1678,  1195, -1678,  3639, 16981,
   -1678,  1235, -1678, -1678,  3350,  1347, -1678,  1183, 17663, -1678,
   17663, -1678,  1184,  1176,  1376, 18708,  1199, 19448,  1377,  1200,
    1212,  1214,  1250,  1379,  1203, -1678, -1678, -1678, 18756,  1213,
    1389, 19493, 19581, 11891, 17663, 19402, 14338,  4464, 15549, 13927,
   16782, 17551, 17551, 17551, 17551,  2842,  2842,  2842,  2842,  2842,
    1307,  1307,   784,   784,   784,  1237,  1237,  1237,  1237, -1678,
    1217, -1678,  1221,  1222,  1225,  1229, -1678, -1678, 19448, 16981,
    3639,  3639, -1678,   730, 15598,   783, -1678, 17906, -1678, -1678,
   19618, 15175,  1226, -1678,  1223,  1271, -1678,   240, 15175, -1678,
   -1678, -1678, 15175, -1678, 15175, -1678,   942, 13747,  1232,   241,
     814,   241,   230, -1678, -1678,   180,  1395,  1327, 15175, -1678,
    1215,  1239,   439, 19354,  1197,  1234, -1678,  1236,   109, 15175,
    5362,  1240, -1678, -1678,   888, -1678, -1678,  1238,  1241,  1246,
   -1678,  1251, 17767, -1678, 17767, -1678, -1678,  1253,  1243,  1402,
    1311,  1248, -1678,  1444,  1252,  1254,  1255, -1678,  1317,  1259,
    1451, -1678, -1678,   439, -1678,  1429, -1678,  1266, -1678, -1678,
    1268,  1273,   145, -1678, -1678, 19448,  1276,  1279, -1678, 15159,
   -1678, -1678, -1678, -1678, -1678, -1678,  1337,  3639,  1128,  1289,
    3639, -1678, 19448, 18812, -1678, -1678, 17663, -1678, 17663, -1678,
   17663, -1678, -1678, -1678, -1678, 17663, 18996, -1678, -1678, 17663,
   -1678, 17663, -1678, 12707, 17663,  1280,  8035, -1678, -1678, -1678,
   -1678,   730, -1678, -1678, -1678, -1678,   713, 16235, 15598,  1367,
   -1678,  4493,  1316,  3920, -1678, -1678, -1678,   901,  4579,   113,
     116,  1282,   888,   690,   146, 19354, -1678, -1678, -1678,  1319,
   12915, 13527, 19354, -1678,  3187, -1678,  6403,   153,  1474,  1407,
   15175, -1678, 19354,  5362,  5362,  1374,  1197,  1643,  1197,  1295,
   19354,  1297, -1678,  1907,  1296,  2116, -1678, -1678,   109, -1678,
   -1678,  1361, -1678, -1678, 17767, -1678, 17767, -1678, 17767, -1678,
   -1678, -1678, -1678, 17767, -1678, 18996, -1678,  2133, -1678,  8239,
   -1678, -1678, -1678, -1678, 10687, -1678, -1678, -1678,  6607,  3639,
   -1678, -1678,  1303, 17663, 18860, 19448, 19448, 19448,  1362, 19448,
   18916, 12707, -1678, -1678,   730, 15598, 15598, 16981, -1678,  1488,
   17087,    80, -1678, 16235,   888, 16307, -1678,  1321, -1678,   124,
    1305,   127, -1678, 16590, -1678, -1678, -1678,   128, -1678, -1678,
    3358, -1678,  1308, -1678,  1423,   789, -1678, 16413, -1678, 16413,
   -1678, -1678,  1494,   901, -1678, 15704, -1678, -1678, -1678, -1678,
    3227,  1495,  1427, 15175, -1678, 19354,  1313,  1318,  1322,  1197,
    1324, -1678,  1374,  1197, -1678, -1678, -1678, -1678,  2313,  1320,
   17767,  1375, -1678, -1678, -1678,  1378, -1678,  6607, 10891, 10687,
   -1678, -1678, -1678,  6607, -1678, -1678, 19448, 17663, 17663, 17663,
    8443,  1323,  1325, -1678, 17663, -1678, 15598, -1678, -1678, -1678,
   -1678, -1678,  3639,  2937,  4493, -1678, -1678, -1678, -1678, -1678,
   -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678,
   -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678,
   -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678,
   -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678,
   -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678,
   -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678,
   -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678,
   -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678,   219, -1678,
    1316, -1678, -1678, -1678, -1678, -1678,   115,   392, -1678,  1499,
     130, 16781,  1423,  1500, -1678,  3639,   789, -1678, -1678,  1326,
    1507, 15175, -1678, 19354, -1678, -1678,   151,  1328,  3639,   667,
    1197,  1324, 15881, -1678,  1197, -1678, 17767, 17767, -1678, -1678,
   -1678, -1678,  8647, 19448, 19448, 19448, -1678, -1678, -1678, 19448,
   -1678,  1739,  1514,  1517,  1329, -1678, -1678, 17663, 16590, 16590,
    1461, -1678,  3358,  3358,   666, -1678, -1678, -1678, 17663,  1449,
   -1678,  1351,  1338,   131, 17663, -1678, 16781, -1678, 17663, 19354,
    1453, -1678,  1525, -1678,  1528, -1678,   545, -1678, -1678, -1678,
    1339,   667, -1678,   667, -1678, -1678,  8851,  1341,  1446, -1678,
    1436,  1383, -1678, -1678,  1460,  3639,  1380,  2937, -1678, -1678,
   19448, -1678, -1678,  1393, -1678,  1531, -1678, -1678, -1678, -1678,
   19448,  1554,   710, -1678, -1678, 19448,  1373, 19448, -1678,   155,
    1384,  9055,  3639, -1678,  3639, -1678,  9259, -1678, -1678, -1678,
    1385, -1678,  1390,  1394, 16981,   690,  1406, -1678, -1678, -1678,
   17663,  1408,   119, -1678,  1510, -1678, -1678, -1678, -1678, -1678,
   -1678,  9463, -1678, 15598,  1009, -1678,  1401, 16981,   677, -1678,
   19448, -1678,  1396,  1586,   652,   119, -1678, -1678,  1513, -1678,
   15598,  1399, -1678,  1197,   135, -1678, -1678, -1678, -1678,  3639,
   -1678,  1403,  1404,   133, -1678,  1324,   652,   189,  1197,  1409,
   -1678,   671,  3639,   350,  1585,  1520,  1324, -1678, -1678, -1678,
   -1678,   190,  1593,  1527, 15175, -1678,   671,  9667,  9871,   357,
    1594,  1529, 15175, -1678, 19354, -1678, -1678, -1678,  1597,  1530,
   15175, -1678, 19354, 15175, -1678, 19354, 19354
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   202,   462,     0,   894,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   988,
     976,     0,   760,     0,   766,   767,   768,    29,   832,   963,
     964,   169,   170,   769,     0,   150,     0,     0,     0,     0,
      30,     0,     0,     0,     0,   221,     0,     0,     0,     0,
       0,     0,   431,   432,   433,   430,   429,   428,     0,     0,
       0,     0,   250,     0,     0,     0,    37,    38,    40,    41,
      39,   773,   775,   776,   770,   771,     0,     0,     0,   777,
     772,     0,   743,    32,    33,    34,    36,    35,     0,   774,
       0,     0,     0,     0,   778,   434,   571,    31,     0,   168,
     138,   968,   761,     0,     0,     4,   124,   126,   831,     0,
     742,     0,     6,     0,     0,   220,     7,     9,     8,    10,
       0,     0,   426,     0,   476,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   532,   474,   950,   951,   553,   547,
     548,   549,   550,   551,   552,   457,   556,     0,   456,   922,
     744,   751,     0,   834,   546,   425,   925,   926,   938,   475,
       0,     0,     0,   478,   477,   923,   924,   921,   958,   962,
       0,   536,   833,    11,   431,   432,   433,     0,     0,    36,
       0,   124,   220,     0,  1028,   475,  1029,     0,  1031,  1032,
     555,   470,     0,   463,   468,     0,     0,   518,   519,   520,
     521,    29,   963,   769,   746,    37,    38,    40,    41,    39,
       0,     0,  1052,   943,   744,     0,   745,   497,     0,   499,
     537,   538,   539,   540,   541,   542,   543,   545,     0,   992,
       0,   841,   756,   240,     0,  1052,   454,   755,   749,     0,
     765,   745,   971,   972,   978,   970,   757,     0,   455,     0,
     759,   544,     0,   203,     0,     0,   459,   203,   148,   461,
       0,     0,   154,   156,     0,     0,   158,     0,    75,    76,
      81,    82,    67,    68,    59,    79,    90,    91,     0,    62,
       0,    66,    74,    72,    93,    85,    84,    57,    80,   100,
     101,    58,    96,    55,    97,    56,    98,    54,   102,    89,
      94,    99,    86,    87,    61,    88,    92,    53,    83,    69,
     103,    77,   105,    70,    60,    47,    48,    49,    50,    51,
      52,    71,   106,   104,   108,    64,    45,    46,    73,  1099,
    1100,    65,  1104,    44,    63,    95,     0,     0,   124,   107,
    1043,  1098,     0,  1101,     0,     0,     0,   160,     0,     0,
       0,   211,     0,     0,     0,     0,     0,     0,     0,     0,
     843,     0,   112,   114,   339,     0,     0,   338,   344,     0,
       0,   251,     0,   254,     0,     0,     0,     0,  1049,   236,
     248,   984,   988,   590,   617,   617,   590,   617,     0,  1013,
       0,   780,     0,     0,     0,  1011,     0,    16,     0,   128,
     228,   242,   249,   647,   583,     0,  1037,   563,   565,   567,
     898,   462,   476,     0,     0,   474,   475,   477,   203,     0,
     762,     0,   763,     0,     0,     0,   200,     0,     0,   130,
     330,     0,    28,     0,     0,     0,     0,     0,   201,   219,
       0,   247,   232,   246,   431,   434,   220,   427,   967,     0,
     914,   190,   191,   192,   193,   194,   196,   197,   199,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   976,     0,   189,
     967,   967,   998,     0,     0,     0,     0,     0,     0,     0,
       0,   424,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   496,   498,   899,   900,     0,
     913,   912,   330,   330,   967,     0,   969,   959,   984,     0,
     220,     0,     0,   162,     0,   896,   891,   841,     0,   476,
     474,     0,   996,     0,   588,   840,   987,   765,   476,   474,
     475,   130,     0,   330,   453,     0,   915,   758,     0,   138,
     290,     0,   570,     0,   165,     0,   203,   460,     0,     0,
       0,     0,     0,   157,   188,   159,  1099,  1100,  1096,  1097,
       0,  1103,  1089,     0,     0,     0,     0,    78,    43,    65,
      42,  1044,   195,   198,   161,   138,     0,   178,   187,     0,
       0,     0,     0,     0,     0,   115,     0,     0,     0,   842,
     113,    18,     0,   109,     0,   340,     0,   163,     0,     0,
     164,   252,   253,  1033,     0,     0,   476,   474,   475,   478,
     477,     0,  1079,   260,     0,   985,     0,     0,     0,     0,
     841,   841,     0,     0,     0,     0,   166,     0,     0,   779,
    1012,   832,     0,     0,  1010,   837,  1009,   127,     5,    13,
      14,     0,   258,     0,     0,   576,     0,     0,     0,   841,
       0,   753,     0,   752,   747,   577,     0,     0,     0,     0,
     898,   134,     0,   843,   897,  1108,   452,   465,   479,   931,
     949,   145,   137,   141,   142,   143,   144,   425,     0,   554,
     835,   836,   125,   841,     0,  1053,     0,     0,     0,   843,
     331,     0,     0,     0,   476,   207,   208,   206,   474,   475,
     203,   182,   180,   181,   183,   559,   222,   256,     0,   966,
       0,     0,   502,   504,   503,   515,     0,     0,   535,   500,
     501,   505,   507,   506,   524,   525,   522,   523,   526,   527,
     528,   529,   530,   516,   517,   509,   510,   508,   511,   512,
     514,   531,   513,     0,     0,  1002,     0,   841,  1036,     0,
    1035,  1052,   928,   958,   238,   230,   244,     0,  1037,   234,
     220,     0,   466,   469,   471,   481,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   902,     0,
     901,   904,   927,   908,  1052,   905,     0,     0,     0,     0,
       0,     0,  1030,   464,   889,   893,   840,   895,   451,   748,
       0,   991,     0,   990,   256,     0,   748,   975,   974,     0,
       0,   901,   904,   973,   905,   473,   292,   294,   134,   574,
     573,   458,     0,   138,   274,   149,   461,     0,     0,     0,
       0,   203,   286,   286,   155,   841,     0,     0,  1088,     0,
    1085,   841,     0,  1059,     0,     0,     0,     0,     0,   839,
       0,    37,    38,    40,    41,    39,     0,     0,   782,   786,
     787,   788,   789,   790,   792,     0,   781,   132,   830,   791,
    1052,  1102,   203,     0,     0,     0,    21,     0,    22,     0,
      19,     0,   110,     0,    20,     0,     0,     0,   121,   843,
       0,   119,   114,   111,   116,     0,   337,   345,   342,     0,
       0,  1022,  1027,  1024,  1023,  1026,  1025,    12,  1077,  1078,
       0,   841,     0,     0,     0,   984,   981,     0,   587,     0,
     601,   840,   589,   840,   616,   604,   610,   613,   607,  1021,
    1020,  1019,     0,  1015,     0,  1016,  1018,   203,     5,     0,
       0,     0,   641,   642,   650,   649,     0,   474,     0,   840,
     582,   586,     0,     0,  1038,     0,   564,     0,   203,  1066,
     898,   316,  1107,     0,     0,     0,   965,   840,  1055,  1051,
     332,   333,   741,   842,   329,     0,     0,     0,     0,   451,
       0,     0,   479,     0,   932,   210,   203,   140,   898,     0,
       0,   258,   561,   224,   910,   911,   534,     0,   624,   625,
       0,   622,   840,   997,     0,     0,   330,   260,     0,   258,
       0,     0,   256,     0,   976,   482,     0,     0,   929,   930,
     960,   961,     0,     0,     0,   877,   848,   849,   850,   857,
       0,    37,    38,    40,    41,    39,     0,     0,   863,   869,
     870,   871,   872,   873,     0,   861,   859,   860,   883,   841,
       0,   891,   995,   994,     0,   258,     0,   916,     0,   764,
       0,   296,     0,   203,   146,   203,     0,   203,     0,     0,
       0,     0,   266,   267,   278,     0,   138,   276,   175,   286,
       0,   286,     0,   840,     0,     0,     0,     0,     0,   840,
    1087,  1090,   841,  1057,     0,  1053,     0,   841,   813,   814,
     811,   812,   847,     0,   841,   839,   594,   619,   619,   594,
     619,   585,     0,     0,  1004,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1093,   212,     0,   215,   179,     0,     0,
       0,   117,     0,   122,   123,   115,   842,   120,     0,   341,
       0,  1034,   167,  1050,  1079,  1070,  1074,   259,   261,   351,
       0,     0,   982,     0,   592,     0,  1014,     0,    17,   203,
    1037,   257,   351,     0,     0,   748,   579,     0,   754,  1039,
       0,  1066,   568,   133,   135,     0,     0,     0,  1108,     0,
     321,   319,   904,   917,  1052,   904,   918,  1052,  1054,     0,
       0,   334,   131,   205,   207,   208,   475,   186,   204,   184,
     185,   209,   139,     0,   898,   255,     0,   898,     0,   533,
    1001,  1000,     0,   330,     0,     0,     0,     0,     0,     0,
     258,   226,   765,   903,   330,     0,   853,   854,   855,   856,
     864,   865,   881,     0,   841,     0,   877,   598,   621,   621,
     598,   621,     0,   852,   885,     0,   840,   888,   890,   892,
       0,   989,     0,   903,     0,     0,     0,   203,   293,   575,
     151,     0,   461,   266,   268,   984,     0,     0,     0,   203,
       0,     0,     0,     0,     0,   280,     0,  1094,     0,     0,
    1080,     0,  1086,  1084,   840,     0,  1058,     0,     0,   784,
     840,   838,     0,     0,   841,     0,     0,   827,   841,     0,
       0,     0,     0,   841,     0,   793,   828,   829,  1008,     0,
     841,   796,   798,   797,     0,     0,   794,   795,   799,   801,
     800,   817,   818,   815,   816,   819,   820,   821,   822,   823,
     808,   809,   803,   804,   802,   805,   806,   807,   810,  1092,
       0,   138,     0,     0,     0,     0,   118,    23,   343,     0,
       0,     0,  1071,  1076,     0,   425,   986,   984,   467,   472,
     480,     0,     0,    15,     0,   425,   653,     0,     0,   655,
     648,   651,     0,   646,     0,  1041,     0,     0,     0,     0,
     532,     0,   478,  1067,   572,     0,   322,     0,     0,   317,
       0,     0,   336,   335,  1066,     0,   351,     0,   898,     0,
     330,     0,   956,   351,  1037,   351,  1040,     0,     0,     0,
     483,     0,     0,   867,   840,   876,   858,     0,     0,   841,
       0,     0,   875,   841,     0,     0,     0,   851,     0,     0,
     841,   862,   882,   993,   351,     0,   138,     0,   289,   275,
       0,     0,     0,   265,   171,   279,     0,     0,   282,     0,
     287,   288,   138,   281,  1095,  1081,     0,     0,  1054,     0,
       0,  1106,   846,   845,   783,   602,   840,   593,     0,   605,
     840,   618,   611,   614,   608,     0,   840,   584,   785,     0,
     623,   840,  1003,   825,     0,     0,   203,    24,    25,    26,
      27,  1073,  1068,  1069,  1072,   262,     0,     0,     0,   432,
     423,     0,     0,     0,   237,   350,   352,     0,   422,     0,
       0,     0,  1037,   425,     0,   591,  1017,   347,   243,   644,
       0,     0,   578,   566,   475,   136,   203,     0,   325,   315,
       0,   318,   324,   330,   330,   558,  1066,   425,  1066,     0,
     999,     0,   955,   425,     0,   425,  1042,   351,   898,   952,
     880,   879,   866,   603,   840,   597,     0,   606,   840,   620,
     612,   615,   609,     0,   868,   840,   884,   425,   138,   203,
     147,   152,   173,   269,   203,   277,   283,   138,   285,     0,
    1082,  1056,     0,     0,     0,   596,   826,   581,     0,  1007,
    1006,   824,   138,   216,  1075,     0,     0,     0,  1045,     0,
       0,     0,   263,     0,  1037,     0,   388,   384,   390,   743,
      36,     0,   378,     0,   383,   387,   400,     0,   398,   403,
       0,   402,     0,   401,     0,   220,   354,     0,   356,     0,
     357,   358,     0,     0,   983,     0,   645,   643,   654,   652,
       0,   326,     0,     0,   313,   323,     0,     0,     0,  1066,
    1060,   233,   558,  1066,   957,   239,   347,   245,   425,     0,
       0,     0,   600,   874,   887,     0,   241,   291,   203,   203,
     138,   272,   172,   284,  1083,  1105,   844,     0,     0,     0,
     203,     0,     0,   450,     0,  1046,     0,   368,   372,   447,
     448,   382,     0,     0,     0,   363,   703,   704,   702,   705,
     706,   723,   725,   724,   694,   666,   664,   665,   684,   699,
     700,   660,   671,   672,   674,   673,   693,   677,   675,   676,
     678,   679,   680,   681,   682,   683,   685,   686,   687,   688,
     689,   690,   692,   691,   661,   662,   663,   667,   668,   670,
     740,   708,   709,   713,   714,   715,   716,   717,   718,   701,
     720,   710,   711,   712,   695,   696,   697,   698,   721,   722,
     726,   728,   727,   729,   730,   707,   732,   731,   734,   736,
     735,   669,   739,   737,   738,   733,   719,   659,   395,   656,
       0,   364,   416,   417,   415,   408,     0,   409,   365,   442,
       0,     0,     0,     0,   446,     0,   220,   229,   346,     0,
       0,     0,   314,   328,   953,   954,     0,     0,     0,     0,
    1066,  1060,     0,   235,  1066,   878,     0,     0,   138,   270,
     153,   174,   203,   595,   580,  1005,   214,   366,   367,   445,
     264,     0,   841,   841,     0,   391,   379,     0,     0,     0,
     397,   399,     0,     0,   404,   411,   412,   410,     0,     0,
     353,  1047,     0,     0,     0,   449,     0,   348,     0,   327,
       0,   639,   843,   134,   843,  1062,     0,   418,   134,   223,
       0,     0,   231,     0,   599,   886,   203,     0,   176,   369,
     124,     0,   370,   371,     0,   840,     0,   840,   393,   389,
     394,   657,   658,     0,   380,   413,   414,   406,   407,   405,
     443,   440,  1079,   359,   355,   444,     0,   349,   640,   842,
       0,   203,   842,  1061,     0,  1065,   203,   134,   225,   227,
       0,   273,     0,   218,     0,   425,     0,   385,   392,   396,
       0,     0,   898,   361,     0,   637,   557,   560,  1063,  1064,
     419,   203,   271,     0,     0,   177,   376,     0,   424,   386,
     441,  1048,     0,   843,   436,   898,   638,   562,     0,   217,
       0,     0,   375,  1066,   898,   300,   439,   438,   437,  1108,
     435,     0,     0,     0,   374,  1060,   436,     0,  1066,     0,
     373,     0,  1108,     0,   305,   303,  1060,   138,   420,   134,
     360,     0,   306,     0,     0,   301,     0,   203,   203,     0,
     309,   299,     0,   302,   308,   362,   213,   421,   310,     0,
       0,   297,   307,     0,   298,   312,   311
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1678, -1678, -1678,  -584, -1678, -1678, -1678,   488,  -385,   -41,
     484, -1678,  -237,  -542, -1678, -1678,   428,  1314,  1829, -1678,
    2714, -1678,  -818, -1678,  -537, -1678,  -664,    32, -1678, -1678,
   -1678, -1678, -1678, -1678, -1678, -1678,  -926, -1678, -1678,  -900,
    -265, -1678, -1678, -1678,  -379, -1678, -1678,  -178,   226,    38,
   -1678, -1678, -1678, -1678, -1678, -1678,    44, -1678, -1678, -1678,
   -1678, -1678, -1678,    57, -1678, -1678,  1107,  1118,  1122,   -78,
    -746,  -935,   582,   651,  -387,   311, -1014, -1678,   -92, -1678,
   -1678, -1678, -1678,  -789,   126, -1678, -1678, -1678, -1678,  -375,
   -1678,  -632, -1678,  -440, -1678, -1678,  1003, -1678,   -74, -1678,
   -1678, -1050, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678,
   -1678, -1678,  -111, -1678,   -21, -1678, -1678, -1678, -1678, -1678,
    -194, -1678,    84, -1015, -1678, -1262,  -408, -1678,  -156,   154,
    -129,  -386, -1678,  -203, -1678, -1678, -1678,    94,   -94,   -80,
      61,  -777,   -51, -1678, -1678,    25, -1678,    60,  -380, -1678,
       4,    -5,   -90,   -83,   -58, -1678, -1678, -1678, -1678, -1678,
   -1678, -1678, -1678, -1678,  -617,  -909, -1678, -1678, -1678, -1678,
   -1678,   659,  1256, -1678,   506, -1678,   358, -1678, -1678, -1678,
   -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678,
   -1678, -1678, -1678,  -151,  -435,  -566, -1678, -1678, -1678, -1678,
   -1678,   434, -1678, -1678, -1678, -1678, -1678, -1678, -1678, -1678,
   -1009,  -376,  2718,    41, -1678,   326,  -430, -1678, -1678,  -492,
    3662,  3722, -1678,   542, -1678, -1678,   511,  -145,  -650, -1678,
   -1678,   596,   369,  -634, -1678,   365, -1678, -1678, -1678, -1678,
   -1678,   572, -1678, -1678, -1678,    79,  -931,  -144,  -449,  -439,
   -1678,  -103,  -136, -1678, -1678,    45,    48,   870,   -70, -1678,
   -1678,  1598,   -77, -1678,  -363,   104,  -153, -1678,   252, -1678,
   -1678, -1678,  -447,  1263, -1678, -1678, -1678, -1678, -1678,   745,
     298, -1678, -1678, -1678,  -358,  -690, -1678,  1216, -1300,  -248,
     -57,  -205,    26,   790, -1678, -1677, -1678,  -303, -1126, -1359,
    -290,   129, -1678,   470,   546, -1678, -1678, -1678, -1678,   495,
   -1678,   595,  -813
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   115,   968,   668,   191,   349,   781,
     369,   370,   371,   372,   919,   920,   921,   117,   118,   119,
     120,   121,   988,  1223,   428,  1016,   702,   703,   576,   267,
    1718,   582,  1624,  1719,  1973,   904,   123,   124,   722,   723,
     731,   362,   605,  1928,  1177,  1391,  1995,   450,   192,   704,
    1019,  1258,  1459,   127,   671,  1038,   705,   737,  1042,   643,
    1037,   246,   557,   706,   672,  1039,   452,   389,   411,   130,
    1021,   971,   944,  1197,  1651,  1317,  1102,  1870,  1722,   855,
    1108,   581,   864,  1110,  1502,   847,  1091,  1094,  1306,  2002,
    2003,   692,   693,   718,   719,   376,   377,   379,  1685,  1848,
    1849,  1405,  1555,  1674,  1842,  1982,  2005,  1881,  1932,  1933,
    1934,  1661,  1662,  1663,  1664,  1883,  1884,  1890,  1944,  1667,
    1668,  1672,  1835,  1836,  1837,  1919,  2040,  1556,  1557,   193,
     132,  2019,  2020,  1840,  1559,  1560,  1561,  1562,   133,   134,
     577,   578,   135,   136,   137,   138,   139,   140,   141,   142,
     260,   143,   144,   145,  1699,   146,  1018,  1257,   147,   689,
     690,   691,   264,   420,   572,   678,   679,  1353,   680,  1354,
     148,   149,   649,   650,  1343,  1344,  1468,  1469,   150,   889,
    1069,   151,   890,  1070,   152,   891,  1071,   153,   892,  1072,
     154,   893,  1073,   652,  1346,  1471,   155,   894,   156,   157,
    1912,   158,   673,  1687,   674,  1213,   976,  1423,  1420,  1828,
    1829,   159,   160,   161,   249,   162,   250,   261,   431,   564,
     163,  1347,  1348,   898,   899,   164,  1133,   998,   620,  1134,
    1076,  1280,  1077,  1472,  1473,  1283,  1284,  1079,  1479,  1480,
    1080,   825,   547,   205,   206,   707,   695,   529,  1232,  1233,
     813,   814,   460,   166,   252,   167,   168,   195,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   740,   180,
     256,   257,   646,   240,   241,   776,   777,  1359,  1360,   404,
     405,   962,   181,   634,   182,   688,   183,   352,  1850,  1902,
     390,   439,   713,   714,  1124,  1859,  1914,  1915,  1227,  1402,
     940,  1403,   941,   942,   870,   871,   872,   353,   354,   901,
     591,  1650,   993
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     194,   196,   457,   198,   199,   200,   201,   203,   204,   510,
     207,   208,   209,   210,   539,   350,   230,   231,   232,   233,
     234,   235,   236,   237,   239,   425,   258,   537,   422,   444,
    1095,   427,   846,   445,   530,   531,   122,   263,   681,   266,
     561,   412,   126,   991,  1544,   415,   416,   274,   128,   277,
     268,   683,   360,   446,   363,   272,   685,   423,   987,  1228,
    1126,   129,  1225,   726,   457,   834,  1017,   771,   902,  1004,
    1217,   248,   986,   453,  1112,   253,   918,   923,   254,  1098,
     811,   266,   358,   165,   967,  1313,  1256,   509,  1085,  1736,
     812,   820,   816,   817,   -78,  1426,   556,   -43,  1041,   -78,
     441,   573,   -43,   424,  1267,   265,  1243,   425,  1248,   860,
     422,   626,   565,   427,   -42,   629,   842,   359,   780,   -42,
     573,   566,  1677,   839,  1892,  1679,   843,   610,   612,   614,
    1500,   617,   929,  -381,   255,    14,  1744,  1830,   550,  1899,
    1899,   549,  1736,    14,   427,   573,  -935,   398,   946,  1006,
    1302,  1893,   946,   946,   946,   946,  -451,    14,   131,   657,
    1436,   559,  1415,     3,   558,  1322,  1323,    14,   197,  1225,
    1145,  1146,  1147,  1322,  1323,   938,   939,   527,   528,   862,
     732,   733,   734,    14,  1921,   424,   606,   527,   528,  1910,
    1078,  1148,   542,  1984,  1149,  1150,  1151,  1152,  1153,  1154,
    1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,
    1165,  1166,  1167,  1168,  1169,  1170,   424,  1229,  1577,   458,
     259,   527,   528,  -745,   527,   528,  -934,  2033,  2049,  1171,
     125,  -977,  1352,  1887,  1911,  1172,  1691,  -752,  1985,   424,
    -933,  -936,  -932,   966,   653,   534,   655,  1649,   534,  -946,
     607,  1888,  1437,  -937,   568,   658,   438,   568,  -636,  -944,
    1325,   684,  1230,  1578,   266,   579,   262,   590,  1503,  1187,
    1889,  -980,  2034,  2050,   458,   374,  -935,  -979,   738,  1737,
    1738,  -320,  -569,  -939,   -78,  1544,  -451,   -43,  -919,  -842,
     442,   574,  -920,  -842,  1569,   570,  1270,  1501,   861,   575,
    -633,   627,  1493,   637,   -42,   630,   636,  -320,  -840,   640,
     656,   458,  1678,  1894,   601,  1680,  1097,  -304,  1585,   511,
    1320,   930,  1324,  -381,  1224,  1428,  1745,  1831,  1692,  1900,
    1954,  -753,  2030,  -842,   931,  1458,  -747,   947,  1007,   527,
     528,  1052,  1406,  1623,  1684,   225,   225,   863,  2031,   459,
     783,   201,  1252,  1739,  1231,  1579,  -934,   456,  1355,  2046,
     728,  -977,   821,   724,  2035,  2051,   427,  -942,  -941,  -945,
    -933,  -936,  -932,  1183,  1184,   535,   783,  1843,   535,  1844,
     266,   424,   457,  -937,  1209,  1478,  1253,   239,   648,   266,
     266,   648,   266,   736,   375,  -633,  1587,   662,   783,  -626,
     350,  -980,   827,  1593,   459,  1595,   413,  -979, -1052,   783,
    -948,  -634,   783,  -939,   417,  1435,   203,  -108,  -919,   540,
    1143,  1282,  -920,  1895,   708,   497,  -633,   533,   973,   438,
     730,  1224,  -108,  2042,  1617,   720,   548,   498,   727,   269,
    2058,  1576,  1896,   212,    40,  1897,   635, -1052,  -107,   412,
     787,   788,   453,   739,   741,   651,   651,   270,   651,  1144,
    1700,   792,  1702,  -107,   742,   743,   744,   745,   747,   748,
     749,   750,   751,   752,   753,   754,   755,   756,   757,   758,
     759,   760,   761,   762,   763,   764,   765,   766,   767,   768,
     769,   770,   116,   772,   399,   739,   739,   775,  1200,   694,
     378,   212,    40,   725,   401,   952,   954,   795,   796,   797,
     798,   799,   800,   801,   802,   803,   804,   805,   806,   807,
    1414,  1509,   533,   935,   418,  2043,  -943,   720,   720,   739,
     819,   419,  2059,  -635,   980,  1490,   795,   793,   248,   823,
     974,   275,   253,  1292,   348,   254,  1235,  1708,   831,  -754,
     833,   510,   994,   533,   995,   975,  1236,  1965,   720,  1964,
     399,   388,   536,   271,   225,   399,   850,   664,   851,  1319,
     402,   403,   794,  1857,   527,   528,  1035,  1861,  -746,   361,
     622,   399,   131,   911,   410,  1036,   388,   399,   664,  -906,
     388,   388,   384,   977,   430,   382,  1264,   780,   527,   528,
    1481,   255,   622,   681,  -906,   383,   385,   782,   854,  1047,
    1434,  1293,  1043,   386,  1272,  -909,   683,  1638,   388,   925,
     111,   685,  1445,   387,  -748,  1447,  1245,   911,  1245,   509,
    -909,   849,  1033,   815,  1092,  1093,   402,   403,   399,  1235,
     659,   402,   403,   623,   918,   400,   912,  1400,  1401,  1236,
    1356,   355,  1282,  1470,   125,   782,  1470,   402,   403,  1968,
    -946,  1969,  1482,   402,   403,   437,   838,   399,  1178,   844,
    1179,   424,  1180,   399,   433,  1421,  1182,   391,   223,   223,
     664,  1023,   710,   561,   994,   995,   399,   437, -1052,   392,
     437,  1086,  1088,   664,   546,  1173,  -907,  1947,   660,  1087,
     393,   225,   666,  1349,   711,  1351,  1715,   394,  1422,   438,
     225,  -907,  1001,   401,   402,   403,  1948,   225,   395,  1949,
    1114,   669,   670,  1474, -1052,  1476,  1120, -1052,   660,   225,
     666,   660,   666,   666,  1920,  -124,   790,   413,  1923,  -124,
     682,  1026,  1247,   402,   403,  1249,  1250,   414,   665,   402,
     403,   116,   773,   774,  1416,   116,  -124,   380,  1357,   580,
     938,   939,   402,   403,  1594,   681,   381,  1417,   396,   694,
    1321,  1322,  1323,   397,  1034,  2016,  2017,  2018,   683,   429,
    1460,    55,   436,   685,  1304,  1305,   818,  1418,   437,  1546,
     454,   185,   186,    65,    66,    67,  1195,   454,   185,   186,
      65,    66,    67,   440,  1046,  1497,  1322,  1323,   443,  1573,
     454,   185,   186,    65,    66,    67,  1589,   491,   492,   493,
     494,   495,   496,  1451,   497,   448,   684,   449,  1600,   783,
    1601,    14,  1400,  1401,  1461,  1090,   498,  1168,  1169,  1170,
     461,   783,   783,   600,   462,  1245,  1645,  1646,   609,   611,
     613,   266,   616,  1171,  1536,   463,  1096,   432,   434,   435,
     464,   511,   455,   465,   225,  -627,  1917,  1918,  1492,   455,
    2038,  2039,  1682,   466,   169,   467,  2012,  1945,  1946,  1941,
    1942,   468,   455,   588,   131,   589,  -628,  2025,  -629,   227,
     229,  1234,  1237,  1107,  -630,  1547,  -631,   223,   500,  1122,
    1127,  1548,  2036,   454,  1549,   186,    65,    66,    67,  1550,
     501,   502,  1017,   503,   532,  -940,   116,  -632,   681,  -746,
     538,   406,   545,   543,   498,   712,   438,   551,  -944,   348,
     533,   683,   554,   783,  1297,   783,   685,   555,   388,   494,
     495,   496,   594,   497,  1204,   563,  1205,  -744,   851,  1619,
     571,  1551,  1552,   562,  1553,   498,   125,   592,   584,  1207,
    1564, -1091,   595,   596,  1740,  1628,  1709,   602,   603,   619,
    1711,   618,  1712,  1216,  1713,   455,   621,   628,   426,  1714,
     631,   632,  1337,   641,  1554,   642,  1271,   686,   684,  1341,
     687,   600,   388,   785,   388,   388,   388,   388,  1241,   122,
     131,   727,   696,   727,   697,   126,   698,   700,   795,  -129,
    1591,   128,    55,   709,   730,   826,   735,   810,   824,   659,
     828,   829,  1259,   835,   129,  1260,   852,  1261,   836,  1440,
     573,   720,  1441,   715,   223,  1225,   355,   856,   859,   600,
    1225,   590,   225,   223,   874,   873,   165,   726,   903,   928,
     223,  2004,   905,   841,   907,   906,   131,   908,   943,   910,
     426,   914,   223,   909,   116,  1225,  1244,   913,  1244,   694,
     932,   945,   125,   794,  2004,   933,  1865,   936,   948,  1301,
     937,  1717,   950,  2026,   900,   248,  1218,   951,   953,   253,
    1723,   426,   254,   955,   956,  1961,  1307,   694,   815,   815,
    1966,   957,   958,   964,   969,  1730,   970,   972,   552,   979,
     924,   712,  -769,   978,   560,   981,   982,   225,   985,   990,
     997,   131,  1225,   989,   999,  1308,  1003,  1005,   125,  1002,
    1008,  1429,  1009,   169,  1010,  1430,  1011,   169,  1012,  1465,
    1020,   684,   131,  1696,  1697,   961,   963,  1024,   255,  1991,
     681,  1028,  1408,  1029,  1031,  1431,   225,  1032,   225,  1040,
    1048,  1050,  1648,   683,  1022,   732,   733,   734,   685,  1049,
     131,  -750,  1089,  1099,  1109,  1113,   454,    63,    64,    65,
      66,    67,  1111,  1872,   225,   865,  1117,    72,   504,  1118,
     844,  1119,   844,   125,  1121,  1135,  1136,   223,  1410,  1517,
    1137,  1138,  1139,  1521,  1140,  1141,  2027,  1186,  1527,  1176,
    1190,  1192,  1188,  1193,   125,  1532,  1194,  1199,   116,  2041,
     727,  2048,  1203,  1206,  1214,  1212,   388,  1215,  1221,  1226,
     506,  1219,  1924,  1925,  1254,  1443,  1263,  1266,  1269,   681,
    1274,   122,   125,  -947,  1275,   625,  1285,   126,   455,   131,
    1286,   131,   683,   128,   633,   225,   638,   685,   720,  1287,
    1288,   645,  1960,  1409,  1963,  1735,   129,  1294,  1648,   720,
    1410,   225,   225,   663,  1289,  1295,  1290,  1546,  1296,  1291,
    1298,   983,   984,  1310,  1312,  1244,  1315,  1316,   165,  1318,
     992,  1327,  1648,  1328,  1648,   530,  1329,  1224,   169,  1334,
    1648,  1335,  1224, -1107,  1336,   682,  1171,   266,  1485,  1339,
    1340,  1390,  1068,   729,  1081,  1397,  1392,  1499,  1404,    14,
    1393,   125,  1424,   125,  1605,  1394,  1395,  1224,  1609,  1438,
    1407,  1926,  1439,   694,   116,  1616,   694,  1036,  1425,  1488,
    1444,  1446,  1452,  1448,   425,  1464,  1450,   422,  1105,   116,
     427,  1453,  1455,  2015,  1456,  1462,  1059,  1165,  1166,  1167,
    1168,  1169,  1170,   131,  1463,  1484,  1075,  1486,  1487,  1477,
    1489,  1494,  1507,   684,   373,   223,  1171,  1498,  1504,  1510,
    1515,  1511,  1514,  1547,  1224,  1516,  1520,  1525,  1526,  1548,
     116,   454,  1549,   186,    65,    66,    67,  1550,  1531,  1181,
     712,  1528,   408,  1519,  1522,   409,  1565,  1683,   645,  1580,
    1581,  1604,  1583,  1570,  1530,  1535,  1523,  1571,  1524,  1572,
    1537,  1538,   727,  1567,  1539,   225,   225,  1566,  1540,   457,
    1196,  1575,  1586,  1582,  1588,   125,  1584,  1596,  1592,  1551,
    1552,  1597,  1553,  1598,  1590,   720,   169,  1603,  1606,  1599,
     223,  1602,  1607,  1608,  1613,   116,  1610,  1614,  1611,  1612,
    1615,  1618,   684,   455,  1631,  1620,  1621,   682,  1125,   715,
     600,  1622,  1568,   131,  1629,  1625,   116,  1648,  1626,  1653,
    1642,  1681,   810,   810,  1563,  1686,  1666,   725,  1693,   223,
    1694,   223,  1698,  1703,  1563,  1704,  1706,  1841,  1710,  1728,
    2047,  1725,  1734,  1742,   116,  1743,  1839,  1838,  1845,  1851,
    1852,  1854,  1866,  1898,  1904,  1867,  1855,   223,  1864,  1856,
    1858,  1908,  1877,  1935,  1878,  1907,  1937,   694,  1913,  1943,
    1939,   388,  1951,  1952,  1959,  1953,  1958,  1962,  -377,  1967,
    1971,  1279,  1279,  1068,  1974,   125, -1109, -1109, -1109, -1109,
   -1109,   489,   490,   491,   492,   493,   494,   495,   496,  1558,
     497,  1972,  1975,  1977,  1210,   225,  1979,  1893,  1980,  1558,
    1983,  1690,   498,  1994,   841,  1695,   841,  2010,   720,   720,
    1220,   116,  1986,   116,  1992,   116,  1000,  1993,   223,  1999,
    1733,  2001,  1238,  2006,  2013,  2014,  2022,  1075,  2024,  2044,
     169,  2028,  2029,  2045,   223,   223,  1331,  2052,  2060,  2037,
    2053,  2063,  2061,  2064,  1396,  2009,   789,   228,   228,  1265,
     682,  1211,   784,  2023,  1491,  1627,   225,  1871,   786,   926,
    2021,   600,  1862,  1886,  1741,  1268,  1891,  1673,  2055,  1903,
    2032,   225,   225,  1654,  1860,  1350,  1342,  1419,  1475,  1546,
    1281,  1467,   654,  1299,  1466,   647,  1721,   721,  1956,  1988,
     900,  1045,  1981,  1123,  1399,  1333,  1563,  1389,  1906,     0,
    1644,     0,  1563,     0,  1563,     0,     0,   694,   373,   373,
     373,   615,   373,     0,     0,     0,     0,     0,  1853,     0,
     131,    14,     0,     0,     0,     0,  1563,   116,     0,     0,
    1082,     0,  1083,     0,     0,     0,     0,     0,  1326,     0,
       0,     0,  1330,     0,     0,     0,   169,   511,     0,     0,
     667,   447,     0,     0,     0,     0,     0,     0,  1103,     0,
     131,   169,     0,   225,     0,     0,     0,  1936,  1938,     0,
       0,  1558,     0,     0,     0,     0,     0,  1558,     0,  1558,
    1869,  1721,     0,     0,     0,  1547,     0,     0,   223,   223,
       0,  1548,   125,   454,  1549,   186,    65,    66,    67,  1550,
       0,  1558,   169,   131,  1068,  1068,  1068,  1068,  1068,  1068,
       0,     0,   131,  1675,  1068,     0,     0,  1563,     0,     0,
       0,     0,     0,     0,     0,   116,     0,     0,     0,  1191,
    1901,     0,   125,     0,     0,     0,     0,   116,     0,     0,
       0,  1551,  1552,     0,  1553,   645,  1202,  1506,     0,  1997,
     211,  1433,     0,   992,     0,     0,     0,     0,  1075,  1075,
    1075,  1075,  1075,  1075,     0,   455,   228,   169,  1075,     0,
       0,     0,    50,     0,  1701,   125,  1909,     0,   222,   222,
       0,     0,   682,     0,   125,     0,     0,     0,   169,   245,
       0,  1454,  1558,     0,  1457,  1901,     0,     0,     0,   457,
       0,   131,     0,     0,     0,     0,  1246,   131,  1246,   215,
     216,   217,   218,   219,   131,   245,   169,  1541,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   223,     0,
       0,   188,     0,     0,    91,     0,     0,    93,    94,  1846,
      95,   189,    97,  1546,   454,    63,    64,    65,    66,    67,
       0,     0,     0,  1505,     0,    72,   504,     0,     0,  1508,
       0,     0,   922,   922,     0,   107,     0,     0,     0,     0,
    1929,   682,     0,   125,     0,     0,     0,     0,     0,   125,
    1068,     0,  1068,     0,     0,    14,   125,     0,     0,   223,
       0,     0,     0,   169,     0,   169,   505,   169,   506,  1103,
    1314,     0,     0,   228,   223,   223,     0,     0,     0,     0,
       0,   507,   228,   508,   639,     0,   455,     0,     0,   228,
       0,     0,     0,     0,     0,  1542,  1543,     0,     0,     0,
       0,   228,     0,     0,  1075,     0,  1075,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1547,
       0,     0,     0,     0,   116,  1548,   131,   454,  1549,   186,
      65,    66,    67,  1550,     0,   348,     0,     0,     0,  2054,
       0,  1671,     0,     0,     0,     0,     0,  2062,     0,     0,
       0,     0,     0,     0,     0,  2065,     0,     0,  2066,     0,
       0,   694,     0,     0,   116,     0,   223,   222,     0,     0,
       0,     0,     0,     0,     0,  1551,  1552,     0,  1553,   169,
     131,     0,     0,     0,   694,     0,     0,     0,     0,     0,
       0,     0,  1068,   694,  1068,  1246,  1068,     0,   125,   455,
       0,  1068,  1630,     0,     0,  1632,     0,   116,  1705,  1442,
       0,     0,   116,     0,     0,   131,   116,   245,     0,   245,
     131,     0,  1546,     0,     0,     0,     0,     0,     0,  1998,
       0,     0,     0,     0,     0,   388,   228,     0,   600,  1546,
       0,   348,     0,     0,     0,   131,  1075,     0,  1075,     0,
    1075,  1827,   125,  1676,     0,  1075,     0,     0,  1834,     0,
       0,     0,     0,     0,    14,   348,     0,   348,     0,     0,
    1483,     0,     0,   348,     0,     0,   245,   169,     0,     0,
       0,    14,     0,     0,     0,   645,  1103,   125,     0,   169,
       0,     0,   125,     0,     0,     0,     0,     0,  1068,     0,
       0,   131,   131,   211,   222,   116,   116,   116,     0,     0,
       0,   116,     0,   222,     0,     0,     0,   125,   116,   922,
     222,   922,     0,   922,  1724,    50,     0,   922,  1547,   922,
     922,  1185,   222,     0,  1548,     0,   454,  1549,   186,    65,
      66,    67,  1550,   222,     0,  1547,     0,     0,     0,     0,
       0,  1548,  1075,   454,  1549,   186,    65,    66,    67,  1550,
       0,   866,   215,   216,   217,   218,   219,   245,     0,     0,
     245,     0,     0,   125,   125,     0,     0,   645,     0,     0,
       0,     0,     0,     0,  1551,  1552,     0,  1553,   451,     0,
      93,    94,     0,    95,   189,    97,     0,  1574,     0,     0,
       0,  1551,  1552,     0,  1553,     0,     0,     0,   455,     0,
       0,   211,     0,     0,   228,     0,     0,  1707,   107,  1546,
       0,   867,     0,     0,     0,   455,   245,     0,     0,   600,
       0,     0,     0,    50,  1716,     0,     0,  1882,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     348,     0,     0,     0,  1068,  1068,     0,     0,     0,     0,
     116,    14,     0,     0,     0,     0,     0,   222,     0,  1930,
     215,   216,   217,   218,   219,     0,  1827,  1827,     0,     0,
    1834,  1834,     0,     0,     0,     0,     0,     0,     0,   228,
       0,     0,   188,     0,   600,    91,     0,     0,    93,    94,
       0,    95,   189,    97,     0,   868,   169,     0,  1075,  1075,
       0,     0,     0,     0,   116,     0,     0,     0,     0,   245,
       0,   245,     0,     0,   888,  1547,   107,     0,   228,     0,
     228,  1548,     0,   454,  1549,   186,    65,    66,    67,  1550,
    1905,     0,     0,     0,     0,     0,   169,     0,     0,   116,
       0,     0,     0,  1916,   116,     0,   228,   888,     0,     0,
       0,     0,  1996,   512,   513,   514,   515,   516,   517,   518,
     519,   520,   521,   522,   523,   524,     0,     0,     0,   116,
       0,  1551,  1552,     0,  1553,  2011,     0,     0,     0,   169,
       0,     0,     0,     0,   169,     0,     0,     0,   169,     0,
     922,     0,     0,     0,     0,   455,     0,     0,   525,   526,
       0,     0,     0,     0,  1863,   245,   245,     0,     0,     0,
       0,     0,     0,     0,   245,     0,     0,   228,     0,     0,
    1976,     0,     0,     0,     0,   116,   116,     0,     0,     0,
       0,     0,     0,   228,   228,   222,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1916,     0,  1989,
       0,     0,     0,   541,   513,   514,   515,   516,   517,   518,
     519,   520,   521,   522,   523,   524,     0,     0,     0,     0,
       0,     0,     0,     0,   527,   528,     0,   169,   169,   169,
       0,     0,     0,   169,     0,     0,     0,     0,     0,     0,
     169,     0,     0,     0,     0,   469,   470,   471,   525,   526,
       0,     0,     0,     0,   992,     0,     0,     0,     0,     0,
     222,     0,     0,     0,     0,   472,   473,   992,   474,   475,
     476,   477,   478,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,     0,   497,   245,     0,     0,     0,     0,     0,   222,
       0,   222,     0,     0,   498,     0,   541,   513,   514,   515,
     516,   517,   518,   519,   520,   521,   522,   523,   524,     0,
       0,     0,     0,     0,   527,   528,     0,   222,   888,     0,
       0,   211,     0,   212,    40,   245,     0,   228,   228,     0,
       0,     0,   245,   245,   888,   888,   888,   888,   888,     0,
       0,   525,   526,    50,     0,     0,   888,   541,   513,   514,
     515,   516,   517,   518,   519,   520,   521,   522,   523,   524,
       0,     0,   245,     0,     0,     0,     0,   224,   224,     0,
       0,     0,   169,     0,     0,     0,     0,   699,   247,     0,
     215,   216,   217,   218,   219,     0,     0,     0,   222,     0,
       0,     0,   525,   526,     0,     0,     0,     0,   288,     0,
     351,     0,   245,     0,   222,   222,   808,     0,    93,    94,
       0,    95,   189,    97,     0,     0,     0,   527,   528,     0,
       0,     0,     0,     0,   499,     0,   169,     0,   245,   245,
       0,     0,     0,     0,     0,   290,   107,     0,   222,     0,
     809,     0,   111,     0,   245,     0,     0,     0,   211,     0,
       0,     0,     0,  1432,     0,     0,   245,     0,     0,     0,
       0,   169,     0,     0,   888,     0,   169,   228,   527,   528,
      50,     0,     0,     0,     0,     0,     0,     0,   593,   245,
     837,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   169,     0,     0,     0,     0,     0,     0,     0,   245,
       0,     0,     0,   245,     0,     0,   586,   215,   216,   217,
     218,   219,   587,     0,   245, -1109, -1109, -1109, -1109, -1109,
    1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,   228,   188,
       0,   934,    91,   342,     0,    93,    94,     0,    95,   189,
      97,  1171,     0,   228,   228,     0,     0,   169,   169,     0,
       0,     0,     0,   346,     0,     0,     0,     0,   222,   222,
       0,     0,     0,   107,   347,     0,     0,     0,     0,     0,
       0,     0,   245,     0,     0,     0,   245,     0,   245,     0,
       0,     0,     0,     0,     0,     0,   224,     0,     0,     0,
       0,     0,     0,     0,   888,   888,   888,   888,   888,   888,
     222,     0,     0,   888,   888,   888,   888,   888,   888,   888,
     888,   888,   888,   888,   888,   888,   888,   888,   888,   888,
     888,   888,   888,   888,   888,   888,   888,   888,   888,   888,
     888,     0,   351,     0,   351,   228,   469,   470,   471,     0,
       0,     0,     0,     0,     0,    34,    35,    36,     0,   888,
       0,     0,     0,     0,     0,     0,   472,   473,   213,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,     0,   497,     0,   245,     0,   245,     0,     0,
       0,   351,     0,     0,     0,   498,     0,     0,   222,     0,
       0,  1276,  1277,  1278,   211,     0,     0,     0,     0,     0,
       0,     0,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,   224,   220,   245,    50,     0,   245,     0,
      89,    90,   224,     0,     0,     0,     0,     0,     0,   224,
       0,     0,     0,     0,    99,   245,   245,   245,   245,   245,
     245,   224,     0,   222,     0,   245,     0,     0,   104,   222,
       0,     0,   224,   215,   216,   217,   218,   219,     0,     0,
       0,     0,     0,     0,   222,   222,     0,   888,     0,     0,
       0,     0,   351,     0,     0,   351,     0,   245,     0,     0,
       0,    93,    94,   245,    95,   189,    97,   888,     0,   888,
       0,     0,     0,  1013,   513,   514,   515,   516,   517,   518,
     519,   520,   521,   522,   523,   524,     0,     0,     0,   107,
       0,     0,     0,   888,     0,     0,     0,     0,     0,     0,
    1412,  1013,   513,   514,   515,   516,   517,   518,   519,   520,
     521,   522,   523,   524,     0,   247,     0,     0,   525,   526,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   245,
     245,     0,     0,   245,     0,     0,   222,   469,   470,   471,
       0,     0,     0,     0,     0,     0,   525,   526,     0,     0,
       0,     0,     0,     0,     0,   866,   224,   472,   473,     0,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,     0,   497,     0,     0,     0,     0,     0,
       0,   245,     0,   245,   527,   528,   498,     0,     0,     0,
       0,     0,     0,     0,   351,   211,   869,     0,     0,     0,
       0,     0,     0,   895,     0,   867,     0,     0,     0,     0,
       0,     0,   527,   528,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,   245,     0,     0,   245,
       0,     0,     0,     0,     0,   888,   895,   888,     0,   888,
       0,     0,     0,     0,   888,   222,     0,  1014,   888,     0,
     888,     0,     0,   888,   215,   216,   217,   218,   219,     0,
       0,     0,     0,     0,     0,     0,   245,   245,     0,     0,
     245,   288,     0,     0,     0,   699,   188,   245,     0,    91,
       0,     0,    93,    94,     0,    95,   189,    97,     0,  1332,
     351,   351,     0,     0,     0,     0,     0,     0,     0,   351,
       0,     0,     0,     0,     0,     0,     0,     0,   290,     0,
     107,     0,     0,     0,     0,     0,   499,     0,  1025,     0,
       0,   211,     0,   245,   224,   245,     0,   245,     0,   211,
       0,     0,   245,     0,   222,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,   245,     0,
       0,    50,   888,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   245,   245,     0,     0,     0,     0,
       0,     0,   245,     0,   245,     0,     0,     0,     0,   586,
     215,   216,   217,   218,   219,   587,     0,     0,   215,   216,
     217,   218,   219,     0,     0,     0,   245,     0,   245,   224,
       0,     0,   188,     0,   245,    91,   342,     0,    93,    94,
       0,    95,   189,    97,  1832, -1108,    93,    94,  1833,    95,
     189,    97,     0,     0,     0,     0,   346,     0,     0,   245,
       0,     0,  1074,     0,     0,     0,   107,   347,   224,     0,
     224,     0,     0,     0,   107,  1670,   888,   888,   888,     0,
       0,     0,     0,   888,     0,   245,     0,     0,     0,     0,
       0,   245,     0,   245,     0,     0,   224,   895,     0,     0,
    1116,     0,     0,     0,     0,     0,     0,   351,   351,     0,
       0,     0,     0,   895,   895,   895,   895,   895,     0,   469,
     470,   471,     0,     0,     0,   895,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   472,
     473,  1175,   474,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,     0,   497,   224,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   498,     0,
       0,  1198,     0,   224,   224,     0,     0,     0,     0,     0,
     288,     0,     0,     0,   245,     0,     0,     0,     0,     0,
       0,   226,   226,   351,   211,     0,     0,   245,  1198,     0,
       0,   245,   251,     0,     0,   245,   245,   224,     0,   351,
       0,     0,     0,     0,     0,     0,    50,   290,     0,     0,
     245,   351,     0,     0,     0,     0,   888,     0,     0,     0,
     211,     0,     0,   895,     0,     0,     0,   888,     0,     0,
       0,     0,     0,   888,     0,     0,     0,   888,  1255,     0,
       0,     0,    50,   215,   216,   217,   218,   219,     0,     0,
       0,     0,     0,     0,   351,     0,     0,     0,     0,     0,
       0,     0,   247,     0,   245,     0,     0,     0,   406,     0,
       0,    93,    94,  1074,    95,   189,    97,     0,   586,   215,
     216,   217,   218,   219,   587,     0,     0,     0,     0,     0,
     965,   245,     0,   245,     0,     0,     0,     0,     0,   107,
       0,   188,     0,   407,    91,   342,     0,    93,    94,   888,
      95,   189,    97,     0,     0,     0,     0,   224,   224,     0,
       0,     0,   245,     0,     0,   346,     0,   351,     0,     0,
       0,   351,     0,   869,     0,   107,   347,     0,     0,   245,
       0,     0,     0,     0,     0,     0,     0,     0,   245,     0,
       0,     0,     0,   895,   895,   895,   895,   895,   895,   224,
       0,   245,   895,   895,   895,   895,   895,   895,   895,   895,
     895,   895,   895,   895,   895,   895,   895,   895,   895,   895,
     895,   895,   895,   895,   895,   895,   895,   895,   895,   895,
     469,   470,   471,     0,     0,     0,     0,     0,     0,     0,
     226,     0,     0,     0,     0,     0,     0,     0,   895,     0,
     472,   473,     0,   474,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,     0,   497,     0,     0,
     351,     0,   351,     0,     0,     0,     0,     0,     0,   498,
       0,     0,     0,     0,     0,     0,     0,   224,     0,     0,
       0,     0,     0,     0,     0,   541,   513,   514,   515,   516,
     517,   518,   519,   520,   521,   522,   523,   524,     0,     0,
     351,     0,     0,   351,     0,  1013,   513,   514,   515,   516,
     517,   518,   519,   520,   521,   522,   523,   524,     0,     0,
       0,   211,     0,     0,  1074,  1074,  1074,  1074,  1074,  1074,
     525,   526,   224,     0,  1074,     0,     0,     0,   224,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
     525,   526,     0,   224,   224,     0,   895,   226,     0,     0,
       0,     0,   351,     0,     0,     0,   226,     0,   351,  1669,
       0,     0,     0,   226,     0,     0,   895,     0,   895,     0,
     215,   216,   217,   218,   219,   226,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   251,     0,     0,     0,
       0,   996,   895,     0,     0,     0,   527,   528,    93,    94,
       0,    95,   189,    97,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   469,   470,   471,   527,   528,     0,     0,
       0,     0,     0,     0,   351,   351,   107,  1670,     0,     0,
       0,     0,  1545,   472,   473,   224,   474,   475,   476,   477,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,     0,
     497,     0,     0,     0,     0,     0,     0,     0,     0,   251,
       0,     0,   498,     0,     0,     0,     0,     0,     0,     0,
       0,  1364,     0,     0,     0,     0,     0,     0,     0,     0,
    1074,     0,  1074,     0,     0,     0,     0,     0,     0,   875,
     876,     0,     0,     0,     0,   877,     0,   878,     0,     0,
     226,     0,     0,     0,     0,     0,     0,     0,     0,   879,
       0,     0,     0,     0,     0,     0,     0,    34,    35,    36,
     211,   351,     0,     0,   351,     0,     0,     0,     0,     0,
     213,     0,     0,     0,   895,     0,   895,     0,   895,     0,
       0,     0,    50,   895,   224,     0,     0,   895,     0,   895,
       0,     0,   895,     0,     0,     0,     0,   896,     0,     0,
       0,   351,     0,     0,     0,     0,  1652,     0,     0,  1665,
       0,     0,   351,     0,     0,     0,     0,     0,   880,   881,
     882,   883,   884,   885,     0,    81,    82,    83,    84,    85,
     896,     0,     0,     0,  1025,     0,   220,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,     0,     0,     0,    99,   897,     0,     0,
       0,     0,  1074,     0,  1074,   886,  1074,     0,     0,     0,
     104,  1074,     0,   224,     0,   107,   887,     0,     0,     0,
       0,     0,     0,   351,     0,     0,     0,     0,     0,     0,
     927,   895,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1731,  1732,     0,     0,   351,     0,     0,
       0,     0,     0,  1665,     0,     0,     0,     0,   226,     0,
     469,   470,   471,     0,     0,     0,     0,     0,     0,     0,
       0,   351,     0,   351,     0,     0,     0,     0,     0,   351,
     472,   473,     0,   474,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,     0,   497,  1074,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   498,
     469,   470,   471,     0,     0,   895,   895,   895,     0,     0,
       0,     0,   895,   226,  1880,     0,   351,     0,     0,     0,
     472,   473,  1665,   474,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,     0,   497,     0,     0,
       0,     0,   226,     0,   226,     0,     0,     0,     0,   498,
    1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,
    1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,
     226,   896,     0,     0,  1655,     0,     0,     0,     0,     0,
       0,     0,     0,  1171,     0,     0,     0,   896,   896,   896,
     896,   896,     0,     0,     0,     0,     0,     0,     0,   896,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   351,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1051,   351,     0,   211,     0,   351,     0,     0,     0,
       0,  1104,     0,     0,  1074,  1074,     0,     0,     0,     0,
       0,   226,     0,     0,     0,  1931,    50,  1128,  1129,  1130,
    1131,  1132,     0,     0,     0,   895,     0,   226,   226,  1142,
     288,     0,     0,     0,     0,     0,   895,     0,     0,  1656,
       0,     0,   895,     0,     0,     0,   895,     0,     0,     0,
       0,  1189,  1657,   215,   216,   217,   218,   219,  1658,     0,
       0,   251,     0,     0,     0,     0,     0,   290,     0,   351,
       0,     0,     0,     0,     0,   188,     0,     0,    91,  1659,
     211,    93,    94,     0,    95,  1660,    97,   896,     0,     0,
       0,     0,     0,     0,     0,     0,   351,     0,   351,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,   107,
    -424,     0,     0,     0,     0,     0,     0,     0,   895,   454,
     185,   186,    65,    66,    67,     0,   251,     0,     0,     0,
       0,  2008,     0,     0,     0,     0,     0,     0,   586,   215,
     216,   217,   218,   219,   587,     0,     0,  1242,  1652,     0,
       0,     0,     0,   351,     0,     0,     0,     0,     0,     0,
       0,   188,     0,     0,    91,   342,   351,    93,    94,     0,
      95,   189,    97,     0,     0,     0,     0,     0,     0,     0,
       0,   226,   226,     0,     0,   346,     0,     0,     0,     0,
       0,   455,     0,     0,     0,   107,   347,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   896,   896,   896,
     896,   896,   896,   251,     0,     0,   896,   896,   896,   896,
     896,   896,   896,   896,   896,   896,   896,   896,   896,   896,
     896,   896,   896,   896,   896,   896,   896,   896,   896,   896,
     896,   896,   896,   896,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   896,   497,     0,     0,     0,  1132,  1345,     0,
       0,  1345,     0,     0,     0,   498,  1358,  1361,  1362,  1363,
    1365,  1366,  1367,  1368,  1369,  1370,  1371,  1372,  1373,  1374,
    1375,  1376,  1377,  1378,  1379,  1380,  1381,  1382,  1383,  1384,
    1385,  1386,  1387,  1388,   469,   470,   471,     0,     0,     0,
       0,   226,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1398,     0,   472,   473,     0,   474,   475,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
       0,   497,     0,     0,   211,     0,   212,    40,     0,     0,
       0,   211,     0,   498,     0,     0,   251,     0,     0,   211,
       0,     0,   226,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,   226,   226,     0,
     896,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     896,     0,   896,   215,   216,   217,   218,   219,     0,     0,
     215,   216,   217,   218,   219,     0,     0,     0,   215,   216,
     217,   218,   219,     0,     0,     0,   896,     0,     0,   808,
       0,    93,    94,     0,    95,   189,    97,     0,    93,    94,
    1495,    95,   189,    97,     0,     0,    93,    94,     0,    95,
     189,    97,     0,     0,     0,     0,     0,     0,     0,   107,
    1512,     0,  1513,   840,     0,   111,   107,   735,     0,   226,
     469,   470,   471,     0,   107,  1022,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1262,  1533,     0,     0,     0,
     472,   473,     0,   474,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,     0,   497,   469,   470,
     471,     0,     0,     0,     0,     0,     0,     0,     0,   498,
       0,     0,     0,     0,     0,     0,     0,     0,   472,   473,
       0,   474,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,     0,   497,     0,     0,     0,     0,
       0,     0,   211,     0,   959,     0,   960,   498,   896,     0,
     896,     0,   896,     0,     0,     0,     0,   896,   251,     0,
       0,   896,     0,   896,    50,     0,   896,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   278,   279,     0,   280,   281,     0,     0,   282,   283,
     284,   285,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   215,   216,   217,   218,   219,   286,   287,  1634,     0,
    1635,     0,  1636,     0,     0,   544,     0,  1637,     0,     0,
       0,  1639,     0,  1640,     0,     0,  1641,     0,     0,    93,
      94,  1273,    95,   189,    97,   289,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   251,     0,   291,
     292,   293,   294,   295,   296,   297,     0,   107,     0,   211,
       0,   212,    40,     0,     0,   896,     0,     0,     0,     0,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,    50,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,     0,   333,     0,   778,
     335,   336,   337,     0,     0,     0,   338,   597,   215,   216,
     217,   218,   219,   598,     0,  1726,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     5,     6,     7,     8,     9,
     599,     0,     0,     0,     0,    10,    93,    94,     0,    95,
     189,    97,   343,     0,   344,     0,     0,   345,     0,   896,
     896,   896,     0,     0,     0,     0,   896,     0,     0,     0,
     716,     0,     0,     0,   107,  1885,     0,     0,   779,     0,
     111,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,  1873,
    1874,  1875,     0,    43,     0,     0,  1879,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,     0,   184,   185,   186,    65,    66,    67,     0,     0,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     187,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,   211,     0,     0,     0,     0,    87,
       0,     0,     0,     0,   188,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   189,    97,    50,   717,     0,    99,
       0,     0,   100,     0,   915,   916,     0,     0,   101,   896,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   190,
     896,     0,     0,     0,   111,   112,   896,   113,   114,     0,
     896,     0,     0,   215,   216,   217,   218,   219,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       5,     6,     7,     8,     9,     0,     0,     0,   917,  1978,
      10,    93,    94,     0,    95,   189,    97,     0,     0,  1940,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
    1950,     0,     0,     0,     0,     0,  1955,     0,     0,   107,
    1957,     0,     0,     0,     0,    14,     0,    15,    16,     0,
       0,     0,   896,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,    56,
      57,    58,  2000,    59,  -203,    60,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,    88,
      89,    90,    91,    92,     0,    93,    94,     0,    95,    96,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,   102,     0,   103,     0,   104,   105,
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
      53,    54,    55,    56,    57,    58,     0,    59,     0,    60,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,    88,    89,    90,    91,    92,     0,    93,
      94,     0,    95,    96,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
     103,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,  1208,   111,   112,     0,   113,   114,     5,     6,
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
       0,    59,     0,    60,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,    88,    89,    90,
      91,    92,     0,    93,    94,     0,    95,    96,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,   102,     0,   103,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,  1413,   111,   112,     0,
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
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
     701,   111,   112,     0,   113,   114,     5,     6,     7,     8,
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
       0,     0,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,     0,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   189,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     108,     0,   109,   110,  1015,   111,   112,     0,   113,   114,
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
      57,    58,     0,    59,  -203,     0,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,     0,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,   188,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   189,
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
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   188,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   189,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,  1174,   111,   112,     0,   113,   114,     5,     6,
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
       0,    59,     0,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   188,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   189,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,  1222,   111,   112,     0,
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
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
    1251,   111,   112,     0,   113,   114,     5,     6,     7,     8,
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
       0,     0,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,     0,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   189,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     108,     0,   109,   110,  1309,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,  1311,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,     0,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,     0,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,   188,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   189,
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
       0,    49,  1496,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   188,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   189,    97,    98,     0,     0,    99,     0,
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
       0,    59,     0,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   188,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   189,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,  1643,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,  -295,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
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
       0,     0,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,     0,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   189,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     108,     0,   109,   110,  1876,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,  1927,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,     0,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,     0,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,   188,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   189,
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
       0,     0,    43,    44,    45,    46,     0,    47,  1970,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   188,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   189,    97,    98,     0,     0,    99,     0,
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
       0,    59,     0,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   188,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   189,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,  1987,   111,   112,     0,
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
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
    1990,   111,   112,     0,   113,   114,     5,     6,     7,     8,
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
       0,     0,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,     0,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   189,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     108,     0,   109,   110,  2007,   111,   112,     0,   113,   114,
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
      57,    58,     0,    59,     0,     0,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,     0,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,   188,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   189,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   108,     0,   109,   110,  2056,   111,
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
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   188,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   189,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,  2057,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,   569,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,     0,    61,    62,   185,   186,    65,    66,
      67,     0,    68,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   188,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   189,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,   853,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
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
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,  1106,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,     0,    61,    62,   185,   186,    65,    66,    67,     0,
      68,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   189,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     108,     0,   109,   110,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,  1720,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,     0,    61,    62,   185,   186,
      65,    66,    67,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,   188,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   189,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   108,     0,   109,   110,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,  1868,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,   185,   186,    65,    66,    67,     0,    68,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   188,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   189,    97,     0,     0,     0,    99,     0,
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
       0,    59,     0,     0,    61,    62,   185,   186,    65,    66,
      67,     0,    68,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   188,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   189,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   356,   421,    13,     0,
       0,     0,     0,     0,     0,     0,     0,   791,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
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
     104,   105,   106,     0,     0,   107,   108,     0,     0,     0,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     356,     0,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
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
     190,     0,   357,     0,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   356,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
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
     106,     0,     0,   107,   190,     0,     0,   848,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,  1148,     0,    10,  1149,  1150,  1151,  1152,  1153,
    1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,
    1164,  1165,  1166,  1167,  1168,  1169,  1170,     0,     0,  1239,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1171,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
       0,   184,   185,   186,    65,    66,    67,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   187,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   188,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   189,    97,     0,  1240,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   190,     0,
       0,     0,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   356,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   791,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
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
       0,   107,   190,     0,     0,     0,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   356,   421,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
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
     104,   105,   106,     0,     0,   107,   108,   469,   470,   471,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   472,   473,     0,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,     0,   497,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,   498,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,   202,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,     0,   184,   185,   186,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   187,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   189,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,  1303,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     190,     0,     0,     0,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,  1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,
    1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,     0,     0,   238,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1171,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
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
     106,     0,     0,   107,   190,   469,   470,   471,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   472,   473,     0,   474,   475,
     476,   477,   478,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,     0,   497,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,   498,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
       0,   184,   185,   186,    65,    66,    67,     0,     0,    69,
      70,     0,     0,     0,     0,     0,     0,     0,     0,   187,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,   188,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   189,    97,     0,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,  1688,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   190,     0,
     273,   470,   471,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
     472,   473,     0,   474,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,     0,   497,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,   498,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
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
       0,   107,   190,     0,   276,     0,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   421,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
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
     104,   105,   106,     0,     0,   107,   108,   469,   470,   471,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   472,   473,     0,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,     0,   497,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,   498,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,     0,   184,   185,   186,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   187,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   189,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,  1689,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     190,   567,     0,     0,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   356,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
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
     106,     0,     0,   107,   190,     0,     0,     0,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,  1153,  1154,  1155,  1156,  1157,
    1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,   746,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1171,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
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
       0,     0,   104,   105,   106,     0,     0,   107,   190,     0,
       0,     0,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
       0,   497,     0,   791,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   498,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
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
       0,   107,   190,     0,     0,     0,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,  1150,  1151,  1152,  1153,  1154,  1155,  1156,
    1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
    1167,  1168,  1169,  1170,     0,     0,     0,   830,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1171,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
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
     104,   105,   106,     0,     0,   107,   190,     0,     0,     0,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,     0,   497,     0,
       0,   832,     0,     0,     0,     0,     0,     0,     0,     0,
     498,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
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
     190,     0,     0,     0,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
       0,   497,     0,     0,     0,  1300,     0,     0,     0,     0,
       0,     0,     0,   498,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
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
     106,     0,     0,   107,   190,     0,     0,     0,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   356,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
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
       0,     0,   104,   105,   106,     0,     0,   107,  1427,   469,
     470,   471,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   472,
     473,  1500,   474,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,     0,   497,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,   498,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,     0,   184,   185,   186,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   187,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   188,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   189,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,  1501,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   190,   469,   470,   471,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
     857,     0,    10,   472,   473,     0,   474,   475,   476,   477,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,     0,
     497,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,   498,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,   661,    39,    40,     0,   858,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,   184,
     185,   186,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   187,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   188,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   189,    97,     0,   278,   279,    99,   280,   281,   100,
       0,   282,   283,   284,   285,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   190,     0,     0,   286,
     287,   111,   112,     0,   113,   114,  1152,  1153,  1154,  1155,
    1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,
    1166,  1167,  1168,  1169,  1170,     0,     0,     0,   289,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1171,     0,
       0,     0,   291,   292,   293,   294,   295,   296,   297,     0,
       0,     0,   211,     0,   212,    40,     0,     0,     0,     0,
       0,     0,     0,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,    50,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   211,
     333,     0,   334,   335,   336,   337,     0,     0,     0,   338,
     597,   215,   216,   217,   218,   219,   598,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,   278,   279,     0,
     280,   281,     0,   599,   282,   283,   284,   285,     0,    93,
      94,     0,    95,   189,    97,   343,     0,   344,     0,     0,
     345,     0,   286,   287,     0,   288,     0,     0,   215,   216,
     217,   218,   219,     0,     0,     0,     0,   107,     0,     0,
       0,   779,     0,   111,     0,     0,     0,     0,     0,     0,
     188,   289,     0,    91,    92,     0,    93,    94,     0,    95,
     189,    97,   290,     0,     0,   291,   292,   293,   294,   295,
     296,   297,     0,     0,     0,   211,     0,     0,     0,     0,
       0,     0,     0,     0,   107,     0,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,    50,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,     0,   333,     0,     0,   335,   336,   337,     0,
       0,     0,   338,   339,   215,   216,   217,   218,   219,   340,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   341,     0,     0,    91,
     342,     0,    93,    94,     0,    95,   189,    97,   343,     0,
     344,     0,     0,   345,   278,   279,     0,   280,   281,     0,
     346,   282,   283,   284,   285,     0,     0,     0,     0,     0,
     107,   347,     0,     0,     0,  1847,     0,     0,     0,   286,
     287,     0,   288,   473,     0,   474,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   289,   497,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   290,
       0,   498,   291,   292,   293,   294,   295,   296,   297,     0,
       0,     0,   211,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,    50,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,     0,
     333,     0,     0,   335,   336,   337,     0,     0,     0,   338,
     339,   215,   216,   217,   218,   219,   340,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   341,     0,     0,    91,   342,     0,    93,
      94,     0,    95,   189,    97,   343,     0,   344,     0,     0,
     345,   278,   279,     0,   280,   281,     0,   346,   282,   283,
     284,   285,     0,     0,     0,     0,     0,   107,   347,     0,
       0,     0,  1922,     0,     0,     0,   286,   287,     0,   288,
       0,     0,   474,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   289,   497,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   290,     0,   498,   291,
     292,   293,   294,   295,   296,   297,     0,     0,     0,   211,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,    50,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,     0,   333,     0,   334,
     335,   336,   337,     0,     0,     0,   338,   339,   215,   216,
     217,   218,   219,   340,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     341,     0,     0,    91,   342,     0,    93,    94,     0,    95,
     189,    97,   343,     0,   344,     0,     0,   345,   278,   279,
       0,   280,   281,     0,   346,   282,   283,   284,   285,     0,
       0,     0,     0,     0,   107,   347,     0,     0,     0,     0,
       0,     0,     0,   286,   287,     0,   288,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,     0,   497,     0,     0,     0,     0,
       0,     0,   289,     0,     0,     0,     0,   498,     0,     0,
       0,     0,     0,   290,     0,     0,   291,   292,   293,   294,
     295,   296,   297,     0,     0,     0,   211,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,    50,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,     0,   333,     0,     0,   335,   336,   337,
       0,     0,     0,   338,   339,   215,   216,   217,   218,   219,
     340,     0,     0,     0,     0,     0,     0,     0,   211,     0,
       0,     0,     0,     0,     0,     0,     0,   341,     0,     0,
      91,   342,     0,    93,    94,     0,    95,   189,    97,   343,
      50,   344,     0,     0,   345,     0,   278,   279,     0,   280,
     281,   346,  1647,   282,   283,   284,   285,     0,     0,     0,
       0,   107,   347,  1656,     0,     0,     0,     0,     0,     0,
       0,   286,   287,     0,   288,     0,  1657,   215,   216,   217,
     218,   219,  1658,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   188,
     289,     0,    91,    92,     0,    93,    94,     0,    95,  1660,
      97,   290,     0,     0,   291,   292,   293,   294,   295,   296,
     297,     0,     0,     0,   211,     0,     0,     0,     0,     0,
       0,     0,     0,   107,     0,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,    50,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,     0,   333,     0,     0,   335,   336,   337,     0,     0,
       0,   338,   339,   215,   216,   217,   218,   219,   340,     0,
       0,   211,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1115,     0,     0,     0,   341,     0,     0,    91,   342,
       0,    93,    94,    50,    95,   189,    97,   343,     0,   344,
       0,     0,   345,  1746,  1747,  1748,  1749,  1750,     0,   346,
    1751,  1752,  1753,  1754,     0,     0,     0,     0,     0,   107,
     347,     0,     0,     0,     0,     0,     0,  1755,  1756,  1757,
     215,   216,   217,   218,   219, -1109, -1109, -1109, -1109,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   188,   497,     0,    91,     0,  1758,    93,    94,
       0,    95,   189,    97,     0,   498,     0,     0,     0,     0,
       0,  1759,  1760,  1761,  1762,  1763,  1764,  1765,     0,     0,
       0,   211,     0,     0,     0,     0,   107,     0,     0,     0,
       0,     0,  1766,  1767,  1768,  1769,  1770,  1771,  1772,  1773,
    1774,  1775,  1776,    50,  1777,  1778,  1779,  1780,  1781,  1782,
    1783,  1784,  1785,  1786,  1787,  1788,  1789,  1790,  1791,  1792,
    1793,  1794,  1795,  1796,  1797,  1798,  1799,  1800,  1801,  1802,
    1803,  1804,  1805,  1806,  1807,     0,     0,     0,  1808,  1809,
     215,   216,   217,   218,   219,     0,  1810,  1811,  1812,  1813,
    1814,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1815,  1816,  1817,     0,   211,     0,    93,    94,
       0,    95,   189,    97,  1818,     0,  1819,  1820,     0,  1821,
       0,     0,     0,     0,     0,     0,  1822,  1823,    50,  1824,
       0,  1825,  1826,     0,   278,   279,   107,   280,   281,     0,
       0,   282,   283,   284,   285,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   286,
     287,     0,     0,     0,     0,   215,   216,   217,   218,   219,
       0,  1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,
    1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,   289,     0,
     367,     0,     0,    93,    94,     0,    95,   189,    97,     0,
       0,  1171,   291,   292,   293,   294,   295,   296,   297,     0,
       0,     0,   211,     0,     0,     0,     0,     0,     0,     0,
       0,   107,     0,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,    50,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   211,
     333,     0,   334,   335,   336,   337,     0,     0,     0,   338,
     597,   215,   216,   217,   218,   219,   598,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,   278,   279,     0,
     280,   281,     0,   599,   282,   283,   284,   285,     0,    93,
      94,     0,    95,   189,    97,   343,     0,   344,     0,     0,
     345,     0,   286,   287,     0,     0,     0,     0,   215,   216,
     217,   218,   219,     0,     0,     0,     0,   107,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   289,     0,   917,     0,     0,    93,    94,     0,    95,
     189,    97,     0,     0,     0,   291,   292,   293,   294,   295,
     296,   297,     0,     0,     0,   211,     0,     0,     0,     0,
       0,     0,     0,     0,   107,     0,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,    50,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   211,   333,     0,  1356,   335,   336,   337,     0,
       0,     0,   338,   597,   215,   216,   217,   218,   219,   598,
       0,     0,     0,     0,    50,     0,     0,     0,     0,     0,
     278,   279,     0,   280,   281,     0,   599,   282,   283,   284,
     285,     0,    93,    94,     0,    95,   189,    97,   343,     0,
     344,     0,     0,   345,     0,   286,   287,     0,     0,     0,
       0,   215,   216,   217,   218,   219,     0,     0,     0,     0,
     107,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   289,     0,     0,     0,     0,    93,
      94,     0,    95,   189,    97,     0,     0,     0,   291,   292,
     293,   294,   295,   296,   297,     0,     0,     0,   211,     0,
       0,     0,     0,     0,     0,     0,     0,   107,     0,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
      50,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,     0,   333,     0,     0,   335,
     336,   337,     0,     0,     0,   338,   597,   215,   216,   217,
     218,   219,   598,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   599,
       0,     0,     0,     0,     0,    93,    94,     0,    95,   189,
      97,   343,     0,   344,     0,     0,   345,   469,   470,   471,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   107,     0,     0,     0,   472,   473,     0,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,     0,   497,   469,   470,   471,     0,     0,
       0,     0,     0,     0,     0,     0,   498,     0,     0,     0,
       0,     0,     0,     0,     0,   472,   473,     0,   474,   475,
     476,   477,   478,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,     0,   497,   469,   470,   471,     0,     0,     0,     0,
       0,     0,     0,     0,   498,     0,     0,     0,     0,     0,
       0,     0,     0,   472,   473,     0,   474,   475,   476,   477,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,     0,
     497,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   498,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   469,   470,   471,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   472,   473,   583,   474,   475,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
       0,   497,   469,   470,   471,     0,     0,     0,     0,     0,
       0,     0,     0,   498,     0,     0,     0,     0,     0,     0,
       0,     0,   472,   473,   585,   474,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,     0,   497,
     469,   470,   471,     0,     0,     0,     0,     0,     0,     0,
       0,   498,     0,     0,     0,     0,     0,     0,     0,     0,
     472,   473,   604,   474,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,  1100,   497,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   498,
   -1109, -1109, -1109, -1109,  1158,  1159,  1160,  1161,  1162,  1163,
    1164,  1165,  1166,  1167,  1168,  1169,  1170,     0,    29,     0,
       0,     0,     0,     0,     0,     0,    34,    35,    36,   211,
    1171,   212,    40,   608,     0,     0,     0,     0,     0,   213,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   214,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     822,     0,     0,     0,     0,     0,  1101,    75,   215,   216,
     217,   218,   219,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   220,     0,     0,     0,     0,
     188,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     189,    97,     0,   875,   876,    99,     0,     0,   845,   877,
       0,   878,     0,     0,     0,     0,     0,     0,     0,   104,
       0,     0,     0,   879,   107,   221,     0,     0,     0,     0,
     111,    34,    35,    36,   211,     0,     0,     0,     0,   469,
     470,   471,     0,     0,   213,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,   472,
     473,     0,   474,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,     0,   497,     0,     0,     0,
       0,     0,   880,   881,   882,   883,   884,   885,   498,    81,
      82,    83,    84,    85,     0,     0,     0,  1053,  1054,     0,
     220,     0,     0,     0,     0,   188,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   189,    97,  1055,     0,     0,
      99,     0,     0,     0,     0,  1056,  1057,  1058,   211,   886,
       0,     0,     0,     0,   104,     0,     0,     0,  1059,   107,
     887,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,    29,     0,     0,     0,   553,     0,     0,     0,
      34,    35,    36,   211,     0,   212,    40,     0,     0,     0,
       0,     0,     0,   213,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,  1060,  1061,  1062,  1063,
    1064,  1065,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   214,     0,  1066,     0,     0,     0,     0,   188,
       0,     0,    91,    92,     0,    93,    94,     0,    95,   189,
      97,    75,   215,   216,   217,   218,   219,     0,    81,    82,
      83,    84,    85,  1067,     0,     0,     0,     0,     0,   220,
       0,     0,     0,   107,   188,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   189,    97,    29,     0,     0,    99,
       0,     0,     0,     0,    34,    35,    36,   211,     0,   212,
      40,     0,     0,   104,     0,     0,     0,   213,   107,   221,
       0,     0,   624,     0,   111,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   214,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   644,    75,   215,   216,   217,   218,
     219,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,   220,     0,     0,     0,     0,   188,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   189,    97,
      29,  1044,     0,    99,     0,     0,     0,     0,    34,    35,
      36,   211,     0,   212,    40,     0,     0,   104,     0,     0,
       0,   213,   107,   221,     0,     0,     0,     0,   111,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     214,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
     215,   216,   217,   218,   219,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,   220,     0,     0,
       0,     0,   188,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   189,    97,    29,     0,     0,    99,     0,     0,
       0,     0,    34,    35,    36,   211,     0,   212,    40,     0,
       0,   104,     0,     0,     0,   213,   107,   221,     0,     0,
       0,     0,   111,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   214,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1201,    75,   215,   216,   217,   218,   219,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,   220,     0,     0,     0,     0,   188,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   189,    97,    29,     0,
       0,    99,     0,     0,     0,     0,    34,    35,    36,   211,
       0,   212,    40,     0,     0,   104,     0,     0,     0,   213,
     107,   221,     0,     0,     0,     0,   111,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   214,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,   215,   216,
     217,   218,   219,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   220,     0,     0,     0,     0,
     188,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     189,    97,     0,     0,     0,    99,     0,     0,   469,   470,
     471,     0,     0,     0,     0,     0,     0,     0,     0,   104,
       0,     0,     0,     0,   107,   221,     0,     0,   472,   473,
     111,   474,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,     0,   497,   469,   470,   471,     0,
       0,     0,     0,     0,     0,     0,     0,   498,     0,     0,
       0,     0,     0,     0,     0,     0,   472,   473,     0,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,     0,   497,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   498,   469,   470,   471,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   949,   472,   473,     0,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,     0,   497,   469,   470,   471,     0,     0,     0,
       0,     0,     0,     0,     0,   498,     0,     0,     0,     0,
       0,     0,     0,  1030,   472,   473,     0,   474,   475,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
       0,   497,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   498,   469,   470,   471,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1084,   472,   473,     0,   474,   475,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
       0,   497,  1145,  1146,  1147,     0,     0,     0,     0,     0,
       0,     0,     0,   498,     0,     0,     0,     0,     0,     0,
       0,  1411,     0,  1148,     0,     0,  1149,  1150,  1151,  1152,
    1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,
    1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1145,  1146,
    1147,  1171,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1148,
       0,  1449,  1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,
    1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
    1167,  1168,  1169,  1170,     0,     0,  1145,  1146,  1147,     0,
       0,     0,     0,     0,     0,     0,     0,  1171,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1148,     0,  1338,
    1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,
    1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,
    1169,  1170,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1145,  1146,  1147,  1171,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1148,     0,  1518,  1149,  1150,  1151,  1152,
    1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,
    1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,     0,     0,
    1145,  1146,  1147,     0,     0,     0,     0,     0,     0,     0,
       0,  1171,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1148,     0,  1529,  1149,  1150,  1151,  1152,  1153,  1154,
    1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,
    1165,  1166,  1167,  1168,  1169,  1170,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1145,  1146,  1147,  1171,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1148,     0,  1633,
    1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,
    1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,
    1169,  1170,     0,    34,    35,    36,   211,     0,   212,    40,
       0,     0,     0,     0,     0,  1171,   213,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1727,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   242,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   243,     0,     0,     0,
       0,     0,     0,     0,     0,   215,   216,   217,   218,   219,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,   220,  1729,     0,     0,     0,   188,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   189,    97,     0,
       0,     0,    99,     0,    34,    35,    36,   211,     0,   212,
      40,     0,     0,     0,     0,     0,   104,   675,     0,     0,
       0,   107,   244,     0,     0,     0,     0,   111,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   214,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   215,   216,   217,   218,
     219,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,   220,     0,     0,     0,     0,   188,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   189,    97,
       0,     0,     0,    99,     0,    34,    35,    36,   211,     0,
     212,    40,     0,     0,     0,     0,     0,   104,   213,     0,
       0,     0,   107,   676,     0,     0,     0,     0,   677,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   242,     0,     0,
     211,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   215,   216,   217,
     218,   219,    50,    81,    82,    83,    84,    85,     0,     0,
     364,   365,     0,     0,   220,     0,     0,     0,     0,   188,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   189,
      97,     0,     0,     0,    99,     0,     0,     0,     0,   215,
     216,   217,   218,   219,     0,     0,     0,     0,   104,     0,
       0,     0,     0,   107,   244,     0,     0,     0,     0,   111,
       0,   366,     0,     0,   367,     0,     0,    93,    94,     0,
      95,   189,    97,     0,     0,     0,   469,   470,   471,     0,
       0,     0,     0,     0,     0,     0,   368,     0,     0,     0,
       0,     0,     0,     0,     0,   107,   472,   473,  1027,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,     0,   497,   469,   470,   471,     0,     0,     0,
       0,     0,     0,     0,     0,   498,     0,     0,     0,     0,
       0,     0,     0,     0,   472,   473,     0,   474,   475,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
       0,   497,  1145,  1146,  1147,     0,     0,     0,     0,     0,
       0,     0,     0,   498,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1148,  1534,     0,  1149,  1150,  1151,  1152,
    1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,
    1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1145,  1146,
    1147,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1171,     0,     0,     0,     0,     0,     0,     0,  1148,
       0,     0,  1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,
    1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
    1167,  1168,  1169,  1170,  1146,  1147,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1171,     0,     0,
       0,     0,     0,     0,  1148,     0,     0,  1149,  1150,  1151,
    1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,
    1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,   471,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1171,     0,     0,     0,     0,   472,   473,     0,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,  1147,   497,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   498,     0,     0,     0,
       0,     0,  1148,     0,     0,  1149,  1150,  1151,  1152,  1153,
    1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,
    1164,  1165,  1166,  1167,  1168,  1169,  1170,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   472,   473,
    1171,   474,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,     0,   497,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   498
};

static const yytype_int16 yycheck[] =
{
       5,     6,   131,     8,     9,    10,    11,    12,    13,   165,
      15,    16,    17,    18,   192,    56,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   108,    31,   180,   108,   123,
     848,   108,   569,   123,   170,   171,     4,    33,   414,    44,
     245,    98,     4,   693,  1403,   102,   103,    52,     4,    54,
      46,   414,    57,   123,    59,    51,   414,   108,   690,   990,
     873,     4,   988,   443,   193,   557,   730,   497,   605,   719,
     979,    30,   689,   130,   863,    30,   618,   619,    30,   856,
     529,    86,    57,     4,   668,  1099,  1021,   165,   834,     9,
     529,   538,   532,   533,     9,  1221,   241,     9,   788,    14,
       9,     9,    14,   108,  1039,    44,  1006,   190,  1008,     9,
     190,     9,   256,   190,     9,     9,   565,    57,   503,    14,
       9,   257,     9,   563,     9,     9,   565,   364,   365,   366,
      32,   368,     9,     9,    30,    48,     9,     9,   221,     9,
       9,   221,     9,    48,   221,     9,    70,    86,     9,     9,
    1085,    36,     9,     9,     9,     9,    70,    48,     4,    70,
      83,   244,  1212,     0,   244,   106,   107,    48,   197,  1095,
      10,    11,    12,   106,   107,    50,    51,   135,   136,    32,
     445,   446,   447,    48,  1861,   190,   115,   135,   136,    38,
     824,    31,   197,    38,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,   221,    38,    38,    70,
     197,   135,   136,   161,   135,   136,    70,    38,    38,    69,
       4,    70,  1141,    14,    83,   161,    83,   161,    83,   244,
      70,    70,    70,   201,   395,    70,   397,  1547,    70,   197,
     179,    32,   175,    70,   259,   399,   182,   262,    70,   197,
     201,   414,    83,    83,   269,   270,   197,   182,   201,   919,
      51,    70,    83,    83,    70,    83,   200,    70,   456,   199,
     200,   194,     8,    70,   199,  1644,   200,   199,    70,   194,
     199,   199,    70,   198,    54,   263,  1042,   199,   198,   267,
      70,   199,  1316,   386,   199,   199,   386,   198,   183,   386,
     199,    70,   199,   198,   355,   199,   853,   198,  1444,   165,
    1109,   198,  1111,   199,   988,  1225,   199,   199,   175,   199,
     199,   161,   199,   198,   198,  1270,   161,   198,   198,   135,
     136,   198,   198,   198,   198,    19,    20,   200,  2025,   200,
     503,   356,  1016,  1653,   175,   175,   200,   131,   198,  2036,
     443,   200,   540,   443,   175,   175,   443,   197,   197,   197,
     200,   200,   200,   915,   916,   200,   529,  1677,   200,  1679,
     385,   386,   511,   200,   968,  1294,  1018,   392,   393,   394,
     395,   396,   397,   450,   202,    70,  1446,   402,   551,    70,
     441,   200,   547,  1453,   200,  1455,   166,   200,   161,   562,
     197,    70,   565,   200,    83,  1228,   421,   182,   200,   193,
     161,  1055,   200,    31,   429,    57,    70,   197,    54,   182,
     200,  1095,   197,    83,  1484,   440,    90,    69,   443,   197,
      83,   200,    50,    83,    84,    53,   385,   200,   182,   506,
     507,   508,   509,   458,   459,   394,   395,   197,   397,   200,
    1586,   512,  1588,   197,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,     4,   498,    83,   500,   501,   502,   945,   420,
      83,    83,    84,   443,   158,   650,   651,   512,   513,   514,
     515,   516,   517,   518,   519,   520,   521,   522,   523,   524,
    1210,  1334,   197,   198,   193,   175,   197,   532,   533,   534,
     535,   200,   175,    70,   679,  1312,   541,   512,   497,   544,
     166,    53,   497,    90,    56,   497,   995,  1597,   553,   161,
     555,   707,   696,   197,   698,   181,   995,  1916,   563,    14,
      83,    73,   202,   197,   238,    83,   571,    90,   573,  1106,
     159,   160,   512,  1699,   135,   136,   781,  1703,   161,   200,
     102,    83,   428,   102,    96,   197,    98,    83,    90,   182,
     102,   103,   197,   676,    90,   121,  1036,   982,   135,   136,
      81,   497,   102,   979,   197,   131,   197,   503,   576,   814,
    1227,   158,   790,   197,  1044,   182,   979,  1526,   130,   624,
     202,   979,  1254,   197,   161,  1257,  1006,   102,  1008,   707,
     197,   571,   777,   529,    75,    76,   159,   160,    83,  1088,
     158,   159,   160,   165,  1186,    90,   165,   102,   103,  1088,
     131,    56,  1286,  1287,   428,   551,  1290,   159,   160,  1921,
     197,  1923,  1296,   159,   160,   165,   562,    83,   905,   565,
     907,   676,   909,    83,    90,   167,   913,   197,    19,    20,
      90,   738,   205,   888,   828,   829,    83,   165,   161,   197,
     165,   835,   836,    90,   206,   900,   182,    31,   400,   835,
      70,   375,   404,  1138,   206,  1140,  1615,    70,   200,   182,
     384,   197,   717,   158,   159,   160,    50,   391,    70,    53,
     865,   199,   200,  1289,   197,  1291,   871,   200,   430,   403,
     432,   433,   434,   435,  1860,   161,   510,   166,  1864,   165,
     414,   746,  1007,   159,   160,  1010,  1011,   197,   158,   159,
     160,   263,   500,   501,   167,   267,   182,   122,  1143,   271,
      50,    51,   159,   160,  1454,  1141,   131,   180,    70,   690,
     105,   106,   107,    70,   779,   123,   124,   125,  1141,   200,
    1272,   111,    32,  1141,    75,    76,   534,   200,   165,     6,
     120,   121,   122,   123,   124,   125,   941,   120,   121,   122,
     123,   124,   125,   197,   809,   105,   106,   107,   197,  1426,
     120,   121,   122,   123,   124,   125,  1448,    50,    51,    52,
      53,    54,    55,  1263,    57,   117,   979,    38,  1462,   982,
    1464,    48,   102,   103,  1274,   840,    69,    53,    54,    55,
     199,   994,   995,   355,   199,  1225,   133,   134,   364,   365,
     366,   856,   368,    69,  1391,   199,   852,   112,   113,   114,
     199,   707,   192,   199,   538,    70,   199,   200,  1315,   192,
     199,   200,  1562,   199,     4,   199,   199,  1892,  1893,  1888,
    1889,   199,   192,   288,   730,   290,    70,  2013,    70,    19,
      20,   994,   995,   861,    70,   112,    70,   238,    70,   873,
     874,   118,  2028,   120,   121,   122,   123,   124,   125,   126,
      70,   200,  1576,   161,   197,   197,   428,    70,  1294,   161,
     197,   165,    49,   199,    69,   437,   182,   161,   197,   441,
     197,  1294,   204,  1086,  1079,  1088,  1294,     9,   450,    53,
      54,    55,   347,    57,   949,   197,   951,   161,   953,  1486,
       8,   168,   169,   161,   171,    69,   730,   197,   199,   964,
    1407,   161,    14,   161,  1654,  1502,  1598,   199,   199,     9,
    1604,   200,  1606,   978,  1608,   192,   199,    14,   108,  1613,
     131,   131,  1127,   198,   201,   182,  1043,    14,  1141,  1134,
     102,   503,   504,   505,   506,   507,   508,   509,  1003,   967,
     846,  1006,   198,  1008,   198,   967,   198,   198,  1013,   197,
    1450,   967,   111,   203,   200,     9,   197,   529,   197,   158,
     198,   198,  1027,   198,   967,  1030,    94,  1032,   198,  1234,
       9,  1036,  1237,   438,   375,  1961,   441,   199,    14,   551,
    1966,   182,   716,   384,     9,   197,   967,  1427,   197,    83,
     391,  1982,   200,   565,   200,   199,   902,   199,   133,   199,
     190,   199,   403,   200,   576,  1991,  1006,   200,  1008,   990,
     198,   197,   846,  1013,  2005,   198,  1710,   198,   198,  1084,
     199,  1618,   204,  2014,   596,  1044,   982,     9,     9,  1044,
    1627,   221,  1044,   204,   204,  1913,  1092,  1018,   994,   995,
    1918,   204,   204,    70,    32,  1642,   134,   181,   238,     9,
     622,   623,   161,   137,   244,   198,   161,   791,    14,     9,
       9,   967,  2048,   194,   183,  1093,     9,    14,   902,   198,
       9,  1225,   198,   263,   198,  1225,   198,   267,   198,  1284,
     133,  1294,   988,  1583,  1584,   657,   658,   204,  1044,  1967,
    1526,   204,  1203,   204,   201,  1225,   830,     9,   832,    14,
     198,   204,  1547,  1526,   197,  1430,  1431,  1432,  1526,   198,
    1016,   161,   198,   102,   199,     9,   120,   121,   122,   123,
     124,   125,   199,  1720,   858,   590,   137,   131,   132,   161,
    1086,     9,  1088,   967,   198,   197,    70,   538,  1203,  1344,
      70,    70,    70,  1348,    70,   197,  2019,     9,  1353,   200,
      14,   199,   201,   183,   988,  1360,     9,   200,   730,  2032,
    1225,  2039,    14,   204,    14,   200,   738,   198,   194,    32,
     174,   199,  1866,  1867,   197,  1240,   197,    32,    14,  1615,
     197,  1209,  1016,   197,    14,   375,    52,  1209,   192,  1095,
     197,  1097,  1615,  1209,   384,   929,   386,  1615,  1263,    70,
      70,   391,  1912,  1203,  1914,  1650,  1209,   197,  1653,  1274,
    1275,   945,   946,   403,    70,   161,    70,     6,     9,    70,
     198,   686,   687,   199,   199,  1225,   197,   137,  1209,    14,
     695,   183,  1677,   137,  1679,  1431,   161,  1961,   428,     9,
    1685,   198,  1966,   175,   175,   979,    69,  1312,  1304,   204,
       9,    83,   824,   443,   826,   199,   201,  1322,     9,    48,
     201,  1095,   137,  1097,  1469,   201,   201,  1991,  1473,    14,
     197,  1868,    83,  1254,   846,  1480,  1257,   197,   199,  1307,
     198,   200,   198,   197,  1427,     9,   197,  1427,   860,   861,
    1427,   200,   200,  2003,   199,   137,    91,    50,    51,    52,
      53,    54,    55,  1209,   204,   200,   824,    32,    77,   158,
     199,   198,   137,  1526,    60,   716,    69,   199,   183,    32,
     204,   198,   198,   112,  2048,     9,     9,   137,     9,   118,
     902,   120,   121,   122,   123,   124,   125,   126,     9,   911,
     912,   198,    88,   204,   204,    91,  1411,  1563,   538,    14,
      83,     9,   197,  1418,   201,   198,   204,  1422,   204,  1424,
     199,   199,  1427,   200,   199,  1099,  1100,   201,   199,  1558,
     942,   199,   198,  1438,   198,  1209,   197,   199,   198,   168,
     169,   200,   171,   197,  1449,  1450,   576,   204,   137,   198,
     791,   198,   204,     9,   137,   967,   204,   198,   204,   204,
       9,    32,  1615,   192,   175,   199,   198,  1141,   873,   874,
     982,   198,   201,  1319,   137,   199,   988,  1862,   199,   112,
     200,   199,   994,   995,  1405,   166,   170,  1427,    14,   830,
      83,   832,   118,   198,  1415,   198,   200,  1675,   137,   137,
    2037,   198,    14,   182,  1016,   200,    83,   199,    14,    14,
      83,   198,   137,    14,    14,   137,   198,   858,   198,   197,
     196,    14,   199,     9,   199,   199,     9,  1448,   200,    68,
     201,  1043,    83,   182,     9,   197,    83,     9,   102,   200,
     199,  1053,  1054,  1055,   161,  1319,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,  1405,
      57,   115,   102,   183,   969,  1239,   173,    36,    14,  1415,
     197,  1576,    69,   179,  1086,  1580,  1088,   176,  1583,  1584,
     985,  1093,   198,  1095,   199,  1097,   716,   197,   929,   183,
    1647,   183,   997,    83,   198,     9,    83,  1055,   199,    14,
     730,   198,   198,    83,   945,   946,  1118,    14,    14,   200,
      83,    14,    83,    83,  1186,  1994,   509,    19,    20,  1037,
    1294,   970,   504,  2010,  1313,  1499,  1300,  1719,   506,   626,
    2005,  1143,  1706,  1744,  1655,  1040,  1830,  1553,  2046,  1842,
    2026,  1315,  1316,  1549,  1702,  1139,  1135,  1213,  1290,     6,
    1054,  1286,   396,  1081,  1285,   392,  1624,   441,  1906,  1962,
    1172,   791,  1952,   873,  1194,  1119,  1587,  1172,  1846,    -1,
    1541,    -1,  1593,    -1,  1595,    -1,    -1,  1598,   364,   365,
     366,   367,   368,    -1,    -1,    -1,    -1,    -1,  1693,    -1,
    1536,    48,    -1,    -1,    -1,    -1,  1617,  1209,    -1,    -1,
     830,    -1,   832,    -1,    -1,    -1,    -1,    -1,  1113,    -1,
      -1,    -1,  1117,    -1,    -1,    -1,   846,  1563,    -1,    -1,
     406,   123,    -1,    -1,    -1,    -1,    -1,    -1,   858,    -1,
    1576,   861,    -1,  1407,    -1,    -1,    -1,  1882,  1883,    -1,
      -1,  1587,    -1,    -1,    -1,    -1,    -1,  1593,    -1,  1595,
    1718,  1719,    -1,    -1,    -1,   112,    -1,    -1,  1099,  1100,
      -1,   118,  1536,   120,   121,   122,   123,   124,   125,   126,
      -1,  1617,   902,  1619,  1286,  1287,  1288,  1289,  1290,  1291,
      -1,    -1,  1628,  1557,  1296,    -1,    -1,  1708,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1307,    -1,    -1,    -1,   929,
    1841,    -1,  1576,    -1,    -1,    -1,    -1,  1319,    -1,    -1,
      -1,   168,   169,    -1,   171,   945,   946,  1329,    -1,  1975,
      81,  1226,    -1,  1228,    -1,    -1,    -1,    -1,  1286,  1287,
    1288,  1289,  1290,  1291,    -1,   192,   238,   967,  1296,    -1,
      -1,    -1,   103,    -1,   201,  1619,  1851,    -1,    19,    20,
      -1,    -1,  1526,    -1,  1628,    -1,    -1,    -1,   988,    30,
      -1,  1266,  1708,    -1,  1269,  1906,    -1,    -1,    -1,  1998,
      -1,  1717,    -1,    -1,    -1,    -1,  1006,  1723,  1008,   140,
     141,   142,   143,   144,  1730,    56,  1016,  1399,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1239,    -1,
      -1,   162,    -1,    -1,   165,    -1,    -1,   168,   169,  1683,
     171,   172,   173,     6,   120,   121,   122,   123,   124,   125,
      -1,    -1,    -1,  1328,    -1,   131,   132,    -1,    -1,  1334,
      -1,    -1,   618,   619,    -1,   196,    -1,    -1,    -1,    -1,
     201,  1615,    -1,  1717,    -1,    -1,    -1,    -1,    -1,  1723,
    1462,    -1,  1464,    -1,    -1,    48,  1730,    -1,    -1,  1300,
      -1,    -1,    -1,  1093,    -1,  1095,   172,  1097,   174,  1099,
    1100,    -1,    -1,   375,  1315,  1316,    -1,    -1,    -1,    -1,
      -1,   187,   384,   189,   386,    -1,   192,    -1,    -1,   391,
      -1,    -1,    -1,    -1,    -1,  1400,  1401,    -1,    -1,    -1,
      -1,   403,    -1,    -1,  1462,    -1,  1464,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,
      -1,    -1,    -1,    -1,  1536,   118,  1872,   120,   121,   122,
     123,   124,   125,   126,    -1,  1547,    -1,    -1,    -1,  2044,
      -1,  1553,    -1,    -1,    -1,    -1,    -1,  2052,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  2060,    -1,    -1,  2063,    -1,
      -1,  1982,    -1,    -1,  1576,    -1,  1407,   238,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   168,   169,    -1,   171,  1209,
    1926,    -1,    -1,    -1,  2005,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1604,  2014,  1606,  1225,  1608,    -1,  1872,   192,
      -1,  1613,  1507,    -1,    -1,  1510,    -1,  1619,   201,  1239,
      -1,    -1,  1624,    -1,    -1,  1961,  1628,   288,    -1,   290,
    1966,    -1,     6,    -1,    -1,    -1,    -1,    -1,    -1,  1975,
      -1,    -1,    -1,    -1,    -1,  1647,   538,    -1,  1650,     6,
      -1,  1653,    -1,    -1,    -1,  1991,  1604,    -1,  1606,    -1,
    1608,  1663,  1926,  1558,    -1,  1613,    -1,    -1,  1670,    -1,
      -1,    -1,    -1,    -1,    48,  1677,    -1,  1679,    -1,    -1,
    1300,    -1,    -1,  1685,    -1,    -1,   347,  1307,    -1,    -1,
      -1,    48,    -1,    -1,    -1,  1315,  1316,  1961,    -1,  1319,
      -1,    -1,  1966,    -1,    -1,    -1,    -1,    -1,  1710,    -1,
      -1,  2047,  2048,    81,   375,  1717,  1718,  1719,    -1,    -1,
      -1,  1723,    -1,   384,    -1,    -1,    -1,  1991,  1730,   905,
     391,   907,    -1,   909,  1629,   103,    -1,   913,   112,   915,
     916,   917,   403,    -1,   118,    -1,   120,   121,   122,   123,
     124,   125,   126,   414,    -1,   112,    -1,    -1,    -1,    -1,
      -1,   118,  1710,   120,   121,   122,   123,   124,   125,   126,
      -1,    31,   140,   141,   142,   143,   144,   438,    -1,    -1,
     441,    -1,    -1,  2047,  2048,    -1,    -1,  1407,    -1,    -1,
      -1,    -1,    -1,    -1,   168,   169,    -1,   171,   166,    -1,
     168,   169,    -1,   171,   172,   173,    -1,  1427,    -1,    -1,
      -1,   168,   169,    -1,   171,    -1,    -1,    -1,   192,    -1,
      -1,    81,    -1,    -1,   716,    -1,    -1,   201,   196,     6,
      -1,    91,    -1,    -1,    -1,   192,   497,    -1,    -1,  1841,
      -1,    -1,    -1,   103,   201,    -1,    -1,  1742,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1862,    -1,    -1,    -1,  1866,  1867,    -1,    -1,    -1,    -1,
    1872,    48,    -1,    -1,    -1,    -1,    -1,   538,    -1,  1881,
     140,   141,   142,   143,   144,    -1,  1888,  1889,    -1,    -1,
    1892,  1893,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   791,
      -1,    -1,   162,    -1,  1906,   165,    -1,    -1,   168,   169,
      -1,   171,   172,   173,    -1,   175,  1536,    -1,  1866,  1867,
      -1,    -1,    -1,    -1,  1926,    -1,    -1,    -1,    -1,   590,
      -1,   592,    -1,    -1,   595,   112,   196,    -1,   830,    -1,
     832,   118,    -1,   120,   121,   122,   123,   124,   125,   126,
    1845,    -1,    -1,    -1,    -1,    -1,  1576,    -1,    -1,  1961,
      -1,    -1,    -1,  1858,  1966,    -1,   858,   628,    -1,    -1,
      -1,    -1,  1974,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,    -1,    -1,  1991,
      -1,   168,   169,    -1,   171,  1997,    -1,    -1,    -1,  1619,
      -1,    -1,    -1,    -1,  1624,    -1,    -1,    -1,  1628,    -1,
    1186,    -1,    -1,    -1,    -1,   192,    -1,    -1,    59,    60,
      -1,    -1,    -1,    -1,   201,   686,   687,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   695,    -1,    -1,   929,    -1,    -1,
    1935,    -1,    -1,    -1,    -1,  2047,  2048,    -1,    -1,    -1,
      -1,    -1,    -1,   945,   946,   716,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1962,    -1,  1964,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,  1717,  1718,  1719,
      -1,    -1,    -1,  1723,    -1,    -1,    -1,    -1,    -1,    -1,
    1730,    -1,    -1,    -1,    -1,    10,    11,    12,    59,    60,
      -1,    -1,    -1,    -1,  2019,    -1,    -1,    -1,    -1,    -1,
     791,    -1,    -1,    -1,    -1,    30,    31,  2032,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,   824,    -1,    -1,    -1,    -1,    -1,   830,
      -1,   832,    -1,    -1,    69,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   858,   859,    -1,
      -1,    81,    -1,    83,    84,   866,    -1,  1099,  1100,    -1,
      -1,    -1,   873,   874,   875,   876,   877,   878,   879,    -1,
      -1,    59,    60,   103,    -1,    -1,   887,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,   903,    -1,    -1,    -1,    -1,    19,    20,    -1,
      -1,    -1,  1872,    -1,    -1,    -1,    -1,   198,    30,    -1,
     140,   141,   142,   143,   144,    -1,    -1,    -1,   929,    -1,
      -1,    -1,    59,    60,    -1,    -1,    -1,    -1,    31,    -1,
      56,    -1,   943,    -1,   945,   946,   166,    -1,   168,   169,
      -1,   171,   172,   173,    -1,    -1,    -1,   135,   136,    -1,
      -1,    -1,    -1,    -1,   199,    -1,  1926,    -1,   969,   970,
      -1,    -1,    -1,    -1,    -1,    68,   196,    -1,   979,    -1,
     200,    -1,   202,    -1,   985,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,  1225,    -1,    -1,   997,    -1,    -1,    -1,
      -1,  1961,    -1,    -1,  1005,    -1,  1966,  1239,   135,   136,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,  1020,
     198,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1991,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1040,
      -1,    -1,    -1,  1044,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,  1055,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,  1300,   162,
      -1,   198,   165,   166,    -1,   168,   169,    -1,   171,   172,
     173,    69,    -1,  1315,  1316,    -1,    -1,  2047,  2048,    -1,
      -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,  1099,  1100,
      -1,    -1,    -1,   196,   197,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1113,    -1,    -1,    -1,  1117,    -1,  1119,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   238,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1135,  1136,  1137,  1138,  1139,  1140,
    1141,    -1,    -1,  1144,  1145,  1146,  1147,  1148,  1149,  1150,
    1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,
    1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,
    1171,    -1,   288,    -1,   290,  1407,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    78,    79,    80,    -1,  1190,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    91,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,  1226,    -1,  1228,    -1,    -1,
      -1,   347,    -1,    -1,    -1,    69,    -1,    -1,  1239,    -1,
      -1,    78,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   146,   147,   148,   149,   150,    -1,    -1,
      -1,    -1,    -1,   375,   157,  1266,   103,    -1,  1269,    -1,
     163,   164,   384,    -1,    -1,    -1,    -1,    -1,    -1,   391,
      -1,    -1,    -1,    -1,   177,  1286,  1287,  1288,  1289,  1290,
    1291,   403,    -1,  1294,    -1,  1296,    -1,    -1,   191,  1300,
      -1,    -1,   414,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,  1315,  1316,    -1,  1318,    -1,    -1,
      -1,    -1,   438,    -1,    -1,   441,    -1,  1328,    -1,    -1,
      -1,   168,   169,  1334,   171,   172,   173,  1338,    -1,  1340,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,    -1,    -1,   196,
      -1,    -1,    -1,  1364,    -1,    -1,    -1,    -1,    -1,    -1,
     204,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,   497,    -1,    -1,    59,    60,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1400,
    1401,    -1,    -1,  1404,    -1,    -1,  1407,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,   538,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,  1462,    -1,  1464,   135,   136,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   590,    81,   592,    -1,    -1,    -1,
      -1,    -1,    -1,   595,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,   135,   136,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1507,    -1,    -1,  1510,
      -1,    -1,    -1,    -1,    -1,  1516,   628,  1518,    -1,  1520,
      -1,    -1,    -1,    -1,  1525,  1526,    -1,   198,  1529,    -1,
    1531,    -1,    -1,  1534,   140,   141,   142,   143,   144,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1547,  1548,    -1,    -1,
    1551,    31,    -1,    -1,    -1,   198,   162,  1558,    -1,   165,
      -1,    -1,   168,   169,    -1,   171,   172,   173,    -1,   175,
     686,   687,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   695,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
     196,    -1,    -1,    -1,    -1,    -1,   199,    -1,   201,    -1,
      -1,    81,    -1,  1604,   716,  1606,    -1,  1608,    -1,    81,
      -1,    -1,  1613,    -1,  1615,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,  1629,    -1,
      -1,   103,  1633,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1645,  1646,    -1,    -1,    -1,    -1,
      -1,    -1,  1653,    -1,  1655,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,    -1,   140,   141,
     142,   143,   144,    -1,    -1,    -1,  1677,    -1,  1679,   791,
      -1,    -1,   162,    -1,  1685,   165,   166,    -1,   168,   169,
      -1,   171,   172,   173,   166,   175,   168,   169,   170,   171,
     172,   173,    -1,    -1,    -1,    -1,   186,    -1,    -1,  1710,
      -1,    -1,   824,    -1,    -1,    -1,   196,   197,   830,    -1,
     832,    -1,    -1,    -1,   196,   197,  1727,  1728,  1729,    -1,
      -1,    -1,    -1,  1734,    -1,  1736,    -1,    -1,    -1,    -1,
      -1,  1742,    -1,  1744,    -1,    -1,   858,   859,    -1,    -1,
     866,    -1,    -1,    -1,    -1,    -1,    -1,   873,   874,    -1,
      -1,    -1,    -1,   875,   876,   877,   878,   879,    -1,    10,
      11,    12,    -1,    -1,    -1,   887,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      31,   903,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,   929,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,   943,    -1,   945,   946,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,  1845,    -1,    -1,    -1,    -1,    -1,
      -1,    19,    20,   969,    81,    -1,    -1,  1858,   970,    -1,
      -1,  1862,    30,    -1,    -1,  1866,  1867,   979,    -1,   985,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    68,    -1,    -1,
    1881,   997,    -1,    -1,    -1,    -1,  1887,    -1,    -1,    -1,
      81,    -1,    -1,  1005,    -1,    -1,    -1,  1898,    -1,    -1,
      -1,    -1,    -1,  1904,    -1,    -1,    -1,  1908,  1020,    -1,
      -1,    -1,   103,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,  1040,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1044,    -1,  1935,    -1,    -1,    -1,   165,    -1,
      -1,   168,   169,  1055,   171,   172,   173,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,
     201,  1962,    -1,  1964,    -1,    -1,    -1,    -1,    -1,   196,
      -1,   162,    -1,   200,   165,   166,    -1,   168,   169,  1980,
     171,   172,   173,    -1,    -1,    -1,    -1,  1099,  1100,    -1,
      -1,    -1,  1993,    -1,    -1,   186,    -1,  1113,    -1,    -1,
      -1,  1117,    -1,  1119,    -1,   196,   197,    -1,    -1,  2010,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2019,    -1,
      -1,    -1,    -1,  1135,  1136,  1137,  1138,  1139,  1140,  1141,
      -1,  2032,  1144,  1145,  1146,  1147,  1148,  1149,  1150,  1151,
    1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,
    1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     238,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1190,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
    1226,    -1,  1228,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1239,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
    1266,    -1,    -1,  1269,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
      -1,    81,    -1,    -1,  1286,  1287,  1288,  1289,  1290,  1291,
      59,    60,  1294,    -1,  1296,    -1,    -1,    -1,  1300,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      59,    60,    -1,  1315,  1316,    -1,  1318,   375,    -1,    -1,
      -1,    -1,  1328,    -1,    -1,    -1,   384,    -1,  1334,   129,
      -1,    -1,    -1,   391,    -1,    -1,  1338,    -1,  1340,    -1,
     140,   141,   142,   143,   144,   403,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   414,    -1,    -1,    -1,
      -1,   201,  1364,    -1,    -1,    -1,   135,   136,   168,   169,
      -1,   171,   172,   173,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,   135,   136,    -1,    -1,
      -1,    -1,    -1,    -1,  1400,  1401,   196,   197,    -1,    -1,
      -1,    -1,  1404,    30,    31,  1407,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   497,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1462,    -1,  1464,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    -1,    -1,
     538,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,  1507,    -1,    -1,  1510,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,  1516,    -1,  1518,    -1,  1520,    -1,
      -1,    -1,   103,  1525,  1526,    -1,    -1,  1529,    -1,  1531,
      -1,    -1,  1534,    -1,    -1,    -1,    -1,   595,    -1,    -1,
      -1,  1547,    -1,    -1,    -1,    -1,  1548,    -1,    -1,  1551,
      -1,    -1,  1558,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,    -1,   146,   147,   148,   149,   150,
     628,    -1,    -1,    -1,   201,    -1,   157,    -1,    -1,    -1,
      -1,   162,   163,   164,   165,   166,    -1,   168,   169,    -1,
     171,   172,   173,    -1,    -1,    -1,   177,   595,    -1,    -1,
      -1,    -1,  1604,    -1,  1606,   186,  1608,    -1,    -1,    -1,
     191,  1613,    -1,  1615,    -1,   196,   197,    -1,    -1,    -1,
      -1,    -1,    -1,  1629,    -1,    -1,    -1,    -1,    -1,    -1,
     628,  1633,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1645,  1646,    -1,    -1,  1653,    -1,    -1,
      -1,    -1,    -1,  1655,    -1,    -1,    -1,    -1,   716,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1677,    -1,  1679,    -1,    -1,    -1,    -1,    -1,  1685,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,  1710,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      10,    11,    12,    -1,    -1,  1727,  1728,  1729,    -1,    -1,
      -1,    -1,  1734,   791,  1736,    -1,  1742,    -1,    -1,    -1,
      30,    31,  1744,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,   830,    -1,   832,    -1,    -1,    -1,    -1,    69,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
     858,   859,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,   875,   876,   877,
     878,   879,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   887,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1845,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   201,  1858,    -1,    81,    -1,  1862,    -1,    -1,    -1,
      -1,   859,    -1,    -1,  1866,  1867,    -1,    -1,    -1,    -1,
      -1,   929,    -1,    -1,    -1,  1881,   103,   875,   876,   877,
     878,   879,    -1,    -1,    -1,  1887,    -1,   945,   946,   887,
      31,    -1,    -1,    -1,    -1,    -1,  1898,    -1,    -1,   126,
      -1,    -1,  1904,    -1,    -1,    -1,  1908,    -1,    -1,    -1,
      -1,   201,   139,   140,   141,   142,   143,   144,   145,    -1,
      -1,   979,    -1,    -1,    -1,    -1,    -1,    68,    -1,  1935,
      -1,    -1,    -1,    -1,    -1,   162,    -1,    -1,   165,   166,
      81,   168,   169,    -1,   171,   172,   173,  1005,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1962,    -1,  1964,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,   196,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1980,   120,
     121,   122,   123,   124,   125,    -1,  1044,    -1,    -1,    -1,
      -1,  1993,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,   144,   145,    -1,    -1,  1005,  2010,    -1,
      -1,    -1,    -1,  2019,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   162,    -1,    -1,   165,   166,  2032,   168,   169,    -1,
     171,   172,   173,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1099,  1100,    -1,    -1,   186,    -1,    -1,    -1,    -1,
      -1,   192,    -1,    -1,    -1,   196,   197,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1135,  1136,  1137,
    1138,  1139,  1140,  1141,    -1,    -1,  1144,  1145,  1146,  1147,
    1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,
    1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,  1190,    57,    -1,    -1,    -1,  1135,  1136,    -1,
      -1,  1139,    -1,    -1,    -1,    69,  1144,  1145,  1146,  1147,
    1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,
    1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,    10,    11,    12,    -1,    -1,    -1,
      -1,  1239,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1190,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    81,    -1,    83,    84,    -1,    -1,
      -1,    81,    -1,    69,    -1,    -1,  1294,    -1,    -1,    81,
      -1,    -1,  1300,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,  1315,  1316,    -1,
    1318,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1338,    -1,  1340,   140,   141,   142,   143,   144,    -1,    -1,
     140,   141,   142,   143,   144,    -1,    -1,    -1,   140,   141,
     142,   143,   144,    -1,    -1,    -1,  1364,    -1,    -1,   166,
      -1,   168,   169,    -1,   171,   172,   173,    -1,   168,   169,
    1318,   171,   172,   173,    -1,    -1,   168,   169,    -1,   171,
     172,   173,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   196,
    1338,    -1,  1340,   200,    -1,   202,   196,   197,    -1,  1407,
      10,    11,    12,    -1,   196,   197,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   201,  1364,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    83,    -1,    85,    69,  1516,    -1,
    1518,    -1,  1520,    -1,    -1,    -1,    -1,  1525,  1526,    -1,
      -1,  1529,    -1,  1531,   103,    -1,  1534,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,    -1,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,    28,    29,  1516,    -1,
    1518,    -1,  1520,    -1,    -1,   137,    -1,  1525,    -1,    -1,
      -1,  1529,    -1,  1531,    -1,    -1,  1534,    -1,    -1,   168,
     169,   201,   171,   172,   173,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1615,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,   196,    -1,    81,
      -1,    83,    84,    -1,    -1,  1633,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,    -1,   129,    -1,   131,
     132,   133,   134,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,   144,   145,    -1,  1633,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,
     162,    -1,    -1,    -1,    -1,    13,   168,   169,    -1,   171,
     172,   173,   174,    -1,   176,    -1,    -1,   179,    -1,  1727,
    1728,  1729,    -1,    -1,    -1,    -1,  1734,    -1,    -1,    -1,
      38,    -1,    -1,    -1,   196,  1743,    -1,    -1,   200,    -1,
     202,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,  1727,
    1728,  1729,    -1,    91,    -1,    -1,  1734,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   120,   121,   122,   123,   124,   125,    -1,    -1,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,   141,   142,   143,   144,    -1,   146,   147,
     148,   149,   150,    -1,    81,    -1,    -1,    -1,    -1,   157,
      -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,
     168,   169,    -1,   171,   172,   173,   103,   175,    -1,   177,
      -1,    -1,   180,    -1,   111,   112,    -1,    -1,   186,  1887,
      -1,    -1,    -1,   191,   192,   193,    -1,    -1,   196,   197,
    1898,    -1,    -1,    -1,   202,   203,  1904,   205,   206,    -1,
    1908,    -1,    -1,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    -1,   165,  1937,
      13,   168,   169,    -1,   171,   172,   173,    -1,    -1,  1887,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
    1898,    -1,    -1,    -1,    -1,    -1,  1904,    -1,    -1,   196,
    1908,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,
      -1,    -1,  1980,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,
      93,    94,    -1,    96,    -1,    98,    -1,   100,    -1,    -1,
     103,   104,    -1,    -1,    -1,   108,   109,   110,   111,   112,
     113,   114,  1980,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,    -1,   127,   128,   129,   130,   131,   132,
      -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,
     143,   144,    -1,   146,   147,   148,   149,   150,    -1,    -1,
      -1,   154,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,
     163,   164,   165,   166,    -1,   168,   169,    -1,   171,   172,
     173,   174,    -1,    -1,   177,    -1,    -1,   180,    -1,    -1,
      -1,    -1,    -1,   186,   187,    -1,   189,    -1,   191,   192,
     193,    -1,    -1,   196,   197,    -1,   199,   200,    -1,   202,
     203,    -1,   205,   206,     3,     4,     5,     6,     7,    -1,
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
     109,   110,   111,   112,   113,   114,    -1,   116,    -1,   118,
     119,   120,   121,   122,   123,   124,   125,    -1,   127,   128,
     129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,   144,    -1,   146,   147,   148,
     149,   150,    -1,    -1,    -1,   154,    -1,    -1,   157,    -1,
      -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,   168,
     169,    -1,   171,   172,   173,   174,    -1,    -1,   177,    -1,
      -1,   180,    -1,    -1,    -1,    -1,    -1,   186,   187,    -1,
     189,    -1,   191,   192,   193,    -1,    -1,   196,   197,    -1,
     199,   200,   201,   202,   203,    -1,   205,   206,     3,     4,
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
      -1,   116,    -1,   118,   119,   120,   121,   122,   123,   124,
     125,    -1,   127,   128,   129,   130,   131,   132,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
      -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,   154,
      -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,
     165,   166,    -1,   168,   169,    -1,   171,   172,   173,   174,
      -1,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,
      -1,   186,   187,    -1,   189,    -1,   191,   192,   193,    -1,
      -1,   196,   197,    -1,   199,   200,   201,   202,   203,    -1,
     205,   206,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     111,    -1,   113,   114,    -1,   116,    -1,    -1,   119,   120,
     121,   122,   123,   124,   125,    -1,   127,   128,   129,    -1,
     131,   132,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,    -1,   146,   147,   148,   149,   150,
      -1,    -1,    -1,   154,    -1,    -1,   157,    -1,    -1,    -1,
      -1,   162,   163,   164,   165,   166,    -1,   168,   169,    -1,
     171,   172,   173,   174,    -1,    -1,   177,    -1,    -1,   180,
      -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,
     191,   192,   193,    -1,    -1,   196,   197,    -1,   199,   200,
     201,   202,   203,    -1,   205,   206,     3,     4,     5,     6,
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
      -1,    -1,   119,   120,   121,   122,   123,   124,   125,    -1,
     127,   128,   129,    -1,   131,   132,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,   144,    -1,   146,
     147,   148,   149,   150,    -1,    -1,    -1,   154,    -1,    -1,
     157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,
      -1,   168,   169,    -1,   171,   172,   173,   174,    -1,    -1,
     177,    -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,   186,
      -1,    -1,    -1,    -1,   191,   192,   193,    -1,    -1,   196,
     197,    -1,   199,   200,   201,   202,   203,    -1,   205,   206,
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
     113,   114,    -1,   116,   117,    -1,   119,   120,   121,   122,
     123,   124,   125,    -1,   127,   128,   129,    -1,   131,   132,
      -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,
     143,   144,    -1,   146,   147,   148,   149,   150,    -1,    -1,
      -1,   154,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,
     163,   164,   165,   166,    -1,   168,   169,    -1,   171,   172,
     173,   174,    -1,    -1,   177,    -1,    -1,   180,    -1,    -1,
      -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,
     193,    -1,    -1,   196,   197,    -1,   199,   200,    -1,   202,
     203,    -1,   205,   206,     3,     4,     5,     6,     7,    -1,
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
     109,   110,   111,    -1,   113,   114,    -1,   116,    -1,    -1,
     119,   120,   121,   122,   123,   124,   125,    -1,   127,   128,
     129,    -1,   131,   132,    -1,    -1,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,   144,    -1,   146,   147,   148,
     149,   150,    -1,    -1,    -1,   154,    -1,    -1,   157,    -1,
      -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,   168,
     169,    -1,   171,   172,   173,   174,    -1,    -1,   177,    -1,
      -1,   180,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,
      -1,    -1,   191,   192,   193,    -1,    -1,   196,   197,    -1,
     199,   200,   201,   202,   203,    -1,   205,   206,     3,     4,
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
      -1,   116,    -1,    -1,   119,   120,   121,   122,   123,   124,
     125,    -1,   127,   128,   129,    -1,   131,   132,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
      -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,   154,
      -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,
     165,   166,    -1,   168,   169,    -1,   171,   172,   173,   174,
      -1,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,
      -1,   186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,
      -1,   196,   197,    -1,   199,   200,   201,   202,   203,    -1,
     205,   206,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     111,    -1,   113,   114,    -1,   116,    -1,    -1,   119,   120,
     121,   122,   123,   124,   125,    -1,   127,   128,   129,    -1,
     131,   132,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,    -1,   146,   147,   148,   149,   150,
      -1,    -1,    -1,   154,    -1,    -1,   157,    -1,    -1,    -1,
      -1,   162,   163,   164,   165,   166,    -1,   168,   169,    -1,
     171,   172,   173,   174,    -1,    -1,   177,    -1,    -1,   180,
      -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,
     191,   192,   193,    -1,    -1,   196,   197,    -1,   199,   200,
     201,   202,   203,    -1,   205,   206,     3,     4,     5,     6,
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
      -1,    -1,   119,   120,   121,   122,   123,   124,   125,    -1,
     127,   128,   129,    -1,   131,   132,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,   144,    -1,   146,
     147,   148,   149,   150,    -1,    -1,    -1,   154,    -1,    -1,
     157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,
      -1,   168,   169,    -1,   171,   172,   173,   174,    -1,    -1,
     177,    -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,   186,
      -1,    -1,    -1,    -1,   191,   192,   193,    -1,    -1,   196,
     197,    -1,   199,   200,   201,   202,   203,    -1,   205,   206,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,    92,
      93,    94,    95,    96,    -1,    98,    -1,   100,    -1,    -1,
     103,   104,    -1,    -1,    -1,   108,   109,   110,   111,    -1,
     113,   114,    -1,   116,    -1,    -1,   119,   120,   121,   122,
     123,   124,   125,    -1,   127,   128,   129,    -1,   131,   132,
      -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,
     143,   144,    -1,   146,   147,   148,   149,   150,    -1,    -1,
      -1,   154,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,
     163,   164,   165,   166,    -1,   168,   169,    -1,   171,   172,
     173,   174,    -1,    -1,   177,    -1,    -1,   180,    -1,    -1,
      -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,
     193,    -1,    -1,   196,   197,    -1,   199,   200,    -1,   202,
     203,    -1,   205,   206,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    87,    -1,
      -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,    98,
      -1,   100,   101,    -1,   103,   104,    -1,    -1,    -1,   108,
     109,   110,   111,    -1,   113,   114,    -1,   116,    -1,    -1,
     119,   120,   121,   122,   123,   124,   125,    -1,   127,   128,
     129,    -1,   131,   132,    -1,    -1,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,   144,    -1,   146,   147,   148,
     149,   150,    -1,    -1,    -1,   154,    -1,    -1,   157,    -1,
      -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,   168,
     169,    -1,   171,   172,   173,   174,    -1,    -1,   177,    -1,
      -1,   180,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,
      -1,    -1,   191,   192,   193,    -1,    -1,   196,   197,    -1,
     199,   200,    -1,   202,   203,    -1,   205,   206,     3,     4,
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
      -1,   116,    -1,    -1,   119,   120,   121,   122,   123,   124,
     125,    -1,   127,   128,   129,    -1,   131,   132,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
      -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,   154,
      -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,
     165,   166,    -1,   168,   169,    -1,   171,   172,   173,   174,
      -1,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,
      -1,   186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,
      -1,   196,   197,    -1,   199,   200,   201,   202,   203,    -1,
     205,   206,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    77,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,
      91,    92,    93,    94,    -1,    96,    -1,    98,    -1,   100,
      -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,
     111,    -1,   113,   114,    -1,   116,    -1,    -1,   119,   120,
     121,   122,   123,   124,   125,    -1,   127,   128,   129,    -1,
     131,   132,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,    -1,   146,   147,   148,   149,   150,
      -1,    -1,    -1,   154,    -1,    -1,   157,    -1,    -1,    -1,
      -1,   162,   163,   164,   165,   166,    -1,   168,   169,    -1,
     171,   172,   173,   174,    -1,    -1,   177,    -1,    -1,   180,
      -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,
     191,   192,   193,    -1,    -1,   196,   197,    -1,   199,   200,
      -1,   202,   203,    -1,   205,   206,     3,     4,     5,     6,
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
      -1,    -1,   119,   120,   121,   122,   123,   124,   125,    -1,
     127,   128,   129,    -1,   131,   132,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,   144,    -1,   146,
     147,   148,   149,   150,    -1,    -1,    -1,   154,    -1,    -1,
     157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,
      -1,   168,   169,    -1,   171,   172,   173,   174,    -1,    -1,
     177,    -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,   186,
      -1,    -1,    -1,    -1,   191,   192,   193,    -1,    -1,   196,
     197,    -1,   199,   200,   201,   202,   203,    -1,   205,   206,
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
     113,   114,    -1,   116,    -1,    -1,   119,   120,   121,   122,
     123,   124,   125,    -1,   127,   128,   129,    -1,   131,   132,
      -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,
     143,   144,    -1,   146,   147,   148,   149,   150,    -1,    -1,
      -1,   154,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,
     163,   164,   165,   166,    -1,   168,   169,    -1,   171,   172,
     173,   174,    -1,    -1,   177,    -1,    -1,   180,    -1,    -1,
      -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,
     193,    -1,    -1,   196,   197,    -1,   199,   200,    -1,   202,
     203,    -1,   205,   206,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    87,    -1,
      -1,    -1,    91,    92,    93,    94,    -1,    96,    97,    98,
      -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,
     109,   110,   111,    -1,   113,   114,    -1,   116,    -1,    -1,
     119,   120,   121,   122,   123,   124,   125,    -1,   127,   128,
     129,    -1,   131,   132,    -1,    -1,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,   144,    -1,   146,   147,   148,
     149,   150,    -1,    -1,    -1,   154,    -1,    -1,   157,    -1,
      -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,   168,
     169,    -1,   171,   172,   173,   174,    -1,    -1,   177,    -1,
      -1,   180,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,
      -1,    -1,   191,   192,   193,    -1,    -1,   196,   197,    -1,
     199,   200,    -1,   202,   203,    -1,   205,   206,     3,     4,
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
      -1,   116,    -1,    -1,   119,   120,   121,   122,   123,   124,
     125,    -1,   127,   128,   129,    -1,   131,   132,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
      -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,   154,
      -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,
     165,   166,    -1,   168,   169,    -1,   171,   172,   173,   174,
      -1,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,
      -1,   186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,
      -1,   196,   197,    -1,   199,   200,   201,   202,   203,    -1,
     205,   206,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     111,    -1,   113,   114,    -1,   116,    -1,    -1,   119,   120,
     121,   122,   123,   124,   125,    -1,   127,   128,   129,    -1,
     131,   132,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,    -1,   146,   147,   148,   149,   150,
      -1,    -1,    -1,   154,    -1,    -1,   157,    -1,    -1,    -1,
      -1,   162,   163,   164,   165,   166,    -1,   168,   169,    -1,
     171,   172,   173,   174,    -1,    -1,   177,    -1,    -1,   180,
      -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,
     191,   192,   193,    -1,    -1,   196,   197,    -1,   199,   200,
     201,   202,   203,    -1,   205,   206,     3,     4,     5,     6,
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
      -1,    -1,   119,   120,   121,   122,   123,   124,   125,    -1,
     127,   128,   129,    -1,   131,   132,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,   144,    -1,   146,
     147,   148,   149,   150,    -1,    -1,    -1,   154,    -1,    -1,
     157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,
      -1,   168,   169,    -1,   171,   172,   173,   174,    -1,    -1,
     177,    -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,   186,
      -1,    -1,    -1,    -1,   191,   192,   193,    -1,    -1,   196,
     197,    -1,   199,   200,   201,   202,   203,    -1,   205,   206,
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
     113,   114,    -1,   116,    -1,    -1,   119,   120,   121,   122,
     123,   124,   125,    -1,   127,   128,   129,    -1,   131,   132,
      -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,
     143,   144,    -1,   146,   147,   148,   149,   150,    -1,    -1,
      -1,   154,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,
     163,   164,   165,   166,    -1,   168,   169,    -1,   171,   172,
     173,   174,    -1,    -1,   177,    -1,    -1,   180,    -1,    -1,
      -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,
     193,    -1,    -1,   196,   197,    -1,   199,   200,   201,   202,
     203,    -1,   205,   206,     3,     4,     5,     6,     7,    -1,
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
     109,   110,   111,    -1,   113,   114,    -1,   116,    -1,    -1,
     119,   120,   121,   122,   123,   124,   125,    -1,   127,   128,
     129,    -1,   131,   132,    -1,    -1,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,   144,    -1,   146,   147,   148,
     149,   150,    -1,    -1,    -1,   154,    -1,    -1,   157,    -1,
      -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,   168,
     169,    -1,   171,   172,   173,   174,    -1,    -1,   177,    -1,
      -1,   180,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,
      -1,    -1,   191,   192,   193,    -1,    -1,   196,   197,    -1,
     199,   200,   201,   202,   203,    -1,   205,   206,     3,     4,
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
      -1,   116,    -1,    -1,   119,   120,   121,   122,   123,   124,
     125,    -1,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
      -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,   154,
      -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,
     165,   166,    -1,   168,   169,    -1,   171,   172,   173,    -1,
      -1,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,
      -1,   186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,
      -1,   196,   197,    -1,   199,   200,    -1,   202,   203,    -1,
     205,   206,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     111,    -1,   113,   114,    -1,   116,    -1,    -1,   119,   120,
     121,   122,   123,   124,   125,    -1,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,    -1,   146,   147,   148,   149,   150,
      -1,    -1,    -1,   154,    -1,    -1,   157,    -1,    -1,    -1,
      -1,   162,   163,   164,   165,   166,    -1,   168,   169,    -1,
     171,   172,   173,    -1,    -1,    -1,   177,    -1,    -1,   180,
      -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,
     191,   192,   193,    -1,    -1,   196,   197,    -1,   199,   200,
      -1,   202,   203,    -1,   205,   206,     3,     4,     5,     6,
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
      -1,    -1,   119,   120,   121,   122,   123,   124,   125,    -1,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,   144,    -1,   146,
     147,   148,   149,   150,    -1,    -1,    -1,   154,    -1,    -1,
     157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,
      -1,   168,   169,    -1,   171,   172,   173,    -1,    -1,    -1,
     177,    -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,   186,
      -1,    -1,    -1,    -1,   191,   192,   193,    -1,    -1,   196,
     197,    -1,   199,   200,    -1,   202,   203,    -1,   205,   206,
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
     113,   114,    -1,   116,    -1,    -1,   119,   120,   121,   122,
     123,   124,   125,    -1,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,
     143,   144,    -1,   146,   147,   148,   149,   150,    -1,    -1,
      -1,   154,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,
     163,   164,   165,   166,    -1,   168,   169,    -1,   171,   172,
     173,    -1,    -1,    -1,   177,    -1,    -1,   180,    -1,    -1,
      -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,
     193,    -1,    -1,   196,   197,    -1,   199,   200,    -1,   202,
     203,    -1,   205,   206,     3,     4,     5,     6,     7,    -1,
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
     109,   110,   111,    -1,   113,   114,    -1,   116,    -1,    -1,
     119,   120,   121,   122,   123,   124,   125,    -1,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,   144,    -1,   146,   147,   148,
     149,   150,    -1,    -1,    -1,   154,    -1,    -1,   157,    -1,
      -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,   168,
     169,    -1,   171,   172,   173,    -1,    -1,    -1,   177,    -1,
      -1,   180,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,
      -1,    -1,   191,   192,   193,    -1,    -1,   196,   197,    -1,
     199,   200,    -1,   202,   203,    -1,   205,   206,     3,     4,
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
      -1,   116,    -1,    -1,   119,   120,   121,   122,   123,   124,
     125,    -1,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
      -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,   154,
      -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,
     165,   166,    -1,   168,   169,    -1,   171,   172,   173,    -1,
      -1,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,
      -1,   186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,
      -1,   196,   197,    -1,   199,   200,    -1,   202,   203,    -1,
     205,   206,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,
     121,   122,   123,   124,   125,    -1,    -1,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,    -1,   146,   147,   148,   149,   150,
      -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,
      -1,   162,   163,   164,   165,   166,    -1,   168,   169,    -1,
     171,   172,   173,    -1,    -1,    -1,   177,    -1,    -1,   180,
      -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,
     191,   192,   193,    -1,    -1,   196,   197,    -1,    -1,    -1,
      -1,   202,   203,    -1,   205,   206,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   120,   121,   122,   123,   124,   125,    -1,
      -1,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,   144,    -1,   146,
     147,   148,   149,   150,    -1,    -1,    -1,    -1,    -1,    -1,
     157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,
      -1,   168,   169,    -1,   171,   172,   173,    -1,    -1,    -1,
     177,    -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,   186,
      -1,    -1,    -1,    -1,   191,   192,   193,    -1,    -1,   196,
     197,    -1,   199,    -1,    -1,   202,   203,    -1,   205,   206,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,   121,   122,
     123,   124,   125,    -1,    -1,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,
     143,   144,    -1,   146,   147,   148,   149,   150,    -1,    -1,
      -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,
     163,   164,   165,   166,    -1,   168,   169,    -1,   171,   172,
     173,    -1,    -1,    -1,   177,    -1,    -1,   180,    -1,    -1,
      -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,
     193,    -1,    -1,   196,   197,    -1,    -1,   200,    -1,   202,
     203,    -1,   205,   206,     3,     4,     5,     6,     7,    -1,
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
      -1,   120,   121,   122,   123,   124,   125,    -1,    -1,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,   144,    -1,   146,   147,   148,
     149,   150,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,
      -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,   168,
     169,    -1,   171,   172,   173,    -1,   175,    -1,   177,    -1,
      -1,   180,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,
      -1,    -1,   191,   192,   193,    -1,    -1,   196,   197,    -1,
      -1,    -1,    -1,   202,   203,    -1,   205,   206,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   120,   121,   122,   123,   124,
     125,    -1,    -1,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
      -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,    -1,
      -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,
     165,   166,    -1,   168,   169,    -1,   171,   172,   173,    -1,
      -1,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,
      -1,   186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,
      -1,   196,   197,    -1,    -1,    -1,    -1,   202,   203,    -1,
     205,   206,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,
     121,   122,   123,   124,   125,    -1,    -1,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,    -1,   146,   147,   148,   149,   150,
      -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,
      -1,   162,   163,   164,   165,   166,    -1,   168,   169,    -1,
     171,   172,   173,    -1,    -1,    -1,   177,    -1,    -1,   180,
      -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,
     191,   192,   193,    -1,    -1,   196,   197,    10,    11,    12,
      -1,   202,   203,    -1,   205,   206,     3,     4,     5,     6,
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
      -1,    -1,    -1,   120,   121,   122,   123,   124,   125,    -1,
      -1,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,   144,    -1,   146,
     147,   148,   149,   150,    -1,    -1,    -1,    -1,    -1,    -1,
     157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,
      -1,   168,   169,    -1,   171,   172,   173,    -1,    -1,    -1,
     177,    -1,    -1,   180,    -1,    -1,    -1,    -1,   201,   186,
      -1,    -1,    -1,    -1,   191,   192,   193,    -1,    -1,   196,
     197,    -1,    -1,    -1,    -1,   202,   203,    -1,   205,   206,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,   121,   122,
     123,   124,   125,    -1,    -1,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,
     143,   144,    -1,   146,   147,   148,   149,   150,    -1,    -1,
      -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,
     163,   164,   165,   166,    -1,   168,   169,    -1,   171,   172,
     173,    -1,    -1,    -1,   177,    -1,    -1,   180,    -1,    -1,
      -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,
     193,    -1,    -1,   196,   197,    10,    11,    12,    -1,   202,
     203,    -1,   205,   206,     3,     4,     5,     6,     7,    -1,
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
      -1,   120,   121,   122,   123,   124,   125,    -1,    -1,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,   144,    -1,   146,   147,   148,
     149,   150,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,
      -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,   168,
     169,    -1,   171,   172,   173,    -1,    -1,    -1,   177,    -1,
      -1,   180,    -1,    -1,    -1,    -1,   201,   186,    -1,    -1,
      -1,    -1,   191,   192,   193,    -1,    -1,   196,   197,    -1,
     199,    11,    12,   202,   203,    -1,   205,   206,     3,     4,
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
      -1,    -1,    -1,    -1,    -1,   120,   121,   122,   123,   124,
     125,    -1,    -1,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
      -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,    -1,
      -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,
     165,   166,    -1,   168,   169,    -1,   171,   172,   173,    -1,
      -1,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,
      -1,   186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,
      -1,   196,   197,    -1,   199,    -1,    -1,   202,   203,    -1,
     205,   206,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,
     121,   122,   123,   124,   125,    -1,    -1,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,    -1,   146,   147,   148,   149,   150,
      -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,
      -1,   162,   163,   164,   165,   166,    -1,   168,   169,    -1,
     171,   172,   173,    -1,    -1,    -1,   177,    -1,    -1,   180,
      -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,
     191,   192,   193,    -1,    -1,   196,   197,    10,    11,    12,
      -1,   202,   203,    -1,   205,   206,     3,     4,     5,     6,
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
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   120,   121,   122,   123,   124,   125,    -1,
      -1,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,   144,    -1,   146,
     147,   148,   149,   150,    -1,    -1,    -1,    -1,    -1,    -1,
     157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,
      -1,   168,   169,    -1,   171,   172,   173,    -1,    -1,    -1,
     177,    -1,    -1,   180,    -1,    -1,    -1,    -1,   201,   186,
      -1,    -1,    -1,    -1,   191,   192,   193,    -1,    -1,   196,
     197,   198,    -1,    -1,    -1,   202,   203,    -1,   205,   206,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,   121,   122,
     123,   124,   125,    -1,    -1,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,
     143,   144,    -1,   146,   147,   148,   149,   150,    -1,    -1,
      -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,
     163,   164,   165,   166,    -1,   168,   169,    -1,   171,   172,
     173,    -1,    -1,    -1,   177,    -1,    -1,   180,    -1,    -1,
      -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,
     193,    -1,    -1,   196,   197,    -1,    -1,    -1,    -1,   202,
     203,    -1,   205,   206,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    32,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   120,   121,   122,   123,   124,   125,    -1,    -1,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,   144,    -1,   146,   147,   148,
     149,   150,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,
      -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,   168,
     169,    -1,   171,   172,   173,    -1,    -1,    -1,   177,    -1,
      -1,   180,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,
      -1,    -1,   191,   192,   193,    -1,    -1,   196,   197,    -1,
      -1,    -1,    -1,   202,   203,    -1,   205,   206,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   120,   121,   122,   123,   124,
     125,    -1,    -1,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
      -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,    -1,
      -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,
     165,   166,    -1,   168,   169,    -1,   171,   172,   173,    -1,
      -1,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,
      -1,   186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,
      -1,   196,   197,    -1,    -1,    -1,    -1,   202,   203,    -1,
     205,   206,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    50,
      51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,
     121,   122,   123,   124,   125,    -1,    -1,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,    -1,   146,   147,   148,   149,   150,
      -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,
      -1,   162,   163,   164,   165,   166,    -1,   168,   169,    -1,
     171,   172,   173,    -1,    -1,    -1,   177,    -1,    -1,   180,
      -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,
     191,   192,   193,    -1,    -1,   196,   197,    -1,    -1,    -1,
      -1,   202,   203,    -1,   205,   206,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   120,   121,   122,   123,   124,   125,    -1,
      -1,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,   144,    -1,   146,
     147,   148,   149,   150,    -1,    -1,    -1,    -1,    -1,    -1,
     157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,
      -1,   168,   169,    -1,   171,   172,   173,    -1,    -1,    -1,
     177,    -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,   186,
      -1,    -1,    -1,    -1,   191,   192,   193,    -1,    -1,   196,
     197,    -1,    -1,    -1,    -1,   202,   203,    -1,   205,   206,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    50,    51,    -1,
      -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,
      83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,   121,   122,
     123,   124,   125,    -1,    -1,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,
     143,   144,    -1,   146,   147,   148,   149,   150,    -1,    -1,
      -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,
     163,   164,   165,   166,    -1,   168,   169,    -1,   171,   172,
     173,    -1,    -1,    -1,   177,    -1,    -1,   180,    -1,    -1,
      -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,
     193,    -1,    -1,   196,   197,    -1,    -1,    -1,    -1,   202,
     203,    -1,   205,   206,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   120,   121,   122,   123,   124,   125,    -1,    -1,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,   144,    -1,   146,   147,   148,
     149,   150,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,
      -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,   168,
     169,    -1,   171,   172,   173,    -1,    -1,    -1,   177,    -1,
      -1,   180,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,
      -1,    -1,   191,   192,   193,    -1,    -1,   196,   197,    10,
      11,    12,    -1,   202,   203,    -1,   205,   206,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    69,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   120,   121,   122,   123,   124,
     125,    -1,    -1,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
      -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,    -1,
      -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,
     165,   166,    -1,   168,   169,    -1,   171,   172,   173,    -1,
      -1,    -1,   177,    -1,    -1,   180,    -1,    -1,   199,    -1,
      -1,   186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,
      -1,   196,   197,    10,    11,    12,    -1,   202,   203,    -1,
     205,   206,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,
     121,   122,   123,   124,   125,    -1,    -1,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,    -1,   146,   147,   148,   149,   150,
      -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,
      -1,   162,   163,   164,   165,   166,    -1,   168,   169,    -1,
     171,   172,   173,    -1,     3,     4,   177,     6,     7,   180,
      -1,    10,    11,    12,    13,   186,    -1,    -1,    -1,    -1,
     191,   192,   193,    -1,    -1,   196,   197,    -1,    -1,    28,
      29,   202,   203,    -1,   205,   206,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,    81,
     129,    -1,   131,   132,   133,   134,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,   144,   145,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,   162,    10,    11,    12,    13,    -1,   168,
     169,    -1,   171,   172,   173,   174,    -1,   176,    -1,    -1,
     179,    -1,    28,    29,    -1,    31,    -1,    -1,   140,   141,
     142,   143,   144,    -1,    -1,    -1,    -1,   196,    -1,    -1,
      -1,   200,    -1,   202,    -1,    -1,    -1,    -1,    -1,    -1,
     162,    57,    -1,   165,   166,    -1,   168,   169,    -1,   171,
     172,   173,    68,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   196,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,    -1,   129,    -1,    -1,   132,   133,   134,    -1,
      -1,    -1,   138,   139,   140,   141,   142,   143,   144,   145,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   162,    -1,    -1,   165,
     166,    -1,   168,   169,    -1,   171,   172,   173,   174,    -1,
     176,    -1,    -1,   179,     3,     4,    -1,     6,     7,    -1,
     186,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
     196,   197,    -1,    -1,    -1,   201,    -1,    -1,    -1,    28,
      29,    -1,    31,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    57,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      -1,    69,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,    -1,
     129,    -1,    -1,   132,   133,   134,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,   144,   145,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   162,    -1,    -1,   165,   166,    -1,   168,
     169,    -1,   171,   172,   173,   174,    -1,   176,    -1,    -1,
     179,     3,     4,    -1,     6,     7,    -1,   186,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,   196,   197,    -1,
      -1,    -1,   201,    -1,    -1,    -1,    28,    29,    -1,    31,
      -1,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    57,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    69,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,    -1,   129,    -1,   131,
     132,   133,   134,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     162,    -1,    -1,   165,   166,    -1,   168,   169,    -1,   171,
     172,   173,   174,    -1,   176,    -1,    -1,   179,     3,     4,
      -1,     6,     7,    -1,   186,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,   196,   197,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    29,    -1,    31,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    68,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,    -1,   129,    -1,    -1,   132,   133,   134,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
     145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   162,    -1,    -1,
     165,   166,    -1,   168,   169,    -1,   171,   172,   173,   174,
     103,   176,    -1,    -1,   179,    -1,     3,     4,    -1,     6,
       7,   186,   187,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,   196,   197,   126,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    29,    -1,    31,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   162,
      57,    -1,   165,   166,    -1,   168,   169,    -1,   171,   172,
     173,    68,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   196,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,    -1,   129,    -1,    -1,   132,   133,   134,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,   144,   145,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,   162,    -1,    -1,   165,   166,
      -1,   168,   169,   103,   171,   172,   173,   174,    -1,   176,
      -1,    -1,   179,     3,     4,     5,     6,     7,    -1,   186,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   196,
     197,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
     140,   141,   142,   143,   144,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,   162,    57,    -1,   165,    -1,    57,   168,   169,
      -1,   171,   172,   173,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,   196,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,    -1,    -1,    -1,   138,   139,
     140,   141,   142,   143,   144,    -1,   146,   147,   148,   149,
     150,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   162,   163,   164,    -1,    81,    -1,   168,   169,
      -1,   171,   172,   173,   174,    -1,   176,   177,    -1,   179,
      -1,    -1,    -1,    -1,    -1,    -1,   186,   187,   103,   189,
      -1,   191,   192,    -1,     3,     4,   196,     6,     7,    -1,
      -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      29,    -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,
      -1,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    57,    -1,
     165,    -1,    -1,   168,   169,    -1,   171,   172,   173,    -1,
      -1,    69,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   196,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,    81,
     129,    -1,   131,   132,   133,   134,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,   144,   145,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,   162,    10,    11,    12,    13,    -1,   168,
     169,    -1,   171,   172,   173,   174,    -1,   176,    -1,    -1,
     179,    -1,    28,    29,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,    -1,    -1,    -1,    -1,   196,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,   165,    -1,    -1,   168,   169,    -1,   171,
     172,   173,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   196,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,    81,   129,    -1,   131,   132,   133,   134,    -1,
      -1,    -1,   138,   139,   140,   141,   142,   143,   144,   145,
      -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
       3,     4,    -1,     6,     7,    -1,   162,    10,    11,    12,
      13,    -1,   168,   169,    -1,   171,   172,   173,   174,    -1,
     176,    -1,    -1,   179,    -1,    28,    29,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
     196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,   168,
     169,    -1,   171,   172,   173,    -1,    -1,    -1,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   196,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,    -1,   129,    -1,    -1,   132,
     133,   134,    -1,    -1,    -1,   138,   139,   140,   141,   142,
     143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   162,
      -1,    -1,    -1,    -1,    -1,   168,   169,    -1,   171,   172,
     173,   174,    -1,   176,    -1,    -1,   179,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   196,    -1,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,   199,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,   199,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,   199,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    38,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    70,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      69,    83,    84,   199,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     198,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,   144,    -1,   146,   147,   148,   149,   150,    -1,
      -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,
     162,   163,   164,   165,   166,    -1,   168,   169,    -1,   171,
     172,   173,    -1,    50,    51,   177,    -1,    -1,   198,    56,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   191,
      -1,    -1,    -1,    70,   196,   197,    -1,    -1,    -1,    -1,
     202,    78,    79,    80,    81,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,    69,   146,
     147,   148,   149,   150,    -1,    -1,    -1,    50,    51,    -1,
     157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,
      -1,   168,   169,    -1,   171,   172,   173,    70,    -1,    -1,
     177,    -1,    -1,    -1,    -1,    78,    79,    80,    81,   186,
      -1,    -1,    -1,    -1,   191,    -1,    -1,    -1,    91,   196,
     197,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    70,    -1,    -1,    -1,   137,    -1,    -1,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,   139,   140,   141,   142,
     143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   120,    -1,   157,    -1,    -1,    -1,    -1,   162,
      -1,    -1,   165,   166,    -1,   168,   169,    -1,   171,   172,
     173,   139,   140,   141,   142,   143,   144,    -1,   146,   147,
     148,   149,   150,   186,    -1,    -1,    -1,    -1,    -1,   157,
      -1,    -1,    -1,   196,   162,   163,   164,   165,   166,    -1,
     168,   169,    -1,   171,   172,   173,    70,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,   191,    -1,    -1,    -1,    91,   196,   197,
      -1,    -1,   200,    -1,   202,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
     144,    -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,
      -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,
     164,   165,   166,    -1,   168,   169,    -1,   171,   172,   173,
      70,    71,    -1,   177,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,   191,    -1,    -1,
      -1,    91,   196,   197,    -1,    -1,    -1,    -1,   202,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,    -1,   146,   147,   148,   149,
     150,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,
      -1,    -1,   162,   163,   164,   165,   166,    -1,   168,   169,
      -1,   171,   172,   173,    70,    -1,    -1,   177,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    -1,    83,    84,    -1,
      -1,   191,    -1,    -1,    -1,    91,   196,   197,    -1,    -1,
      -1,    -1,   202,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,   142,   143,   144,    -1,
     146,   147,   148,   149,   150,    -1,    -1,    -1,    -1,    -1,
      -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,
     166,    -1,   168,   169,    -1,   171,   172,   173,    70,    -1,
      -1,   177,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,   191,    -1,    -1,    -1,    91,
     196,   197,    -1,    -1,    -1,    -1,   202,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,
     142,   143,   144,    -1,   146,   147,   148,   149,   150,    -1,
      -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,
     162,   163,   164,   165,   166,    -1,   168,   169,    -1,   171,
     172,   173,    -1,    -1,    -1,   177,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   191,
      -1,    -1,    -1,    -1,   196,   197,    -1,    -1,    30,    31,
     202,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   137,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,    -1,    31,    -1,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,   137,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,   137,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    -1,   137,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    -1,   137,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,   137,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,    -1,    -1,    -1,    69,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   131,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,   144,
      -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,    -1,
      -1,    -1,   157,   137,    -1,    -1,    -1,   162,   163,   164,
     165,   166,    -1,   168,   169,    -1,   171,   172,   173,    -1,
      -1,    -1,   177,    -1,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,    -1,    -1,    -1,   191,    91,    -1,    -1,
      -1,   196,   197,    -1,    -1,    -1,    -1,   202,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,   143,
     144,    -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,
      -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,
     164,   165,   166,    -1,   168,   169,    -1,   171,   172,   173,
      -1,    -1,    -1,   177,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,   191,    91,    -1,
      -1,    -1,   196,   197,    -1,    -1,    -1,    -1,   202,    -1,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,   103,   146,   147,   148,   149,   150,    -1,    -1,
     111,   112,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,
     163,   164,   165,   166,    -1,   168,   169,    -1,   171,   172,
     173,    -1,    -1,    -1,   177,    -1,    -1,    -1,    -1,   140,
     141,   142,   143,   144,    -1,    -1,    -1,    -1,   191,    -1,
      -1,    -1,    -1,   196,   197,    -1,    -1,    -1,    -1,   202,
      -1,   162,    -1,    -1,   165,    -1,    -1,   168,   169,    -1,
     171,   172,   173,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   196,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    32,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    12,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      69,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   208,   209,     0,   210,     3,     4,     5,     6,     7,
      13,    27,    28,    29,    48,    50,    51,    56,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    70,
      71,    72,    73,    74,    78,    79,    80,    81,    82,    83,
      84,    86,    87,    91,    92,    93,    94,    96,    98,   100,
     103,   104,   108,   109,   110,   111,   112,   113,   114,   116,
     118,   119,   120,   121,   122,   123,   124,   125,   127,   128,
     129,   130,   131,   132,   138,   139,   140,   141,   142,   143,
     144,   146,   147,   148,   149,   150,   154,   157,   162,   163,
     164,   165,   166,   168,   169,   171,   172,   173,   174,   177,
     180,   186,   187,   189,   191,   192,   193,   196,   197,   199,
     200,   202,   203,   205,   206,   211,   214,   224,   225,   226,
     227,   228,   234,   243,   244,   255,   256,   260,   263,   270,
     276,   336,   337,   345,   346,   349,   350,   351,   352,   353,
     354,   355,   356,   358,   359,   360,   362,   365,   377,   378,
     385,   388,   391,   394,   397,   403,   405,   406,   408,   418,
     419,   420,   422,   427,   432,   452,   460,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     476,   489,   491,   493,   120,   121,   122,   138,   162,   172,
     197,   214,   255,   336,   358,   464,   358,   197,   358,   358,
     358,   358,   108,   358,   358,   450,   451,   358,   358,   358,
     358,    81,    83,    91,   120,   140,   141,   142,   143,   144,
     157,   197,   225,   378,   419,   422,   427,   464,   468,   464,
     358,   358,   358,   358,   358,   358,   358,   358,    38,   358,
     480,   481,   120,   131,   197,   225,   268,   419,   420,   421,
     423,   427,   461,   462,   463,   472,   477,   478,   358,   197,
     357,   424,   197,   357,   369,   347,   358,   236,   357,   197,
     197,   197,   357,   199,   358,   214,   199,   358,     3,     4,
       6,     7,    10,    11,    12,    13,    28,    29,    31,    57,
      68,    71,    72,    73,    74,    75,    76,    77,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   129,   131,   132,   133,   134,   138,   139,
     145,   162,   166,   174,   176,   179,   186,   197,   214,   215,
     216,   227,   494,   514,   515,   518,    27,   199,   352,   354,
     358,   200,   248,   358,   111,   112,   162,   165,   187,   217,
     218,   219,   220,   224,    83,   202,   302,   303,    83,   304,
     122,   131,   121,   131,   197,   197,   197,   197,   214,   274,
     497,   197,   197,    70,    70,    70,    70,    70,   347,    83,
      90,   158,   159,   160,   486,   487,   165,   200,   224,   224,
     214,   275,   497,   166,   197,   497,   497,    83,   193,   200,
     370,    28,   346,   349,   358,   360,   464,   469,   231,   200,
      90,   425,   486,    90,   486,   486,    32,   165,   182,   498,
     197,     9,   199,   197,   345,   359,   465,   468,   117,    38,
     254,   166,   273,   497,   120,   192,   255,   337,    70,   200,
     459,   199,   199,   199,   199,   199,   199,   199,   199,    10,
      11,    12,    30,    31,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    57,    69,   199,
      70,    70,   200,   161,   132,   172,   174,   187,   189,   276,
     335,   336,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    59,    60,   135,   136,   454,
     459,   459,   197,   197,    70,   200,   202,   473,   197,   254,
     255,    14,   358,   199,   137,    49,   214,   449,    90,   346,
     360,   161,   464,   137,   204,     9,   434,   269,   346,   360,
     464,   498,   161,   197,   426,   454,   459,   198,   358,    32,
     234,     8,   371,     9,   199,   234,   235,   347,   348,   358,
     214,   288,   238,   199,   199,   199,   139,   145,   518,   518,
     182,   517,   197,   111,   518,    14,   161,   139,   145,   162,
     214,   216,   199,   199,   199,   249,   115,   179,   199,   217,
     219,   217,   219,   217,   219,   224,   217,   219,   200,     9,
     435,   199,   102,   165,   200,   464,     9,   199,    14,     9,
     199,   131,   131,   464,   490,   347,   346,   360,   464,   468,
     469,   198,   182,   266,   138,   464,   479,   480,   358,   379,
     380,   347,   400,   400,   379,   400,   199,    70,   454,   158,
     487,    82,   358,   464,    90,   158,   487,   224,   213,   199,
     200,   261,   271,   409,   411,    91,   197,   202,   372,   373,
     375,   418,   422,   471,   473,   491,    14,   102,   492,   366,
     367,   368,   298,   299,   452,   453,   198,   198,   198,   198,
     198,   201,   233,   234,   256,   263,   270,   452,   358,   203,
     205,   206,   214,   499,   500,   518,    38,   175,   300,   301,
     358,   494,   245,   246,   346,   354,   355,   358,   360,   464,
     200,   247,   247,   247,   247,   197,   497,   264,   254,   358,
     475,   358,   358,   358,   358,   358,    32,   358,   358,   358,
     358,   358,   358,   358,   358,   358,   358,   358,   358,   358,
     358,   358,   358,   358,   358,   358,   358,   358,   358,   358,
     358,   423,   358,   475,   475,   358,   482,   483,   131,   200,
     215,   216,   472,   473,   274,   214,   275,   497,   497,   273,
     255,    38,   349,   352,   354,   358,   358,   358,   358,   358,
     358,   358,   358,   358,   358,   358,   358,   358,   166,   200,
     214,   455,   456,   457,   458,   472,   300,   300,   475,   358,
     479,   254,   198,   358,   197,   448,     9,   434,   198,   198,
      38,   358,    38,   358,   426,   198,   198,   198,   472,   300,
     200,   214,   455,   456,   472,   198,   231,   292,   200,   354,
     358,   358,    94,    32,   234,   286,   199,    27,   102,    14,
       9,   198,    32,   200,   289,   518,    31,    91,   175,   227,
     511,   512,   513,   197,     9,    50,    51,    56,    58,    70,
     139,   140,   141,   142,   143,   144,   186,   197,   225,   386,
     389,   392,   395,   398,   404,   419,   427,   428,   430,   431,
     214,   516,   231,   197,   242,   200,   199,   200,   199,   200,
     199,   102,   165,   200,   199,   111,   112,   165,   220,   221,
     222,   223,   224,   220,   214,   358,   303,   428,    83,     9,
     198,   198,   198,   198,   198,   198,   198,   199,    50,    51,
     507,   509,   510,   133,   279,   197,     9,   198,   198,   137,
     204,     9,   434,     9,   434,   204,   204,   204,   204,    83,
      85,   214,   488,   214,    70,   201,   201,   210,   212,    32,
     134,   278,   181,    54,   166,   181,   413,   360,   137,     9,
     434,   198,   161,   518,   518,    14,   371,   298,   229,   194,
       9,   435,   518,   519,   454,   454,   201,     9,   434,   183,
     464,   358,   198,     9,   435,    14,     9,   198,     9,   198,
     198,   198,   198,    14,   198,   201,   232,   233,   363,   257,
     133,   277,   197,   497,   204,   201,   358,    32,   204,   204,
     137,   201,     9,   434,   358,   498,   197,   267,   262,   272,
      14,   492,   265,   254,    71,   464,   358,   498,   198,   198,
     204,   201,   198,    50,    51,    70,    78,    79,    80,    91,
     139,   140,   141,   142,   143,   144,   157,   186,   214,   387,
     390,   393,   396,   399,   419,   430,   437,   439,   440,   444,
     447,   214,   464,   464,   137,   277,   454,   459,   454,   198,
     358,   293,    75,    76,   294,   229,   357,   231,   348,   102,
      38,   138,   283,   464,   428,   214,    32,   234,   287,   199,
     290,   199,   290,     9,   434,    91,   227,   137,   161,     9,
     434,   198,   499,   500,   501,   518,   519,   499,   428,   428,
     428,   428,   428,   433,   436,   197,    70,    70,    70,    70,
      70,   197,   428,   161,   200,    10,    11,    12,    31,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    69,   161,   498,   201,   419,   200,   251,   219,   219,
     219,   214,   219,   220,   220,   224,     9,   435,   201,   201,
      14,   464,   199,   183,     9,   434,   214,   280,   419,   200,
     479,   138,   464,    14,   358,   358,   204,   358,   201,   210,
     518,   280,   200,   412,    14,   198,   358,   372,   472,   199,
     518,   194,   201,   230,   233,   243,    32,   505,   453,    38,
      83,   175,   455,   456,   458,   455,   456,   458,   518,    38,
     175,   358,   428,   246,   354,   355,   464,   247,   246,   247,
     247,   201,   233,   298,   197,   419,   278,   364,   258,   358,
     358,   358,   201,   197,   300,   279,    32,   278,   518,    14,
     277,   497,   423,   201,   197,    14,    78,    79,    80,   214,
     438,   438,   440,   442,   443,    52,   197,    70,    70,    70,
      70,    70,    90,   158,   197,   161,     9,   434,   198,   448,
      38,   358,   278,   201,    75,    76,   295,   357,   234,   201,
     199,    95,   199,   283,   464,   197,   137,   282,    14,   231,
     290,   105,   106,   107,   290,   201,   518,   183,   137,   161,
     518,   214,   175,   511,     9,   198,   175,   434,   137,   204,
       9,   434,   433,   381,   382,   428,   401,   428,   429,   401,
     381,   401,   372,   374,   376,   198,   131,   215,   428,   484,
     485,   428,   428,   428,    32,   428,   428,   428,   428,   428,
     428,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   428,   428,   428,   428,   428,   428,   428,   428,   516,
      83,   252,   201,   201,   201,   201,   223,   199,   428,   510,
     102,   103,   506,   508,     9,   308,   198,   197,   349,   354,
     358,   137,   204,   201,   492,   308,   167,   180,   200,   408,
     415,   167,   200,   414,   137,   199,   505,   197,   246,   345,
     359,   465,   468,   518,   371,   519,    83,   175,    14,    83,
     498,   498,   464,   358,   198,   298,   200,   298,   197,   137,
     197,   300,   198,   200,   518,   200,   199,   518,   278,   259,
     426,   300,   137,   204,     9,   434,   439,   442,   383,   384,
     440,   402,   440,   441,   402,   383,   402,   158,   372,   445,
     446,    81,   440,   464,   200,   357,    32,    77,   234,   199,
     348,   282,   479,   283,   198,   428,   101,   105,   199,   358,
      32,   199,   291,   201,   183,   518,   214,   137,   518,   519,
      32,   198,   428,   428,   198,   204,     9,   434,   137,   204,
       9,   434,   204,   204,   204,   137,     9,   434,   198,   137,
     201,     9,   434,   428,    32,   198,   231,   199,   199,   199,
     199,   214,   518,   518,   506,   419,     6,   112,   118,   121,
     126,   168,   169,   171,   201,   309,   334,   335,   336,   341,
     342,   343,   344,   452,   479,   358,   201,   200,   201,    54,
     358,   358,   358,   371,   464,   199,   200,    38,    83,   175,
      14,    83,   358,   197,   197,   505,   198,   308,   198,   298,
     358,   300,   198,   308,   492,   308,   199,   200,   197,   198,
     440,   440,   198,   204,     9,   434,   137,   204,     9,   434,
     204,   204,   204,   137,   198,     9,   434,   308,    32,   231,
     199,   198,   198,   198,   239,   199,   199,   291,   231,   137,
     518,   175,   518,   137,   428,   428,   428,   428,   372,   428,
     428,   428,   200,   201,   508,   133,   134,   187,   215,   495,
     518,   281,   419,   112,   344,    31,   126,   139,   145,   166,
     172,   318,   319,   320,   321,   419,   170,   326,   327,   129,
     197,   214,   328,   329,   310,   255,   518,     9,   199,     9,
     199,   199,   492,   335,   198,   305,   166,   410,   201,   201,
     358,    83,   175,    14,    83,   358,   300,   300,   118,   361,
     505,   201,   505,   198,   198,   201,   200,   201,   308,   298,
     137,   440,   440,   440,   440,   372,   201,   231,   237,   240,
      32,   234,   285,   231,   518,   198,   428,   137,   137,   137,
     231,   419,   419,   497,    14,   215,     9,   199,   200,   495,
     492,   321,   182,   200,     9,   199,     3,     4,     5,     6,
       7,    10,    11,    12,    13,    27,    28,    29,    57,    71,
      72,    73,    74,    75,    76,    77,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   138,   139,
     146,   147,   148,   149,   150,   162,   163,   164,   174,   176,
     177,   179,   186,   187,   189,   191,   192,   214,   416,   417,
       9,   199,   166,   170,   214,   329,   330,   331,   199,    83,
     340,   254,   311,   495,   495,    14,   255,   201,   306,   307,
     495,    14,    83,   358,   198,   198,   197,   505,   196,   502,
     361,   505,   305,   201,   198,   440,   137,   137,    32,   234,
     284,   285,   231,   428,   428,   428,   201,   199,   199,   428,
     419,   314,   518,   322,   323,   427,   319,    14,    32,    51,
     324,   327,     9,    36,   198,    31,    50,    53,    14,     9,
     199,   216,   496,   340,    14,   518,   254,   199,    14,   358,
      38,    83,   407,   200,   503,   504,   518,   199,   200,   332,
     505,   502,   201,   505,   440,   440,   231,    99,   250,   201,
     214,   227,   315,   316,   317,     9,   434,     9,   434,   201,
     428,   417,   417,    68,   325,   330,   330,    31,    50,    53,
     428,    83,   182,   197,   199,   428,   496,   428,    83,     9,
     435,   229,     9,   435,    14,   506,   229,   200,   332,   332,
      97,   199,   115,   241,   161,   102,   518,   183,   427,   173,
      14,   507,   312,   197,    38,    83,   198,   201,   504,   518,
     201,   229,   199,   197,   179,   253,   214,   335,   336,   183,
     428,   183,   296,   297,   453,   313,    83,   201,   419,   251,
     176,   214,   199,   198,     9,   435,   123,   124,   125,   338,
     339,   296,    83,   281,   199,   505,   453,   519,   198,   198,
     199,   502,   338,    38,    83,   175,   505,   200,   199,   200,
     333,   519,    83,   175,    14,    83,   502,   231,   229,    38,
      83,   175,    14,    83,   358,   333,   201,   201,    83,   175,
      14,    83,   358,    14,    83,   358,   358
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   207,   209,   208,   210,   210,   211,   211,   211,   211,
     211,   211,   211,   211,   212,   211,   213,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   215,   215,   215,   215,   215,   215,   215,   215,
     215,   215,   215,   215,   215,   215,   215,   215,   215,   215,
     215,   215,   215,   215,   215,   215,   215,   215,   215,   215,
     215,   215,   215,   215,   215,   215,   215,   215,   215,   215,
     215,   215,   215,   215,   215,   215,   215,   215,   215,   215,
     215,   215,   215,   215,   215,   215,   215,   215,   215,   215,
     215,   215,   215,   215,   215,   215,   215,   216,   216,   217,
     217,   218,   218,   219,   220,   220,   220,   220,   221,   221,
     222,   223,   223,   223,   224,   224,   225,   225,   225,   226,
     227,   228,   228,   229,   229,   230,   230,   231,   231,   232,
     232,   233,   233,   233,   233,   234,   234,   234,   235,   234,
     236,   234,   237,   234,   238,   234,   234,   234,   234,   234,
     234,   234,   234,   234,   234,   234,   234,   234,   234,   234,
     234,   239,   234,   240,   234,   234,   241,   234,   242,   234,
     234,   234,   234,   234,   234,   234,   234,   234,   234,   234,
     234,   234,   234,   234,   234,   234,   234,   234,   234,   234,
     234,   243,   244,   244,   245,   245,   246,   246,   246,   247,
     247,   249,   248,   250,   250,   252,   251,   253,   253,   254,
     254,   255,   257,   256,   258,   256,   259,   256,   261,   260,
     262,   260,   264,   263,   265,   263,   266,   263,   267,   263,
     269,   268,   271,   270,   272,   270,   273,   273,   274,   275,
     276,   276,   276,   276,   276,   277,   277,   278,   278,   279,
     279,   280,   280,   281,   281,   282,   282,   283,   283,   283,
     284,   284,   285,   285,   286,   286,   287,   287,   288,   288,
     289,   289,   289,   289,   290,   290,   290,   291,   291,   292,
     292,   293,   293,   294,   294,   295,   295,   296,   296,   296,
     296,   296,   296,   296,   296,   297,   297,   297,   297,   297,
     297,   297,   297,   298,   298,   298,   298,   298,   298,   298,
     298,   299,   299,   299,   299,   299,   299,   299,   299,   300,
     300,   301,   301,   301,   301,   301,   301,   302,   302,   303,
     303,   303,   304,   304,   304,   304,   305,   305,   306,   307,
     308,   308,   310,   309,   311,   309,   309,   309,   309,   312,
     309,   313,   309,   309,   309,   309,   309,   309,   309,   309,
     314,   314,   314,   315,   316,   316,   317,   317,   318,   318,
     319,   319,   320,   320,   321,   321,   321,   321,   321,   321,
     321,   322,   322,   323,   324,   324,   325,   325,   326,   326,
     327,   328,   328,   328,   329,   329,   329,   329,   330,   330,
     330,   330,   330,   330,   330,   331,   331,   331,   332,   332,
     333,   333,   334,   334,   335,   335,   336,   336,   337,   337,
     337,   337,   337,   337,   337,   338,   338,   339,   339,   339,
     340,   340,   340,   340,   341,   341,   342,   342,   343,   343,
     344,   345,   346,   346,   346,   346,   346,   346,   347,   347,
     348,   348,   349,   349,   349,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   358,   358,   358,   358,   359,
     360,   360,   360,   360,   360,   360,   360,   360,   360,   360,
     360,   360,   360,   360,   360,   360,   360,   360,   360,   360,
     360,   360,   360,   360,   360,   360,   360,   360,   360,   360,
     360,   360,   360,   360,   360,   360,   360,   360,   360,   360,
     360,   360,   360,   360,   360,   360,   360,   360,   360,   360,
     360,   360,   360,   360,   360,   360,   360,   360,   360,   360,
     360,   360,   360,   360,   360,   360,   360,   360,   360,   360,
     360,   360,   360,   360,   360,   360,   360,   361,   361,   363,
     362,   364,   362,   366,   365,   367,   365,   368,   365,   369,
     365,   370,   365,   371,   371,   371,   372,   372,   373,   373,
     374,   374,   375,   375,   376,   376,   377,   378,   378,   379,
     379,   380,   380,   381,   381,   382,   382,   383,   383,   384,
     384,   385,   386,   387,   388,   389,   390,   391,   392,   393,
     394,   395,   396,   397,   398,   399,   400,   400,   401,   401,
     402,   402,   403,   404,   405,   405,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   407,   407,   407,
     407,   408,   409,   409,   410,   410,   411,   411,   412,   412,
     413,   414,   414,   415,   415,   415,   416,   416,   416,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   418,   419,   419,   420,   420,   420,   420,   420,   421,
     421,   422,   422,   422,   422,   423,   423,   423,   424,   424,
     424,   425,   425,   425,   426,   426,   427,   427,   427,   427,
     427,   427,   427,   427,   427,   427,   427,   427,   427,   427,
     427,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   428,   428,   428,   428,   428,   429,   429,   430,   431,
     431,   432,   432,   432,   432,   432,   432,   432,   433,   433,
     434,   434,   435,   435,   436,   436,   436,   436,   437,   437,
     437,   437,   437,   438,   438,   438,   438,   439,   439,   440,
     440,   440,   440,   440,   440,   440,   440,   440,   440,   440,
     440,   440,   440,   440,   441,   441,   442,   442,   443,   443,
     443,   443,   444,   444,   445,   445,   446,   446,   447,   447,
     448,   448,   449,   449,   451,   450,   452,   453,   453,   454,
     454,   455,   455,   455,   456,   456,   457,   457,   458,   458,
     459,   459,   460,   460,   460,   461,   461,   462,   462,   463,
     463,   464,   464,   464,   464,   464,   464,   464,   464,   464,
     464,   464,   465,   466,   466,   466,   466,   466,   466,   466,
     467,   467,   467,   467,   467,   467,   467,   467,   467,   468,
     469,   469,   470,   470,   470,   471,   471,   471,   472,   472,
     473,   473,   473,   474,   474,   474,   475,   475,   476,   476,
     477,   477,   477,   477,   477,   477,   478,   478,   478,   478,
     478,   479,   479,   479,   479,   479,   479,   480,   480,   481,
     481,   481,   481,   481,   481,   481,   481,   482,   482,   483,
     483,   483,   483,   484,   484,   485,   485,   485,   485,   486,
     486,   486,   486,   487,   487,   487,   487,   487,   487,   488,
     488,   488,   489,   489,   489,   489,   489,   489,   489,   489,
     489,   489,   489,   490,   490,   491,   491,   492,   492,   493,
     493,   493,   493,   494,   494,   495,   495,   496,   496,   497,
     497,   498,   498,   499,   499,   500,   501,   501,   501,   501,
     502,   502,   503,   503,   504,   504,   505,   505,   506,   506,
     507,   508,   508,   509,   509,   509,   509,   510,   510,   510,
     511,   511,   511,   511,   512,   512,   513,   513,   513,   513,
     514,   515,   516,   516,   517,   517,   518,   518,   518,   518,
     518,   518,   518,   518,   518,   518,   518,   519,   519
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
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       3,     3,     1,     2,     1,     2,     3,     4,     3,     1,
       2,     1,     2,     2,     1,     3,     1,     3,     2,     2,
       2,     5,     4,     2,     0,     1,     3,     2,     0,     2,
       1,     1,     1,     1,     1,     3,     5,     8,     0,     4,
       0,     6,     0,    10,     0,     4,     2,     3,     2,     3,
       2,     3,     3,     3,     3,     3,     3,     5,     1,     1,
       1,     0,     9,     0,    10,     5,     0,    13,     0,     5,
       3,     3,     3,     3,     5,     5,     5,     3,     3,     2,
       2,     2,     2,     2,     2,     3,     2,     2,     3,     2,
       2,     2,     1,     0,     3,     3,     1,     1,     1,     3,
       2,     0,     4,     9,     0,     0,     4,     2,     0,     1,
       0,     1,     0,    10,     0,    11,     0,    11,     0,     9,
       0,    10,     0,     8,     0,     9,     0,     7,     0,     8,
       0,     8,     0,     7,     0,     8,     1,     1,     1,     1,
       1,     2,     3,     3,     2,     2,     0,     2,     0,     2,
       0,     1,     3,     1,     3,     2,     0,     1,     2,     4,
       1,     4,     1,     4,     1,     4,     1,     4,     3,     5,
       3,     4,     4,     5,     5,     4,     0,     1,     1,     4,
       0,     5,     0,     2,     0,     3,     0,     7,     8,     6,
       2,     5,     6,     4,     0,     4,     5,     7,     6,     6,
       7,     9,     8,     6,     7,     5,     2,     4,     5,     3,
       0,     3,     4,     6,     5,     5,     6,     8,     7,     2,
       0,     1,     2,     2,     3,     4,     4,     3,     1,     1,
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
       1,     4,     1,     1,     1,     1,     1,     1,     3,     1,
       3,     1,     1,     1,     3,     1,     1,     1,     2,     1,
       0,     0,     1,     1,     3,     0,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       2,     1,     1,     4,     3,     4,     1,     1,     1,     1,
       1,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     2,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     5,     4,     3,     1,     3,     3,
       1,     1,     1,     1,     1,     3,     3,     3,     2,     0,
       1,     0,     1,     0,     5,     3,     3,     1,     1,     1,
       1,     3,     2,     1,     1,     1,     1,     1,     3,     1,
       1,     1,     3,     1,     2,     2,     4,     3,     4,     1,
       1,     1,     1,     1,     3,     1,     2,     0,     5,     3,
       3,     1,     3,     1,     2,     0,     5,     3,     2,     0,
       3,     0,     4,     2,     0,     3,     3,     1,     0,     1,
       1,     1,     1,     3,     1,     1,     1,     3,     1,     1,
       3,     3,     2,     2,     2,     2,     4,     5,     5,     5,
       5,     1,     1,     1,     1,     1,     1,     3,     3,     4,
       4,     3,     3,     1,     1,     1,     1,     3,     1,     4,
       1,     1,     1,     1,     1,     3,     3,     4,     4,     3,
       1,     1,     7,     9,     9,     7,     6,     8,     1,     2,
       4,     4,     1,     1,     1,     4,     1,     0,     1,     2,
       1,     1,     1,     3,     3,     3,     0,     1,     1,     3,
       3,     2,     3,     6,     0,     1,     4,     2,     0,     5,
       3,     3,     1,     6,     4,     4,     2,     2,     0,     5,
       3,     3,     1,     2,     0,     5,     3,     3,     1,     2,
       2,     1,     2,     1,     4,     3,     3,     6,     3,     1,
       1,     1,     4,     4,     4,     4,     4,     4,     2,     2,
       4,     2,     2,     1,     3,     3,     3,     0,     2,     5,
       6,     6,     7,     1,     2,     1,     2,     1,     4,     1,
       4,     3,     0,     1,     3,     2,     4,     1,     2,     0,
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
#line 752 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
#line 7198 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 755 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 7206 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 762 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 7212 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 763 "hphp.y" /* yacc.c:1646  */
    { }
#line 7218 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 766 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 7224 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 767 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7230 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 768 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7236 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 769 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7242 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 770 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7248 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 771 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 7254 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 772 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 7262 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 775 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 7269 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 7275 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 778 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7281 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 779 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 7287 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 780 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7293 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 781 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7301 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 785 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7310 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 790 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7319 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 795 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7326 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 798 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7333 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 801 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7341 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 805 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7349 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 809 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7357 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 813 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7365 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 817 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7373 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 820 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7380 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 825 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7386 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 826 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7392 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 827 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7398 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 828 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7404 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 829 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7410 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 830 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7416 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 831 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7422 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7428 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7434 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 834 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7440 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7446 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7452 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7458 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 917 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7464 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 919 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7470 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 924 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7476 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 925 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7483 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 931 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7489 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 935 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7495 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 936 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7501 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 938 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7507 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 940 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7513 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 945 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7519 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 946 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7526 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 952 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7532 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 956 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7539 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 958 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7546 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 960 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7553 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 965 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7559 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 967 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7565 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 970 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7571 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 972 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7577 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 973 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7583 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 978 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7592 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 985 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7601 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 993 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7608 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 996 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7615 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 1002 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 7621 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1003 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval)); }
#line 7627 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1008 "hphp.y" /* yacc.c:1646  */
    {
                                         _p->onUsing((yyval), (yyvsp[-2]), true, (yyvsp[-1]), nullptr);
                                       }
#line 7635 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 1015 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7641 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1016 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7647 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1021 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 7653 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1022 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval), (yyval), (yyvsp[0])); }
#line 7660 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1027 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7666 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1028 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7672 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1029 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7678 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1030 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7684 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1033 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7690 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1037 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7696 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1042 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7702 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1043 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7709 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1045 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7717 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1049 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7724 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1052 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7732 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1056 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7739 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1058 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7747 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1061 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7754 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1063 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7762 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1066 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7768 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1067 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7774 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1068 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7780 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1069 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7786 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1070 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7792 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1071 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7798 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1072 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7804 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1073 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7810 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1074 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7816 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1075 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7822 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1076 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7828 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1077 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 7834 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1078 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 7840 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1079 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 7846 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1080 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 7853 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1084 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7860 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1086 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 7868 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1091 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7875 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1093 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 7883 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1097 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 7891 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1106 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7897 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1107 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 7903 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1110 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false); }
#line 7909 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1111 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 7915 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1113 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7923 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1117 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7931 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1121 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7939 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1125 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7947 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1129 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7955 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1133 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7963 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1138 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7971 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1141 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 7977 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1142 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 7986 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1146 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7992 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1147 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7998 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1148 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8004 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1149 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8010 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1150 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8016 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1151 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8022 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1152 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 8028 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1153 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8034 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1154 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8040 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1155 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 8046 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1156 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8052 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1157 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 8062 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1179 "hphp.y" /* yacc.c:1646  */
    { _p->pushLabelScope(LS::Using);
                                         _p->onNewLabelScope(false);
                                         (yyval) = (yyvsp[-1]); }
#line 8070 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1185 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1; }
#line 8076 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1186 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8082 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1195 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), nullptr, (yyvsp[-2]));
                                         _p->onExprListElem((yyval), &(yyval), (yyvsp[0])); }
#line 8089 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1197 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0])); }
#line 8095 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 1207 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8101 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 1208 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8107 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1212 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false); }
#line 8113 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1213 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8119 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1222 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 8125 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1223 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8131 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1227 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::Finally);}
#line 8138 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1229 "hphp.y" /* yacc.c:1646  */
    { _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->popLabelScope();
                                         _p->onCompleteLabelScope(false);}
#line 8146 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1235 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8152 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1236 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8158 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1240 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 8164 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1241 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8170 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1245 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 8176 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1251 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8185 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1258 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8195 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1266 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8204 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1273 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8214 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1281 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8223 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1287 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8233 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1296 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8240 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1300 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 8246 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1304 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8253 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1308 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 8259 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1314 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8266 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1317 "hphp.y" /* yacc.c:1646  */
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
#line 8284 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1332 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8291 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 1335 "hphp.y" /* yacc.c:1646  */
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
#line 8309 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1349 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8316 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 1352 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8324 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1357 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8331 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1360 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8339 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1366 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 8345 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1369 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 8351 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1373 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8358 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1376 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8369 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1384 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8376 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1387 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8387 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1395 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8393 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1396 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 8400 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1400 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8406 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1403 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8412 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1406 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 8418 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1407 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 8424 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1408 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 8432 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1411 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 8438 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1412 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 8444 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1416 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8450 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1417 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8456 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1420 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8462 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1421 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8468 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1424 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8474 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1425 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8480 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1428 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 8486 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1430 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8492 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1433 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 8498 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1435 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8504 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1439 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8510 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1440 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8516 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1443 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8522 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1444 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8528 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1445 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8534 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1449 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8540 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1451 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8546 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1454 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8552 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1456 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8558 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8564 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1461 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8570 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1464 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8576 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1466 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8582 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1470 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8588 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1472 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8595 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1477 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8601 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1478 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8607 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1479 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8613 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1480 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8619 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1485 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8625 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1487 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8631 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1488 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8637 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1491 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8643 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1492 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8649 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1497 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8655 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1498 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8661 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1503 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8667 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1504 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8673 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1507 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8679 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1508 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8685 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1511 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8691 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1512 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8697 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1520 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8704 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1526 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8711 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1532 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8719 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1536 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8725 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1540 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8732 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1545 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8739 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1550 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8747 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1553 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8753 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1559 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8760 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1563 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8767 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1568 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8774 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1573 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8781 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1578 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8788 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1583 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8795 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1589 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8802 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1595 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8809 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1603 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 8816 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1608 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 8823 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1613 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 8831 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1617 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8837 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1620 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 8844 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1624 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 8851 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1628 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 8859 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1631 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8865 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1636 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8872 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1639 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8879 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1643 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8886 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1647 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8893 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1651 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8900 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1655 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8907 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1660 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8914 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1665 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8921 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1671 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8927 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1672 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8933 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1675 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,false);}
#line 8939 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1676 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),true,false);}
#line 8945 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1677 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,true);}
#line 8951 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1679 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),false, false);}
#line 8957 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1681 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),false,true);}
#line 8963 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1683 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),true, false);}
#line 8969 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1687 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8975 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1688 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 8981 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1691 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8987 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1692 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8993 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1693 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 8999 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1697 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9005 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1699 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9011 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1700 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 9017 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1701 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9023 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1706 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9029 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1707 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9035 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1710 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9042 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1715 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9048 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1721 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9054 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1722 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9060 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1725 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[0]));}
#line 9066 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1726 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 9073 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1729 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[-1]));}
#line 9079 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1730 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-3]));}
#line 9086 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1732 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9093 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1735 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL, true);}
#line 9100 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1737 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9106 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 359:
#line 1740 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9114 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1747 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9124 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1755 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9132 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1762 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9142 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1768 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 9148 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1770 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9154 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1772 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9160 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1774 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 9166 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1776 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 9172 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1777 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 9179 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1780 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 9185 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1783 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9191 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1784 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9197 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1785 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 9203 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1791 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 9209 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1796 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 9216 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1799 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 9224 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1806 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 9230 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1807 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 9237 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1812 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 9244 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1815 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 9250 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1822 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 9257 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1824 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 9263 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1828 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9269 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9275 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9281 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9287 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1838 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 9298 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1844 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 9304 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1846 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 9310 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1847 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 9316 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1851 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,0,(yyvsp[0]),0);}
#line 9322 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 1853 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),0,(yyvsp[0]),0);}
#line 9328 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1858 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9334 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 394:
#line 1861 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9340 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1862 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 9346 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1866 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 9352 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1867 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 9358 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1871 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 9365 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1874 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 9372 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1879 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 9379 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1884 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 9385 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1885 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 9392 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1887 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 9398 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1891 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 9404 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1892 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 9410 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1893 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 9416 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1894 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 9422 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1898 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9428 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1899 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 9434 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1900 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 9440 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1901 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 9446 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1902 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 9452 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1904 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 9458 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1906 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 9464 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1910 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 9472 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1913 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 9478 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1914 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 9484 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1918 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9490 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1919 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9496 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1923 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9502 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1924 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9508 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1927 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9514 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1928 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9520 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1931 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9526 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1932 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9532 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1935 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9538 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1937 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9544 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1940 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9550 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1941 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9556 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1942 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9562 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1943 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9568 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1944 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9574 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1945 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9580 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 1946 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9586 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 1950 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9592 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 1951 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9598 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 1954 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9604 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 1955 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9610 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 1956 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9616 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 1960 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9622 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 1962 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9628 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9634 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 1964 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9640 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 1968 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9646 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 1970 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9652 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 1974 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9658 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 1976 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9664 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 1980 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9672 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 1984 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9679 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 1988 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9685 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 1992 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9691 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 1996 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9697 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 1998 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9703 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 1999 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9709 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9715 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 2001 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9721 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 2002 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9727 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 2005 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9733 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 2006 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9739 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 2010 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9745 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 2011 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9751 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 2015 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 9757 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 2016 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 9763 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 9769 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 2018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9775 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 2022 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9781 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 2027 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9787 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 2031 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 9793 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 2035 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9799 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 2039 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 9805 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 2043 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9811 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 2048 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9817 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 2052 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9823 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 2056 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9829 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 2057 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9835 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 2058 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9841 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 2059 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9847 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 2060 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9853 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 2064 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9859 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 2069 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 9865 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 2070 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 9871 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 2071 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 9877 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 2074 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 9883 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 2075 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 9889 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 2076 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 9895 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 2077 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 9901 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 2078 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 9907 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 2079 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 9913 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 2080 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 9919 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 2081 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 9925 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 2082 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 9931 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 2083 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 9937 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 2084 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 9943 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 2085 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 9949 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 2086 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 9955 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 2087 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 9961 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 2088 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 9967 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 2089 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 9973 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 2090 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 9979 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 2091 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 9985 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 2092 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 9991 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 2093 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 9997 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2094 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 10003 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2095 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 10009 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2096 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 10015 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2097 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 10021 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2098 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 10027 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2099 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 10033 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2100 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 10039 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2101 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 10045 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2102 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 10051 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2103 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 10057 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2104 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 10063 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2105 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 10069 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2106 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 10075 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2107 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 10081 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2108 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 10087 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2109 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 10093 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2110 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 10099 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2111 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 10105 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2112 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 10111 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2113 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 10117 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2114 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 10123 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2115 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 10129 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2116 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 10135 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2117 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 10141 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2118 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 10148 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2120 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 10154 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2121 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 10161 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2123 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 10167 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2125 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 10173 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2126 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10179 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2127 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 10185 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2128 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 10191 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2129 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 10197 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2130 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10203 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2131 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 10209 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2132 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 10215 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2133 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 10221 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2134 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 10227 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2135 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 10233 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2136 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 10239 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2137 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 10245 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2138 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 10251 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2139 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 10257 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2140 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10263 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2141 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10269 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2142 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10275 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2143 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10281 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2144 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10287 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2145 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10293 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2146 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10299 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2147 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10305 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2148 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 10311 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2149 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 10317 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2150 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10323 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2157 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 10329 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2158 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10335 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2163 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10344 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2169 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10356 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2178 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10365 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2184 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[-12]),
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10377 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2195 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),0,
                                                     NULL,NULL,NULL);}
#line 10390 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2203 "hphp.y" /* yacc.c:1646  */
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
#line 10405 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2214 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10415 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2222 "hphp.y" /* yacc.c:1646  */
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
#line 10430 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2233 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10440 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2239 "hphp.y" /* yacc.c:1646  */
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
#line 10457 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2251 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),0,
                                                     NULL,NULL,NULL);}
#line 10470 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 570:
#line 2259 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10483 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2267 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10493 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 572:
#line 2275 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 10506 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2286 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10512 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2287 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10518 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2289 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10524 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
#line 2293 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10531 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2295 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10537 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
#line 2302 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10543 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
#line 2305 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10549 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
#line 2312 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10555 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2315 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10561 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
#line 2320 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10567 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2321 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10573 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2326 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10579 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2327 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10585 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2331 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval), (yyvsp[-1]), T_ARRAY);}
#line 10591 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2335 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10597 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2336 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10603 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2341 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10609 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2342 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10615 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2347 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10621 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2348 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10627 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2353 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10633 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2354 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10639 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2360 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10645 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2362 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10651 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2367 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10657 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2368 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10663 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2374 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10669 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2376 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10675 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2380 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10681 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2384 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10687 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2388 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10693 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2392 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10699 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2396 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10705 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2400 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10711 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2404 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10717 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2408 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10723 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2412 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10729 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2416 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10735 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2420 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10741 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2424 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10747 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2428 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10753 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2432 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10759 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2436 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10765 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2441 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10771 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2442 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10777 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2447 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10783 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2448 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10789 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2453 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10795 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2454 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10801 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2459 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10809 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2466 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10817 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2473 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10823 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2475 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10829 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2479 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10835 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2480 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10841 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2481 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10847 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2482 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10853 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10859 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2484 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10865 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2485 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10871 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2486 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10877 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2487 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10884 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2489 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10890 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2490 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10896 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2494 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10902 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2495 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 10908 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2496 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 10914 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2497 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 10920 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2504 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 10926 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2507 "hphp.y" /* yacc.c:1646  */
    { Token t1; _p->onArray(t1,(yyvsp[-1]));
                                         Token t2; _p->onArray(t2,(yyvsp[0]));
                                         Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onCallParam((yyvsp[-1]),NULL,t1,0,0);
                                         _p->onCallParam((yyval), &(yyvsp[-1]),t2,0,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),file,0,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),line,0,0);
                                         (yyval).setText("");}
#line 10940 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2518 "hphp.y" /* yacc.c:1646  */
    { Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onArray((yyvsp[-2]),(yyvsp[-5]));
                                         _p->onArray((yyvsp[-1]),(yyvsp[-3]));
                                         _p->onCallParam((yyvsp[-4]),NULL,(yyvsp[-2]),0,0);
                                         _p->onCallParam((yyval), &(yyvsp[-4]),(yyvsp[-1]),0,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),file,0,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),line,0,0);
                                         (yyval).setText((yyvsp[0]).text());}
#line 10954 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2529 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 10960 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2530 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 10966 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2535 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10972 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2536 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10978 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2539 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-1]),0,(yyvsp[0]),0);}
#line 10984 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2540 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10990 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2543 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10997 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2547 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11005 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2550 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11011 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 653:
#line 2553 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         if ((yyvsp[0]).htmlTrim()) {
                                           (yyvsp[0]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));
                                         }
                                       }
#line 11023 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2560 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11029 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2561 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11035 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2565 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11041 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2567 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 11047 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2569 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 11053 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2573 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11059 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2574 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11065 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2575 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11071 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2576 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11077 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2577 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11083 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2578 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11089 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2579 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11095 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2580 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11101 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2581 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11107 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2582 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11113 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2583 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11119 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2584 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11125 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2585 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11131 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2586 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11137 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2587 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11143 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2588 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11149 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2589 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11155 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2590 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11161 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2591 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11167 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2592 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11173 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2593 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11179 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2594 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11185 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2595 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11191 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2596 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11197 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2597 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11203 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2598 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11209 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2599 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11215 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2600 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11221 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2601 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11227 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2602 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11233 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2603 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11239 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2604 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11245 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2605 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11251 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2606 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11257 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2607 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11263 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2608 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11269 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2609 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11275 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2610 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11281 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2611 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11287 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2612 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11293 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2613 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11299 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2614 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11305 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2615 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11311 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2616 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11317 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2617 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11323 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2618 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11329 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2619 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11335 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2620 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11341 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2621 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11347 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2622 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11353 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2623 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11359 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2624 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11365 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2625 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11371 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2626 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11377 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2627 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11383 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2628 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11389 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2629 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11395 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2630 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11401 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2631 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11407 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2632 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11413 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2633 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11419 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2634 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11425 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2635 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11431 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2636 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11437 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2637 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11443 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2638 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11449 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2639 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11455 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2640 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11461 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2641 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11467 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2642 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11473 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2643 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11479 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2644 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11485 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2645 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11491 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2646 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11497 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2647 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11503 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2648 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11509 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2649 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11515 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2650 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11521 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2651 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11527 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2652 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11533 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2653 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11539 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2654 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11545 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2659 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11551 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2663 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11557 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2664 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11563 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2668 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11569 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2669 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11575 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2670 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11581 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2671 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11588 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2673 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11595 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2677 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11601 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2686 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11607 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2689 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11613 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2690 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11620 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2692 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11627 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2702 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11633 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2706 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11639 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2707 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11645 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2708 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11651 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2712 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11657 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2713 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11663 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2714 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11669 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2718 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11675 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2719 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11681 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2720 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11687 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2724 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11693 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2725 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11699 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2729 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11705 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2730 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11711 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2731 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11717 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2732 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 11724 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2734 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 11730 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2735 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 11736 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2736 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 11742 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2737 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 11748 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2738 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 11754 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2739 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 11760 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2740 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 11766 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2741 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 11772 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2742 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 11778 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2745 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11784 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2747 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11790 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2751 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11796 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2752 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11802 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2754 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11808 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2755 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11814 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2757 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11820 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2758 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11826 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2759 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11832 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2760 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11838 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2761 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11844 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2762 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11850 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2763 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11856 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2764 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11862 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2765 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11868 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2767 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 11874 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2769 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 11880 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2771 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 11886 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2773 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 11892 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2775 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 11898 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2776 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 11904 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2777 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 11910 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2778 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 11916 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2779 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 11922 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2780 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 11928 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2781 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 11934 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2782 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 11940 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2783 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 11946 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2784 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 11952 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2785 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 11958 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2786 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 11964 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2787 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 11970 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2788 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 11976 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2789 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 11982 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2790 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11988 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2791 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11994 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2793 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 12000 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2795 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 12006 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2797 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 12012 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2799 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 12018 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2800 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 12024 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2802 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 12031 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2804 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 12037 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2807 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 12044 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2811 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 12050 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2814 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 12056 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2815 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 12062 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2819 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12068 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2820 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12074 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2826 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12080 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2832 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12086 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12092 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2837 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12098 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 832:
#line 2838 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12104 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2839 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12110 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2840 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12116 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2841 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 12122 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2842 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 12128 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2844 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 12135 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2849 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12141 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2850 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12147 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2854 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12153 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2855 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12159 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2858 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 12165 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2859 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12171 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2865 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12177 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2867 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12183 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2869 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12189 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2870 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12195 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2874 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 12201 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2875 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 12207 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2876 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 12213 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2879 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12219 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2881 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12225 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2884 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 12231 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2885 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 12237 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2886 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 12243 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2887 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12249 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2891 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 12256 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2894 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 12264 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2901 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12270 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2902 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12276 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2905 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 12284 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2908 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12290 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2909 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12296 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2910 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12302 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2912 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12308 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2913 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12314 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2915 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12320 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2916 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12326 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 2917 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12332 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2918 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12338 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2919 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12344 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2920 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12350 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2925 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12356 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2926 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12362 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 2931 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12368 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 877:
#line 2932 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12374 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 2937 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12380 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 2939 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12386 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 2941 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12392 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 2942 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12398 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 2946 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12404 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 2947 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12410 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 2952 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12416 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 2953 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12422 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
#line 2958 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12428 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 2961 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12434 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 2966 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12440 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 2967 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12446 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 2970 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12452 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 2971 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 12459 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 2978 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 12465 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 2980 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 12471 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 2983 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 12477 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 2985 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12483 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 2988 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12489 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 2991 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12495 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 2992 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12501 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 2996 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 12507 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 2997 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 12513 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3001 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12519 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3002 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 12525 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3003 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 12531 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12537 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3009 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12543 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3017 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12549 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12555 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3022 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12561 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3024 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12567 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3032 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12573 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3033 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12579 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3038 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12585 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3039 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12591 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3041 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12597 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3046 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12603 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3048 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12609 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3054 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12623 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3065 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12637 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3080 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12651 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3092 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12665 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3104 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12671 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3105 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12677 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3106 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12683 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3107 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12689 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3108 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12695 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3109 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12701 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3111 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12715 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3128 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12721 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3130 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12727 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3132 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12733 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3133 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12739 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3137 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12745 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3141 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12751 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3142 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12757 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3143 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12763 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3144 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12769 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
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
#line 12783 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
#line 3161 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12789 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3163 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12795 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
#line 3172 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12801 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
#line 3173 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12807 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3174 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12813 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3175 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12819 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
#line 3176 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12825 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3177 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12831 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
#line 3178 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12837 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
#line 3180 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12843 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
#line 3182 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12849 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3186 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12855 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3190 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12861 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3191 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12867 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3197 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12873 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3201 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12879 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3205 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12885 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3212 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 12891 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3221 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 12897 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3225 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 12903 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3229 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12909 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3232 "hphp.y" /* yacc.c:1646  */
    { _p->onIndirectRef((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 12915 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3238 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12921 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3239 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12927 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3240 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12933 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3244 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12939 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3245 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 12945 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3246 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 12951 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3253 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12957 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3254 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12963 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3259 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 12969 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3260 "hphp.y" /* yacc.c:1646  */
    { (yyval)++;}
#line 12975 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3265 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12981 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3266 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12987 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3267 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12993 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3270 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13007 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3281 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13013 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3282 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13019 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3286 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13025 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3287 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13031 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3290 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13045 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3299 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13051 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3303 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 13057 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3304 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 13063 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3306 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 13069 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3307 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 13075 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3308 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 13081 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3309 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 13087 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3314 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13093 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3315 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13099 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3319 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13105 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3320 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13111 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3321 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13117 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3322 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13123 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3325 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13129 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3327 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 13135 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 995:
#line 3328 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13141 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3329 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 13147 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3334 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13153 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
#line 3335 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13159 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3339 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13165 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3340 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13171 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3341 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13177 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3342 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13183 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3347 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13189 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1004:
#line 3348 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13195 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1005:
#line 3353 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13201 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1006:
#line 3355 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13207 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1007:
#line 3357 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13213 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1008:
#line 3358 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13219 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3362 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 13225 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3364 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 13231 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3365 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 13237 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3367 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 13244 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3372 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13250 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3374 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13256 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3376 "hphp.y" /* yacc.c:1646  */
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[-2]),
                                           !(yyvsp[-1]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[0])
                                         );
                                       }
#line 13270 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3386 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 13276 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3388 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 13282 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3389 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13288 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3392 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 13294 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3393 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 13300 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3394 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 13306 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3398 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 13312 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3399 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 13318 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3400 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13324 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3401 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13330 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3402 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13336 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1027:
#line 3403 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13342 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1028:
#line 3404 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 13348 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1029:
#line 3405 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 13354 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1030:
#line 3406 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 13360 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1031:
#line 3407 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 13366 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1032:
#line 3408 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 13372 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1033:
#line 3412 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 13378 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1034:
#line 3413 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 13384 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1035:
#line 3418 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13390 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1036:
#line 3420 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13396 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1039:
#line 3434 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13404 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1040:
#line 3439 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 13412 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1041:
#line 3443 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13420 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1042:
#line 3448 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 13428 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3454 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13434 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3455 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13440 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13446 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3460 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13452 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3466 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13458 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3470 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 13464 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3476 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13470 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3480 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 13477 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3487 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13483 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3488 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13489 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3492 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 13497 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3495 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 13504 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1055:
#line 3501 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13510 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1056:
#line 3506 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-3]); }
#line 13516 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1057:
#line 3507 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13522 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1058:
#line 3508 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13528 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1059:
#line 3509 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13534 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3530 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13540 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3531 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13546 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3540 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13552 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3551 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13558 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3553 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13564 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3557 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13570 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1076:
#line 3560 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13576 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1077:
#line 3564 "hphp.y" /* yacc.c:1646  */
    {}
#line 13582 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1078:
#line 3565 "hphp.y" /* yacc.c:1646  */
    {}
#line 13588 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1079:
#line 3566 "hphp.y" /* yacc.c:1646  */
    {}
#line 13594 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1080:
#line 3572 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13601 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1081:
#line 3577 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13611 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1082:
#line 3586 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13617 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1083:
#line 3592 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13626 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1084:
#line 3600 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 13632 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1085:
#line 3601 "hphp.y" /* yacc.c:1646  */
    { }
#line 13638 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1086:
#line 3607 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 13644 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1087:
#line 3609 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 13650 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1088:
#line 3610 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 13660 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1089:
#line 3615 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 13667 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1090:
#line 3621 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("array"); }
#line 13674 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1091:
#line 3626 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13680 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1092:
#line 3631 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 13688 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1093:
#line 3635 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13694 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1094:
#line 3640 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 13700 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1095:
#line 3642 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 13706 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1096:
#line 3648 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 13713 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1097:
#line 3650 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 13721 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1098:
#line 3653 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13727 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1099:
#line 3654 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13735 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1100:
#line 3657 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13743 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1101:
#line 3660 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13749 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1102:
#line 3663 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 13757 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1103:
#line 3666 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13764 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1104:
#line 3668 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 13773 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1105:
#line 3674 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 13782 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1106:
#line 3680 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 13792 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1107:
#line 3688 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13798 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;

  case 1108:
#line 3689 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13804 "hphp.5.tab.cpp" /* yacc.c:1646  */
    break;


#line 13808 "hphp.5.tab.cpp" /* yacc.c:1646  */
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
#line 3692 "hphp.y" /* yacc.c:1906  */

/* !PHP5_ONLY*/
bool Parser::parseImpl5() {
/* !END */
/* !PHP7_ONLY*/
/* REMOVED */
/* !END */
  return yyparse(this) == 0;
}
