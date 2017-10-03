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



int Compiler7parse (HPHP::HPHP_PARSER_NS::Parser *_p);

#endif /* !YY_YY_HPHP_Y_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 898 "hphp.7.tab.cpp" /* yacc.c:358  */

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
#define YYLAST   19688

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  207
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  312
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1105
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  2054

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
    2657,  2662,  2663,  2667,  2668,  2669,  2670,  2672,  2676,  2677,
    2688,  2689,  2691,  2693,  2705,  2706,  2707,  2711,  2712,  2713,
    2717,  2718,  2719,  2722,  2724,  2728,  2729,  2730,  2731,  2733,
    2734,  2735,  2736,  2737,  2738,  2739,  2740,  2741,  2742,  2745,
    2750,  2751,  2752,  2754,  2755,  2757,  2758,  2759,  2760,  2761,
    2762,  2763,  2764,  2765,  2767,  2769,  2771,  2773,  2775,  2776,
    2777,  2778,  2779,  2780,  2781,  2782,  2783,  2784,  2785,  2786,
    2787,  2788,  2789,  2790,  2791,  2793,  2795,  2797,  2799,  2800,
    2803,  2804,  2808,  2812,  2814,  2818,  2819,  2823,  2829,  2832,
    2836,  2837,  2838,  2839,  2840,  2841,  2842,  2847,  2849,  2853,
    2854,  2857,  2858,  2862,  2865,  2867,  2869,  2873,  2874,  2875,
    2876,  2879,  2883,  2884,  2885,  2886,  2890,  2892,  2899,  2900,
    2901,  2902,  2907,  2908,  2909,  2910,  2912,  2913,  2915,  2916,
    2917,  2918,  2919,  2923,  2925,  2929,  2931,  2934,  2937,  2939,
    2941,  2944,  2946,  2950,  2952,  2955,  2958,  2964,  2966,  2969,
    2970,  2975,  2978,  2982,  2982,  2987,  2990,  2991,  2995,  2996,
    3000,  3001,  3002,  3006,  3011,  3016,  3017,  3021,  3026,  3031,
    3032,  3036,  3038,  3039,  3044,  3046,  3051,  3062,  3076,  3088,
    3103,  3104,  3105,  3106,  3107,  3108,  3109,  3119,  3128,  3130,
    3132,  3136,  3140,  3141,  3142,  3143,  3144,  3160,  3161,  3164,
    3171,  3172,  3173,  3174,  3175,  3176,  3177,  3178,  3180,  3185,
    3189,  3190,  3194,  3197,  3204,  3208,  3217,  3224,  3232,  3234,
    3235,  3239,  3240,  3241,  3243,  3248,  3249,  3260,  3261,  3262,
    3263,  3274,  3277,  3280,  3281,  3282,  3283,  3294,  3298,  3299,
    3300,  3302,  3303,  3304,  3308,  3310,  3313,  3315,  3316,  3317,
    3318,  3321,  3323,  3324,  3328,  3330,  3333,  3335,  3336,  3337,
    3341,  3343,  3346,  3349,  3351,  3353,  3357,  3358,  3360,  3361,
    3367,  3368,  3370,  3380,  3382,  3384,  3387,  3388,  3389,  3393,
    3394,  3395,  3396,  3397,  3398,  3399,  3400,  3401,  3402,  3403,
    3407,  3408,  3412,  3414,  3422,  3424,  3428,  3432,  3437,  3441,
    3449,  3450,  3454,  3455,  3461,  3462,  3471,  3472,  3480,  3483,
    3487,  3490,  3495,  3500,  3502,  3503,  3504,  3507,  3509,  3515,
    3516,  3520,  3521,  3525,  3526,  3530,  3531,  3534,  3539,  3540,
    3544,  3547,  3549,  3553,  3559,  3560,  3561,  3565,  3569,  3577,
    3582,  3594,  3596,  3600,  3603,  3605,  3610,  3615,  3621,  3624,
    3629,  3634,  3636,  3643,  3645,  3648,  3649,  3652,  3655,  3656,
    3661,  3663,  3667,  3673,  3683,  3684
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
  "dim_offset", "variable_no_calls", "dimmable_variable_no_calls",
  "assignment_list", "array_pair_list", "non_empty_array_pair_list",
  "collection_init", "non_empty_collection_init", "static_collection_init",
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

#define YYPACT_NINF -1729

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1729)))

#define YYTABLE_NINF -1089

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1089)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1729,   172, -1729, -1729,  5601, 15189, 15189,    24, 15189, 15189,
   15189, 15189, 12537, 15189, -1729, 15189, 15189, 15189, 15189, 18300,
   18300, 15189, 15189, 15189, 15189, 15189, 15189, 15189, 15189, 12741,
   18977, 15189,   208,   217, -1729, -1729, -1729,   137, -1729,   218,
   -1729, -1729, -1729,   178, 15189, -1729,   217,   244,   248,   264,
   -1729,   217, 12945, 16474, 13149, -1729, 16225, 11517,   210, 15189,
   19221,    97,   272,    82,   316, -1729, -1729, -1729,   308,   347,
     365,   379, -1729, 16474,   384,   395,   349,   386,   569,   571,
     583, -1729, -1729, -1729, -1729, -1729, 15189,   500,  1590, -1729,
   -1729, 16474, -1729, -1729, -1729, -1729, 16474, -1729, 16474, -1729,
     505,   483, 16474, 16474, -1729,   213, -1729, -1729, 13353, -1729,
   -1729,   487,   573,   584,   584, -1729,   654,   523,   552,   493,
   -1729,    87, -1729,   511,   593,   683, -1729, -1729, -1729, -1729,
    1763,   606, -1729,   149, -1729,   524,   540,   543,   581,   588,
     595,   597,   600,  5147, -1729, -1729, -1729, -1729, -1729,    65,
     665,   681,   745,   747,   760, -1729,   762,   764, -1729,    78,
     568, -1729,   631,    62, -1729,  2773,   164, -1729, -1729,  4274,
     149,   149,   667,    69, -1729,   166,   115,   668,   175, -1729,
   -1729,   781, -1729,   696, -1729, -1729,   678,   697, -1729, 15189,
   -1729,   683,   606, 19436,  4303, 19436, 15189, 19436, 19436, 16758,
   16758,   682, 17821, 19436,   833, 16474,   815,   815,   634,   815,
   -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729,    84,
   15189,   707, -1729, -1729,   736,   701,   510,   704,   510,   815,
     815,   815,   815,   815,   815,   815,   815, 18300, 18470,   699,
     891,   696, -1729, 15189,   707, -1729,   743, -1729,   749,   714,
   -1729,   167, -1729, -1729, -1729,   510,   149, -1729, 13557, -1729,
   -1729, 15189, 10089,   904,    91, 19436, 11109, -1729, 15189, 15189,
   16474, -1729, -1729,  5252,   719, -1729, 12521, -1729, -1729, -1729,
   -1729, -1729, -1729, -1729, -1729, -1729, -1729, 17400, -1729, 17400,
   -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729,
   -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729,
   -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729,
   -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729,
   -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729,    95,    99,
     697, -1729, -1729, -1729, -1729,   716,  3286,   103, -1729, -1729,
     758,   916, -1729,   774, 16948, 15189, -1729,   734,   737, 12929,
   -1729,   313, 13541, 15612, 15612, 15612, 16474, 15612,   739,   931,
     746, -1729,    68, -1729, 17884,   101, -1729,   933,   105,   817,
   -1729,   825, -1729, 18300, 15189, 15189,   761,   780, -1729, -1729,
   17988, 12741, 15189, 15189, 15189, 15189, 15189,   107,   314,   541,
   -1729, 15393, 18300,   536, -1729, 16474, -1729,   436,   523, -1729,
   -1729, -1729, -1729, 19078,   950,   864, -1729, -1729, -1729,    71,
   15189,   769,   775, 19436,   776,  2482,   782,  6213, 15189, -1729,
     455,   767,   602,   455,   519,   492, -1729, 16474, 17400,   785,
    5335, 16225, -1729, 13761,   783,   783,   783,   783, -1729, -1729,
    2900, -1729, -1729, -1729, -1729, -1729,   683, -1729, 15189, 15189,
   -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, 15189,
   15189, 15189, 15189, 13965, 15189, 15189, 15189, 15189, 15189, 15189,
   15189, 15189, 15189, 15189, 15189, 15189, 15189, 15189, 15189, 15189,
   15189, 15189, 15189, 15189, 15189, 15189, 15189, 19179, 15189, -1729,
   15189, 15189, 15189, 15565, 16474, 16474, 16474, 16474, 16474,  1763,
     877,   771, 11313, 15189, 15189, 15189, 15189, 15189, 15189, 15189,
   15189, 15189, 15189, 15189, 15189, -1729, -1729, -1729, -1729,  3806,
   -1729, -1729,  5335,  5335, 15189, 15189, 17988,   788,   683, 14169,
    5540, -1729, 15189, -1729,   792,   982,   835,   797,   802, 15718,
     510, 14373, -1729, 14577, -1729,   714,   807,   809,  2765, -1729,
     145,  5335, -1729,  4898, -1729, -1729, 15173, -1729, -1729, 11721,
   -1729, 15189, -1729,   909, 10293,   999,   810, 15377,   996,   124,
      67, -1729, -1729, -1729,   829, -1729, -1729, -1729, 17400, -1729,
    1912,   816,  1003, 17745, 16474, -1729, -1729, -1729, -1729, -1729,
   -1729, -1729, -1729, -1729,   819, -1729, -1729,   814,   820,   821,
     831,   832,   842,   329,   843,   850,  1314, 15765, -1729, -1729,
   16474, 16474, 15189,   510,    97, -1729, 17745,   959, -1729, -1729,
   -1729,   510,   146,   147,   860,   861,  3168,   179,   863,   865,
     664,   929,   866,   510,   148,   868, 18518,   871,  1058,  1060,
     879,   880,   886,   887, -1729,  3841, 16474, -1729, -1729,  1002,
    2800,   472, -1729, -1729, -1729,   523, -1729, -1729, -1729,  1046,
     962,   917,   373,   940, 15189,   966,  1097,   910, -1729,   946,
   -1729,   185, -1729, 17400, 17400,  1096,   904,    71, -1729,   918,
    1104, -1729, 17400,   116, -1729,   433,   173, -1729, -1729, -1729,
   -1729, -1729, -1729, -1729,  1384,  2980, -1729, -1729, -1729, -1729,
    1108,   944, -1729, 18300, 15189,   932,  1120, 19436,  1117,   150,
    1126,   939,   941,   942, 19436,   943,  3193,  6417, -1729, -1729,
   -1729, -1729, -1729, -1729,  1009,  3577, 19436,   945,  3105, 13134,
   19619, 16758, 16049, 15189, 19388, 16226,  3779, 14351, 14758,  4920,
   16395, 16581, 16581, 16581, 16581,  2385,  2385,  2385,  2385,  2385,
     874,   874,   703,   703,   703,   634,   634,   634, -1729,   815,
     949,   951, 18578,   947,  1135,     6, 15189,    20,   707,   198,
   -1729, -1729, -1729,  1131,   864, -1729,   683, 18092, -1729, -1729,
   -1729, 16758, 16758, 16758, 16758, 16758, 16758, 16758, 16758, 16758,
   16758, 16758, 16758, 16758, -1729, 15189,   220, -1729,   203, -1729,
     707,   266,   948,   952,   953,  4078,   153,   963, -1729, 19436,
   17849, -1729, 16474, -1729,   510,   485, 18300, 19436, 18300, 18626,
    1009,   222,   510,   212, -1729,   185,   995,   961, 15189, -1729,
     243, -1729, -1729, -1729,  6621,   672, -1729, -1729, 19436, 19436,
     217, -1729, -1729, -1729, 15189,  1059, 17620, 17745, 16474, 10497,
     964,   965, -1729,  1153,  2811,  1028, -1729,  1005, -1729,  1158,
     970,  3908, 17400, 17745, 17745, 17745, 17745, 17745,   973,  1102,
    1106,  1107,  1111,  1113,   977, 17745,    18, -1729, -1729, -1729,
   -1729, -1729, -1729,    14, -1729, 19530, -1729, -1729,    59, -1729,
    6825,  5069,   984, 15765, -1729, 15765, -1729, 15765, -1729, 16474,
   16474, 15765, -1729, 15765, 15765, 16474, -1729,  1182,   997, -1729,
     334, -1729, -1729,  4151, -1729, 19530,  1183, 18300,  1000, -1729,
   -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729,  1013,  1191,
   16474,  5069,  1001, 17988, 18196,  1188, -1729, 15189, -1729, 15189,
   -1729, 15189, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729,
    1004, -1729, 15189, -1729, -1729,  5805, -1729, 17400,  5069,  1007,
   -1729, -1729, -1729, -1729,  1190,  1012, 15189, 19078, -1729, -1729,
   15565,  1006, -1729, 17400, -1729,  1019,  7029,  1186,    63, -1729,
   -1729,   139,  3806,  4898, -1729, 17400, -1729, -1729,   510, 19436,
   -1729, 11925, -1729, 17745, 13761,   783, 13761, -1729,   783,   783,
   -1729, 12129, -1729, -1729,  7233, -1729,    73,  1017,  5069,   962,
   -1729, -1729, -1729, -1729, 16226, 15189, -1729, -1729, 15189, -1729,
   15189, -1729,  4349,  1022,  5335,   929,  1189,   962, 17400,  1206,
    1009, 16474, 19179,   510,  4618,  1026,   200,  1027, -1729, -1729,
    1211,  2541,  2541, 17849, -1729, -1729, -1729,  1176,  1037,  1165,
    1170,  1177,  1179,  1180,   100,  1049,   424, -1729, -1729, -1729,
   -1729, -1729,  1090, -1729, -1729, -1729, -1729,  1243,  1056,   792,
     510,   510, 14781,   962,  4898, -1729, -1729,  4681,   679,   217,
   11109, -1729,  7437,  1057,  7641,  1061, 17620, 18300,  1072,  1133,
     510, 19530,  1257, -1729, -1729, -1729, -1729,   660, -1729,    90,
   17400,  1089,  1136,  1114, 17400, 16474,  3592, -1729, -1729, -1729,
    1265, -1729,  1078,  1108,   805,   805,  1213,  1213, 17582,  1076,
    1279, 17745, 17745, 17745, 17745, 17745, 17745, 19078,  3505, 17101,
   17745, 17745, 17745, 17745, 17500, 17745, 17745, 17745, 17745, 17745,
   17745, 17745, 17745, 17745, 17745, 17745, 17745, 17745, 17745, 17745,
   17745, 17745, 17745, 17745, 17745, 17745, 17745, 17745, 16474, -1729,
   -1729,  1207, -1729, -1729,  1088,  1094,  1095, -1729,  1098, -1729,
   -1729,   355,  1314, -1729,  1092, -1729, 17745,   510, -1729, -1729,
     160, -1729,   671,  1288, -1729, -1729,   155,  1101,   510, 12333,
   19436, 18686, -1729,  2483, -1729,  6009,   864,  1288, -1729,   470,
      25, -1729, 19436,  1163,  1116, -1729,  1105,  1186, -1729, -1729,
   -1729, 14985, 17400,   904, 17400,   104,  1287,  1220,   298, -1729,
     707,   304, -1729, -1729, 18300, 15189, 19436, 19530, -1729, -1729,
   -1729,  5033, -1729, -1729, -1729, -1729, -1729, -1729,  1110,    73,
   -1729,  1115,    73,  1119, 16226, 19436, 18734,  1123,  5335,  1124,
    1121, 17400,  1129,  1112, 17400,   962, -1729,   714,   358,  5335,
   15189, -1729, -1729, -1729, -1729, -1729, -1729,  1168,  1122,  1308,
    1233, 17849, 17849, 17849, 17849, 17849, 17849,  1172, -1729, 19078,
     327, 17849, -1729, -1729, -1729, 18300, 19436,  1134, -1729,   217,
    1299,  1256, 11109, -1729, -1729, -1729,  1137, 15189,  1133,   510,
   17988, 17620,  1139, 17745,  7845,   772,  1140, 15189,    70,    92,
   -1729,  1152, -1729, 17400, 16474, -1729,  1201, -1729, -1729, 17351,
    1309,  1142, 17745, -1729, 17745, -1729,  1144,  1148,  1334, 18790,
    1150, 19530,  1346,  1160,  1161,  1162,  1219,  1348,  1164, -1729,
   -1729, -1729, 18838,  1167,  1360, 19575, 16944, 11905, 17745, 19484,
   14148, 14555, 15563, 13941, 17702, 18070, 18070, 18070, 18070,  2751,
    2751,  2751,  2751,  2751,   900,   900,   805,   805,   805,  1213,
    1213,  1213,  1213, -1729,  1173, -1729,  1171,  1178,  1181,  1193,
   -1729, -1729, 19530, 16474, 17400, 17400, -1729,   671,  5069,   698,
   -1729, 17988, -1729, -1729, 16758, 15189,  1175, -1729,  1196,  1261,
   -1729,   119, 15189, -1729, -1729, -1729, 15189, -1729, 15189, -1729,
     904, 13761,  1198,   323,   783,   323,   204, -1729, -1729,   144,
    1358,  1315, 15189, -1729,  1202,   510, 19436,  1186,  1203, -1729,
    1204,    73, 15189,  5335,  1205, -1729, -1729,   864, -1729, -1729,
    1209,  1212,  1208, -1729,  1216, 17849, -1729, 17849, -1729, -1729,
    1217,  1200,  1397,  1283,  1218, -1729,  1415,  1224,  1229,  1230,
   -1729,  1298,  1238,  1430, -1729, -1729,   510, -1729,  1410, -1729,
    1244, -1729, -1729,  1246,  1248,   156, -1729, -1729, 19530,  1249,
    1250, -1729,  5078, -1729, -1729, -1729, -1729, -1729, -1729,  1310,
   17400, -1729, 17400, -1729, 19530, 18894, -1729, -1729, 17745, -1729,
   17745, -1729, 17745, -1729, -1729, -1729, -1729, 17745, 19078, -1729,
   -1729, 17745, -1729, 17745, -1729, 12721, 17745,  1252,  8049, -1729,
   -1729, -1729, -1729,   671, -1729, -1729, -1729, -1729,   643, 16402,
    5069,  1338, -1729,  2527,  1290,  3265, -1729, -1729, -1729,   877,
    4383,   121,   125,  1262,   864,   771,   158, 19436, -1729, -1729,
   -1729,  1297,  4805,  4974, 19436, -1729,  3248, -1729,  6417,   106,
    1450,  1382, 15189, -1729, 19436,  5335,  1349,  1186,  1759,  1186,
    1268, 19436,  1270, -1729,  1845,  1269,  2137, -1729, -1729,    73,
   -1729, -1729,  1333, -1729, -1729, 17849, -1729, 17849, -1729, 17849,
   -1729, -1729, -1729, -1729, 17849, -1729, 19078, -1729,  2153, -1729,
    8253, -1729, -1729, -1729, -1729, 10701, -1729, -1729, -1729,  6621,
   17400, -1729,  1274, 17745, 18942, 19530, 19530, 19530,  1336, 19530,
   18998, 12721, -1729, -1729,   671,  5069,  5069, 16474, -1729,  1460,
   17254,    89, -1729, 16402,   864,  4371, -1729,  1295, -1729,   127,
    1280,   128, -1729, 16757, -1729, -1729, -1729,   129, -1729, -1729,
    2206, -1729,  1282, -1729,  1407,   683, -1729, 16580, -1729, 16580,
   -1729, -1729,  1478,   877, -1729, 15871, -1729, -1729, -1729, -1729,
    2537,  1479,  1412, 15189, -1729, 19436,  1320,  1316,  1186,  1323,
   -1729,  1349,  1186, -1729, -1729, -1729, -1729,  2172,  1322, 17849,
    1375, -1729, -1729, -1729,  1385, -1729,  6621, 10905, 10701, -1729,
   -1729, -1729,  6621, -1729, -1729, 19530, 17745, 17745, 17745,  8457,
    1326,  1327, -1729, 17745, -1729,  5069, -1729, -1729, -1729, -1729,
   -1729, 17400,  2584,  2527, -1729, -1729, -1729, -1729, -1729, -1729,
   -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729,
   -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729,
   -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729,
   -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729,
   -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729,
   -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729,
   -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729,
   -1729, -1729, -1729, -1729, -1729, -1729,   180, -1729,  1290, -1729,
   -1729, -1729, -1729, -1729,   142,   572, -1729,  1507,   131, 16948,
    1407,  1513, -1729, 17400,   683, -1729, -1729,  1329,  1515, 15189,
   -1729, 19436, -1729,   123,  1330, 17400,   590,  1186,  1323, 16048,
   -1729,  1186, -1729, 17849, 17849, -1729, -1729, -1729, -1729,  8661,
   19530, 19530, 19530, -1729, -1729, -1729, 19530, -1729,  2254,  1522,
    1523,  1332, -1729, -1729, 17745, 16757, 16757,  1466, -1729,  2206,
    2206,   687, -1729, -1729, -1729, 17745,  1452, -1729,  1354,  1340,
     136, 17745, -1729, 16948, -1729, 17745, 19436,  1456, -1729,  1531,
   -1729,  1532, -1729,   430, -1729, -1729, -1729,  1342,   590, -1729,
     590, -1729, -1729,  8865,  1344,  1429, -1729,  1451,  1391, -1729,
   -1729,  1453, 17400,  1371,  2584, -1729, -1729, 19530, -1729, -1729,
    1386, -1729,  1521, -1729, -1729, -1729, -1729, 19530,  1547,   664,
   -1729, -1729, 19530,  1365, 19530, -1729,   133,  1366,  9069, 17400,
   -1729, 17400, -1729,  9273, -1729, -1729, -1729,  1367, -1729,  1368,
    1389, 16474,   771,  1388, -1729, -1729, -1729, 17745,  1390,    74,
   -1729,  1486, -1729, -1729, -1729, -1729, -1729, -1729,  9477, -1729,
    5069,   984, -1729,  1398, 16474,   722, -1729, 19530, -1729,  1377,
    1568,   763,    74, -1729, -1729,  1495, -1729,  5069,  1380, -1729,
    1186,   120, -1729, -1729, -1729, -1729, 17400, -1729,  1387,  1393,
     138, -1729,  1323,   763,   157,  1186,  1392, -1729,   605, 17400,
     177,  1566,  1503,  1323, -1729, -1729, -1729, -1729,   163,  1573,
    1506, 15189, -1729,   605,  9681,  9885,   259,  1584,  1519, 15189,
   -1729, 19436, -1729, -1729, -1729,  1592,  1525, 15189, -1729, 19436,
   15189, -1729, 19436, 19436
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   202,   462,     0,   893,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   985,
     973,     0,   759,     0,   765,   766,   767,    29,   831,   961,
     962,   169,   170,   768,     0,   150,     0,     0,     0,     0,
      30,     0,     0,     0,     0,   221,     0,     0,     0,     0,
       0,     0,   431,   432,   433,   430,   429,   428,     0,     0,
       0,     0,   250,     0,     0,     0,    37,    38,    40,    41,
      39,   772,   774,   775,   769,   770,     0,     0,     0,   776,
     771,     0,   742,    32,    33,    34,    36,    35,     0,   773,
       0,     0,     0,     0,   777,   434,   571,    31,     0,   168,
     138,     0,   760,     0,     0,     4,   124,   126,   830,     0,
     741,     0,     6,     0,     0,   220,     7,     9,     8,    10,
       0,     0,   426,     0,   476,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   532,   474,   950,   951,   553,   547,
     548,   549,   550,   551,   552,   457,   556,     0,   456,   921,
     743,   750,     0,   833,   546,   425,   924,   925,   937,   475,
       0,     0,     0,   478,   477,   922,   923,   920,   957,   960,
     536,   832,    11,   431,   432,   433,     0,     0,    36,     0,
     124,   220,     0,  1025,   475,  1026,     0,  1028,  1029,   555,
     470,     0,   463,   468,     0,     0,   518,   519,   520,   521,
      29,   961,   768,   745,    37,    38,    40,    41,    39,     0,
       0,  1049,   943,   743,     0,   744,   497,     0,   499,   537,
     538,   539,   540,   541,   542,   543,   545,     0,   989,     0,
     840,   755,   240,     0,  1049,   454,   754,   748,     0,   764,
     744,   968,   969,   975,   967,   756,     0,   455,     0,   758,
     544,     0,   203,     0,     0,   459,   203,   148,   461,     0,
       0,   154,   156,     0,     0,   158,     0,    75,    76,    81,
      82,    67,    68,    59,    79,    90,    91,     0,    62,     0,
      66,    74,    72,    93,    85,    84,    57,    80,   100,   101,
      58,    96,    55,    97,    56,    98,    54,   102,    89,    94,
      99,    86,    87,    61,    88,    92,    53,    83,    69,   103,
      77,   105,    70,    60,    47,    48,    49,    50,    51,    52,
      71,   106,   104,   108,    64,    45,    46,    73,  1096,  1097,
      65,  1101,    44,    63,    95,     0,     0,   124,   107,  1040,
    1095,     0,  1098,     0,     0,     0,   160,     0,     0,     0,
     211,     0,     0,     0,     0,     0,     0,     0,     0,   842,
       0,   112,   114,   339,     0,     0,   338,   344,     0,     0,
     251,     0,   254,     0,     0,     0,     0,  1046,   236,   248,
     981,   985,   590,   617,   617,   590,   617,     0,  1010,     0,
     779,     0,     0,     0,  1008,     0,    16,     0,   128,   228,
     242,   249,   647,   583,     0,  1034,   563,   565,   567,   897,
     462,   476,     0,     0,   474,   475,   477,   203,     0,   964,
     761,     0,   762,     0,     0,     0,   200,     0,     0,   130,
     330,     0,    28,     0,     0,     0,     0,     0,   201,   219,
       0,   247,   232,   246,   431,   434,   220,   427,   966,     0,
     913,   190,   191,   192,   193,   194,   196,   197,   199,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   973,     0,   189,
     966,   966,   995,     0,     0,     0,     0,     0,     0,     0,
       0,   424,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   496,   498,   898,   899,     0,
     912,   911,   330,   330,   966,     0,   981,     0,   220,     0,
       0,   162,     0,   895,   890,   840,     0,   476,   474,     0,
     993,     0,   588,   839,   984,   764,   476,   474,   475,   130,
       0,   330,   453,     0,   914,   757,     0,   138,   290,     0,
     570,     0,   165,     0,   203,   460,     0,     0,     0,     0,
       0,   157,   188,   159,  1096,  1097,  1093,  1094,     0,  1100,
    1086,     0,     0,     0,     0,    78,    43,    65,    42,  1041,
     195,   198,   161,   138,     0,   178,   187,     0,     0,     0,
       0,     0,     0,   115,     0,     0,     0,   841,   113,    18,
       0,   109,     0,   340,     0,   163,     0,     0,   164,   252,
     253,  1030,     0,     0,   476,   474,   475,   478,   477,     0,
    1076,   260,     0,   982,     0,     0,     0,     0,   840,   840,
       0,     0,     0,     0,   166,     0,     0,   778,  1009,   831,
       0,     0,  1007,   836,  1006,   127,     5,    13,    14,     0,
     258,     0,     0,   576,     0,     0,   840,     0,   752,     0,
     751,   746,   577,     0,     0,     0,     0,   897,   134,     0,
     842,   896,  1105,   452,   465,   479,   930,   949,   145,   137,
     141,   142,   143,   144,   425,     0,   554,   834,   835,   125,
     840,     0,  1050,     0,     0,     0,   842,   331,     0,     0,
       0,   476,   207,   208,   206,   474,   475,   203,   182,   180,
     181,   183,   559,   222,   256,     0,   965,     0,     0,   502,
     504,   503,   515,     0,     0,   535,   500,   501,   505,   507,
     506,   524,   525,   522,   523,   526,   527,   528,   529,   530,
     516,   517,   509,   510,   508,   511,   512,   514,   531,   513,
       0,     0,   999,     0,   840,  1033,     0,  1032,  1049,   927,
     238,   230,   244,     0,  1034,   234,   220,     0,   466,   469,
     471,   481,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   901,     0,   900,   903,   926,   907,
    1049,   904,     0,     0,     0,     0,     0,     0,  1027,   464,
     888,   892,   839,   894,   451,   747,     0,   988,     0,   987,
     256,     0,   747,   972,   971,   957,   960,     0,     0,   900,
     903,   970,   904,   473,   292,   294,   134,   574,   573,   458,
       0,   138,   274,   149,   461,     0,     0,     0,     0,   203,
     286,   286,   155,   840,     0,     0,  1085,     0,  1082,   840,
       0,  1056,     0,     0,     0,     0,     0,   838,     0,    37,
      38,    40,    41,    39,     0,     0,   781,   785,   786,   787,
     788,   789,   791,     0,   780,   132,   829,   790,  1049,  1099,
     203,     0,     0,     0,    21,     0,    22,     0,    19,     0,
     110,     0,    20,     0,     0,     0,   121,   842,     0,   119,
     114,   111,   116,     0,   337,   345,   342,     0,     0,  1019,
    1024,  1021,  1020,  1023,  1022,    12,  1074,  1075,     0,   840,
       0,     0,     0,   981,   978,     0,   587,     0,   601,   839,
     589,   839,   616,   604,   610,   613,   607,  1018,  1017,  1016,
       0,  1012,     0,  1013,  1015,   203,     5,     0,     0,     0,
     641,   642,   650,   649,     0,   474,     0,   839,   582,   586,
       0,     0,  1035,     0,   564,     0,   203,  1063,   897,   316,
    1104,     0,     0,     0,   963,   839,  1052,  1048,   332,   333,
     740,   841,   329,     0,     0,     0,     0,   451,     0,     0,
     479,     0,   931,   210,   203,   140,   897,     0,     0,   258,
     561,   224,   909,   910,   534,     0,   624,   625,     0,   622,
     839,   994,     0,     0,   330,   260,     0,   258,     0,     0,
     256,     0,   973,   482,     0,     0,   928,   929,   958,   959,
       0,     0,     0,   876,   847,   848,   849,   856,     0,    37,
      38,    40,    41,    39,     0,     0,   862,   868,   869,   870,
     871,   872,     0,   860,   858,   859,   882,   840,     0,   890,
     992,   991,     0,   258,     0,   915,   763,     0,   296,     0,
     203,   146,   203,     0,   203,     0,     0,     0,     0,   266,
     267,   278,     0,   138,   276,   175,   286,     0,   286,     0,
     839,     0,     0,     0,     0,     0,   839,  1084,  1087,  1055,
     840,  1054,     0,   840,   812,   813,   810,   811,   846,     0,
     840,   838,   594,   619,   619,   594,   619,   585,     0,     0,
    1001,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1090,
     212,     0,   215,   179,     0,     0,     0,   117,     0,   122,
     123,   115,   841,   120,     0,   341,     0,  1031,   167,  1047,
    1076,  1067,  1071,   259,   261,   351,     0,     0,   979,     0,
     592,     0,  1011,     0,    17,   203,  1034,   257,   351,     0,
       0,   747,   579,     0,   753,  1036,     0,  1063,   568,   133,
     135,     0,     0,     0,  1105,     0,   321,   319,   903,   916,
    1049,   903,   917,  1051,     0,     0,   334,   131,   205,   207,
     208,   475,   186,   204,   184,   185,   209,   139,     0,   897,
     255,     0,   897,     0,   533,   998,   997,     0,   330,     0,
       0,     0,     0,     0,     0,   258,   226,   764,   902,   330,
       0,   852,   853,   854,   855,   863,   864,   880,     0,   840,
       0,   876,   598,   621,   621,   598,   621,     0,   851,   884,
       0,   839,   887,   889,   891,     0,   986,     0,   902,     0,
       0,     0,   203,   293,   575,   151,     0,   461,   266,   268,
     981,     0,     0,     0,   203,     0,     0,     0,     0,     0,
     280,     0,  1091,     0,     0,  1077,     0,  1083,  1081,   839,
       0,     0,     0,   783,   839,   837,     0,     0,   840,     0,
       0,   826,   840,     0,     0,     0,     0,   840,     0,   792,
     827,   828,  1005,     0,   840,   795,   797,   796,     0,     0,
     793,   794,   798,   800,   799,   816,   817,   814,   815,   818,
     819,   820,   821,   822,   807,   808,   802,   803,   801,   804,
     805,   806,   809,  1089,     0,   138,     0,     0,     0,     0,
     118,    23,   343,     0,     0,     0,  1068,  1073,     0,   425,
     983,   981,   467,   472,   480,     0,     0,    15,     0,   425,
     653,     0,     0,   655,   648,   651,     0,   646,     0,  1038,
       0,     0,     0,     0,   532,     0,   478,  1064,   572,     0,
     322,     0,     0,   317,     0,   336,   335,  1063,     0,   351,
       0,   897,     0,   330,     0,   955,   351,  1034,   351,  1037,
       0,     0,     0,   483,     0,     0,   866,   839,   875,   857,
       0,     0,   840,     0,     0,   874,   840,     0,     0,     0,
     850,     0,     0,   840,   861,   881,   990,   351,     0,   138,
       0,   289,   275,     0,     0,     0,   265,   171,   279,     0,
       0,   282,     0,   287,   288,   138,   281,  1092,  1078,     0,
       0,  1053,     0,  1103,   845,   844,   782,   602,   839,   593,
       0,   605,   839,   618,   611,   614,   608,     0,   839,   584,
     784,     0,   623,   839,  1000,   824,     0,     0,   203,    24,
      25,    26,    27,  1070,  1065,  1066,  1069,   262,     0,     0,
       0,   432,   423,     0,     0,     0,   237,   350,   352,     0,
     422,     0,     0,     0,  1034,   425,     0,   591,  1014,   347,
     243,   644,     0,     0,   578,   566,   475,   136,   203,     0,
     325,   315,     0,   318,   324,   330,   558,  1063,   425,  1063,
       0,   996,     0,   954,   425,     0,   425,  1039,   351,   897,
     952,   879,   878,   865,   603,   839,   597,     0,   606,   839,
     620,   612,   615,   609,     0,   867,   839,   883,   425,   138,
     203,   147,   152,   173,   269,   203,   277,   283,   138,   285,
       0,  1079,     0,     0,     0,   596,   825,   581,     0,  1004,
    1003,   823,   138,   216,  1072,     0,     0,     0,  1042,     0,
       0,     0,   263,     0,  1034,     0,   388,   384,   390,   742,
      36,     0,   378,     0,   383,   387,   400,     0,   398,   403,
       0,   402,     0,   401,     0,   220,   354,     0,   356,     0,
     357,   358,     0,     0,   980,     0,   645,   643,   654,   652,
       0,   326,     0,     0,   313,   323,     0,     0,  1063,  1057,
     233,   558,  1063,   956,   239,   347,   245,   425,     0,     0,
       0,   600,   873,   886,     0,   241,   291,   203,   203,   138,
     272,   172,   284,  1080,  1102,   843,     0,     0,     0,   203,
       0,     0,   450,     0,  1043,     0,   368,   372,   447,   448,
     382,     0,     0,     0,   363,   703,   704,   702,   705,   706,
     723,   725,   724,   694,   666,   664,   665,   684,   699,   700,
     660,   671,   672,   674,   673,   693,   677,   675,   676,   678,
     679,   680,   681,   682,   683,   685,   686,   687,   688,   689,
     690,   692,   691,   661,   662,   663,   667,   668,   670,   708,
     709,   713,   714,   715,   716,   717,   718,   701,   720,   710,
     711,   712,   695,   696,   697,   698,   721,   722,   726,   728,
     727,   729,   730,   707,   732,   731,   734,   736,   735,   669,
     739,   737,   738,   733,   719,   659,   395,   656,     0,   364,
     416,   417,   415,   408,     0,   409,   365,   442,     0,     0,
       0,     0,   446,     0,   220,   229,   346,     0,     0,     0,
     314,   328,   953,     0,     0,     0,     0,  1063,  1057,     0,
     235,  1063,   877,     0,     0,   138,   270,   153,   174,   203,
     595,   580,  1002,   214,   366,   367,   445,   264,     0,   840,
     840,     0,   391,   379,     0,     0,     0,   397,   399,     0,
       0,   404,   411,   412,   410,     0,     0,   353,  1044,     0,
       0,     0,   449,     0,   348,     0,   327,     0,   639,   842,
     134,   842,  1059,     0,   418,   134,   223,     0,     0,   231,
       0,   599,   885,   203,     0,   176,   369,   124,     0,   370,
     371,     0,   839,     0,   839,   393,   389,   394,   657,   658,
       0,   380,   413,   414,   406,   407,   405,   443,   440,  1076,
     359,   355,   444,     0,   349,   640,   841,     0,   203,   841,
    1058,     0,  1062,   203,   134,   225,   227,     0,   273,     0,
     218,     0,   425,     0,   385,   392,   396,     0,     0,   897,
     361,     0,   637,   557,   560,  1060,  1061,   419,   203,   271,
       0,     0,   177,   376,     0,   424,   386,   441,  1045,     0,
     842,   436,   897,   638,   562,     0,   217,     0,     0,   375,
    1063,   897,   300,   439,   438,   437,  1105,   435,     0,     0,
       0,   374,  1057,   436,     0,  1063,     0,   373,     0,  1105,
       0,   305,   303,  1057,   138,   420,   134,   360,     0,   306,
       0,     0,   301,     0,   203,   203,     0,   309,   299,     0,
     302,   308,   362,   213,   421,   310,     0,     0,   297,   307,
       0,   298,   312,   311
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1729, -1729, -1729,  -574, -1729, -1729, -1729,   531,  -458,   -47,
     489, -1729,  -239,  -542, -1729, -1729,   427,   690,  1775, -1729,
    2418, -1729,  -785, -1729,  -530, -1729,  -685,    21, -1729, -1729,
   -1729, -1729, -1729, -1729, -1729, -1729,  -926, -1729, -1729,  -913,
    -303, -1729, -1729, -1729,  -370, -1729, -1729,  -176,    32,    27,
   -1729, -1729, -1729, -1729, -1729, -1729,    37, -1729, -1729, -1729,
   -1729, -1729, -1729,    39, -1729, -1729,  1103,  1109,  1118,   -86,
    -735,  -931,   579,   648,  -380,   311, -1002, -1729,   -88, -1729,
   -1729, -1729, -1729,  -775,   130, -1729, -1729, -1729, -1729,  -366,
   -1729,  -605, -1729,  -456, -1729, -1729,  1008, -1729,   -68, -1729,
   -1729, -1136, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729,
   -1729, -1729,  -103, -1729,   -14, -1729, -1729, -1729, -1729, -1729,
    -185, -1729,    96, -1068, -1729, -1678,  -396, -1729,  -137,    79,
    -129,  -375, -1729,  -190, -1729, -1729, -1729,   102,   -93,   -60,
      46,  -783,   -52, -1729, -1729,     9, -1729,    93,  -393, -1729,
      11,    -5,   -85,   -74,   -46, -1729, -1729, -1729, -1729, -1729,
   -1729, -1729, -1729, -1729,  -635,  -890, -1729, -1729, -1729, -1729,
   -1729,   405,  1253, -1729,   509, -1729,   364, -1729, -1729, -1729,
   -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729,
   -1729, -1729, -1729,    22,  -593,  -688, -1729, -1729, -1729, -1729,
   -1729,   441, -1729, -1729, -1729, -1729, -1729, -1729, -1729, -1729,
   -1049,  -329,  2709,    28, -1729,   401,  -430, -1729, -1729,  -485,
    3644,  3749, -1729,   265, -1729, -1729,   521,   -99,  -663, -1729,
   -1729,   601,   374,   392, -1729,   375, -1729, -1729, -1729, -1729,
   -1729,   576, -1729, -1729, -1729,    10,  -919,  -148,  -440,  -432,
   -1729,   669,  -138, -1729, -1729,    29,    34,   189,   -83, -1729,
   -1729,   532,   -62, -1729,  -328,     5,  -384,    94,   211, -1729,
   -1729,  -481,  1271, -1729, -1729, -1729, -1729, -1729,   795,   438,
   -1729, -1729, -1729,  -312,  -706, -1729,  1223, -1304,  -234,    51,
    -179,   -33,   789, -1729, -1728, -1729,  -283,  -857, -1317,  -272,
     135, -1729,   479,   556, -1729, -1729, -1729, -1729,   507, -1729,
    2915, -1156
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   115,   966,   666,   190,   348,   778,
     368,   369,   370,   371,   917,   918,   919,   117,   118,   119,
     120,   121,   986,  1219,   427,  1014,   699,   700,   574,   266,
    1707,   580,  1615,  1708,  1960,   902,   123,   124,   719,   720,
     728,   361,   603,  1915,  1173,  1385,  1982,   450,   191,   701,
    1017,  1253,  1452,   127,   669,  1036,   702,   734,  1040,   641,
    1035,   245,   555,   703,   670,  1037,   452,   388,   410,   130,
    1019,   969,   942,  1193,  1641,  1312,  1099,  1857,  1711,   853,
    1105,   579,   862,  1107,  1495,   845,  1088,  1091,  1301,  1989,
    1990,   689,   690,   715,   716,   375,   376,   378,  1675,  1836,
    1837,  1399,  1547,  1664,  1830,  1969,  1992,  1868,  1919,  1920,
    1921,  1651,  1652,  1653,  1654,  1870,  1871,  1877,  1931,  1657,
    1658,  1662,  1823,  1824,  1825,  1906,  2027,  1548,  1549,   192,
     132,  2006,  2007,  1828,  1551,  1552,  1553,  1554,   133,   134,
     575,   576,   135,   136,   137,   138,   139,   140,   141,   142,
     259,   143,   144,   145,  1688,   146,  1016,  1252,   147,   686,
     687,   688,   263,   419,   570,   675,   676,  1347,   677,  1348,
     148,   149,   647,   648,  1337,  1338,  1461,  1462,   150,   887,
    1067,   151,   888,  1068,   152,   889,  1069,   153,   890,  1070,
     154,   891,  1071,   650,  1340,  1464,   155,   892,   156,   157,
    1899,   158,   671,  1677,   672,  1209,   974,  1417,  1414,  1816,
    1817,   159,   160,   161,   248,   162,   249,   260,   431,   562,
     163,  1341,  1342,   896,   897,   164,  1129,   996,   618,  1130,
    1074,  1275,  1075,  1465,  1466,  1278,  1279,  1077,  1472,  1473,
    1078,   821,   545,   204,   205,   704,   692,   529,  1228,  1229,
     809,   810,   460,   166,   251,   167,   168,   194,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   737,   255,
     256,   644,   239,   240,   773,   774,  1353,  1354,   403,   404,
     960,   180,   632,   181,   685,   182,   351,  1838,  1889,   389,
     439,   710,   711,  1122,  1846,  1901,  1902,  1223,  1396,   938,
    1397,   939,   940,   868,   869,   870,   352,   353,   899,   589,
    1640,   991
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     193,   195,   457,   197,   198,   199,   200,   202,   203,   349,
     206,   207,   208,   209,   165,   537,   229,   230,   231,   232,
     233,   234,   235,   236,   238,   122,   257,   989,   510,   681,
     444,   126,   530,   531,   424,   254,   125,   844,   445,   265,
     446,   128,  1015,   129,   262,   777,   426,   273,   421,   276,
     723,   984,   359,  1002,   362,   816,   422,   267,   247,   252,
    1221,  1092,   271,   457,   253,   559,   357,   768,  1429,  1224,
     830,  1095,  1409,   900,   916,   921,   812,   813,  1039,   509,
    1536,   265,   985,   131,   678,   680,  1109,  1213,  1251,   807,
     264,  1238,   965,  1243,  1308,  1083,   441,   808,  1725,   860,
     571,   682,  1493,   423,   -78,   837,  1262,   563,   -43,   -78,
     624,    14,   -42,   -43,   627,   424,   571,   -42,   564,    14,
    1908,    14,    14,   840,   608,   610,   612,   426,   615,   421,
    1667,   841,   397,   858,  1669,  -626,  -381,  1733,  1818,  -633,
    1886,   554,   729,   730,   731,  1886,   548,  1725,  -932,   411,
     358,  1879,  1297,   414,   415,   927,   571,   944,   426,  1004,
     547,  1897,   944,   456,   944,   944,  1221,   944,    14,   557,
     620,  1971,     3,  1561,   546,  1139,   835,  1225,  1880, -1049,
     373,   453,  1569,   556,   423,  -934,  -451,  1430,  -108,  1681,
    1287,   540,  1415,   169,  1874,  2020,  1317,  1318,  1317,  1318,
     438,  2036,  -107,  -108,   379,   429,  1898,  -636,   226,   228,
     936,   937,  1875,   380,  1140,   423,  1972,  -107, -1049,   458,
    1168,   196,  1226,  -744,   538,  1416,  -569,  1570,   211,    40,
    1955,  1876,  1956,   621,  -935,  1639,  -933,  -974,   423,  -752,
    2021,   438,   400,  -931,   511,   534,  2037,  1346,  -634,  -633,
     656,   527,   528,   566,  1183,   534,   566,  -841,  1288,  -944,
    2029,  -841,  -943,   265,   577,  -320,   533,   861,  -939,  1494,
    -938,  -320,  -304,  -936,  -633,  -942,  -751,   588,  -932,  1431,
     735,  1682,  -977,   568,  2018,   412,   442,   573,  1726,  1727,
     572,  1320,   458,  1496,   -78,  2033,   416,   425,   -43,   374,
     625,  1220,   -42,  1578,   628,  1265,   654,   599,  1422,  1486,
    1584,   635,  1586,  -976,  1227,  -934,  -451,  1536,  -841,  1571,
    1668,  1094,   859,   638,  1670,   634,  -381,  1734,  1819,  1247,
    1887,  1315,  2022,  1319,  1451,  1941,  -746,  2017,  2038,  1728,
    1881,  1608,  2045,  -839,   928,   929,   945,   111,  1005,   459,
     200,  1050,  2030,  1400,  1614,   377,  1674,   527,   528,  -753,
    1420,  -941,   817,  1831,  -935,  1832,  -933,  -974,  -918,   725,
    -945,  1179,  1180,  -931,  -919,   535,   533,   933,   425,   265,
     423,   426,   457,   721,   655,   535,   238,   646,   265,   265,
     646,   265,  1205,   458,   349,  1034,   660,  -948,  -939,  1471,
    -938,   533,  -905,  -936,   727,   258,   417,  1220,  1474,   425,
     360,  1248,  -977,   418,   261,   202,   651,  -905,   653,   392,
     224,   224,   459,   705,   222,   222,   550,   971,   604,   691,
     633,   909,   558,  -745,  2046,   717,   620,   381,   724,   649,
     649,   268,   649,  -976,  1951,   269,   823,   382,  -908,   527,
     528,   169,  1697,   736,   738,   169,   393,   909,  1350,   125,
     788,   270,  1196,  -908,   739,   740,   741,   742,   744,   745,
     746,   747,   748,   749,   750,   751,   752,   753,   754,   755,
     756,   757,   758,   759,   760,   761,   762,   763,   764,   765,
     766,   767,   605,   769,   910,   736,   736,   772,  -918,   437,
    1408,   733,   254,  -635,  -919,   383,   131,   791,   792,   793,
     794,   795,   796,   797,   798,   799,   800,   801,   802,   803,
     437,   789,   777,  1568,  1483,   247,   252,   717,   717,   736,
     815,   253,  1394,  1395,   791,   116,   722,   819,   398,   972,
    -906,  1343,   786,  1345,   384,   992,   827,   993,   829,   950,
     952,   227,   227,  1231,   973,  -906,   717,   411,   783,   784,
     453,  1232,   385,   623,   848,   834,   849,   510,   527,   528,
     211,    40,   631,  1314,   636,   398,   386,   978,  1259,   643,
    1576,   390,   662,   398,   274,  -124,  1952,   347,  1428,  -124,
     399,   661,   391,   681,  -747,   852,  1467,   779,  1469,  1033,
     975,   437,   398,  1882,   387,   790,  -124,   527,   528,   662,
    1041,  1240,  1267,  1240,   401,   402,   169,   923,   509,   398,
     527,   528,  1883,   811,   398,  1884,   662,   409,  1628,   387,
    -946,  1045,   726,   387,   387,   667,   668,  1410,   224,   394,
     916,   395,   222,   779,  1438,   527,   528,  1440,   678,   680,
    1411,   401,   402,   396,   836,   447,   398,   842,   400,   401,
     402,   387,   847,   430,  1174,   682,  1175,   398,  1176,   423,
    1412,   412,  1178,   964,   433,  1031,   992,   993,   401,   402,
     413,  1351,  -946,  1084,   993,   398,   436,   428,   437,   111,
     440,   497,   662,  1085,   663,   401,   402,   691,   708,   657,
     401,   402,  1242,   498,  1538,  1244,  1245,   559,   443,   999,
     448,   770,   771, -1049,   936,   937,  1704,    55,  1934,  1169,
    1689,   449,  1691,   461,   707,   643,   454,   184,   185,    65,
      66,    67,   401,   402,   438,  -627,   544,  1935,  1024,   462,
    1936,  1585,   463,   401,   402,   814,    14,  1089,  1090, -1049,
     372,  -628, -1049,   681,  1299,  1300,   494,   495,   496,   125,
     497,   401,   402,   169,  1111,  1316,  1317,  1318,   502,   227,
    1117,  1032,   498,  1394,  1395,   224,  1635,  1636,   407,   222,
     464,   408,  1453,   511,   224,  1565,  1021,   465,   222,  1904,
    1905,   224,   503,   116,   466,   222,   467,   116,   455,   468,
    1044,   578,  1444,   224,  2025,  2026,   131,   222,   678,   680,
    1539,  1932,  1933,  1454,   679,  -629,  1540,  -630,   454,  1541,
     185,    65,    66,    67,  1542,   682,  1928,  1929,  1240,  1485,
    -631,  1844,   500,  1087,   501,  1848,  1580,   658,  1120,  1123,
    1191,   664,   454,   184,   185,    65,    66,    67,  1672,   265,
    2014,  -632,   607,   609,   611,  1528,   614,  -745,  1164,  1165,
    1166,  1093,   405,  2028,   532,  -940,  1543,  1544,   658,  1545,
     664,   658,   664,   664,  1167,   536,   125,  1490,  1317,  1318,
    1104,   541,   543,  1015,   498,   598,  2003,  2004,  2005,   438,
     455,   454,   184,   185,    65,    66,    67,   549,  -944,  1546,
     553,   533,   998,   552,  -743,   681,   227,   432,   434,   435,
     560,   561,   569,   590,   455,   227,   169,   637,   582, -1088,
    1556,  1999,   227,   131,   491,   492,   493,   494,   495,   496,
     593,   497,   125,   600,   227,   594,   601,   224,  1729,   616,
     617,   222,  1200,   498,  1201,   619,   849,   626,   629,  1610,
    1161,  1162,  1163,  1164,  1165,  1166,   630,  1203,   116,   639,
     678,   680,   640,   455,   683,  1619,   684,   693,   709,  1167,
     706,  1212,   347,   694,   695,   165,  1043,   682,  1292,   131,
     697,   387,  -129,   727,  1698,   732,   122,  1582,    55,   820,
    1907,   822,   126,   657,  1910,   824,  1236,   125,   691,   724,
     825,   724,   128,   850,   129,   831,   791,   832,   571,   854,
     857,   588,   872,   871,   903,  1080,   901,  1081,   125,   904,
    1254,   905,  1221,  1255,  1331,  1256,   691,  1221,   723,   717,
     906,  1335,   907,   169,   598,   387,   781,   387,   387,   387,
     387,   908,   926,   911,   131,  1100,   125,   254,   169,   912,
    1991,  1434,  1221,   372,   372,   372,   613,   372,   930,   931,
     806,   934,   941,   943,   935,   131,   946,   949,   227,   951,
     247,   252,   962,  1991,  1214,   948,   253,  1296,   967,  1706,
     598,  1638,  2013,   953,   954,  1073,   811,   842,  1712,   169,
     955,   956,  1266,   131,   839,   665,   968,  1239,   970,  1239,
    1302,  -768,  1719,   976,   790,   116,   977,   980,   979,  1221,
     983,  1303,   987,   988,   224,  1948,  1187,   995,   222,  1686,
    1953,   729,   730,   731,   125,   898,   125,   997,  1423,  1001,
    1000,  1003,   643,  1198,   681,  1006,  1424,  1007,  1425,  1008,
    1009,  1010,  1018,  2012,  1030,  1038,  1046,  1402,  1029,  1022,
    1047,   922,   709,  1026,   169,  1027,  -749,  1048,  2023,  1086,
    1020,  1096,  1110,  1106,  1108,  1114,  1115,  1116,  1118,  1978,
    1131,   131,  1132,   131,  1137,   169,  1133,  1134,   842,  1859,
    1458,  1135,  1724,  1136,  1172,  1638,   959,   961,   224,   678,
     680,  1182,   222,  1241,  1404,  1241,  1189,  1186,  1184,  1188,
    1190,  1195,  1199,   169,  1210,  1215,   682,  1208,  1202,  1638,
    1211,  1638,  1076,  1217,  1249,   165,   724,  1638,  1222,  1258,
    1264,  1261,   681,  1269,  -947,  1270,   122,   224,  1280,   224,
    1436,   222,   126,   222,  1281,  1282,  1947,   125,  1950,  1509,
    1283,  2035,   128,  1513,   129,   227,  1289,  1284,  1519,  1285,
    1286,  1290,  1291,   717,  1293,  1524,  1305,   224,   116,   691,
    1307,   222,   691,  1220,   717,  1404,   387,  1538,  1220,  1310,
    1311,  1313,  1322,  1323,  1329,  1324,  1330,   678,   680,   169,
    1333,   169,  1167,   169,   131,  1100,  1309,   530,  1334,  1386,
    1384,  1391,  1403,  1220,   682,  1387,  1388,  1398,  1401,  1389,
    1418,  1432,   265,  1433,  1419,  1455,   920,   920,  1437,    14,
    1478,  1449,  1492,  1034,  1239,  1439,  1441,  1457,  1073,   227,
    1443,  1446,  1445,  1481,  1057,  1913,  1456,  2002,   224,  1448,
    1470,  1479,   222,  1480,  1477,  1497,  1482,  1487,  1500,  1491,
    1503,  1502,  1506,  1508,   224,   224,   125,   424,   222,   222,
    1220,  1066,  1507,  1079,  1511,  1512,  1517,  1518,   227,   426,
     227,   421,  1520,  1596,  1514,  1515,  1516,  1600,  1522,  1523,
    1529,  1527,  1572,  1539,  1607,   116,  1558,  1530,   679,  1540,
    1531,   454,  1541,   185,    65,    66,    67,  1542,   227,  1102,
     116,  1638,  1532,   131,   169,   210,  1559,  1567,  1573,  1575,
    1557,  1577,  1579,  1583,  1594,  1589,  1595,  1562,  1587,  1555,
    1241,  1563,  1588,  1564,  1590,  1593,   724,    50,  1673,  1555,
    1597,   457,  1598,  1435,  1599,   913,   914,  1574,  1601,  1543,
    1544,   116,  1545,  1602,  1603,  1604,  1605,  1581,   717,  1606,
    1177,   709,  1609,  1611,  1612,  1277,  1613,  1620,  1616,  1617,
    1643,   691,  1632,   455,   214,   215,   216,   217,   218,   227,
    1656,  1671,  1560,  1676,  1683,  1684,  1692,  1687,  1693,  1695,
    1699,  1192,  1714,  1717,  1723,   227,   227,  1731,  1550,   915,
    1732,  1826,    93,    94,  1476,    95,   188,    97,  1550,  1829,
    1827,   169,  1833,  1839,  2034,  1840,   116,   224,   224,   643,
    1100,   222,   222,   169,   454,    63,    64,    65,    66,    67,
     107,   598,  1853,  1843,   722,    72,   504,   116,  1842,  1845,
    1851,  1885,  1854,   806,   839,  1864,  1865,  1891,  1894,  1895,
    1900,  1922,  1924,  1926,  1930,  1938,  1939,  1940,   679,  1945,
    1946,  1949,  1954,  1958,  1959,   116,  1073,  1073,  1073,  1073,
    1073,  1073,  1961,  -377,  1964,  1962,  1073,  1880,   506,  1966,
     125,  1967,  1970,  1680,  1973,  1980,  1979,  1685,  1981,  1993,
     717,  1986,   387,  1988,  1997,  2000,   455,  2001,  2009,  2011,
    2031,  1665,  1274,  1274,  1066,  2015,  2032,  2039,  1555,  2040,
     643,  2016,  2024,   920,  1555,   920,  1555,   920,  2047,   691,
     125,   920,  2048,   920,   920,  1181,  2050,   131,  2051,  1390,
    1566,  1996,   785,   780,  1260,   839,  1207,  2010,  1555,  1484,
    1858,   116,  1618,   116,   782,   116,  2008,  1849,   227,   227,
    1873,  1730,   924,  1878,   511,   224,  1710,  2042,  2019,   222,
    1890,  1663,   125,  1644,  1344,  1847,  1326,   131,   652,  1468,
    1413,   125,  1336,  1276,  1459,  1294,  1460,  1550,  1893,  1943,
    1121,  1230,   645,  1550,   718,  1550,  1975,  1968,  1634,  1393,
     598,   210,  1328,  1277,  1463,  1383,     0,  1463,  1841,     0,
       0,     0,     0,  1475,     0,     0,     0,  1550,  1722,   131,
     679,     0,     0,    50,     0,     0,   224,     0,   131,   898,
     222,     0,     0,     0,     0,  1834,     0,  1555,     0,     0,
       0,   224,   224,     0,     0,   222,   222,   169,     0,     0,
    1073,     0,  1073,     0,     0,     0,     0,     0,  1856,  1710,
     214,   215,   216,   217,   218,     0,   116,     0,   125,     0,
       0,     0,     0,     0,   125,     0,     0,     0,     0,     0,
       0,   125,     0,  1426,     0,   405,     0,   169,    93,    94,
       0,    95,   188,    97,     0,  1538,   227,     0,     0,     0,
    1923,  1925,     0,     0,     0,     0,  1550,     0,     0,     0,
       0,     0,  1888,     0,     0,   131,   107,     0,     0,     0,
     406,   131,     0,     0,   221,   221,     0,     0,   131,   169,
       0,     0,   224,     0,   169,   244,   222,    14,   169,     0,
       0,     0,  1066,  1066,  1066,  1066,  1066,  1066,     0,     0,
       0,     0,  1066,     0,     0,  1984,     0,   227,     0,     0,
       0,   244,     0,   116,  1896,     0,     0,     0,     0,     0,
       0,     0,   227,   227,   210,   116,  1888,  1591,     0,  1592,
       0,  1538,     0,     0,     0,  1499,   457,     0,     0,     0,
    1073,     0,  1073,     0,  1073,     0,    50,     0,     0,  1073,
       0,  1539,   920,     0,     0,     0,     0,  1540,     0,   454,
    1541,   185,    65,    66,    67,  1542,     0,     0,     0,     0,
       0,   125,     0,    14,     0,   169,   169,   169,     0,     0,
       0,   169,     0,   214,   215,   216,   217,   218,   169,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   679,
       0,     0,     0,     0,  1533,     0,     0,  1543,  1544,   451,
    1545,    93,    94,   227,    95,   188,    97,     0,   131,     0,
       0,     0,     0,   864,     0,   125,     0,     0,     0,     0,
       0,   455,     0,     0,     0,     0,     0,  1539,     0,   107,
    1690,     0,     0,  1540,  1073,   454,  1541,   185,    65,    66,
      67,  1542,     0,     0,     0,     0,     0,     0,     0,   691,
     125,     0,     0,     0,     0,   125,  1066,  1700,  1066,  1701,
       0,  1702,   131,   210,     0,     0,  1703,     0,     0,     0,
       0,     0,   691,   865,     0,     0,     0,   679,     0,     0,
     125,   691,   221,  1543,  1544,    50,  1545,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  2041,   131,     0,     0,
       0,     0,   131,     0,  2049,     0,     0,   455,     0,     0,
       0,  1985,  2052,     0,     0,  2053,  1694,     0,   169,     0,
       0,     0,   214,   215,   216,   217,   218,   131,     0,   116,
       0,     0,   244,     0,   244,     0,   125,   125,     0,     0,
     347,     0,     0,     0,   187,     0,  1661,    91,     0,     0,
      93,    94,     0,    95,   188,    97,     0,   866,     0,     0,
       0,  1852,     0,     0,     0,     0,     0,     0,     0,   116,
       0,     0,   169,     0,     0,     0,     0,     0,   107,     0,
       0,     0,     0,   131,   131,     0,     0,     0,  1073,  1073,
       0,   244,     0,     0,     0,     0,  1066,     0,  1066,     0,
    1066,     0,     0,     0,     0,  1066,     0,   169,     0,     0,
       0,   116,   169,  1538,     0,     0,   116,     0,     0,   221,
     116,     0,     0,     0,     0,     0,     0,     0,   221,  1538,
       0,     0,     0,     0,     0,   221,     0,   169,   387,     0,
       0,   598,     0,     0,   347,     0,     0,   221,  1538,     0,
       0,     0,     0,     0,  1815,    14,     0,     0,   221,     0,
       0,  1822,     0,     0,     0,     0,     0,     0,   347,     0,
     347,    14,     0,     0,     0,     0,   347,     0,     0,     0,
       0,     0,     0,   244,     0,     0,   244,     0,     0,     0,
      14,     0,     0,   169,   169,     0,     0,     0,     0,     0,
    1066,     0,     0,     0,     0,     0,     0,   116,   116,   116,
       0,     0,     0,   116,     0,  1911,  1912,     0,     0,  1539,
     116,     0,     0,     0,     0,  1540,     0,   454,  1541,   185,
      65,    66,    67,  1542,     0,  1539,     0,     0,     0,     0,
       0,  1540,   244,   454,  1541,   185,    65,    66,    67,  1542,
       0,     0,     0,     0,  1539,     0,     0,   210,     0,     0,
    1540,     0,   454,  1541,   185,    65,    66,    67,  1542,     0,
       0,     0,     0,     0,     0,  1543,  1544,     0,  1545,    50,
       0,   221,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1543,  1544,     0,  1545,     0,     0,     0,     0,   455,
       0,     0,     0,     0,     0,   210,     0,     0,  1696,     0,
    1543,  1544,     0,  1545,     0,   455,   214,   215,   216,   217,
     218,     0,     0,     0,  1705,     0,     0,    50,     0,     0,
     598,     0,     0,   244,   455,   244,     0,     0,   886,     0,
       0,     0,  1820,  1850,    93,    94,  1821,    95,   188,    97,
     347,     0,     0,     0,  1066,  1066,     0,     0,     0,     0,
     116,     0,     0,     0,   214,   215,   216,   217,   218,  1917,
       0,   886,   107,  1660,     0,     0,  1815,  1815,     0,     0,
    1822,  1822,     0,     0,     0,     0,   187,     0,     0,    91,
       0,     0,    93,    94,   598,    95,   188,    97, -1089, -1089,
   -1089, -1089, -1089,   489,   490,   491,   492,   493,   494,   495,
     496,     0,   497,     0,   116,     0,     0,     0,     0,     0,
     107,     0,     0,     0,   498,  1916,     0,     0,   244,   244,
       0,     0,     0,     0,     0,     0,     0,   244,     0,     0,
       0,     0,     0,     0,   350,     0,     0,     0,     0,   116,
       0,     0,     0,     0,   116,     0,     0,     0,   221,     0,
       0,     0,  1983,   469,   470,   471,   539,   513,   514,   515,
     516,   517,   518,   519,   520,   521,   522,   523,   524,   116,
       0,     0,     0,   472,   473,  1998,   474,   475,   476,   477,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,     0,
     497,   525,   526,     0,     0,     0,     0,   469,   470,   471,
       0,     0,   498,     0,     0,     0,     0,     0,  1645,     0,
       0,     0,   221,     0,     0,   116,   116,   472,   473,     0,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,     0,   497,   244,     0,     0,     0,     0,
       0,   221,     0,   221,     0,     0,   498,     0,   210,     0,
       0,     0,     0,     0,     0,     0,     0,   527,   528,  1271,
    1272,  1273,   210,     0,     0,     0,     0,     0,     0,     0,
      50,   221,   886,     0,     0,     0,     0,     0,     0,   244,
       0,     0,     0,     0,    50,     0,   244,   244,   886,   886,
     886,   886,   886,  1646,     0,     0,     0,     0,     0,     0,
     886,     0,    34,    35,    36,     0,  1647,   214,   215,   216,
     217,   218,  1648,     0,     0,   212,   244,     0,     0,     0,
     696,   214,   215,   216,   217,   218,     0,  1406,     0,   187,
       0,     0,    91,  1649,     0,    93,    94,     0,    95,  1650,
      97,     0,   221,     0,     0,   350,     0,   350,     0,    93,
      94,     0,    95,   188,    97,     0,   244,     0,   221,   221,
       0,     0,     0,   107,     0,     0,     0,     0,   223,   223,
      81,    82,    83,    84,    85,     0,   499,   107,  1023,   246,
       0,   219,   244,   244,     0,     0,     0,    89,    90,     0,
       0,     0,   221,     0,     0,     0,     0,     0,   244,     0,
       0,    99,     0,     0,   350,     0,     0,     0,     0,     0,
     244,     0,     0,     0,     0,   104,     0,     0,   886,   539,
     513,   514,   515,   516,   517,   518,   519,   520,   521,   522,
     523,   524,     0,   244, -1089, -1089, -1089, -1089, -1089,  1159,
    1160,  1161,  1162,  1163,  1164,  1165,  1166,     0,     0,     0,
     469,   470,   471,   244,     0,     0,     0,   244,     0,     0,
    1167,     0,     0,     0,   525,   526,     0,     0,   244,     0,
     472,   473,     0,   474,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   350,   497,     0,   350,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   498,
       0,   221,   221,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   244,     0,     0,     0,   244,
       0,   244,   210,   454,    63,    64,    65,    66,    67,     0,
     527,   528,  1112,     0,    72,   504,   886,   886,   886,   886,
     886,   886,   221,     0,    50,   886,   886,   886,   886,   886,
     886,   886,   886,   886,   886,   886,   886,   886,   886,   886,
     886,   886,   886,   886,   886,   886,   886,   886,   886,   886,
     886,   886,   886,     0,     0,   505,   223,   506,     0,     0,
       0,   214,   215,   216,   217,   218,     0,     0,     0,     0,
     507,   886,   508,   833,     0,   455,     0,     0,     0,     0,
       0,   354,     0,   187,     0,     0,    91,     0,     0,    93,
      94,   210,    95,   188,    97,     0,     0,     0,     0,     0,
     469,   470,   471,     0,     0,     0,     0,   244,     0,   244,
       0,   963,     0,    50,     0,     0,   350,   107,   867,   221,
     472,   473,     0,   474,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   244,   497,     0,   244,
     214,   215,   216,   217,   218,     0,     0,     0,     0,   498,
       0,     0,     0,     0,     0,     0,   244,   244,   244,   244,
     244,   244,     0,     0,   221,     0,   244,     0,    93,    94,
     221,    95,   188,    97,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   223,     0,   221,   221,     0,   886,     0,
       0,     0,   223,     0,     0,     0,   107,   732,   244,   223,
       0,   350,   350,     0,   244,     0,     0,   886,     0,   886,
     350,   223,     0,     0,     0,   469,   470,   471,     0,     0,
       0,     0,   223,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   886,     0,   472,   473,     0,   474,   475,
     476,   477,   478,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,     0,   497,     0,     0,     0,     0,     0,     0,   244,
     244,     0,     0,   244,   498,     0,   221,     0,     0,     0,
       0,   994,   539,   513,   514,   515,   516,   517,   518,   519,
     520,   521,   522,   523,   524,     0,     0,     0,     0,     0,
       0,     0,   586,     0,   587,     0,   246,  1011,   513,   514,
     515,   516,   517,   518,   519,   520,   521,   522,   523,   524,
       0,     0,     0,     0,     0,     0,     0,   525,   526,     0,
     244,     0,   244,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   223,     0,     0,     0,     0,
       0,     0,   525,   526,     0,     0,     0,     0,     0,     0,
       0,   592,  1011,   513,   514,   515,   516,   517,   518,   519,
     520,   521,   522,   523,   524,   244,     0,   244,     0,     0,
       0,     0,  1113,   886,     0,   886,     0,   886,     0,   350,
     350,     0,   886,   221,     0,     0,   886,     0,   886,     0,
       0,   886,   893,   527,   528,     0,  1023,   525,   526,     0,
       0,     0,     0,     0,   244,   244,     0,   287,   244,     0,
       0,     0,     0,     0,     0,   244,     0,     0,   527,   528,
       0,     0,     0,     0,     0,   893,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   210,     0,     0,     0,
       0,     0,     0,   712,   289,     0,   354,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   932,   210,    50,     0,
     244,     0,   244,     0,   244,     0,     0,     0,     0,   244,
       0,   221,     0,   527,   528,   350,     0,     0,     0,    50,
       0,  1012,     0,     0,  1659,   244,     0,   591,   886,     0,
       0,   350,     0,     0,     0,   214,   215,   216,   217,   218,
     244,   244,     0,   350,     0,     0,     0,     0,   244,     0,
     244,     0,   223,     0,     0,   584,   214,   215,   216,   217,
     218,   585,     0,    93,    94,     0,    95,   188,    97,     0,
       0,     0,   244,     0,   244,     0,   696,     0,   187,     0,
     244,    91,   341,     0,    93,    94,   350,    95,   188,    97,
       0,   107,  1660,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   345,     0,   244,     0,     0,     0,     0,     0,
       0,     0,   107,   346,     0,     0,     0,     0,     0,     0,
       0,   886,   886,   886,     0,     0,   223,     0,   886,     0,
     244,     0,     0,   863,     0,     0,   244,     0,   244,     0,
       0,     0,     0,     0,     0,  1141,  1142,  1143,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   350,  1072,
       0,     0,   350,     0,   867,   223,  1144,   223,     0,  1145,
    1146,  1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,
    1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,
    1166,     0,     0,     0,     0,   223,   893,     0,     0,     0,
       0,     0,     0,     0,  1167,     0,     0,     0,     0,     0,
       0,     0,   893,   893,   893,   893,   893,     0,     0,     0,
       0,     0,     0,     0,   893,     0,     0,     0,   981,   982,
       0,     0,     0,     0,     0,     0,     0,   990,   244,     0,
    1171,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     244,     0,     0,   864,   244,     0,     0,     0,   244,   244,
       0,     0,     0,     0,     0,     0,   223,     0,     0,     0,
     350,     0,   350,   244,     0,     0,     0,     0,     0,   886,
    1194,     0,   223,   223,     0,     0,     0,     0,   210,     0,
     886,     0,     0,   225,   225,     0,   886,     0,     0,     0,
     886,     0,     0,   210,   250,     0,     0,  1194,     0,   350,
      50,     0,   350,   865,     0,     0,   223,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,   244,     0,     0,
       0,     0,     0,  1349,     0,     0,     0,     0,     0,     0,
       0,     0,   893,     0,     0,     0,     0,   214,   215,   216,
     217,   218,     0,     0,   244,     0,   244,  1250,     0,     0,
       0,     0,   214,   215,   216,   217,   218,     0,     0,     0,
       0,   350,   886,     0,     0,    93,    94,   350,    95,   188,
      97,   246,     0,     0,   187,   244,     0,    91,     0,     0,
      93,    94,  1072,    95,   188,    97,     0,  1327,     0,     0,
       0,     0,   244,   107,  1020,     0,     0,     0,     0,     0,
       0,   244,     0,     0,     0,     0,   712,   712,   107,     0,
       0,     0,     0,     0,   244,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   223,   223,     0,     0,     0,
       0,     0,   350,   350,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,     0,   497,     0,     0,     0,
     893,   893,   893,   893,   893,   893,   223,     0,   498,   893,
     893,   893,   893,   893,   893,   893,   893,   893,   893,   893,
     893,   893,   893,   893,   893,   893,   893,   893,   893,   893,
     893,   893,   893,   893,   893,   893,   893,     0,     0,     0,
       0,   225,  1206,     0,     0,     0,     0,   210,     0,   211,
      40,     0,     0,     0,     0,   893,     0,     0,  1216,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
    1233,     0,     0,     0,     0,     0,     0,     0,   350,     0,
     350,     0,   210,     0,   957,     0,   958,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   287,
       0,     0,     0,   223,    50,     0,   214,   215,   216,   217,
     218,     0,     0,  1263,     0,     0,     0,   350,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   350,     0,
       0,     0,   804,     0,    93,    94,   289,    95,   188,    97,
       0,   214,   215,   216,   217,   218,     0,     0,     0,   210,
    1072,  1072,  1072,  1072,  1072,  1072,     0,     0,   223,     0,
    1072,     0,   107,     0,   223,     0,   805,     0,   111,    93,
      94,    50,    95,   188,    97,     0,     0,     0,   225,   223,
     223,     0,   893,     0,     0,  1321,     0,   225,     0,  1325,
       0,     0,     0,     0,   225,     0,     0,   107,   350,     0,
       0,   893,     0,   893,     0,     0,   225,   584,   214,   215,
     216,   217,   218,   585,     0,     0,     0,   250,     0,     0,
       0,   350,     0,     0,     0,     0,     0,   893,     0,     0,
     187,     0,     0,    91,   341,     0,    93,    94,     0,    95,
     188,    97,     0,  1119,     0,   350,     0,   350,   469,   470,
     471,     0,     0,   350,   345,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   107,   346,     0,  1537,   472,   473,
     223,   474,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,     0,   497,     0,  1427,     0,   990,
       0,   250,     0,     0,     0,     0,     0,   498,     0,   350,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   469,   470,   471,  1072,     0,  1072,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1447,     0,     0,  1450,
     225,   472,   473,     0,   474,   475,   476,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,     0,   497,     0,
       0,     0,     0,     0,     0,     0,     0,   893,     0,   893,
     498,   893,     0,     0,     0,     0,   893,   223,     0,     0,
     893,     0,   893,     0,     0,   893,     0,   894,  1498,     0,
       0,     0,     0,     0,  1233,     0,     0,     0,     0,  1642,
       0,   350,  1655,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   350,     0,     0,     0,   350,     0,     0,
     894,     0,     0,     0,     0,     0,     0,     0,     0,  1049,
       0,     0,     0,     0,     0,     0,  1918,     0,   512,   513,
     514,   515,   516,   517,   518,   519,   520,   521,   522,   523,
     524,     0,     0,     0,  1072,     0,  1072,     0,  1072,  1534,
    1535,     0,     0,  1072,     0,   223,     0,   539,   513,   514,
     515,   516,   517,   518,   519,   520,   521,   522,   523,   524,
       0,     0,   893,   525,   526,     0,     0,     0,     0,     0,
     350,     0,   895,     0,  1720,  1721,     0,     0,     0,     0,
       0,     0,  1185,     0,  1655,     0,     0,   225,     0,   469,
     470,   471,   525,   526,     0,     0,     0,   350,     0,   350,
       0,     0,     0,     0,     0,   925,     0,     0,     0,   472,
     473,     0,   474,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,     0,   497,     0,  1072,   527,
     528,     0,     0,     0,   287,  1621,     0,  1622,   498,     0,
       0,     0,     0,     0,   350,   893,   893,   893,     0,     0,
       0,   225,   893,     0,  1867,     0,     0,   350,   527,   528,
       0,     0,  1655,     0,     0,     0,     0,     0,     0,     0,
       0,   289,   210,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   210,  1666,     0,     0,     0,     0,
     225,     0,   225,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,  -424,     0,     0,  1646,     0,     0,
     225,   894,     0,   454,   184,   185,    65,    66,    67,     0,
    1647,   214,   215,   216,   217,   218,  1648,   894,   894,   894,
     894,   894,   584,   214,   215,   216,   217,   218,   585,   894,
       0,     0,     0,   187,     0,  1713,    91,    92,     0,    93,
      94,     0,    95,  1650,    97,   187,     0,     0,    91,   341,
    1257,    93,    94,     0,    95,   188,    97,     0,     0,     0,
       0,     0,  1072,  1072,     0,     0,     0,   107,     0,   345,
       0,   225,     0,     0,     0,   455,     0,     0,     0,   107,
     346,     0,     0,   893,     0,     0,     0,   225,   225,     0,
       0,     0,     0,     0,   893,     0,     0,     0,     0,     0,
     893,     0,     0,     0,   893,     0,  1101,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   250,  1124,  1125,  1126,  1127,  1128,     0,   469,   470,
     471,     0,     0,     0,  1138,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1869,   894,   472,   473,
       0,   474,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,     0,   497,   893,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   250,   498,     0,  1995,
       0,   469,   470,   471,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1642,     0,     0,     0,
       0,   472,   473,     0,   474,   475,   476,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,     0,   497,     0,
     225,   225,     0,     0,     0,     0,     0,     0,  1892,     0,
     498,     0,  1237,     0,     0,     0,     0,     0,     0,     0,
    1903,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   894,   894,   894,   894,   894,
     894,   250,     0,     0,   894,   894,   894,   894,   894,   894,
     894,   894,   894,   894,   894,   894,   894,   894,   894,   894,
     894,   894,   894,   894,   894,   894,   894,   894,   894,   894,
     894,   894,     0,     0,     0,   469,   470,   471,     0,  1268,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     894,     0,     0,     0,     0,   472,   473,  1963,   474,   475,
     476,   477,   478,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,     0,   497,     0,  1903,     0,  1976,     0,     0,     0,
       0,     0,     0,     0,   498,     0,     0,     0,   225,     0,
    1128,  1339,  1298,     0,  1339,     0,     0,     0,     0,  1352,
    1355,  1356,  1357,  1359,  1360,  1361,  1362,  1363,  1364,  1365,
    1366,  1367,  1368,  1369,  1370,  1371,  1372,  1373,  1374,  1375,
    1376,  1377,  1378,  1379,  1380,  1381,  1382,     0,     0,     0,
       0,   990,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   250,   990,  1392,     0,     0,     0,   225,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   225,   225,     0,   894,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   894,   497,   894,   210,
       0,   211,    40,     0,   469,   470,   471,     0,     0,   498,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,   894,     0,   472,   473,  1678,   474,   475,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
       0,   497,     0,     0,     0,     0,     0,     0,   214,   215,
     216,   217,   218,   498,     0,   225,     0,  1011,   513,   514,
     515,   516,   517,   518,   519,   520,   521,   522,   523,   524,
       0,     0,  1488,     0,   804,     0,    93,    94,     0,    95,
     188,    97,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1504,     0,  1505,     0,     0,     0,     0,   469,   470,
     471,     0,   525,   526,   107,     0,     0,     0,   838,     0,
     111,     0,     0,     0,     0,     0,     0,  1525,   472,   473,
    1493,   474,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,     0,   497,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   498,     0,     0,
     210,     0,   894,     0,   894,     0,   894,   469,   470,   471,
       0,   894,   250,     0,     0,   894,     0,   894,   527,   528,
     894,     0,    50,     0,     0,  1679,     0,   472,   473,     0,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,     0,   497,     0,     0,     0,     0,   214,
     215,   216,   217,   218,     0,     0,   498,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   187,     0,     0,    91,    92,     0,    93,    94,     0,
      95,   188,    97,     0,     0,     0,     0,     0,     0,     0,
     250,     0,     0,     0,     0,     0,     0,  1624,     0,  1625,
       0,  1626,   469,   470,   471,   107,  1627,   894,     0,     0,
    1629,     0,  1630,     0,     0,  1631,     0,  1494,     0,     0,
       0,     0,   472,   473,     0,   474,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,     0,   497,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   498,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     5,     6,
       7,     8,     9,     0,     0,     0,   499,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     894,   894,   894,     0,     0,     0,     0,   894,     0,     0,
       0,     0,  1715,   713,     0,     0,  1872,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,   581,     0,     0,     0,   183,   184,   185,    65,    66,
      67,     0,     0,    69,    70,  1860,  1861,  1862,     0,     0,
       0,     0,  1866,   186,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   187,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   188,    97,     0,
     714,     0,    99,     0,     0,   100,     0,     0,   894,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,   894,
       0,   107,   189,     0,     0,   894,     0,   111,   112,   894,
     113,   114,     0,     0,     0,     0,     0,     0,     0,     0,
     469,   470,   471,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1965,     0,
     472,   473,     0,   474,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,     0,   497,     0,     0,
       0,     0,     0,     0,     5,     6,     7,     8,     9,   498,
       0,   894,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,  1927,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,  1937,     0,     0,     0,     0,     0,
    1942,     0,     0,     0,  1944,     0,     0,     0,     0,    14,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    47,     0,    48,
       0,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,    56,    57,    58,  1987,    59,  -203,    60,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,   818,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,    88,    89,    90,    91,    92,     0,    93,
      94,     0,    95,    96,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,   102,     0,
     103,     0,   104,   105,   106,     0,     0,   107,   108,     0,
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
       0,     0,     0,    52,    53,    54,    55,    56,    57,    58,
       0,    59,     0,    60,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,    88,    89,    90,
      91,    92,     0,    93,    94,     0,    95,    96,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,   102,     0,   103,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,  1204,   111,   112,     0,
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
      55,    56,    57,    58,     0,    59,     0,    60,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,    88,    89,    90,    91,    92,     0,    93,    94,     0,
      95,    96,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,   102,     0,   103,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
    1407,   111,   112,     0,   113,   114,     5,     6,     7,     8,
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
      87,     0,     0,     0,     0,   187,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   188,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     108,     0,   109,   110,   698,   111,   112,     0,   113,   114,
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
       0,    86,     0,     0,    87,     0,     0,     0,     0,   187,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   188,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   108,     0,   109,   110,  1013,   111,
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
      53,    54,    55,     0,    57,    58,     0,    59,  -203,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   187,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   188,    97,    98,     0,     0,    99,     0,
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
       0,     0,    87,     0,     0,     0,     0,   187,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   188,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,  1170,   111,   112,     0,
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
       0,   187,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   188,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
    1218,   111,   112,     0,   113,   114,     5,     6,     7,     8,
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
      87,     0,     0,     0,     0,   187,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   188,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     108,     0,   109,   110,  1246,   111,   112,     0,   113,   114,
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
       0,    86,     0,     0,    87,     0,     0,     0,     0,   187,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   188,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   108,     0,   109,   110,  1304,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
       0,     0,    43,    44,    45,    46,  1306,    47,     0,    48,
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
       0,    47,     0,    48,     0,    49,  1489,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   187,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   188,    97,    98,
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
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   187,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   188,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
    1633,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
    -295,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,     0,    61,    62,    63,    64,    65,    66,    67,     0,
      68,    69,    70,     0,    72,    73,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   187,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   188,    97,    98,     0,     0,
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
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,     0,    61,    62,    63,    64,
      65,    66,    67,     0,    68,    69,    70,     0,    72,    73,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,   187,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   188,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   108,     0,   109,   110,  1863,   111,
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
    1914,    49,     0,     0,    50,    51,     0,     0,     0,    52,
      53,    54,    55,     0,    57,    58,     0,    59,     0,     0,
      61,    62,    63,    64,    65,    66,    67,     0,    68,    69,
      70,     0,    72,    73,     0,     0,     0,     0,     0,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,     0,     0,     0,    86,     0,     0,    87,     0,
       0,     0,     0,   187,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   188,    97,    98,     0,     0,    99,     0,
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
       0,    47,  1957,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,     0,    61,    62,    63,    64,    65,    66,
      67,     0,    68,    69,    70,     0,    72,    73,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   187,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   188,    97,    98,
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
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
      63,    64,    65,    66,    67,     0,    68,    69,    70,     0,
      72,    73,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   187,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   188,    97,    98,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
    1974,   111,   112,     0,   113,   114,     5,     6,     7,     8,
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
      87,     0,     0,     0,     0,   187,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   188,    97,    98,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     108,     0,   109,   110,  1977,   111,   112,     0,   113,   114,
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
       0,    86,     0,     0,    87,     0,     0,     0,     0,   187,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   188,
      97,    98,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   108,     0,   109,   110,  1994,   111,
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
       0,     0,     0,   187,    89,    90,    91,    92,     0,    93,
      94,     0,    95,   188,    97,    98,     0,     0,    99,     0,
       0,   100,     0,     0,     0,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   108,     0,
     109,   110,  2043,   111,   112,     0,   113,   114,     5,     6,
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
       0,     0,    87,     0,     0,     0,     0,   187,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   188,    97,    98,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,  2044,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,   567,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
     184,   185,    65,    66,    67,     0,    68,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   187,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   188,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,   851,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,    33,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,    41,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    47,
       0,    48,     0,    49,     0,     0,    50,    51,     0,     0,
       0,    52,    53,    54,    55,     0,    57,    58,     0,    59,
       0,     0,    61,    62,   184,   185,    65,    66,    67,     0,
      68,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,    86,     0,     0,
      87,     0,     0,     0,     0,   187,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   188,    97,     0,     0,     0,
      99,     0,     0,   100,     0,     0,     0,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     108,     0,   109,   110,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,  1103,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,    33,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,    41,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    47,     0,    48,     0,    49,     0,     0,
      50,    51,     0,     0,     0,    52,    53,    54,    55,     0,
      57,    58,     0,    59,     0,     0,    61,    62,   184,   185,
      65,    66,    67,     0,    68,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,    86,     0,     0,    87,     0,     0,     0,     0,   187,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   188,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
       0,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   108,     0,   109,   110,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,  1709,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,    33,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,    41,    42,     0,
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
     109,   110,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,  1855,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,    33,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,    41,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    47,     0,    48,     0,    49,     0,     0,    50,    51,
       0,     0,     0,    52,    53,    54,    55,     0,    57,    58,
       0,    59,     0,     0,    61,    62,   184,   185,    65,    66,
      67,     0,    68,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,    86,
       0,     0,    87,     0,     0,     0,     0,   187,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   188,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   108,     0,   109,   110,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,     0,    48,     0,    49,
       0,     0,    50,    51,     0,     0,     0,    52,    53,    54,
      55,     0,    57,    58,     0,    59,     0,     0,    61,    62,
     184,   185,    65,    66,    67,     0,    68,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,    86,     0,     0,    87,     0,     0,     0,
       0,   187,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   188,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   108,     0,   109,   110,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     355,   420,    13,     0,     0,     0,     0,     0,     0,     0,
       0,   787,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
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
     108,     0,     0,     0,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   355,     0,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
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
     106,     0,     0,   107,   189,     0,   356,     0,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   355,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
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
       0,   846,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,  1144,     0,    10,  1145,
    1146,  1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,
    1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,
    1166,     0,     0,  1234,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1167,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,     0,   183,   184,   185,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   186,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   187,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   188,    97,     0,
    1235,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   189,     0,     0,     0,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   355,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   787,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,   183,
     184,   185,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   186,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   187,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   188,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   189,     0,     0,     0,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     355,   420,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
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
     108,   469,   470,   471,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   472,   473,     0,   474,   475,   476,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,     0,   497,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
     498,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,   201,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,     0,   183,   184,   185,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   186,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   187,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   188,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
     583,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   189,     0,     0,     0,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,  1145,  1146,  1147,  1148,  1149,
    1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,
    1160,  1161,  1162,  1163,  1164,  1165,  1166,     0,     0,   237,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1167,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
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
       0,     0,   104,   105,   106,     0,     0,   107,   189,   469,
     470,   471,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   472,
     473,     0,   474,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,     0,   497,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,   498,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,     0,   183,   184,   185,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   186,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   187,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   188,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,   602,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   189,     0,   272,   470,   471,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,   472,   473,     0,   474,   475,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
       0,   497,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,     0,   498,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,   183,
     184,   185,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   186,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   187,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   188,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   189,     0,   275,     0,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   420,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
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
     108,   469,   470,   471,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   472,   473,     0,   474,   475,   476,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,     0,   497,     0,
       0,     0,     0,     0,     0,     0,     0,    15,    16,     0,
     498,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,     0,   183,   184,   185,
      65,    66,    67,     0,     0,    69,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   186,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    87,     0,     0,     0,     0,   187,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   188,
      97,     0,     0,     0,    99,     0,     0,   100,     0,     0,
     606,     0,     0,   101,     0,     0,     0,     0,   104,   105,
     106,     0,     0,   107,   189,   565,     0,     0,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   355,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
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
       0,     0,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,  1149,
    1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,
    1160,  1161,  1162,  1163,  1164,  1165,  1166,   743,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1167,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,     0,   183,   184,   185,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   186,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   187,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   188,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,   189,     0,     0,     0,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,  1146,  1147,  1148,  1149,  1150,  1151,  1152,
    1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,
    1163,  1164,  1165,  1166,     0,     0,     0,   787,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1167,     0,    15,
      16,     0,     0,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,   183,
     184,   185,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   186,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   187,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   188,    97,     0,     0,     0,    99,     0,     0,   100,
       0,     0,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   189,     0,     0,     0,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,     0,   497,     0,
       0,   826,     0,     0,     0,     0,     0,     0,     0,     0,
     498,     0,     0,    15,    16,     0,     0,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,    38,    39,    40,     0,     0,
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
     189,     0,     0,     0,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,  1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,
    1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,
    1166,     0,     0,     0,     0,   828,     0,     0,     0,     0,
       0,     0,     0,     0,  1167,     0,     0,    15,    16,     0,
       0,     0,     0,    17,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,    29,    30,    31,
      32,     0,     0,     0,     0,    34,    35,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    43,     0,
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
     106,     0,     0,   107,   189,     0,     0,     0,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,     0,   497,     0,     0,     0,  1295,
       0,     0,     0,     0,     0,     0,     0,   498,     0,     0,
       0,    15,    16,     0,     0,     0,     0,    17,     0,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,    30,    31,    32,     0,     0,     0,     0,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
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
       0,     0,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   355,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,     0,   183,   184,   185,    65,    66,
      67,     0,     0,    69,    70,     0,     0,     0,     0,     0,
       0,     0,     0,   186,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,    87,     0,     0,     0,     0,   187,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   188,    97,     0,
       0,     0,    99,     0,     0,   100,     0,     0,     0,     0,
       0,   101,     0,     0,     0,     0,   104,   105,   106,     0,
       0,   107,  1421,   469,   470,   471,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   472,   473,     0,   474,   475,   476,   477,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,     0,
     497,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,   498,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,   183,
     184,   185,    65,    66,    67,     0,     0,    69,    70,     0,
       0,     0,     0,     0,     0,     0,     0,   186,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,    87,     0,     0,     0,
       0,   187,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   188,    97,     0,     0,     0,    99,     0,     0,   100,
       0,   843,     0,     0,     0,   101,     0,     0,     0,     0,
     104,   105,   106,     0,     0,   107,   189,   469,   470,   471,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,   855,     0,    10,   472,   473,     0,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,     0,   497,     0,     0,     0,     0,     0,
       0,     0,     0,    15,    16,     0,   498,     0,     0,    17,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,    29,    30,    31,    32,     0,     0,     0,
       0,    34,    35,    36,    37,   659,    39,    40,     0,   856,
       0,     0,     0,     0,    43,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,     0,   183,   184,   185,    65,    66,    67,     0,
       0,    69,    70,     0,     0,     0,     0,     0,     0,     0,
       0,   186,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,   187,    89,    90,    91,    92,
       0,    93,    94,     0,    95,   188,    97,     0,   277,   278,
      99,   279,   280,   100,     0,   281,   282,   283,   284,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     189,     0,     0,   285,   286,   111,   112,     0,   113,   114,
    1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,
    1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,     0,
       0,     0,   288,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1167,     0,     0,     0,   290,   291,   292,   293,
     294,   295,   296,     0,     0,     0,   210,     0,   211,    40,
       0,     0,     0,     0,     0,     0,     0,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,    50,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   210,   332,     0,   775,   334,   335,   336,
       0,     0,     0,   337,   595,   214,   215,   216,   217,   218,
     596,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,   277,   278,     0,   279,   280,     0,   597,   281,   282,
     283,   284,     0,    93,    94,     0,    95,   188,    97,   342,
       0,   343,     0,     0,   344,     0,   285,   286,     0,     0,
       0,     0,   214,   215,   216,   217,   218,     0,     0,     0,
       0,   107,     0,     0,     0,   776,     0,   111,     0,     0,
       0,     0,     0,     0,     0,   288,     0,   366,     0,     0,
      93,    94,     0,    95,   188,    97,     0,     0,     0,   290,
     291,   292,   293,   294,   295,   296,     0,     0,     0,   210,
       0,   211,    40,     0,     0,     0,     0,     0,   107,     0,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,    50,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   210,   332,     0,   333,
     334,   335,   336,     0,     0,     0,   337,   595,   214,   215,
     216,   217,   218,   596,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,   277,   278,     0,   279,   280,     0,
     597,   281,   282,   283,   284,     0,    93,    94,     0,    95,
     188,    97,   342,     0,   343,     0,     0,   344,     0,   285,
     286,     0,   287,     0,     0,   214,   215,   216,   217,   218,
       0,     0,     0,     0,   107,     0,     0,     0,   776,     0,
     111,     0,     0,     0,     0,     0,     0,     0,   288,     0,
     915,     0,     0,    93,    94,     0,    95,   188,    97,   289,
       0,     0,   290,   291,   292,   293,   294,   295,   296,     0,
       0,     0,   210,     0,     0,     0,     0,     0,     0,     0,
       0,   107,     0,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,    50,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,     0,
     332,     0,     0,   334,   335,   336,     0,     0,     0,   337,
     338,   214,   215,   216,   217,   218,   339,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   340,     0,     0,    91,   341,     0,    93,
      94,     0,    95,   188,    97,   342,     0,   343,     0,     0,
     344,   277,   278,     0,   279,   280,     0,   345,   281,   282,
     283,   284,     0,     0,     0,     0,     0,   107,   346,     0,
       0,     0,  1835,     0,     0,     0,   285,   286,     0,   287,
     473,     0,   474,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   288,   497,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   289,     0,   498,   290,
     291,   292,   293,   294,   295,   296,     0,     0,     0,   210,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,    50,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,     0,   332,     0,     0,
     334,   335,   336,     0,     0,     0,   337,   338,   214,   215,
     216,   217,   218,   339,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     340,     0,     0,    91,   341,     0,    93,    94,     0,    95,
     188,    97,   342,     0,   343,     0,     0,   344,   277,   278,
       0,   279,   280,     0,   345,   281,   282,   283,   284,     0,
       0,     0,     0,     0,   107,   346,     0,     0,     0,  1909,
       0,     0,     0,   285,   286,     0,   287,     0,     0,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   288,   497,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   289,     0,   498,   290,   291,   292,   293,
     294,   295,   296,     0,     0,     0,   210,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,    50,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,     0,   332,     0,   333,   334,   335,   336,
       0,     0,     0,   337,   338,   214,   215,   216,   217,   218,
     339,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   340,     0,     0,
      91,   341,     0,    93,    94,     0,    95,   188,    97,   342,
       0,   343,     0,     0,   344,   277,   278,     0,   279,   280,
       0,   345,   281,   282,   283,   284,     0,     0,     0,     0,
       0,   107,   346,     0,     0,     0,     0,     0,     0,     0,
     285,   286,     0,   287,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,     0,   497,     0,     0,     0,     0,     0,     0,   288,
       0,     0,     0,     0,   498,     0,     0,     0,     0,     0,
     289,     0,     0,   290,   291,   292,   293,   294,   295,   296,
       0,     0,     0,   210,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,    50,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
       0,   332,     0,     0,   334,   335,   336,     0,     0,     0,
     337,   338,   214,   215,   216,   217,   218,   339,     0,     0,
       0,     0,     0,     0,     0,   210,     0,     0,     0,     0,
       0,     0,     0,     0,   340,     0,     0,    91,   341,     0,
      93,    94,     0,    95,   188,    97,   342,    50,   343,     0,
       0,   344,     0,   277,   278,     0,   279,   280,   345,  1637,
     281,   282,   283,   284,     0,     0,     0,     0,   107,   346,
       0,     0,     0,     0,     0,     0,     0,     0,   285,   286,
       0,   287,     0,     0,   214,   215,   216,   217,   218,     0,
   -1089, -1089, -1089, -1089,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   288,   497,     0,
       0,     0,    93,    94,     0,    95,   188,    97,   289,     0,
     498,   290,   291,   292,   293,   294,   295,   296,     0,     0,
       0,   210,     0,     0,     0,     0,     0,     0,     0,     0,
     107,     0,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,    50,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,     0,   332,
       0,     0,   334,   335,   336,     0,     0,     0,   337,   338,
     214,   215,   216,   217,   218,   339,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   340,     0,     0,    91,   341,     0,    93,    94,
       0,    95,   188,    97,   342,     0,   343,     0,     0,   344,
    1735,  1736,  1737,  1738,  1739,     0,   345,  1740,  1741,  1742,
    1743,     0,     0,     0,     0,     0,   107,   346,     0,     0,
       0,     0,     0,     0,  1744,  1745,  1746,     0,   472,   473,
       0,   474,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,  1747,   497,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   498,  1748,  1749,
    1750,  1751,  1752,  1753,  1754,     0,     0,     0,   210,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1755,
    1756,  1757,  1758,  1759,  1760,  1761,  1762,  1763,  1764,  1765,
      50,  1766,  1767,  1768,  1769,  1770,  1771,  1772,  1773,  1774,
    1775,  1776,  1777,  1778,     0,  1779,  1780,  1781,  1782,  1783,
    1784,  1785,  1786,  1787,  1788,  1789,  1790,  1791,  1792,  1793,
    1794,  1795,     0,     0,     0,  1796,  1797,   214,   215,   216,
     217,   218,     0,  1798,  1799,  1800,  1801,  1802,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1803,
    1804,  1805,     0,     0,     0,    93,    94,     0,    95,   188,
      97,  1806,     0,  1807,  1808,     0,  1809,     0,     0,     0,
       0,     0,     0,  1810,  1811,     0,  1812,     0,  1813,  1814,
       0,   277,   278,   107,   279,   280,  1143,     0,   281,   282,
     283,   284,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1144,   285,   286,  1145,  1146,
    1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,
    1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
       0,     0,     0,     0,     0,   288,     0,     0,     0,     0,
       0,     0,     0,  1167,     0,     0,     0,     0,     0,   290,
     291,   292,   293,   294,   295,   296,     0,     0,     0,   210,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,    50,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,     0,   332,     0,   333,
     334,   335,   336,     0,     0,     0,   337,   595,   214,   215,
     216,   217,   218,   596,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   277,   278,     0,   279,   280,     0,
     597,   281,   282,   283,   284,     0,    93,    94,     0,    95,
     188,    97,   342,     0,   343,     0,     0,   344,     0,   285,
     286,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   107,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   288,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   290,   291,   292,   293,   294,   295,   296,     0,
       0,     0,   210,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,    50,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,     0,
     332,     0,  1350,   334,   335,   336,     0,     0,     0,   337,
     595,   214,   215,   216,   217,   218,   596,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   277,   278,     0,
     279,   280,     0,   597,   281,   282,   283,   284,     0,    93,
      94,     0,    95,   188,    97,   342,     0,   343,     0,     0,
     344,     0,   285,   286,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   107,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   288,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   290,   291,   292,   293,   294,
     295,   296,     0,     0,     0,   210,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,    50,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   287,   332,     0,     0,   334,   335,   336,     0,
       0,     0,   337,   595,   214,   215,   216,   217,   218,   596,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   597,     0,     0,   289,
       0,     0,    93,    94,     0,    95,   188,    97,   342,     0,
     343,   287,   210,   344,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     107,     0,     0,     0,    50,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   289,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   210,     0,     0,     0,     0,     0,     0,     0,     0,
     584,   214,   215,   216,   217,   218,   585,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   187,     0,     0,    91,   341,     0,    93,
      94,     0,    95,   188,    97,     0,  1501,     0,     0,     0,
       0,     0,  1358,     0,     0,     0,     0,   345,     0,   584,
     214,   215,   216,   217,   218,   585,     0,   107,   346,     0,
     873,   874,     0,     0,     0,     0,   875,     0,   876,     0,
       0,     0,   187,     0,     0,    91,   341,     0,    93,    94,
     877,    95,   188,    97,     0,     0,     0,     0,    34,    35,
      36,   210,     0,     0,     0,     0,   345,     0,     0,     0,
       0,   212,  1141,  1142,  1143,     0,   107,   346,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1144,     0,     0,  1145,  1146,  1147,  1148,
    1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,
    1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,     0,   878,
     879,   880,   881,   882,   883,     0,    81,    82,    83,    84,
      85,  1167,     0,     0,     0,     0,     0,   219,  1097,     0,
       0,     0,   187,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   188,    97,     0,     0,     0,    99,     0,     0,
       0,     0,     0,     0,     0,     0,   884,     0,     0,     0,
      29,   104,     0,     0,     0,     0,   107,   885,    34,    35,
      36,   210,     0,   211,    40,     0,     0,     0,     0,     0,
       0,   212,     0,     0,     0,     0,     0,     0,     0,  1332,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     213,  1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,
    1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1098,    75,
     214,   215,   216,   217,   218,     0,    81,    82,    83,    84,
      85,  1167,     0,     0,     0,     0,     0,   219,     0,     0,
       0,     0,   187,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   188,    97,     0,   873,   874,    99,     0,     0,
       0,   875,     0,   876,     0,     0,     0,     0,     0,     0,
       0,   104,     0,     0,     0,   877,   107,   220,     0,     0,
       0,     0,   111,    34,    35,    36,   210,     0,     0,     0,
       0,   469,   470,   471,     0,     0,   212,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,   472,   473,     0,   474,   475,   476,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,     0,   497,     0,
       0,     0,     0,     0,   878,   879,   880,   881,   882,   883,
     498,    81,    82,    83,    84,    85,     0,     0,     0,  1051,
    1052,     0,   219,     0,     0,     0,     0,   187,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   188,    97,  1053,
       0,     0,    99,     0,     0,     0,     0,  1054,  1055,  1056,
     210,   884,     0,     0,     0,     0,   104,     0,     0,     0,
    1057,   107,   885,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,    29,     0,     0,     0,   542,     0,
       0,     0,    34,    35,    36,   210,     0,   211,    40,     0,
       0,     0,     0,     0,     0,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,  1058,  1059,
    1060,  1061,  1062,  1063,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   213,     0,  1064,     0,     0,     0,
       0,   187,     0,     0,    91,    92,     0,    93,    94,     0,
      95,   188,    97,    75,   214,   215,   216,   217,   218,     0,
      81,    82,    83,    84,    85,  1065,     0,     0,     0,     0,
       0,   219,     0,     0,     0,   107,   187,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   188,    97,    29,     0,
       0,    99,     0,     0,     0,     0,    34,    35,    36,   210,
       0,   211,    40,     0,     0,   104,     0,     0,     0,   212,
     107,   220,     0,     0,   622,     0,   111,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   213, -1089,
   -1089, -1089, -1089,  1154,  1155,  1156,  1157,  1158,  1159,  1160,
    1161,  1162,  1163,  1164,  1165,  1166,   642,    75,   214,   215,
     216,   217,   218,     0,    81,    82,    83,    84,    85,  1167,
       0,     0,     0,     0,     0,   219,     0,     0,     0,     0,
     187,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     188,    97,    29,  1042,     0,    99,     0,     0,     0,     0,
      34,    35,    36,   210,     0,   211,    40,     0,     0,   104,
       0,     0,     0,   212,   107,   220,     0,     0,     0,     0,
     111,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   213,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    75,   214,   215,   216,   217,   218,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,   219,
       0,     0,     0,     0,   187,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   188,    97,    29,     0,     0,    99,
       0,     0,     0,     0,    34,    35,    36,   210,     0,   211,
      40,     0,     0,   104,     0,     0,     0,   212,   107,   220,
       0,     0,     0,     0,   111,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   213,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1197,    75,   214,   215,   216,   217,
     218,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,   219,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
      29,     0,     0,    99,     0,     0,     0,     0,    34,    35,
      36,   210,     0,   211,    40,     0,     0,   104,     0,     0,
       0,   212,   107,   220,     0,     0,     0,     0,   111,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     213,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
     214,   215,   216,   217,   218,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,   219,     0,     0,
       0,     0,   187,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   188,    97,     0,     0,     0,    99,     0,     0,
     469,   470,   471,     0,     0,     0,     0,     0,     0,     0,
       0,   104,     0,     0,     0,     0,   107,   220,     0,     0,
     472,   473,   111,   474,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,     0,   497,   469,   470,
     471,     0,     0,     0,     0,     0,     0,     0,     0,   498,
       0,     0,     0,     0,     0,     0,     0,     0,   472,   473,
       0,   474,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,     0,   497,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   498,   469,   470,
     471,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   551,   472,   473,
       0,   474,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,     0,   497,   469,   470,   471,     0,
       0,     0,     0,     0,     0,     0,     0,   498,     0,     0,
       0,     0,     0,     0,     0,   947,   472,   473,     0,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,     0,   497,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   498,   469,   470,   471,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1028,   472,   473,     0,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,     0,   497,   469,   470,   471,     0,     0,     0,
       0,     0,     0,     0,     0,   498,     0,     0,     0,     0,
       0,     0,     0,  1082,   472,   473,     0,   474,   475,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
       0,   497,     0,     0,     0,     0,     0,     0,     0,     0,
    1141,  1142,  1143,   498,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1144,     0,  1405,  1145,  1146,  1147,  1148,  1149,  1150,
    1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,
    1161,  1162,  1163,  1164,  1165,  1166,     0,     0,  1141,  1142,
    1143,     0,     0,     0,     0,     0,     0,     0,     0,  1167,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1144,
       0,  1442,  1145,  1146,  1147,  1148,  1149,  1150,  1151,  1152,
    1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,
    1163,  1164,  1165,  1166,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1141,  1142,  1143,  1167,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1144,     0,  1510,  1145,  1146,
    1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,
    1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
       0,     0,  1141,  1142,  1143,     0,     0,     0,     0,     0,
       0,     0,     0,  1167,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1144,     0,  1521,  1145,  1146,  1147,  1148,
    1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,
    1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1141,  1142,
    1143,  1167,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1144,
       0,  1623,  1145,  1146,  1147,  1148,  1149,  1150,  1151,  1152,
    1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,
    1163,  1164,  1165,  1166,     0,    34,    35,    36,   210,     0,
     211,    40,     0,     0,     0,     0,     0,  1167,   212,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1716,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   241,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   242,     0,
       0,     0,     0,     0,     0,     0,     0,   214,   215,   216,
     217,   218,     0,    81,    82,    83,    84,    85,     0,     0,
       0,     0,     0,     0,   219,  1718,     0,     0,     0,   187,
      89,    90,    91,    92,     0,    93,    94,     0,    95,   188,
      97,     0,     0,     0,    99,     0,    34,    35,    36,   210,
       0,   211,    40,     0,     0,     0,     0,     0,   104,   673,
       0,     0,     0,   107,   243,     0,     0,     0,     0,   111,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   213,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   214,   215,
     216,   217,   218,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   219,     0,     0,     0,     0,
     187,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     188,    97,     0,     0,     0,    99,     0,    34,    35,    36,
     210,     0,   211,    40,     0,     0,     0,     0,     0,   104,
     212,     0,     0,     0,   107,   674,     0,     0,     0,     0,
     111,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   241,
       0,     0,   210,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   214,
     215,   216,   217,   218,    50,    81,    82,    83,    84,    85,
       0,     0,   363,   364,     0,     0,   219,     0,     0,     0,
       0,   187,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   188,    97,     0,     0,     0,    99,     0,     0,     0,
       0,   214,   215,   216,   217,   218,     0,     0,     0,     0,
     104,     0,     0,     0,     0,   107,   243,     0,     0,     0,
       0,   111,     0,   365,     0,     0,   366,     0,     0,    93,
      94,     0,    95,   188,    97,     0,     0,     0,   469,   470,
     471,     0,     0,     0,     0,     0,     0,     0,   367,     0,
       0,     0,     0,     0,     0,     0,     0,   107,   472,   473,
    1025,   474,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,     0,   497,   469,   470,   471,     0,
       0,     0,     0,     0,     0,     0,     0,   498,     0,     0,
       0,     0,     0,     0,     0,     0,   472,   473,     0,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,     0,   497,  1141,  1142,  1143,     0,     0,     0,
       0,     0,     0,     0,     0,   498,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1144,  1526,     0,  1145,  1146,
    1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,
    1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
    1141,  1142,  1143,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1167,     0,     0,     0,     0,     0,     0,
       0,  1144,     0,     0,  1145,  1146,  1147,  1148,  1149,  1150,
    1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,
    1161,  1162,  1163,  1164,  1165,  1166,  1142,  1143,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1167,
       0,     0,     0,     0,     0,     0,  1144,     0,     0,  1145,
    1146,  1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,
    1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,
    1166,   471,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1167,     0,     0,     0,     0,   472,
     473,     0,   474,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,     0,   497,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   498
};

static const yytype_int16 yycheck[] =
{
       5,     6,   131,     8,     9,    10,    11,    12,    13,    56,
      15,    16,    17,    18,     4,   191,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     4,    31,   690,   165,   413,
     123,     4,   170,   171,   108,    30,     4,   567,   123,    44,
     123,     4,   727,     4,    33,   503,   108,    52,   108,    54,
     443,   686,    57,   716,    59,   536,   108,    46,    30,    30,
     986,   846,    51,   192,    30,   244,    57,   497,  1224,   988,
     555,   854,  1208,   603,   616,   617,   532,   533,   784,   165,
    1397,    86,   687,     4,   413,   413,   861,   977,  1019,   529,
      44,  1004,   666,  1006,  1096,   830,     9,   529,     9,    32,
       9,   413,    32,   108,     9,   561,  1037,   255,     9,    14,
       9,    48,     9,    14,     9,   189,     9,    14,   256,    48,
    1848,    48,    48,   563,   363,   364,   365,   189,   367,   189,
       9,   563,    86,     9,     9,    70,     9,     9,     9,    70,
       9,   240,   445,   446,   447,     9,   220,     9,    70,    98,
      57,     9,  1083,   102,   103,     9,     9,     9,   220,     9,
     220,    38,     9,   131,     9,     9,  1092,     9,    48,   243,
     102,    38,     0,    54,    90,   161,   560,    38,    36,   161,
      83,   130,    38,   243,   189,    70,    70,    83,   182,    83,
      90,   196,   167,     4,    14,    38,   106,   107,   106,   107,
     182,    38,   182,   197,   122,   111,    83,    70,    19,    20,
      50,    51,    32,   131,   200,   220,    83,   197,   200,    70,
     161,   197,    83,   161,   192,   200,     8,    83,    83,    84,
    1908,    51,  1910,   165,    70,  1539,    70,    70,   243,   161,
      83,   182,   158,    70,   165,    70,    83,  1137,    70,    70,
     398,   135,   136,   258,   917,    70,   261,   194,   158,   197,
      83,   198,   197,   268,   269,   194,   197,   200,    70,   199,
      70,   198,   198,    70,    70,   197,   161,   182,   200,   175,
     456,   175,    70,   262,  2012,   166,   199,   266,   199,   200,
     199,   201,    70,   201,   199,  2023,    83,   108,   199,   202,
     199,   986,   199,  1439,   199,  1040,   199,   354,  1221,  1311,
    1446,   385,  1448,    70,   175,   200,   200,  1634,   198,   175,
     199,   851,   198,   385,   199,   385,   199,   199,   199,  1014,
     199,  1106,   175,  1108,  1265,   199,   161,   199,   175,  1643,
     198,  1477,    83,   183,   198,   198,   198,   202,   198,   200,
     355,   198,   175,   198,   198,    83,   198,   135,   136,   161,
    1217,   197,   538,  1667,   200,  1669,   200,   200,    70,   443,
     197,   913,   914,   200,    70,   200,   197,   198,   189,   384,
     385,   443,   511,   443,    70,   200,   391,   392,   393,   394,
     395,   396,   966,    70,   441,   197,   401,   197,   200,  1289,
     200,   197,   182,   200,   200,   197,   193,  1092,    81,   220,
     200,  1016,   200,   200,   197,   420,   394,   197,   396,    70,
      19,    20,   200,   428,    19,    20,   237,    54,   115,   419,
     384,   102,   243,   161,   175,   440,   102,   121,   443,   393,
     394,   197,   396,   200,    14,   197,   545,   131,   182,   135,
     136,   262,  1588,   458,   459,   266,    70,   102,   131,   427,
     512,   197,   943,   197,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   179,   498,   165,   500,   501,   502,   200,   165,
    1206,   450,   497,    70,   200,   197,   427,   512,   513,   514,
     515,   516,   517,   518,   519,   520,   521,   522,   523,   524,
     165,   512,   980,   200,  1307,   497,   497,   532,   533,   534,
     535,   497,   102,   103,   539,     4,   443,   542,    83,   166,
     182,  1134,   510,  1136,   197,   693,   551,   695,   553,   648,
     649,    19,    20,   993,   181,   197,   561,   506,   507,   508,
     509,   993,   197,   374,   569,   560,   571,   704,   135,   136,
      83,    84,   383,  1103,   385,    83,   197,   676,  1034,   390,
    1437,   197,    90,    83,    53,   161,  1903,    56,  1223,   165,
      90,   402,   197,   977,   161,   574,  1284,   503,  1286,   778,
     674,   165,    83,    31,    73,   512,   182,   135,   136,    90,
     786,  1004,  1042,  1006,   159,   160,   427,   622,   704,    83,
     135,   136,    50,   529,    83,    53,    90,    96,  1518,    98,
     197,   810,   443,   102,   103,   199,   200,   167,   237,    70,
    1182,    70,   237,   549,  1249,   135,   136,  1252,   977,   977,
     180,   159,   160,    70,   560,   123,    83,   563,   158,   159,
     160,   130,   569,    90,   903,   977,   905,    83,   907,   674,
     200,   166,   911,   201,    90,   774,   824,   825,   159,   160,
     197,  1139,   197,   831,   832,    83,    32,   200,   165,   202,
     197,    57,    90,   831,   158,   159,   160,   687,   206,   158,
     159,   160,  1005,    69,     6,  1008,  1009,   886,   197,   714,
     117,   500,   501,   161,    50,    51,  1606,   111,    31,   898,
    1577,    38,  1579,   199,   205,   536,   120,   121,   122,   123,
     124,   125,   159,   160,   182,    70,   205,    50,   743,   199,
      53,  1447,   199,   159,   160,   534,    48,    75,    76,   197,
      60,    70,   200,  1137,    75,    76,    53,    54,    55,   727,
      57,   159,   160,   574,   863,   105,   106,   107,   200,   237,
     869,   776,    69,   102,   103,   374,   133,   134,    88,   374,
     199,    91,  1267,   704,   383,  1420,   735,   199,   383,   199,
     200,   390,   161,   262,   199,   390,   199,   266,   192,   199,
     805,   270,  1258,   402,   199,   200,   727,   402,  1137,  1137,
     112,  1879,  1880,  1269,   413,    70,   118,    70,   120,   121,
     122,   123,   124,   125,   126,  1137,  1875,  1876,  1221,  1310,
      70,  1688,    70,   838,    70,  1692,  1441,   399,   871,   872,
     939,   403,   120,   121,   122,   123,   124,   125,  1554,   854,
    2006,    70,   363,   364,   365,  1385,   367,   161,    53,    54,
      55,   850,   165,  2019,   197,   197,   168,   169,   430,   171,
     432,   433,   434,   435,    69,   197,   844,   105,   106,   107,
     859,   199,    49,  1568,    69,   354,   123,   124,   125,   182,
     192,   120,   121,   122,   123,   124,   125,   161,   197,   201,
       9,   197,   713,   204,   161,  1289,   374,   112,   113,   114,
     161,   197,     8,   197,   192,   383,   727,   385,   199,   161,
    1401,   199,   390,   844,    50,    51,    52,    53,    54,    55,
      14,    57,   900,   199,   402,   161,   199,   536,  1644,   200,
       9,   536,   947,    69,   949,   199,   951,    14,   131,  1479,
      50,    51,    52,    53,    54,    55,   131,   962,   427,   198,
    1289,  1289,   182,   192,    14,  1495,   102,   198,   437,    69,
     203,   976,   441,   198,   198,   965,   787,  1289,  1077,   900,
     198,   450,   197,   200,  1589,   197,   965,  1443,   111,   197,
    1847,     9,   965,   158,  1851,   198,  1001,   965,   988,  1004,
     198,  1006,   965,    94,   965,   198,  1011,   198,     9,   199,
      14,   182,     9,   197,   200,   826,   197,   828,   986,   199,
    1025,   200,  1948,  1028,  1123,  1030,  1016,  1953,  1421,  1034,
     199,  1130,   200,   844,   503,   504,   505,   506,   507,   508,
     509,   199,    83,   200,   965,   856,  1014,  1042,   859,   199,
    1969,  1230,  1978,   363,   364,   365,   366,   367,   198,   198,
     529,   198,   133,   197,   199,   986,   198,     9,   536,     9,
    1042,  1042,    70,  1992,   980,   204,  1042,  1082,    32,  1609,
     549,  1539,  2001,   204,   204,   820,   992,   993,  1618,   900,
     204,   204,  1041,  1014,   563,   405,   134,  1004,   181,  1006,
    1089,   161,  1632,   137,  1011,   574,     9,   161,   198,  2035,
      14,  1090,   194,     9,   713,  1900,   927,     9,   713,  1575,
    1905,  1424,  1425,  1426,  1092,   594,  1094,   183,  1221,     9,
     198,    14,   943,   944,  1518,     9,  1221,   198,  1221,   198,
     198,   198,   133,  2000,     9,    14,   198,  1199,   201,   204,
     198,   620,   621,   204,   965,   204,   161,   204,  2015,   198,
     197,   102,     9,   199,   199,   137,   161,     9,   198,  1954,
     197,  1092,    70,  1094,   197,   986,    70,    70,  1084,  1709,
    1279,    70,  1640,    70,   200,  1643,   655,   656,   787,  1518,
    1518,     9,   787,  1004,  1199,  1006,   183,    14,   201,   199,
       9,   200,    14,  1014,    14,   199,  1518,   200,   204,  1667,
     198,  1669,   820,   194,   197,  1205,  1221,  1675,    32,   197,
      14,    32,  1606,   197,   197,    14,  1205,   826,    52,   828,
    1235,   826,  1205,   828,   197,    70,  1899,  1205,  1901,  1338,
      70,  2026,  1205,  1342,  1205,   713,   197,    70,  1347,    70,
      70,   161,     9,  1258,   198,  1354,   199,   856,   727,  1249,
     199,   856,  1252,  1948,  1269,  1270,   735,     6,  1953,   197,
     137,    14,   183,   137,     9,   161,   198,  1606,  1606,  1090,
     204,  1092,    69,  1094,  1205,  1096,  1097,  1425,     9,   201,
      83,   199,  1199,  1978,  1606,   201,   201,     9,   197,   201,
     137,    14,  1307,    83,   199,   137,   616,   617,   198,    48,
    1299,   199,  1317,   197,  1221,   200,   197,     9,  1053,   787,
     197,   200,   198,  1302,    91,  1855,   204,  1990,   927,   200,
     158,    32,   927,    77,   200,   183,   199,   198,   137,   199,
     198,    32,   198,     9,   943,   944,  1314,  1421,   943,   944,
    2035,   820,   204,   822,   204,     9,   137,     9,   826,  1421,
     828,  1421,   198,  1462,   204,   204,   204,  1466,   201,     9,
     199,   198,    14,   112,  1473,   844,   201,   199,   977,   118,
     199,   120,   121,   122,   123,   124,   125,   126,   856,   858,
     859,  1849,   199,  1314,  1205,    81,   200,   199,    83,   197,
    1405,   198,   198,   198,   204,   197,     9,  1412,   199,  1399,
    1221,  1416,   200,  1418,   198,   198,  1421,   103,  1555,  1409,
     137,  1550,   204,  1234,     9,   111,   112,  1432,   204,   168,
     169,   900,   171,   204,   204,   137,   198,  1442,  1443,     9,
     909,   910,    32,   199,   198,  1053,   198,   137,   199,   199,
     112,  1441,   200,   192,   140,   141,   142,   143,   144,   927,
     170,   199,   201,   166,    14,    83,   198,   118,   198,   200,
     137,   940,   198,   137,    14,   943,   944,   182,  1399,   165,
     200,   199,   168,   169,  1295,   171,   172,   173,  1409,  1665,
      83,  1302,    14,    14,  2024,    83,   965,  1096,  1097,  1310,
    1311,  1096,  1097,  1314,   120,   121,   122,   123,   124,   125,
     196,   980,   137,   197,  1421,   131,   132,   986,   198,   196,
     198,    14,   137,   992,   993,   199,   199,    14,   199,    14,
     200,     9,     9,   201,    68,    83,   182,   197,  1137,    83,
       9,     9,   200,   199,   115,  1014,  1281,  1282,  1283,  1284,
    1285,  1286,   161,   102,   183,   102,  1291,    36,   174,   173,
    1528,    14,   197,  1568,   198,   197,   199,  1572,   179,    83,
    1575,   183,  1041,   183,   176,   198,   192,     9,    83,   199,
      14,  1549,  1051,  1052,  1053,   198,    83,    14,  1578,    83,
    1401,   198,   200,   903,  1584,   905,  1586,   907,    14,  1589,
    1568,   911,    83,   913,   914,   915,    14,  1528,    83,  1182,
    1421,  1981,   509,   504,  1035,  1084,   968,  1997,  1608,  1308,
    1708,  1090,  1492,  1092,   506,  1094,  1992,  1695,  1096,  1097,
    1733,  1645,   624,  1818,  1555,  1234,  1615,  2033,  2013,  1234,
    1830,  1545,  1610,  1541,  1135,  1691,  1115,  1568,   395,  1285,
    1209,  1619,  1131,  1052,  1280,  1079,  1281,  1578,  1834,  1893,
     871,   992,   391,  1584,   441,  1586,  1949,  1939,  1533,  1190,
    1139,    81,  1116,  1281,  1282,  1168,    -1,  1285,  1683,    -1,
      -1,    -1,    -1,  1291,    -1,    -1,    -1,  1608,  1637,  1610,
    1289,    -1,    -1,   103,    -1,    -1,  1295,    -1,  1619,  1168,
    1295,    -1,    -1,    -1,    -1,  1673,    -1,  1697,    -1,    -1,
      -1,  1310,  1311,    -1,    -1,  1310,  1311,  1528,    -1,    -1,
    1455,    -1,  1457,    -1,    -1,    -1,    -1,    -1,  1707,  1708,
     140,   141,   142,   143,   144,    -1,  1205,    -1,  1706,    -1,
      -1,    -1,    -1,    -1,  1712,    -1,    -1,    -1,    -1,    -1,
      -1,  1719,    -1,  1221,    -1,   165,    -1,  1568,   168,   169,
      -1,   171,   172,   173,    -1,     6,  1234,    -1,    -1,    -1,
    1869,  1870,    -1,    -1,    -1,    -1,  1697,    -1,    -1,    -1,
      -1,    -1,  1829,    -1,    -1,  1706,   196,    -1,    -1,    -1,
     200,  1712,    -1,    -1,    19,    20,    -1,    -1,  1719,  1610,
      -1,    -1,  1401,    -1,  1615,    30,  1401,    48,  1619,    -1,
      -1,    -1,  1281,  1282,  1283,  1284,  1285,  1286,    -1,    -1,
      -1,    -1,  1291,    -1,    -1,  1962,    -1,  1295,    -1,    -1,
      -1,    56,    -1,  1302,  1839,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1310,  1311,    81,  1314,  1893,  1455,    -1,  1457,
      -1,     6,    -1,    -1,    -1,  1324,  1985,    -1,    -1,    -1,
    1595,    -1,  1597,    -1,  1599,    -1,   103,    -1,    -1,  1604,
      -1,   112,  1182,    -1,    -1,    -1,    -1,   118,    -1,   120,
     121,   122,   123,   124,   125,   126,    -1,    -1,    -1,    -1,
      -1,  1859,    -1,    48,    -1,  1706,  1707,  1708,    -1,    -1,
      -1,  1712,    -1,   140,   141,   142,   143,   144,  1719,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1518,
      -1,    -1,    -1,    -1,  1393,    -1,    -1,   168,   169,   166,
     171,   168,   169,  1401,   171,   172,   173,    -1,  1859,    -1,
      -1,    -1,    -1,    31,    -1,  1913,    -1,    -1,    -1,    -1,
      -1,   192,    -1,    -1,    -1,    -1,    -1,   112,    -1,   196,
     201,    -1,    -1,   118,  1699,   120,   121,   122,   123,   124,
     125,   126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1969,
    1948,    -1,    -1,    -1,    -1,  1953,  1455,  1595,  1457,  1597,
      -1,  1599,  1913,    81,    -1,    -1,  1604,    -1,    -1,    -1,
      -1,    -1,  1992,    91,    -1,    -1,    -1,  1606,    -1,    -1,
    1978,  2001,   237,   168,   169,   103,   171,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  2031,  1948,    -1,    -1,
      -1,    -1,  1953,    -1,  2039,    -1,    -1,   192,    -1,    -1,
      -1,  1962,  2047,    -1,    -1,  2050,   201,    -1,  1859,    -1,
      -1,    -1,   140,   141,   142,   143,   144,  1978,    -1,  1528,
      -1,    -1,   287,    -1,   289,    -1,  2034,  2035,    -1,    -1,
    1539,    -1,    -1,    -1,   162,    -1,  1545,   165,    -1,    -1,
     168,   169,    -1,   171,   172,   173,    -1,   175,    -1,    -1,
      -1,  1699,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1568,
      -1,    -1,  1913,    -1,    -1,    -1,    -1,    -1,   196,    -1,
      -1,    -1,    -1,  2034,  2035,    -1,    -1,    -1,  1853,  1854,
      -1,   346,    -1,    -1,    -1,    -1,  1595,    -1,  1597,    -1,
    1599,    -1,    -1,    -1,    -1,  1604,    -1,  1948,    -1,    -1,
      -1,  1610,  1953,     6,    -1,    -1,  1615,    -1,    -1,   374,
    1619,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   383,     6,
      -1,    -1,    -1,    -1,    -1,   390,    -1,  1978,  1637,    -1,
      -1,  1640,    -1,    -1,  1643,    -1,    -1,   402,     6,    -1,
      -1,    -1,    -1,    -1,  1653,    48,    -1,    -1,   413,    -1,
      -1,  1660,    -1,    -1,    -1,    -1,    -1,    -1,  1667,    -1,
    1669,    48,    -1,    -1,    -1,    -1,  1675,    -1,    -1,    -1,
      -1,    -1,    -1,   438,    -1,    -1,   441,    -1,    -1,    -1,
      48,    -1,    -1,  2034,  2035,    -1,    -1,    -1,    -1,    -1,
    1699,    -1,    -1,    -1,    -1,    -1,    -1,  1706,  1707,  1708,
      -1,    -1,    -1,  1712,    -1,  1853,  1854,    -1,    -1,   112,
    1719,    -1,    -1,    -1,    -1,   118,    -1,   120,   121,   122,
     123,   124,   125,   126,    -1,   112,    -1,    -1,    -1,    -1,
      -1,   118,   497,   120,   121,   122,   123,   124,   125,   126,
      -1,    -1,    -1,    -1,   112,    -1,    -1,    81,    -1,    -1,
     118,    -1,   120,   121,   122,   123,   124,   125,   126,    -1,
      -1,    -1,    -1,    -1,    -1,   168,   169,    -1,   171,   103,
      -1,   536,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   168,   169,    -1,   171,    -1,    -1,    -1,    -1,   192,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,   201,    -1,
     168,   169,    -1,   171,    -1,   192,   140,   141,   142,   143,
     144,    -1,    -1,    -1,   201,    -1,    -1,   103,    -1,    -1,
    1829,    -1,    -1,   588,   192,   590,    -1,    -1,   593,    -1,
      -1,    -1,   166,   201,   168,   169,   170,   171,   172,   173,
    1849,    -1,    -1,    -1,  1853,  1854,    -1,    -1,    -1,    -1,
    1859,    -1,    -1,    -1,   140,   141,   142,   143,   144,  1868,
      -1,   626,   196,   197,    -1,    -1,  1875,  1876,    -1,    -1,
    1879,  1880,    -1,    -1,    -1,    -1,   162,    -1,    -1,   165,
      -1,    -1,   168,   169,  1893,   171,   172,   173,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,  1913,    -1,    -1,    -1,    -1,    -1,
     196,    -1,    -1,    -1,    69,   201,    -1,    -1,   683,   684,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   692,    -1,    -1,
      -1,    -1,    -1,    -1,    56,    -1,    -1,    -1,    -1,  1948,
      -1,    -1,    -1,    -1,  1953,    -1,    -1,    -1,   713,    -1,
      -1,    -1,  1961,    10,    11,    12,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,  1978,
      -1,    -1,    -1,    30,    31,  1984,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    59,    60,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,   787,    -1,    -1,  2034,  2035,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,   820,    -1,    -1,    -1,    -1,
      -1,   826,    -1,   828,    -1,    -1,    69,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,   136,    78,
      79,    80,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,   856,   857,    -1,    -1,    -1,    -1,    -1,    -1,   864,
      -1,    -1,    -1,    -1,   103,    -1,   871,   872,   873,   874,
     875,   876,   877,   126,    -1,    -1,    -1,    -1,    -1,    -1,
     885,    -1,    78,    79,    80,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,    -1,    91,   901,    -1,    -1,    -1,
     198,   140,   141,   142,   143,   144,    -1,   204,    -1,   162,
      -1,    -1,   165,   166,    -1,   168,   169,    -1,   171,   172,
     173,    -1,   927,    -1,    -1,   287,    -1,   289,    -1,   168,
     169,    -1,   171,   172,   173,    -1,   941,    -1,   943,   944,
      -1,    -1,    -1,   196,    -1,    -1,    -1,    -1,    19,    20,
     146,   147,   148,   149,   150,    -1,   199,   196,   201,    30,
      -1,   157,   967,   968,    -1,    -1,    -1,   163,   164,    -1,
      -1,    -1,   977,    -1,    -1,    -1,    -1,    -1,   983,    -1,
      -1,   177,    -1,    -1,   346,    -1,    -1,    -1,    -1,    -1,
     995,    -1,    -1,    -1,    -1,   191,    -1,    -1,  1003,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,  1018,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    -1,
      10,    11,    12,  1038,    -1,    -1,    -1,  1042,    -1,    -1,
      69,    -1,    -1,    -1,    59,    60,    -1,    -1,  1053,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,   438,    57,    -1,   441,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,  1096,  1097,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1110,    -1,    -1,    -1,  1114,
      -1,  1116,    81,   120,   121,   122,   123,   124,   125,    -1,
     135,   136,    91,    -1,   131,   132,  1131,  1132,  1133,  1134,
    1135,  1136,  1137,    -1,   103,  1140,  1141,  1142,  1143,  1144,
    1145,  1146,  1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,
    1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,
    1165,  1166,  1167,    -1,    -1,   172,   237,   174,    -1,    -1,
      -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
     187,  1186,   189,   198,    -1,   192,    -1,    -1,    -1,    -1,
      -1,    56,    -1,   162,    -1,    -1,   165,    -1,    -1,   168,
     169,    81,   171,   172,   173,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,  1222,    -1,  1224,
      -1,   201,    -1,   103,    -1,    -1,   588,   196,   590,  1234,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,  1261,    57,    -1,  1264,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,  1281,  1282,  1283,  1284,
    1285,  1286,    -1,    -1,  1289,    -1,  1291,    -1,   168,   169,
    1295,   171,   172,   173,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   374,    -1,  1310,  1311,    -1,  1313,    -1,
      -1,    -1,   383,    -1,    -1,    -1,   196,   197,  1323,   390,
      -1,   683,   684,    -1,  1329,    -1,    -1,  1332,    -1,  1334,
     692,   402,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,   413,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1358,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,  1394,
    1395,    -1,    -1,  1398,    69,    -1,  1401,    -1,    -1,    -1,
      -1,   201,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   287,    -1,   289,    -1,   497,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    -1,
    1455,    -1,  1457,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   536,    -1,    -1,    -1,    -1,
      -1,    -1,    59,    60,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   346,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,  1500,    -1,  1502,    -1,    -1,
      -1,    -1,   864,  1508,    -1,  1510,    -1,  1512,    -1,   871,
     872,    -1,  1517,  1518,    -1,    -1,  1521,    -1,  1523,    -1,
      -1,  1526,   593,   135,   136,    -1,   201,    59,    60,    -1,
      -1,    -1,    -1,    -1,  1539,  1540,    -1,    31,  1543,    -1,
      -1,    -1,    -1,    -1,    -1,  1550,    -1,    -1,   135,   136,
      -1,    -1,    -1,    -1,    -1,   626,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,   438,    68,    -1,   441,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   198,    81,   103,    -1,
    1595,    -1,  1597,    -1,  1599,    -1,    -1,    -1,    -1,  1604,
      -1,  1606,    -1,   135,   136,   967,    -1,    -1,    -1,   103,
      -1,   198,    -1,    -1,   129,  1620,    -1,   111,  1623,    -1,
      -1,   983,    -1,    -1,    -1,   140,   141,   142,   143,   144,
    1635,  1636,    -1,   995,    -1,    -1,    -1,    -1,  1643,    -1,
    1645,    -1,   713,    -1,    -1,   139,   140,   141,   142,   143,
     144,   145,    -1,   168,   169,    -1,   171,   172,   173,    -1,
      -1,    -1,  1667,    -1,  1669,    -1,   198,    -1,   162,    -1,
    1675,   165,   166,    -1,   168,   169,  1038,   171,   172,   173,
      -1,   196,   197,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   186,    -1,  1699,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   196,   197,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1716,  1717,  1718,    -1,    -1,   787,    -1,  1723,    -1,
    1725,    -1,    -1,   588,    -1,    -1,  1731,    -1,  1733,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1110,   820,
      -1,    -1,  1114,    -1,  1116,   826,    31,   828,    -1,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,   856,   857,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   873,   874,   875,   876,   877,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   885,    -1,    -1,    -1,   683,   684,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   692,  1833,    -1,
     901,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1845,    -1,    -1,    31,  1849,    -1,    -1,    -1,  1853,  1854,
      -1,    -1,    -1,    -1,    -1,    -1,   927,    -1,    -1,    -1,
    1222,    -1,  1224,  1868,    -1,    -1,    -1,    -1,    -1,  1874,
     941,    -1,   943,   944,    -1,    -1,    -1,    -1,    81,    -1,
    1885,    -1,    -1,    19,    20,    -1,  1891,    -1,    -1,    -1,
    1895,    -1,    -1,    81,    30,    -1,    -1,   968,    -1,  1261,
     103,    -1,  1264,    91,    -1,    -1,   977,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,  1922,    -1,    -1,
      -1,    -1,    -1,   198,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1003,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,    -1,    -1,  1949,    -1,  1951,  1018,    -1,    -1,
      -1,    -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      -1,  1323,  1967,    -1,    -1,   168,   169,  1329,   171,   172,
     173,  1042,    -1,    -1,   162,  1980,    -1,   165,    -1,    -1,
     168,   169,  1053,   171,   172,   173,    -1,   175,    -1,    -1,
      -1,    -1,  1997,   196,   197,    -1,    -1,    -1,    -1,    -1,
      -1,  2006,    -1,    -1,    -1,    -1,   871,   872,   196,    -1,
      -1,    -1,    -1,    -1,  2019,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1096,  1097,    -1,    -1,    -1,
      -1,    -1,  1394,  1395,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
    1131,  1132,  1133,  1134,  1135,  1136,  1137,    -1,    69,  1140,
    1141,  1142,  1143,  1144,  1145,  1146,  1147,  1148,  1149,  1150,
    1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,
    1161,  1162,  1163,  1164,  1165,  1166,  1167,    -1,    -1,    -1,
      -1,   237,   967,    -1,    -1,    -1,    -1,    81,    -1,    83,
      84,    -1,    -1,    -1,    -1,  1186,    -1,    -1,   983,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
     995,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1500,    -1,
    1502,    -1,    81,    -1,    83,    -1,    85,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,  1234,   103,    -1,   140,   141,   142,   143,
     144,    -1,    -1,  1038,    -1,    -1,    -1,  1539,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1550,    -1,
      -1,    -1,   166,    -1,   168,   169,    68,   171,   172,   173,
      -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,    81,
    1281,  1282,  1283,  1284,  1285,  1286,    -1,    -1,  1289,    -1,
    1291,    -1,   196,    -1,  1295,    -1,   200,    -1,   202,   168,
     169,   103,   171,   172,   173,    -1,    -1,    -1,   374,  1310,
    1311,    -1,  1313,    -1,    -1,  1110,    -1,   383,    -1,  1114,
      -1,    -1,    -1,    -1,   390,    -1,    -1,   196,  1620,    -1,
      -1,  1332,    -1,  1334,    -1,    -1,   402,   139,   140,   141,
     142,   143,   144,   145,    -1,    -1,    -1,   413,    -1,    -1,
      -1,  1643,    -1,    -1,    -1,    -1,    -1,  1358,    -1,    -1,
     162,    -1,    -1,   165,   166,    -1,   168,   169,    -1,   171,
     172,   173,    -1,   175,    -1,  1667,    -1,  1669,    10,    11,
      12,    -1,    -1,  1675,   186,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   196,   197,    -1,  1398,    30,    31,
    1401,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,  1222,    -1,  1224,
      -1,   497,    -1,    -1,    -1,    -1,    -1,    69,    -1,  1731,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,  1455,    -1,  1457,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1261,    -1,    -1,  1264,
     536,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1508,    -1,  1510,
      69,  1512,    -1,    -1,    -1,    -1,  1517,  1518,    -1,    -1,
    1521,    -1,  1523,    -1,    -1,  1526,    -1,   593,  1323,    -1,
      -1,    -1,    -1,    -1,  1329,    -1,    -1,    -1,    -1,  1540,
      -1,  1833,  1543,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1845,    -1,    -1,    -1,  1849,    -1,    -1,
     626,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   201,
      -1,    -1,    -1,    -1,    -1,    -1,  1868,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,    -1,  1595,    -1,  1597,    -1,  1599,  1394,
    1395,    -1,    -1,  1604,    -1,  1606,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,  1623,    59,    60,    -1,    -1,    -1,    -1,    -1,
    1922,    -1,   593,    -1,  1635,  1636,    -1,    -1,    -1,    -1,
      -1,    -1,   201,    -1,  1645,    -1,    -1,   713,    -1,    10,
      11,    12,    59,    60,    -1,    -1,    -1,  1949,    -1,  1951,
      -1,    -1,    -1,    -1,    -1,   626,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,  1699,   135,
     136,    -1,    -1,    -1,    31,  1500,    -1,  1502,    69,    -1,
      -1,    -1,    -1,    -1,  2006,  1716,  1717,  1718,    -1,    -1,
      -1,   787,  1723,    -1,  1725,    -1,    -1,  2019,   135,   136,
      -1,    -1,  1733,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    81,  1550,    -1,    -1,    -1,    -1,
     826,    -1,   828,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,   126,    -1,    -1,
     856,   857,    -1,   120,   121,   122,   123,   124,   125,    -1,
     139,   140,   141,   142,   143,   144,   145,   873,   874,   875,
     876,   877,   139,   140,   141,   142,   143,   144,   145,   885,
      -1,    -1,    -1,   162,    -1,  1620,   165,   166,    -1,   168,
     169,    -1,   171,   172,   173,   162,    -1,    -1,   165,   166,
     201,   168,   169,    -1,   171,   172,   173,    -1,    -1,    -1,
      -1,    -1,  1853,  1854,    -1,    -1,    -1,   196,    -1,   186,
      -1,   927,    -1,    -1,    -1,   192,    -1,    -1,    -1,   196,
     197,    -1,    -1,  1874,    -1,    -1,    -1,   943,   944,    -1,
      -1,    -1,    -1,    -1,  1885,    -1,    -1,    -1,    -1,    -1,
    1891,    -1,    -1,    -1,  1895,    -1,   857,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   977,   873,   874,   875,   876,   877,    -1,    10,    11,
      12,    -1,    -1,    -1,   885,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1731,  1003,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,  1967,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1042,    69,    -1,  1980,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1997,    -1,    -1,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
    1096,  1097,    -1,    -1,    -1,    -1,    -1,    -1,  1833,    -1,
      69,    -1,  1003,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1845,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1131,  1132,  1133,  1134,  1135,
    1136,  1137,    -1,    -1,  1140,  1141,  1142,  1143,  1144,  1145,
    1146,  1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,
    1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,
    1166,  1167,    -1,    -1,    -1,    10,    11,    12,    -1,   201,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1186,    -1,    -1,    -1,    -1,    30,    31,  1922,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,  1949,    -1,  1951,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,  1234,    -1,
    1131,  1132,   201,    -1,  1135,    -1,    -1,    -1,    -1,  1140,
    1141,  1142,  1143,  1144,  1145,  1146,  1147,  1148,  1149,  1150,
    1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,
    1161,  1162,  1163,  1164,  1165,  1166,  1167,    -1,    -1,    -1,
      -1,  2006,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1289,  2019,  1186,    -1,    -1,    -1,  1295,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1310,  1311,    -1,  1313,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,  1332,    57,  1334,    81,
      -1,    83,    84,    -1,    10,    11,    12,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,  1358,    -1,    30,    31,   201,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,    69,    -1,  1401,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,  1313,    -1,   166,    -1,   168,   169,    -1,   171,
     172,   173,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1332,    -1,  1334,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    59,    60,   196,    -1,    -1,    -1,   200,    -1,
     202,    -1,    -1,    -1,    -1,    -1,    -1,  1358,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      81,    -1,  1508,    -1,  1510,    -1,  1512,    10,    11,    12,
      -1,  1517,  1518,    -1,    -1,  1521,    -1,  1523,   135,   136,
    1526,    -1,   103,    -1,    -1,   201,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,   140,
     141,   142,   143,   144,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   162,    -1,    -1,   165,   166,    -1,   168,   169,    -1,
     171,   172,   173,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1606,    -1,    -1,    -1,    -1,    -1,    -1,  1508,    -1,  1510,
      -1,  1512,    10,    11,    12,   196,  1517,  1623,    -1,    -1,
    1521,    -1,  1523,    -1,    -1,  1526,    -1,   199,    -1,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,     7,    -1,    -1,    -1,   199,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1716,  1717,  1718,    -1,    -1,    -1,    -1,  1723,    -1,    -1,
      -1,    -1,  1623,    38,    -1,    -1,  1732,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,   199,    -1,    -1,    -1,   120,   121,   122,   123,   124,
     125,    -1,    -1,   128,   129,  1716,  1717,  1718,    -1,    -1,
      -1,    -1,  1723,   138,   139,   140,   141,   142,   143,   144,
      -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,    -1,
      -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,
     165,   166,    -1,   168,   169,    -1,   171,   172,   173,    -1,
     175,    -1,   177,    -1,    -1,   180,    -1,    -1,  1874,    -1,
      -1,   186,    -1,    -1,    -1,    -1,   191,   192,   193,  1885,
      -1,   196,   197,    -1,    -1,  1891,    -1,   202,   203,  1895,
     205,   206,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1924,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,    69,
      -1,  1967,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1874,    -1,    -1,    -1,    -1,    27,    28,
      29,    -1,    -1,    -1,  1885,    -1,    -1,    -1,    -1,    -1,
    1891,    -1,    -1,    -1,  1895,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    87,    -1,
      -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,    98,
      -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,
     109,   110,   111,   112,   113,   114,  1967,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,    -1,   127,   128,
     129,   130,   131,   132,    -1,    -1,    -1,    -1,   198,   138,
     139,   140,   141,   142,   143,   144,    -1,   146,   147,   148,
     149,   150,    -1,    -1,    -1,   154,    -1,    -1,   157,    -1,
      -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,   168,
     169,    -1,   171,   172,   173,   174,    -1,    -1,   177,    -1,
      -1,   180,    -1,    -1,    -1,    -1,    -1,   186,   187,    -1,
     189,    -1,   191,   192,   193,    -1,    -1,   196,   197,    -1,
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
     111,   112,   113,   114,    -1,   116,    -1,   118,   119,   120,
     121,   122,   123,   124,   125,    -1,   127,   128,   129,   130,
     131,   132,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,    -1,   146,   147,   148,   149,   150,
      -1,    -1,    -1,   154,    -1,    -1,   157,    -1,    -1,    -1,
      -1,   162,   163,   164,   165,   166,    -1,   168,   169,    -1,
     171,   172,   173,   174,    -1,    -1,   177,    -1,    -1,   180,
      -1,    -1,    -1,    -1,    -1,   186,   187,    -1,   189,    -1,
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
     109,   110,   111,    -1,   113,   114,    -1,   116,   117,    -1,
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
      -1,    -1,    91,    92,    93,    94,    95,    96,    -1,    98,
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
      -1,    96,    -1,    98,    -1,   100,   101,    -1,   103,   104,
      -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,   114,
      -1,   116,    -1,    -1,   119,   120,   121,   122,   123,   124,
     125,    -1,   127,   128,   129,    -1,   131,   132,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
      -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,   154,
      -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,
     165,   166,    -1,   168,   169,    -1,   171,   172,   173,   174,
      -1,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,
      -1,   186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,
      -1,   196,   197,    -1,   199,   200,    -1,   202,   203,    -1,
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
      77,    78,    79,    80,    81,    82,    83,    84,    -1,    86,
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
     197,    -1,   199,   200,    -1,   202,   203,    -1,   205,   206,
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
      99,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,
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
      -1,    96,    97,    98,    -1,   100,    -1,    -1,   103,   104,
      -1,    -1,    -1,   108,   109,   110,   111,    -1,   113,   114,
      -1,   116,    -1,    -1,   119,   120,   121,   122,   123,   124,
     125,    -1,   127,   128,   129,    -1,   131,   132,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
      -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,   154,
      -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,
     165,   166,    -1,   168,   169,    -1,   171,   172,   173,   174,
      -1,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,
      -1,   186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,
      -1,   196,   197,    -1,   199,   200,    -1,   202,   203,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     197,    -1,    -1,    -1,    -1,   202,   203,    -1,   205,   206,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    -1,    29,    -1,    -1,    -1,
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
     193,    -1,    -1,   196,   197,    -1,   199,    -1,    -1,   202,
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
      -1,    -1,   191,   192,   193,    -1,    -1,   196,   197,    -1,
      -1,   200,    -1,   202,   203,    -1,   205,   206,     3,     4,
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
      -1,    -1,    -1,    -1,    -1,   120,   121,   122,   123,   124,
     125,    -1,    -1,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
      -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,    -1,
      -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,
     165,   166,    -1,   168,   169,    -1,   171,   172,   173,    -1,
     175,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,
      -1,   186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,
      -1,   196,   197,    -1,    -1,    -1,    -1,   202,   203,    -1,
     205,   206,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,
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
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     197,    10,    11,    12,    -1,   202,   203,    -1,   205,   206,
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
     103,    -1,    -1,    -1,    -1,   108,    -1,    -1,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,   121,   122,
     123,   124,   125,    -1,    -1,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,
     143,   144,    -1,   146,   147,   148,   149,   150,    -1,    -1,
      -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,
     163,   164,   165,   166,    -1,   168,   169,    -1,   171,   172,
     173,    -1,    -1,    -1,   177,    -1,    -1,   180,    -1,    -1,
     199,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,
     193,    -1,    -1,   196,   197,    -1,    -1,    -1,    -1,   202,
     203,    -1,   205,   206,     3,     4,     5,     6,     7,    -1,
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
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
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
      -1,   196,   197,    -1,   199,    11,    12,   202,   203,    -1,
     205,   206,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      51,    -1,    -1,    69,    -1,    56,    -1,    58,    59,    60,
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
     191,   192,   193,    -1,    -1,   196,   197,    -1,   199,    -1,
      -1,   202,   203,    -1,   205,   206,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     197,    10,    11,    12,    -1,   202,   203,    -1,   205,   206,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,   121,   122,
     123,   124,   125,    -1,    -1,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,
     143,   144,    -1,   146,   147,   148,   149,   150,    -1,    -1,
      -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,
     163,   164,   165,   166,    -1,   168,   169,    -1,   171,   172,
     173,    -1,    -1,    -1,   177,    -1,    -1,   180,    -1,    -1,
     199,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,
     193,    -1,    -1,   196,   197,   198,    -1,    -1,    -1,   202,
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
      -1,    -1,   191,   192,   193,    -1,    -1,   196,   197,    -1,
      -1,    -1,    -1,   202,   203,    -1,   205,   206,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
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
      13,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    50,    51,    -1,
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
      -1,    -1,    -1,    -1,    13,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
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
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   196,   197,    10,    11,    12,    -1,   202,   203,    -1,
     205,   206,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,
     121,   122,   123,   124,   125,    -1,    -1,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,    -1,   146,   147,   148,   149,   150,
      -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,
      -1,   162,   163,   164,   165,   166,    -1,   168,   169,    -1,
     171,   172,   173,    -1,    -1,    -1,   177,    -1,    -1,   180,
      -1,   198,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,
     191,   192,   193,    -1,    -1,   196,   197,    10,    11,    12,
      -1,   202,   203,    -1,   205,   206,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    27,    -1,    13,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    69,    -1,    -1,    56,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    82,    83,    84,    -1,   102,
      -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   120,   121,   122,   123,   124,   125,    -1,
      -1,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,   144,    -1,   146,
     147,   148,   149,   150,    -1,    -1,    -1,    -1,    -1,    -1,
     157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,
      -1,   168,   169,    -1,   171,   172,   173,    -1,     3,     4,
     177,     6,     7,   180,    -1,    10,    11,    12,    13,   186,
      -1,    -1,    -1,    -1,   191,   192,   193,    -1,    -1,   196,
     197,    -1,    -1,    28,    29,   202,   203,    -1,   205,   206,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,    81,   129,    -1,   131,   132,   133,   134,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
     145,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,   162,    10,    11,
      12,    13,    -1,   168,   169,    -1,   171,   172,   173,   174,
      -1,   176,    -1,    -1,   179,    -1,    28,    29,    -1,    -1,
      -1,    -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      -1,   196,    -1,    -1,    -1,   200,    -1,   202,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    -1,   165,    -1,    -1,
     168,   169,    -1,   171,   172,   173,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,    -1,   196,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,    81,   129,    -1,   131,
     132,   133,   134,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,
     162,    10,    11,    12,    13,    -1,   168,   169,    -1,   171,
     172,   173,   174,    -1,   176,    -1,    -1,   179,    -1,    28,
      29,    -1,    31,    -1,    -1,   140,   141,   142,   143,   144,
      -1,    -1,    -1,    -1,   196,    -1,    -1,    -1,   200,    -1,
     202,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
     165,    -1,    -1,   168,   169,    -1,   171,   172,   173,    68,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   196,    -1,    92,    93,    94,    95,    96,    97,    98,
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
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    57,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    69,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,    -1,   129,    -1,    -1,
     132,   133,   134,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     162,    -1,    -1,   165,   166,    -1,   168,   169,    -1,   171,
     172,   173,   174,    -1,   176,    -1,    -1,   179,     3,     4,
      -1,     6,     7,    -1,   186,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,   196,   197,    -1,    -1,    -1,   201,
      -1,    -1,    -1,    28,    29,    -1,    31,    -1,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    57,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    -1,    69,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,    -1,   129,    -1,   131,   132,   133,   134,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
     145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   162,    -1,    -1,
     165,   166,    -1,   168,   169,    -1,   171,   172,   173,   174,
      -1,   176,    -1,    -1,   179,     3,     4,    -1,     6,     7,
      -1,   186,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,   196,   197,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    29,    -1,    31,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
      -1,   129,    -1,    -1,   132,   133,   134,    -1,    -1,    -1,
     138,   139,   140,   141,   142,   143,   144,   145,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   162,    -1,    -1,   165,   166,    -1,
     168,   169,    -1,   171,   172,   173,   174,   103,   176,    -1,
      -1,   179,    -1,     3,     4,    -1,     6,     7,   186,   187,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,   196,   197,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    29,
      -1,    31,    -1,    -1,   140,   141,   142,   143,   144,    -1,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    57,    57,    -1,
      -1,    -1,   168,   169,    -1,   171,   172,   173,    68,    -1,
      69,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     196,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,    -1,   129,
      -1,    -1,   132,   133,   134,    -1,    -1,    -1,   138,   139,
     140,   141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   162,    -1,    -1,   165,   166,    -1,   168,   169,
      -1,   171,   172,   173,   174,    -1,   176,    -1,    -1,   179,
       3,     4,     5,     6,     7,    -1,   186,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,   196,   197,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    57,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    71,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,    -1,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,    -1,    -1,    -1,   138,   139,   140,   141,   142,
     143,   144,    -1,   146,   147,   148,   149,   150,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   162,
     163,   164,    -1,    -1,    -1,   168,   169,    -1,   171,   172,
     173,   174,    -1,   176,   177,    -1,   179,    -1,    -1,    -1,
      -1,    -1,    -1,   186,   187,    -1,   189,    -1,   191,   192,
      -1,     3,     4,   196,     6,     7,    12,    -1,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,    28,    29,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,    -1,   129,    -1,   131,
     132,   133,   134,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,
     162,    10,    11,    12,    13,    -1,   168,   169,    -1,   171,
     172,   173,   174,    -1,   176,    -1,    -1,   179,    -1,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   196,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,    -1,
     129,    -1,   131,   132,   133,   134,    -1,    -1,    -1,   138,
     139,   140,   141,   142,   143,   144,   145,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
       6,     7,    -1,   162,    10,    11,    12,    13,    -1,   168,
     169,    -1,   171,   172,   173,   174,    -1,   176,    -1,    -1,
     179,    -1,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   196,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,    31,   129,    -1,    -1,   132,   133,   134,    -1,
      -1,    -1,   138,   139,   140,   141,   142,   143,   144,   145,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   162,    -1,    -1,    68,
      -1,    -1,   168,   169,    -1,   171,   172,   173,   174,    -1,
     176,    31,    81,   179,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     196,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   144,   145,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   162,    -1,    -1,   165,   166,    -1,   168,
     169,    -1,   171,   172,   173,    -1,   175,    -1,    -1,    -1,
      -1,    -1,    32,    -1,    -1,    -1,    -1,   186,    -1,   139,
     140,   141,   142,   143,   144,   145,    -1,   196,   197,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    -1,
      -1,    -1,   162,    -1,    -1,   165,   166,    -1,   168,   169,
      70,   171,   172,   173,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,
      -1,    91,    10,    11,    12,    -1,   196,   197,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,   139,
     140,   141,   142,   143,   144,    -1,   146,   147,   148,   149,
     150,    69,    -1,    -1,    -1,    -1,    -1,   157,    38,    -1,
      -1,    -1,   162,   163,   164,   165,   166,    -1,   168,   169,
      -1,   171,   172,   173,    -1,    -1,    -1,   177,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,
      70,   191,    -1,    -1,    -1,    -1,   196,   197,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     120,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,   138,   139,
     140,   141,   142,   143,   144,    -1,   146,   147,   148,   149,
     150,    69,    -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,
      -1,    -1,   162,   163,   164,   165,   166,    -1,   168,   169,
      -1,   171,   172,   173,    -1,    50,    51,   177,    -1,    -1,
      -1,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   191,    -1,    -1,    -1,    70,   196,   197,    -1,    -1,
      -1,    -1,   202,    78,    79,    80,    81,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,   139,   140,   141,   142,   143,   144,
      69,   146,   147,   148,   149,   150,    -1,    -1,    -1,    50,
      51,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,
     165,   166,    -1,   168,   169,    -1,   171,   172,   173,    70,
      -1,    -1,   177,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,   186,    -1,    -1,    -1,    -1,   191,    -1,    -1,    -1,
      91,   196,   197,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    70,    -1,    -1,    -1,   137,    -1,
      -1,    -1,    78,    79,    80,    81,    -1,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,   139,   140,
     141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   120,    -1,   157,    -1,    -1,    -1,
      -1,   162,    -1,    -1,   165,   166,    -1,   168,   169,    -1,
     171,   172,   173,   139,   140,   141,   142,   143,   144,    -1,
     146,   147,   148,   149,   150,   186,    -1,    -1,    -1,    -1,
      -1,   157,    -1,    -1,    -1,   196,   162,   163,   164,   165,
     166,    -1,   168,   169,    -1,   171,   172,   173,    70,    -1,
      -1,   177,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,   191,    -1,    -1,    -1,    91,
     196,   197,    -1,    -1,   200,    -1,   202,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,   138,   139,   140,   141,
     142,   143,   144,    -1,   146,   147,   148,   149,   150,    69,
      -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,
     162,   163,   164,   165,   166,    -1,   168,   169,    -1,   171,
     172,   173,    70,    71,    -1,   177,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    -1,    83,    84,    -1,    -1,   191,
      -1,    -1,    -1,    91,   196,   197,    -1,    -1,    -1,    -1,
     202,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,    -1,   146,   147,
     148,   149,   150,    -1,    -1,    -1,    -1,    -1,    -1,   157,
      -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,
     168,   169,    -1,   171,   172,   173,    70,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    -1,    83,
      84,    -1,    -1,   191,    -1,    -1,    -1,    91,   196,   197,
      -1,    -1,    -1,    -1,   202,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
     144,    -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,
      -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,
     164,   165,   166,    -1,   168,   169,    -1,   171,   172,   173,
      70,    -1,    -1,   177,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    -1,    83,    84,    -1,    -1,   191,    -1,    -1,
      -1,    91,   196,   197,    -1,    -1,    -1,    -1,   202,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     120,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,    -1,   146,   147,   148,   149,
     150,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,
      -1,    -1,   162,   163,   164,   165,   166,    -1,   168,   169,
      -1,   171,   172,   173,    -1,    -1,    -1,   177,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   191,    -1,    -1,    -1,    -1,   196,   197,    -1,    -1,
      30,    31,   202,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   137,    30,    31,    -1,    33,
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
      10,    11,    12,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    -1,   137,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,   137,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,    -1,   137,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    -1,   137,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,   137,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    78,    79,    80,    81,    -1,
      83,    84,    -1,    -1,    -1,    -1,    -1,    69,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   137,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,
     143,   144,    -1,   146,   147,   148,   149,   150,    -1,    -1,
      -1,    -1,    -1,    -1,   157,   137,    -1,    -1,    -1,   162,
     163,   164,   165,   166,    -1,   168,   169,    -1,   171,   172,
     173,    -1,    -1,    -1,   177,    -1,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,    -1,   191,    91,
      -1,    -1,    -1,   196,   197,    -1,    -1,    -1,    -1,   202,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,   143,   144,    -1,   146,   147,   148,   149,   150,    -1,
      -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,
     162,   163,   164,   165,   166,    -1,   168,   169,    -1,   171,
     172,   173,    -1,    -1,    -1,   177,    -1,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,   191,
      91,    -1,    -1,    -1,   196,   197,    -1,    -1,    -1,    -1,
     202,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,
     141,   142,   143,   144,   103,   146,   147,   148,   149,   150,
      -1,    -1,   111,   112,    -1,    -1,   157,    -1,    -1,    -1,
      -1,   162,   163,   164,   165,   166,    -1,   168,   169,    -1,
     171,   172,   173,    -1,    -1,    -1,   177,    -1,    -1,    -1,
      -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
     191,    -1,    -1,    -1,    -1,   196,   197,    -1,    -1,    -1,
      -1,   202,    -1,   162,    -1,    -1,   165,    -1,    -1,   168,
     169,    -1,   171,   172,   173,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   196,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,    32,    -1,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69
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
     488,   490,   492,   120,   121,   122,   138,   162,   172,   197,
     214,   255,   336,   358,   464,   358,   197,   358,   358,   358,
     358,   108,   358,   358,   450,   451,   358,   358,   358,   358,
      81,    83,    91,   120,   140,   141,   142,   143,   144,   157,
     197,   225,   378,   419,   422,   427,   464,   468,   464,   358,
     358,   358,   358,   358,   358,   358,   358,    38,   358,   479,
     480,   120,   131,   197,   225,   268,   419,   420,   421,   423,
     427,   461,   462,   463,   472,   476,   477,   358,   197,   357,
     424,   197,   357,   369,   347,   358,   236,   357,   197,   197,
     197,   357,   199,   358,   214,   199,   358,     3,     4,     6,
       7,    10,    11,    12,    13,    28,    29,    31,    57,    68,
      71,    72,    73,    74,    75,    76,    77,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   129,   131,   132,   133,   134,   138,   139,   145,
     162,   166,   174,   176,   179,   186,   197,   214,   215,   216,
     227,   493,   513,   514,   517,    27,   199,   352,   354,   358,
     200,   248,   358,   111,   112,   162,   165,   187,   217,   218,
     219,   220,   224,    83,   202,   302,   303,    83,   304,   122,
     131,   121,   131,   197,   197,   197,   197,   214,   274,   496,
     197,   197,    70,    70,    70,    70,    70,   347,    83,    90,
     158,   159,   160,   485,   486,   165,   200,   224,   224,   214,
     275,   496,   166,   197,   496,   496,    83,   193,   200,   370,
      28,   346,   349,   358,   360,   464,   469,   231,   200,   474,
      90,   425,   485,    90,   485,   485,    32,   165,   182,   497,
     197,     9,   199,   197,   345,   359,   465,   468,   117,    38,
     254,   166,   273,   496,   120,   192,   255,   337,    70,   200,
     459,   199,   199,   199,   199,   199,   199,   199,   199,    10,
      11,    12,    30,    31,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    57,    69,   199,
      70,    70,   200,   161,   132,   172,   174,   187,   189,   276,
     335,   336,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    59,    60,   135,   136,   454,
     459,   459,   197,   197,    70,   200,   197,   254,   255,    14,
     358,   199,   137,    49,   214,   449,    90,   346,   360,   161,
     464,   137,   204,     9,   434,   269,   346,   360,   464,   497,
     161,   197,   426,   454,   459,   198,   358,    32,   234,     8,
     371,     9,   199,   234,   235,   347,   348,   358,   214,   288,
     238,   199,   199,   199,   139,   145,   517,   517,   182,   516,
     197,   111,   517,    14,   161,   139,   145,   162,   214,   216,
     199,   199,   199,   249,   115,   179,   199,   217,   219,   217,
     219,   217,   219,   224,   217,   219,   200,     9,   435,   199,
     102,   165,   200,   464,     9,   199,    14,     9,   199,   131,
     131,   464,   489,   347,   346,   360,   464,   468,   469,   198,
     182,   266,   138,   464,   478,   479,   358,   379,   380,   347,
     400,   400,   379,   400,   199,    70,   454,   158,   486,    82,
     358,   464,    90,   158,   486,   224,   213,   199,   200,   261,
     271,   409,   411,    91,   197,   372,   373,   375,   418,   422,
     471,   473,   490,    14,   102,   491,   366,   367,   368,   298,
     299,   452,   453,   198,   198,   198,   198,   198,   201,   233,
     234,   256,   263,   270,   452,   358,   203,   205,   206,   214,
     498,   499,   517,    38,   175,   300,   301,   358,   493,   245,
     246,   346,   354,   355,   358,   360,   464,   200,   247,   247,
     247,   247,   197,   496,   264,   254,   358,   475,   358,   358,
     358,   358,   358,    32,   358,   358,   358,   358,   358,   358,
     358,   358,   358,   358,   358,   358,   358,   358,   358,   358,
     358,   358,   358,   358,   358,   358,   358,   358,   423,   358,
     475,   475,   358,   481,   482,   131,   200,   215,   216,   474,
     274,   214,   275,   496,   496,   273,   255,    38,   349,   352,
     354,   358,   358,   358,   358,   358,   358,   358,   358,   358,
     358,   358,   358,   358,   166,   200,   214,   455,   456,   457,
     458,   474,   300,   300,   475,   358,   478,   254,   198,   358,
     197,   448,     9,   434,   198,   198,    38,   358,    38,   358,
     426,   198,   198,   198,   472,   473,   474,   300,   200,   214,
     455,   456,   474,   198,   231,   292,   200,   354,   358,   358,
      94,    32,   234,   286,   199,    27,   102,    14,     9,   198,
      32,   200,   289,   517,    31,    91,   175,   227,   510,   511,
     512,   197,     9,    50,    51,    56,    58,    70,   139,   140,
     141,   142,   143,   144,   186,   197,   225,   386,   389,   392,
     395,   398,   404,   419,   427,   428,   430,   431,   214,   515,
     231,   197,   242,   200,   199,   200,   199,   200,   199,   102,
     165,   200,   199,   111,   112,   165,   220,   221,   222,   223,
     224,   220,   214,   358,   303,   428,    83,     9,   198,   198,
     198,   198,   198,   198,   198,   199,    50,    51,   506,   508,
     509,   133,   279,   197,     9,   198,   198,   137,   204,     9,
     434,     9,   434,   204,   204,   204,   204,    83,    85,   214,
     487,   214,    70,   201,   201,   210,   212,    32,   134,   278,
     181,    54,   166,   181,   413,   360,   137,     9,   434,   198,
     161,   517,   517,    14,   371,   298,   229,   194,     9,   435,
     517,   518,   454,   454,   201,     9,   434,   183,   464,   358,
     198,     9,   435,    14,     9,   198,     9,   198,   198,   198,
     198,    14,   198,   201,   232,   233,   363,   257,   133,   277,
     197,   496,   204,   201,   358,    32,   204,   204,   137,   201,
       9,   434,   358,   497,   197,   267,   262,   272,    14,   491,
     265,   254,    71,   464,   358,   497,   198,   198,   204,   201,
     198,    50,    51,    70,    78,    79,    80,    91,   139,   140,
     141,   142,   143,   144,   157,   186,   214,   387,   390,   393,
     396,   399,   419,   430,   437,   439,   440,   444,   447,   214,
     464,   464,   137,   277,   454,   459,   198,   358,   293,    75,
      76,   294,   229,   357,   231,   348,   102,    38,   138,   283,
     464,   428,   214,    32,   234,   287,   199,   290,   199,   290,
       9,   434,    91,   227,   137,   161,     9,   434,   198,   175,
     498,   499,   500,   498,   428,   428,   428,   428,   428,   433,
     436,   197,    70,    70,    70,    70,    70,   197,   428,   161,
     200,    10,    11,    12,    31,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    69,   161,   497,
     201,   419,   200,   251,   219,   219,   219,   214,   219,   220,
     220,   224,     9,   435,   201,   201,    14,   464,   199,   183,
       9,   434,   214,   280,   419,   200,   478,   138,   464,    14,
     358,   358,   204,   358,   201,   210,   517,   280,   200,   412,
      14,   198,   358,   372,   474,   199,   517,   194,   201,   230,
     233,   243,    32,   504,   453,    38,    83,   175,   455,   456,
     458,   455,   456,   517,    38,   175,   358,   428,   246,   354,
     355,   464,   247,   246,   247,   247,   201,   233,   298,   197,
     419,   278,   364,   258,   358,   358,   358,   201,   197,   300,
     279,    32,   278,   517,    14,   277,   496,   423,   201,   197,
      14,    78,    79,    80,   214,   438,   438,   440,   442,   443,
      52,   197,    70,    70,    70,    70,    70,    90,   158,   197,
     161,     9,   434,   198,   448,    38,   358,   278,   201,    75,
      76,   295,   357,   234,   201,   199,    95,   199,   283,   464,
     197,   137,   282,    14,   231,   290,   105,   106,   107,   290,
     201,   517,   183,   137,   161,   517,   214,   175,   510,     9,
     198,   434,   137,   204,     9,   434,   433,   381,   382,   428,
     401,   428,   429,   401,   381,   401,   372,   374,   376,   198,
     131,   215,   428,   483,   484,   428,   428,   428,    32,   428,
     428,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   428,   428,   515,    83,   252,   201,   201,   201,   201,
     223,   199,   428,   509,   102,   103,   505,   507,     9,   308,
     198,   197,   349,   354,   358,   137,   204,   201,   491,   308,
     167,   180,   200,   408,   415,   167,   200,   414,   137,   199,
     504,   197,   246,   345,   359,   465,   468,   517,   371,   518,
      83,   175,    14,    83,   497,   464,   358,   198,   298,   200,
     298,   197,   137,   197,   300,   198,   200,   517,   200,   199,
     517,   278,   259,   426,   300,   137,   204,     9,   434,   439,
     442,   383,   384,   440,   402,   440,   441,   402,   383,   402,
     158,   372,   445,   446,    81,   440,   464,   200,   357,    32,
      77,   234,   199,   348,   282,   478,   283,   198,   428,   101,
     105,   199,   358,    32,   199,   291,   201,   183,   517,   214,
     137,   175,    32,   198,   428,   428,   198,   204,     9,   434,
     137,   204,     9,   434,   204,   204,   204,   137,     9,   434,
     198,   137,   201,     9,   434,   428,    32,   198,   231,   199,
     199,   199,   199,   214,   517,   517,   505,   419,     6,   112,
     118,   121,   126,   168,   169,   171,   201,   309,   334,   335,
     336,   341,   342,   343,   344,   452,   478,   358,   201,   200,
     201,    54,   358,   358,   358,   371,   464,   199,   200,    38,
      83,   175,    14,    83,   358,   197,   504,   198,   308,   198,
     298,   358,   300,   198,   308,   491,   308,   199,   200,   197,
     198,   440,   440,   198,   204,     9,   434,   137,   204,     9,
     434,   204,   204,   204,   137,   198,     9,   434,   308,    32,
     231,   199,   198,   198,   198,   239,   199,   199,   291,   231,
     137,   517,   517,   137,   428,   428,   428,   428,   372,   428,
     428,   428,   200,   201,   507,   133,   134,   187,   215,   494,
     517,   281,   419,   112,   344,    31,   126,   139,   145,   166,
     172,   318,   319,   320,   321,   419,   170,   326,   327,   129,
     197,   214,   328,   329,   310,   255,   517,     9,   199,     9,
     199,   199,   491,   335,   198,   305,   166,   410,   201,   201,
     358,    83,   175,    14,    83,   358,   300,   118,   361,   504,
     201,   504,   198,   198,   201,   200,   201,   308,   298,   137,
     440,   440,   440,   440,   372,   201,   231,   237,   240,    32,
     234,   285,   231,   517,   198,   428,   137,   137,   137,   231,
     419,   419,   496,    14,   215,     9,   199,   200,   494,   491,
     321,   182,   200,     9,   199,     3,     4,     5,     6,     7,
      10,    11,    12,    13,    27,    28,    29,    57,    71,    72,
      73,    74,    75,    76,    77,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   138,   139,   146,   147,
     148,   149,   150,   162,   163,   164,   174,   176,   177,   179,
     186,   187,   189,   191,   192,   214,   416,   417,     9,   199,
     166,   170,   214,   329,   330,   331,   199,    83,   340,   254,
     311,   494,   494,    14,   255,   201,   306,   307,   494,    14,
      83,   358,   198,   197,   504,   196,   501,   361,   504,   305,
     201,   198,   440,   137,   137,    32,   234,   284,   285,   231,
     428,   428,   428,   201,   199,   199,   428,   419,   314,   517,
     322,   323,   427,   319,    14,    32,    51,   324,   327,     9,
      36,   198,    31,    50,    53,    14,     9,   199,   216,   495,
     340,    14,   517,   254,   199,    14,   358,    38,    83,   407,
     200,   502,   503,   517,   199,   200,   332,   504,   501,   201,
     504,   440,   440,   231,    99,   250,   201,   214,   227,   315,
     316,   317,     9,   434,     9,   434,   201,   428,   417,   417,
      68,   325,   330,   330,    31,    50,    53,   428,    83,   182,
     197,   199,   428,   495,   428,    83,     9,   435,   229,     9,
     435,    14,   505,   229,   200,   332,   332,    97,   199,   115,
     241,   161,   102,   517,   183,   427,   173,    14,   506,   312,
     197,    38,    83,   198,   201,   503,   517,   201,   229,   199,
     197,   179,   253,   214,   335,   336,   183,   428,   183,   296,
     297,   453,   313,    83,   201,   419,   251,   176,   214,   199,
     198,     9,   435,   123,   124,   125,   338,   339,   296,    83,
     281,   199,   504,   453,   518,   198,   198,   199,   501,   338,
      38,    83,   175,   504,   200,   199,   200,   333,   518,    83,
     175,    14,    83,   501,   231,   229,    38,    83,   175,    14,
      83,   358,   333,   201,   201,    83,   175,    14,    83,   358,
      14,    83,   358,   358
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
     418,   419,   419,   420,   420,   420,   420,   420,   421,   421,
     422,   422,   422,   422,   423,   423,   423,   424,   424,   424,
     425,   425,   425,   426,   426,   427,   427,   427,   427,   427,
     427,   427,   427,   427,   427,   427,   427,   427,   427,   427,
     428,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   428,   428,   428,   428,   429,   429,   430,   431,   431,
     432,   432,   432,   432,   432,   432,   432,   433,   433,   434,
     434,   435,   435,   436,   436,   436,   436,   437,   437,   437,
     437,   437,   438,   438,   438,   438,   439,   439,   440,   440,
     440,   440,   440,   440,   440,   440,   440,   440,   440,   440,
     440,   440,   440,   441,   441,   442,   442,   443,   443,   443,
     443,   444,   444,   445,   445,   446,   446,   447,   447,   448,
     448,   449,   449,   451,   450,   452,   453,   453,   454,   454,
     455,   455,   455,   456,   456,   457,   457,   458,   458,   459,
     459,   460,   460,   460,   461,   461,   462,   462,   463,   463,
     464,   464,   464,   464,   464,   464,   464,   464,   464,   464,
     464,   465,   466,   466,   466,   466,   466,   466,   466,   466,
     467,   467,   467,   467,   467,   467,   467,   467,   467,   468,
     469,   469,   470,   470,   471,   471,   471,   472,   473,   473,
     473,   474,   474,   474,   474,   475,   475,   476,   476,   476,
     476,   476,   476,   477,   477,   477,   477,   477,   478,   478,
     478,   478,   478,   478,   479,   479,   480,   480,   480,   480,
     480,   480,   480,   480,   481,   481,   482,   482,   482,   482,
     483,   483,   484,   484,   484,   484,   485,   485,   485,   485,
     486,   486,   486,   486,   486,   486,   487,   487,   487,   488,
     488,   488,   488,   488,   488,   488,   488,   488,   488,   488,
     489,   489,   490,   490,   491,   491,   492,   492,   492,   492,
     493,   493,   494,   494,   495,   495,   496,   496,   497,   497,
     498,   498,   499,   500,   500,   500,   500,   501,   501,   502,
     502,   503,   503,   504,   504,   505,   505,   506,   507,   507,
     508,   508,   508,   508,   509,   509,   509,   510,   510,   510,
     510,   511,   511,   512,   512,   512,   512,   513,   514,   515,
     515,   516,   516,   517,   517,   517,   517,   517,   517,   517,
     517,   517,   517,   517,   518,   518
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
       3,     2,     2,     2,     2,     4,     5,     5,     5,     5,
       1,     1,     1,     1,     1,     1,     3,     3,     4,     4,
       3,     3,     1,     1,     1,     1,     3,     1,     4,     3,
       1,     1,     1,     1,     1,     3,     3,     4,     4,     3,
       1,     1,     7,     9,     7,     6,     8,     1,     4,     4,
       1,     1,     1,     4,     2,     1,     0,     1,     1,     1,
       3,     3,     3,     0,     1,     1,     3,     3,     2,     3,
       6,     0,     1,     4,     2,     0,     5,     3,     3,     1,
       6,     4,     4,     2,     2,     0,     5,     3,     3,     1,
       2,     0,     5,     3,     3,     1,     2,     2,     1,     2,
       1,     4,     3,     3,     6,     3,     1,     1,     1,     4,
       4,     4,     4,     4,     4,     2,     2,     4,     2,     2,
       1,     3,     3,     3,     0,     2,     5,     6,     6,     7,
       1,     2,     1,     2,     1,     4,     1,     4,     3,     0,
       1,     3,     2,     3,     1,     1,     0,     0,     3,     1,
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
#line 752 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
#line 7194 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 755 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 7202 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 762 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 7208 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 763 "hphp.y" /* yacc.c:1646  */
    { }
#line 7214 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 766 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 7220 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 767 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7226 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 768 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7232 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 769 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7238 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 770 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7244 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 771 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 7250 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 772 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 7258 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 775 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 7265 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 7271 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 778 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7277 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 779 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 7283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 780 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 781 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7297 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 785 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7306 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 790 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7315 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 795 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7322 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 798 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7329 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 801 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7337 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 805 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7345 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 809 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7353 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 813 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7361 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 817 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7369 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 820 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7376 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 825 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7382 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 826 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7388 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 827 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7394 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 828 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 829 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 830 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 831 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7418 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 834 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7454 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 917 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 919 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7466 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 924 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7472 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 925 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7479 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 931 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 935 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7491 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 936 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7497 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 938 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7503 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 940 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7509 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 945 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7515 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 946 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7522 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 952 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7528 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 956 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7535 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 958 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7542 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 960 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7549 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 965 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7555 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 967 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7561 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 970 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7567 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 972 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7573 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 973 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7579 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 978 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7588 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 985 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7597 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 993 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7604 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 996 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7611 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 1002 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 7617 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1003 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval)); }
#line 7623 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1008 "hphp.y" /* yacc.c:1646  */
    {
                                         _p->onUsing((yyval), (yyvsp[-2]), true, (yyvsp[-1]), nullptr);
                                       }
#line 7631 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 1015 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7637 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1016 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7643 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1021 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 7649 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1022 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval), (yyval), (yyvsp[0])); }
#line 7656 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1027 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7662 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1028 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7668 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1029 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7674 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1030 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7680 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1033 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7686 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1037 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7692 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1042 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7698 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1043 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7705 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1045 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7713 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1049 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7720 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1052 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7728 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1056 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7735 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1058 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7743 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1061 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7750 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1063 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1066 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7764 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1067 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7770 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1068 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1069 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1070 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7788 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1071 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7794 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1072 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7800 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1073 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7806 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1074 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7812 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1075 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7818 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1076 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7824 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1077 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 7830 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1078 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 7836 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1079 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 7842 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1080 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 7849 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1084 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7856 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1086 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 7864 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1091 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7871 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1093 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 7879 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1097 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 7887 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1106 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7893 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1107 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 7899 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1110 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false); }
#line 7905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1111 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 7911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1113 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7919 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1117 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7927 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1121 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7935 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1125 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7943 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1129 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7951 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1133 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1138 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7967 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1141 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 7973 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1142 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 7982 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1146 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7988 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1147 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7994 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1148 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8000 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1149 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8006 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1150 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8012 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1151 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8018 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1152 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 8024 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1153 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8030 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1154 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8036 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1155 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 8042 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1156 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8048 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1157 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 8058 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1179 "hphp.y" /* yacc.c:1646  */
    { _p->pushLabelScope(LS::Using);
                                         _p->onNewLabelScope(false);
                                         (yyval) = (yyvsp[-1]); }
#line 8066 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1185 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1; }
#line 8072 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1186 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8078 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1195 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), nullptr, (yyvsp[-2]));
                                         _p->onExprListElem((yyval), &(yyval), (yyvsp[0])); }
#line 8085 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1197 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0])); }
#line 8091 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 1207 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8097 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 1208 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8103 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1212 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false); }
#line 8109 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1213 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8115 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1222 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 8121 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1223 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8127 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1227 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::Finally);}
#line 8134 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1229 "hphp.y" /* yacc.c:1646  */
    { _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->popLabelScope();
                                         _p->onCompleteLabelScope(false);}
#line 8142 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1235 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8148 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1236 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8154 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1240 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 8160 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1241 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8166 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1245 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 8172 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1251 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8181 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1258 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8191 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1266 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8200 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1273 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8210 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1281 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8219 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1287 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8229 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1296 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8236 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1300 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 8242 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1304 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8249 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1308 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 8255 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1314 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8262 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 8280 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1332 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8287 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 8305 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1349 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8312 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 1352 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8320 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1357 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8327 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1360 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8335 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1366 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 8341 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1369 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 8347 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1373 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8354 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1376 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8365 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1384 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8372 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1387 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8383 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1395 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8389 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1396 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 8396 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1400 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8402 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1403 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8408 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1406 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 8414 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1407 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 8420 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1408 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 8428 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1411 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 8434 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1412 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 8440 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1416 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8446 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1417 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8452 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1420 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8458 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1421 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8464 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1424 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8470 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1425 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8476 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1428 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 8482 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1430 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8488 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1433 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 8494 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1435 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8500 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1439 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8506 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1440 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8512 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1443 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8518 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1444 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8524 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1445 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8530 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1449 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8536 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1451 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8542 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1454 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8548 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1456 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8554 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8560 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1461 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8566 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1464 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8572 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1466 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8578 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1470 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8584 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1472 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8591 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1477 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8597 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1478 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8603 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1479 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8609 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1480 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8615 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1485 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8621 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1487 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8627 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1488 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8633 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1491 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8639 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1492 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8645 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1497 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8651 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1498 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8657 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1503 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8663 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1504 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8669 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1507 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8675 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1508 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8681 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1511 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8687 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1512 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8693 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1520 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8700 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1526 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8707 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1532 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8715 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1536 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8721 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1540 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8728 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1545 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8735 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1550 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8743 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1553 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8749 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1559 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8756 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1563 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8763 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1568 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8770 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1573 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8777 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1578 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8784 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1583 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8791 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1589 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8798 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1595 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8805 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1603 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 8812 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1608 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 8819 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1613 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 8827 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1617 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8833 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1620 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 8840 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1624 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 8847 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1628 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 8855 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1631 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8861 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1636 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8868 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1639 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1643 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8882 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1647 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8889 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1651 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8896 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1655 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8903 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1660 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8910 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1665 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8917 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1671 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8923 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1672 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1675 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,false);}
#line 8935 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1676 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),true,false);}
#line 8941 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1677 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,true);}
#line 8947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1679 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),false, false);}
#line 8953 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1681 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),false,true);}
#line 8959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1683 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),true, false);}
#line 8965 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1687 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1688 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 8977 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1691 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1692 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1693 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 8995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1697 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9001 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1699 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9007 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1700 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 9013 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1701 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1706 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1707 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1710 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9038 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1715 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9044 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1721 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9050 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1722 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9056 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1725 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[0]));}
#line 9062 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1726 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 9069 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1729 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[-1]));}
#line 9075 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1730 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-3]));}
#line 9082 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1732 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9089 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1735 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL, true);}
#line 9096 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1737 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9102 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 359:
#line 1740 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9110 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1747 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9120 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1755 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9128 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1762 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9138 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1768 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 9144 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1770 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9150 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1772 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9156 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1774 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 9162 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1776 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 9168 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1777 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 9175 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1780 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 9181 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1783 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9187 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1784 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9193 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1785 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 9199 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1791 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 9205 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1796 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 9212 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1799 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 9220 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1806 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 9226 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1807 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 9233 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1812 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 9240 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1815 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 9246 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1822 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 9253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1824 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 9259 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1828 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9265 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9271 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9277 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1838 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 9294 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1844 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 9300 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1846 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 9306 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1847 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 9312 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1851 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,0,(yyvsp[0]),0);}
#line 9318 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 1853 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),0,(yyvsp[0]),0);}
#line 9324 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1858 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9330 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 394:
#line 1861 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9336 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1862 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 9342 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1866 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 9348 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1867 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 9354 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1871 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 9361 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1874 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 9368 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1879 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 9375 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1884 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 9381 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1885 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 9388 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1887 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 9394 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1891 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 9400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1892 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 9406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1893 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 9412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1894 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 9418 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1898 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1899 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 9430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1900 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 9436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1901 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 9442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1902 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 9448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1904 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 9454 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1906 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 9460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1910 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 9468 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1913 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 9474 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1914 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 9480 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1918 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9486 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1919 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9492 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1923 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9498 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1924 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9504 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1927 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9510 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1928 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9516 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1931 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9522 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1932 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9528 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1935 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9534 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1937 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9540 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1940 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9546 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1941 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9552 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1942 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9558 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1943 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9564 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1944 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9570 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1945 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9576 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 1946 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9582 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 1950 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9588 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 1951 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9594 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 1954 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9600 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 1955 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9606 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 1956 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9612 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 1960 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9618 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 1962 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9624 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9630 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 1964 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9636 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 1968 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9642 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 1970 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9648 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 1974 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9654 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 1976 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9660 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 1980 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9668 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 1984 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9675 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 1988 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9681 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 1992 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9687 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 1996 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9693 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 1998 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9699 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 1999 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9705 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9711 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 2001 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9717 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 2002 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9723 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 2005 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9729 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 2006 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9735 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 2010 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9741 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 2011 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9747 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 2015 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 9753 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 2016 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 9759 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 9765 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 2018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9771 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 2022 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9777 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 2027 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9783 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 2031 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 9789 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 2035 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9795 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 2039 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 9801 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 2043 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9807 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 2048 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9813 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 2052 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9819 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 2056 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9825 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 2057 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9831 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 2058 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9837 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 2059 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9843 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 2060 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9849 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 2064 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9855 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 2069 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 9861 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 2070 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 9867 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 2071 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 9873 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 2074 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 9879 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 2075 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 9885 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 2076 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 9891 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 2077 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 9897 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 2078 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 9903 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 2079 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 9909 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 2080 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 9915 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 2081 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 9921 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 2082 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 9927 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 2083 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 9933 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 2084 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 9939 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 2085 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 9945 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 2086 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 9951 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 2087 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 9957 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 2088 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 9963 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 2089 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 9969 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 2090 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 9975 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 2091 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 9981 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 2092 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 9987 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 2093 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 9993 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2094 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 9999 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2095 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 10005 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2096 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 10011 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2097 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 10017 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2098 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 10023 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2099 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 10029 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2100 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 10035 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2101 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 10041 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2102 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 10047 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2103 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 10053 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2104 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 10059 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2105 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 10065 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2106 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 10071 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2107 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 10077 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2108 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 10083 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2109 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 10089 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2110 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 10095 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2111 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 10101 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2112 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 10107 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2113 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 10113 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2114 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 10119 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2115 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 10125 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2116 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 10131 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2117 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 10137 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2118 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 10144 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2120 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 10150 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2121 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 10157 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2123 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 10163 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2125 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 10169 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2126 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10175 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2127 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 10181 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2128 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 10187 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2129 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 10193 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2130 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10199 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2131 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 10205 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2132 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 10211 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2133 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 10217 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2134 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 10223 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2135 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 10229 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2136 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 10235 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2137 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 10241 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2138 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 10247 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2139 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 10253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2140 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10259 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2141 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10265 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2142 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10271 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2143 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10277 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2144 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2145 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2146 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10295 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2147 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10301 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2148 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 10307 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2149 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 10313 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2150 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10319 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2157 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 10325 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2158 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2163 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10340 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10352 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2178 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10361 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10373 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10386 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10401 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2214 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10411 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10426 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2233 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10436 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10453 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10466 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10479 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2267 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10489 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10502 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2286 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10508 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2287 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10514 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2289 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10520 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
#line 2293 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10527 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2295 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10533 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
#line 2302 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10539 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
#line 2305 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10545 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
#line 2312 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10551 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2315 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10557 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
#line 2320 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10563 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2321 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10569 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2326 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10575 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2327 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10581 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2331 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval), (yyvsp[-1]), T_ARRAY);}
#line 10587 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2335 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10593 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2336 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10599 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2341 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10605 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2342 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10611 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2347 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10617 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2348 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10623 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2353 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10629 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2354 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10635 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2360 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10641 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2362 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10647 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2367 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10653 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2368 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10659 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2374 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10665 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2376 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10671 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2380 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10677 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2384 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10683 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2388 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10689 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2392 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10695 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2396 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10701 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2400 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10707 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2404 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10713 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2408 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10719 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2412 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10725 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2416 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10731 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2420 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10737 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2424 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10743 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2428 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10749 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2432 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10755 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2436 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10761 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2441 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10767 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2442 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10773 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2447 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10779 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2448 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10785 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2453 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10791 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2454 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10797 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2459 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10805 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2466 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10813 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2473 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10819 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2475 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10825 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2479 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10831 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2480 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10837 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2481 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10843 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2482 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10849 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10855 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2484 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10861 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2485 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10867 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2486 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10873 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2487 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10880 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2489 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10886 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2490 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10892 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2494 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10898 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2495 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 10904 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2496 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 10910 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2497 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 10916 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2504 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 10922 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10936 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10950 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2529 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 10956 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2530 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 10962 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2535 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10968 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2536 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10974 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2539 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-1]),0,(yyvsp[0]),0);}
#line 10980 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2540 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10986 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2543 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10993 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2547 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11001 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2550 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11007 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 11019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2560 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2561 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2565 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11037 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2567 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 11043 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2569 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 11049 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2573 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11055 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2574 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11061 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2575 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11067 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2576 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11073 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2577 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11079 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2578 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11085 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2579 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11091 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2580 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11097 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2581 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11103 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2582 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11109 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2583 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11115 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2584 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11121 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2585 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11127 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2586 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11133 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2587 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11139 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2588 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11145 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2589 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11151 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2590 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11157 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2591 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11163 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2592 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11169 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2593 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11175 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2594 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11181 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2595 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11187 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2596 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11193 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2597 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11199 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2598 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11205 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2599 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11211 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2600 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11217 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2601 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11223 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2602 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11229 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2603 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11235 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2604 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11241 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2605 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11247 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2606 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2607 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11259 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2608 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11265 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2609 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11271 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2610 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11277 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2611 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2612 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2613 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11295 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2614 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11301 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2615 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11307 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2616 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11313 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2617 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11319 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2618 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11325 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2619 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2620 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11337 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2621 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11343 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2622 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11349 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2623 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11355 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2624 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11361 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2625 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11367 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2626 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11373 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2627 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11379 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2628 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11385 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2629 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11391 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2630 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11397 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2631 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11403 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2632 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11409 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2633 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11415 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2634 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11421 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2635 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11427 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2636 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11433 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2637 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11439 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2638 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11445 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2639 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11451 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2640 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11457 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2641 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11463 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2642 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11469 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2643 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11475 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2644 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11481 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2645 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11487 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2646 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11493 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2647 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11499 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2648 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11505 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2649 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11511 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2650 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11517 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2651 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11523 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2652 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11529 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2653 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11535 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2658 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11541 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2662 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11547 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2663 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11553 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2667 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11559 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2668 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11565 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2669 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11571 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2670 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11578 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2672 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11585 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2676 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11591 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2685 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11597 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2688 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11603 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2689 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11610 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2691 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11617 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2701 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11623 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2705 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11629 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2706 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11635 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2707 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11641 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2711 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11647 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2712 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11653 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2713 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11659 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2717 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11665 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2718 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11671 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2719 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11677 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2723 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11683 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2724 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11689 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2728 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11695 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2729 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11701 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2730 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11707 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2731 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 11714 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2733 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 11720 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2734 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 11726 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2735 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 11732 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2736 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 11738 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2737 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 11744 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2738 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 11750 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2739 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 11756 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2740 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 11762 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2741 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 11768 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2744 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11774 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2746 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11780 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2750 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11786 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2751 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11792 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2753 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11798 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2754 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11804 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2756 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11810 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2757 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11816 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2758 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11822 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2759 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11828 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2760 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11834 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2761 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11840 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2762 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11846 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2763 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11852 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2764 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11858 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2766 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 11864 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2768 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 11870 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2770 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 11876 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2772 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 11882 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2774 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 11888 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2775 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 11894 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2776 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 11900 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2777 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 11906 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2778 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 11912 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2779 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 11918 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2780 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 11924 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2781 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 11930 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2782 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 11936 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2783 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 11942 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2784 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 11948 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2785 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 11954 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2786 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 11960 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2787 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 11966 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2788 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 11972 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2789 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11978 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2790 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11984 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2792 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 11990 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2794 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 11996 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2796 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 12002 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2798 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 12008 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2799 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 12014 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2801 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 12021 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2803 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 12027 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2806 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 12034 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2810 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 12040 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2813 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 12046 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2814 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 12052 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2818 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12058 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2819 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12064 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2825 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12070 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2831 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12076 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12082 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2836 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12088 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2837 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12094 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 832:
#line 2838 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12100 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2839 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12106 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2840 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 12112 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2841 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 12118 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2843 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 12125 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2848 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12131 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2849 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12137 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2853 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12143 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2854 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12149 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2857 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 12155 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2858 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12161 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2864 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12167 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2866 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12173 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2868 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12179 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2869 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12185 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2873 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 12191 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2874 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 12197 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2875 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 12203 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2878 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12209 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2880 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12215 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2883 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 12221 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2884 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 12227 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2885 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 12233 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2886 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12239 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2890 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 12246 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2893 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 12254 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 2900 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12260 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2901 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12266 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2904 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 12274 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2907 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12280 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2908 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12286 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2909 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12292 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2911 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12298 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2912 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12304 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2914 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12310 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2915 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12316 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2916 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12322 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 2917 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12328 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2918 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12334 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2919 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12340 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2924 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12346 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2925 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12352 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2930 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12358 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 2931 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12364 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 877:
#line 2936 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12370 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 2938 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12376 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 2940 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12382 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 2941 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12388 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 2945 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12394 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 2946 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 2951 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 2952 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 2957 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12418 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
#line 2960 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 2965 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 2966 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 2969 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 2970 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 12449 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 2977 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 12455 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 2979 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 12461 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 2982 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 12467 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 2984 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12473 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 2987 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12479 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 2990 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 2991 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12491 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 2995 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 12497 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 2996 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 12503 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 3000 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12509 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3001 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 12515 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3002 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 12521 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3006 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12527 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3011 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12533 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3016 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12539 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3017 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12545 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3021 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12551 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3026 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12557 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3031 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12563 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3032 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12569 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3037 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12575 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3038 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12581 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3040 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12587 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3045 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12593 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3047 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12599 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3053 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12613 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3064 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12627 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3079 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12641 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3091 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12655 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3103 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12661 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3104 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12667 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3105 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12673 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3106 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12679 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3107 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12685 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3108 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12691 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3110 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12705 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3127 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12711 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3129 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12717 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3131 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12723 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3132 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12729 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3136 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12735 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3140 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12741 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3141 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12747 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3142 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12753 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3143 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12759 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3151 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12773 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
#line 3160 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12779 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
#line 3162 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12785 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3166 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12791 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
#line 3171 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12797 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
#line 3172 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12803 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3173 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12809 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3174 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12815 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
#line 3175 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12821 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3176 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12827 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
#line 3177 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12833 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
#line 3179 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12839 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
#line 3181 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12845 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3185 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12851 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3189 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12857 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3190 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12863 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3196 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12869 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3200 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3207 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 12881 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3216 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 12887 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3220 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 12893 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3224 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12899 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3233 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3234 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3235 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12917 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3239 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12923 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3240 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 12929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3241 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 12935 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3243 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]) = 1; _p->onIndirectRef((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12941 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3248 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3249 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12953 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3260 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3261 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12965 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3262 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3265 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12985 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3276 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12991 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3277 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12997 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3281 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13003 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3282 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13009 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3285 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13023 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3294 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13029 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3298 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 13035 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3299 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 13041 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3301 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 13047 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3302 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 13053 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3303 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 13059 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3304 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 13065 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3309 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13071 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3310 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13077 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3314 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13083 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3315 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13089 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3316 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13095 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3317 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13101 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3320 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13107 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3322 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 13113 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3323 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13119 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3324 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 13125 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3329 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13131 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 995:
#line 3330 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13137 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3334 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13143 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3335 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13149 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
#line 3336 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13155 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3337 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13161 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3342 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13167 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3343 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13173 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3348 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13179 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3350 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13185 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1004:
#line 3352 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13191 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1005:
#line 3353 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13197 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1006:
#line 3357 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 13203 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1007:
#line 3359 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 13209 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1008:
#line 3360 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 13215 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3362 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 13222 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3367 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13228 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3369 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13234 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3371 "hphp.y" /* yacc.c:1646  */
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[-2]),
                                           !(yyvsp[-1]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[0])
                                         );
                                       }
#line 13248 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3381 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 13254 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3383 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 13260 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3384 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13266 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3387 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 13272 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3388 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 13278 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3389 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 13284 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3393 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 13290 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3394 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 13296 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3395 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13302 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3396 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13308 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3397 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13314 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3398 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13320 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3399 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 13326 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3400 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 13332 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1027:
#line 3401 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 13338 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1028:
#line 3402 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 13344 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1029:
#line 3403 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 13350 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1030:
#line 3407 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 13356 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1031:
#line 3408 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 13362 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1032:
#line 3413 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13368 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1033:
#line 3415 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13374 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1036:
#line 3429 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13382 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1037:
#line 3434 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 13390 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1038:
#line 3438 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13398 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1039:
#line 3443 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 13406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1040:
#line 3449 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1041:
#line 3450 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13418 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1042:
#line 3454 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3455 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3461 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3465 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 13442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3471 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3475 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 13455 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3482 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13461 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3483 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13467 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3487 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 13475 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3490 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 13482 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3496 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13488 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3501 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]); }
#line 13494 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3502 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13500 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1055:
#line 3503 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13506 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1056:
#line 3504 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13512 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3525 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13518 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3526 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13524 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3535 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13530 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3546 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13536 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3548 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13542 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3552 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13548 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3555 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13554 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3559 "hphp.y" /* yacc.c:1646  */
    {}
#line 13560 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3560 "hphp.y" /* yacc.c:1646  */
    {}
#line 13566 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1076:
#line 3561 "hphp.y" /* yacc.c:1646  */
    {}
#line 13572 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1077:
#line 3567 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13579 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1078:
#line 3572 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13589 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1079:
#line 3581 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13595 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1080:
#line 3587 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13604 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1081:
#line 3595 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 13610 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1082:
#line 3596 "hphp.y" /* yacc.c:1646  */
    { }
#line 13616 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1083:
#line 3602 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 13622 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1084:
#line 3604 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 13628 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1085:
#line 3605 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 13638 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1086:
#line 3610 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 13645 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1087:
#line 3616 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("array"); }
#line 13652 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1088:
#line 3621 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13658 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1089:
#line 3626 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 13666 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1090:
#line 3630 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13672 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1091:
#line 3635 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 13678 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1092:
#line 3637 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 13684 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1093:
#line 3643 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 13691 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1094:
#line 3645 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 13699 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1095:
#line 3648 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13705 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1096:
#line 3649 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13713 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1097:
#line 3652 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13721 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1098:
#line 3655 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13727 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1099:
#line 3658 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 13735 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1100:
#line 3661 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13742 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1101:
#line 3663 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 13751 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1102:
#line 3669 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 13760 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1103:
#line 3675 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 13770 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1104:
#line 3683 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1105:
#line 3684 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;


#line 13786 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 3687 "hphp.y" /* yacc.c:1906  */

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}
