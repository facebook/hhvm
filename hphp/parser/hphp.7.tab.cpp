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
#define YYLAST   19740

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  207
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  312
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1106
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  2055

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
    2997,  3001,  3002,  3003,  3007,  3012,  3017,  3018,  3022,  3027,
    3032,  3033,  3037,  3039,  3040,  3045,  3047,  3052,  3063,  3077,
    3089,  3104,  3105,  3106,  3107,  3108,  3109,  3110,  3120,  3129,
    3131,  3133,  3137,  3141,  3142,  3143,  3144,  3145,  3161,  3162,
    3165,  3172,  3173,  3174,  3175,  3176,  3177,  3178,  3179,  3181,
    3186,  3190,  3191,  3195,  3198,  3205,  3209,  3218,  3225,  3233,
    3235,  3236,  3240,  3241,  3242,  3244,  3249,  3250,  3261,  3262,
    3263,  3264,  3275,  3278,  3281,  3282,  3283,  3284,  3295,  3299,
    3300,  3301,  3303,  3304,  3305,  3309,  3311,  3314,  3316,  3317,
    3318,  3319,  3322,  3324,  3325,  3329,  3331,  3334,  3336,  3337,
    3338,  3342,  3344,  3347,  3350,  3352,  3354,  3358,  3359,  3361,
    3362,  3368,  3369,  3371,  3381,  3383,  3385,  3388,  3389,  3390,
    3394,  3395,  3396,  3397,  3398,  3399,  3400,  3401,  3402,  3403,
    3404,  3408,  3409,  3413,  3415,  3423,  3425,  3429,  3433,  3438,
    3442,  3450,  3451,  3455,  3456,  3462,  3463,  3472,  3473,  3481,
    3484,  3488,  3491,  3496,  3501,  3503,  3504,  3505,  3508,  3510,
    3516,  3517,  3521,  3522,  3526,  3527,  3531,  3532,  3535,  3540,
    3541,  3545,  3548,  3550,  3554,  3560,  3561,  3562,  3566,  3570,
    3578,  3583,  3595,  3597,  3601,  3604,  3606,  3611,  3616,  3622,
    3625,  3630,  3635,  3637,  3644,  3646,  3649,  3650,  3653,  3656,
    3657,  3662,  3664,  3668,  3674,  3684,  3685
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

#define YYPACT_NINF -1672

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1672)))

#define YYTABLE_NINF -1090

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1090)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1672,   183, -1672, -1672,  5190, 15000, 15000,   -41, 15000, 15000,
   15000, 15000, 12348, 15000, -1672, 15000, 15000, 15000, 15000, 18230,
   18230, 15000, 15000, 15000, 15000, 15000, 15000, 15000, 15000, 12552,
   18907, 15000,    -9,    23, -1672, -1672, -1672,   170, -1672,   215,
   -1672, -1672, -1672,   188, 15000, -1672,    23,   178,   219,   222,
   -1672,    23, 12756, 16806, 12960, -1672, 15883, 11124,    -8, 15000,
    2839,   166,   107,   457,   444, -1672, -1672, -1672,   225,   246,
     249,   295, -1672, 16806,   365,   400,   288,   390,   428,   540,
     548, -1672, -1672, -1672, -1672, -1672, 15000,   542,  2292, -1672,
   -1672, 16806, -1672, -1672, -1672, -1672, 16806, -1672, 16806, -1672,
     482,   462, 16806, 16806, -1672,   347, -1672, -1672, 13164, -1672,
   -1672,   367,   629,   649,   649, -1672,   622,   518,   389,   489,
   -1672,    98, -1672,   497,   587,   677, -1672, -1672, -1672, -1672,
   15423,   793, -1672,   102, -1672,   525,   527,   544,   556,   567,
     577,   579,   584, 13352, -1672, -1672, -1672, -1672, -1672,   164,
     647,   715,   743,   758,   762, -1672,   770,   775, -1672,   204,
     646, -1672,   678,   384, -1672,  1897,   174, -1672, -1672,  1912,
     102,   102,   650,   172, -1672,   134,   101,   653,   199, -1672,
   -1672,   783, -1672,   694, -1672, -1672,   666,   691, -1672, 15000,
   -1672,   677,   793, 19407,  2694, 19407, 15000, 19407, 19407, 19671,
   19671,   665, 17786, 19407,   817, 16806,   798,   798,   639,   798,
   -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672,    71,
   15000,   687, -1672, -1672,   709,   689,   610,   702,   610,   798,
     798,   798,   798,   798,   798,   798,   798, 18230, 18400,   670,
     866,   694, -1672, 15000,   687, -1672,   717, -1672,   721,   704,
   -1672,   138, -1672, -1672, -1672,   610,   102, -1672, 13368, -1672,
   -1672, 15000,  9696,   880,    99, 19407, 10716, -1672, 15000, 15000,
   16806, -1672, -1672, 14984,   710, -1672, 17082, -1672, -1672, -1672,
   -1672, -1672, -1672, -1672, -1672, -1672, -1672, 17365, -1672, 17365,
   -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672,
   -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672,
   -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672,
   -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672,
   -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672,    88,    97,
     691, -1672, -1672, -1672, -1672,   722,  3797,   117, -1672, -1672,
     751,   906, -1672,   763, 16606, 15000, -1672,   723,   724, 17130,
   -1672,    62, 17178, 16500, 16500, 16500, 16806, 16500,   726,   918,
     729, -1672,    85, -1672, 17814,   110, -1672,   919,   118,   801,
   -1672,   803, -1672, 18230, 15000, 15000,   737,   754, -1672, -1672,
   17918, 12552, 15000, 15000, 15000, 15000, 15000,   123,    66,   530,
   -1672, 15204, 18230,   562, -1672, 16806, -1672,   228,   518, -1672,
   -1672, -1672, -1672, 19008,   927,   841, -1672, -1672, -1672,    82,
   15000,   747,   753, 19407,   755,  2303,   757,  5820, 15000, -1672,
     480,   746,   652,   480,   532,   512, -1672, 16806, 17365,   759,
   11328, 15883, -1672, 13572,   752,   752,   752,   752, -1672, -1672,
    3349, -1672, -1672, -1672, -1672, -1672,   677, -1672, 15000, 15000,
   -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, 15000,
   15000, 15000, 15000, 13776, 15000, 15000, 15000, 15000, 15000, 15000,
   15000, 15000, 15000, 15000, 15000, 15000, 15000, 15000, 15000, 15000,
   15000, 15000, 15000, 15000, 15000, 15000, 15000, 19109, 15000, -1672,
   15000, 15000, 15000,  4826, 16806, 16806, 16806, 16806, 16806, 15423,
     848,  1080, 10920, 15000, 15000, 15000, 15000, 15000, 15000, 15000,
   15000, 15000, 15000, 15000, 15000, -1672, -1672, -1672, -1672, 19151,
   -1672, -1672, 11328, 11328, 15000, 15000, 17918,   765,   677, 13980,
    4719, -1672, 15000, -1672,   766,   951,   807,   768,   771, 15376,
     610, 14184, -1672, 14388, -1672,   704,   776,   780,  2317, -1672,
     122, 11328, -1672, 19186, -1672, -1672, 17249, -1672, -1672, 11532,
   -1672, 15000, -1672,   873,  9900,   964,   781, 15188,   965,    96,
      84, -1672, -1672, -1672,   800, -1672, -1672, -1672, 17365, -1672,
    1226,   786,   977, 17710, 16806, -1672, -1672, -1672, -1672, -1672,
   -1672, -1672, -1672, -1672,   796, -1672, -1672,   802,   806,   808,
     810,   812,   811,   120,   814,   818,  4708, 16653, -1672, -1672,
   16806, 16806, 15000,   610,   166, -1672, 17710,   917, -1672, -1672,
   -1672,   610,   141,   142,   809,   820,  2604,   223,   821,   822,
     690,   883,   825,   610,   143,   826, 18448,   823,  1019,  1021,
     844,   852,   853,   854, -1672,  5043, 16806, -1672, -1672,   975,
    3034,    59, -1672, -1672, -1672,   518, -1672, -1672, -1672,  1030,
     929,   887,   267,   903, 15000,   932,  1062,   875, -1672,   913,
   -1672,   155, -1672, 17365, 17365,  1061,   880,    82, -1672,   890,
    1072, -1672, 17365,    77, -1672,   388,   181, -1672, -1672, -1672,
   -1672, -1672, -1672, -1672,  1594,  3238, -1672, -1672, -1672, -1672,
    1079,   907, -1672, 18230, 15000,   895,  1089, 19407,  1090,   146,
    1097,   912,   922,   926, 19407,   928,  3104,  6024, -1672, -1672,
   -1672, -1672, -1672, -1672,   978,  4302, 19407,   925,  4017, 12945,
   19590, 19671, 15707, 15000, 19359, 15884, 12531, 14162, 14569,  3769,
   15372, 16053, 16053, 16053, 16053,  2547,  2547,  2547,  2547,  2547,
    1246,  1246,   761,   761,   761,   639,   639,   639, -1672,   798,
     930,   931, 18508,   935,  1128,   411, 15000,   485,   687,   209,
   -1672, -1672, -1672,  1127,   841, -1672,   677, 18022, -1672, -1672,
   -1672, 19671, 19671, 19671, 19671, 19671, 19671, 19671, 19671, 19671,
   19671, 19671, 19671, 19671, -1672, 15000,   488, -1672,   156, -1672,
     687,   517,   944,   949,   946,  4271,   149,   957, -1672, 19407,
    4033, -1672, 16806, -1672,   610,   386, 18230, 19407, 18230, 18556,
     978,    92,   610,   162, -1672,   155,   987,   960, 15000, -1672,
     167, -1672, -1672, -1672,  6228,   682, -1672, -1672, 19407, 19407,
      23, -1672, -1672, -1672, 15000,  1057, 17585, 17710, 16806, 10104,
     962,   963, -1672,  1154, 16305,  1028, -1672,  1005, -1672,  1159,
     973, 17172, 17365, 17710, 17710, 17710, 17710, 17710,   976,  1102,
    1104,  1105,  1110,  1112,   988, 17710,    35, -1672, -1672, -1672,
   -1672, -1672, -1672,    19, -1672, 19501, -1672, -1672,    49, -1672,
    6432,  4446,   984, 16653, -1672, 16653, -1672, 16653, -1672, 16806,
   16806, 16653, -1672, 16653, 16653, 16806, -1672,  1179,   990, -1672,
     271, -1672, -1672,  4526, -1672, 19501,  1175, 18230,   993, -1672,
   -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672,  1010,  1188,
   16806,  4446,  1014, 17918, 18126,  1203, -1672, 15000, -1672, 15000,
   -1672, 15000, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672,
    1015, -1672, 15000, -1672, -1672,  5412, -1672, 17365,  4446,  1018,
   -1672, -1672, -1672, -1672,  1207,  1026, 15000, 19008, -1672, -1672,
    4826,  1027, -1672, 17365, -1672,  1042,  6636,  1205,    45, -1672,
   -1672,    91, 19151, 19186, -1672, 17365, -1672, -1672,   610, 19407,
   -1672, 11736, -1672, 17710, 13572,   752, 13572, -1672,   752,   752,
   -1672, 11940, -1672, -1672,  6840, -1672,    67,  1041,  4446,   929,
   -1672, -1672, -1672, -1672, 15884, 15000, -1672, -1672, 15000, -1672,
   15000, -1672,  4671,  1044, 11328,   883,  1214,   929, 17365,  1225,
     978, 16806, 19109,   610,  5007,  1055,   185,  1058, -1672, -1672,
    1242,  3813,  3813,  4033, -1672, -1672, -1672,  1209,  1066,  1199,
    1206,  1208,  1210,  1211,   113,  1076,   545, -1672, -1672, -1672,
   -1672, -1672,  1114, -1672, -1672, -1672, -1672,  1268,  1085,   766,
     610,   610, 14592,   929, 19186, -1672, -1672,  5058,   685,    23,
   10716, -1672,  7044,  1086,  7248,  1091, 17585, 18230,  1087,  1152,
     610, 19501,  1277, -1672, -1672, -1672, -1672,   570, -1672,   301,
   17365,  1111,  1156,  1134, 17365, 16806,  3518, -1672, -1672, -1672,
    1296, -1672,  1108,  1079,   523,   523,  1239,  1239, 17547,  1107,
    1300, 17710, 17710, 17710, 17710, 17710, 17710, 19008,  2502, 16759,
   17710, 17710, 17710, 17710, 17465, 17710, 17710, 17710, 17710, 17710,
   17710, 17710, 17710, 17710, 17710, 17710, 17710, 17710, 17710, 17710,
   17710, 17710, 17710, 17710, 17710, 17710, 17710, 17710, 16806, -1672,
   -1672,  1230, -1672, -1672,  1113,  1115,  1117, -1672,  1118, -1672,
   -1672,   431,  4708, -1672,  1121, -1672, 17710,   610, -1672, -1672,
     147, -1672,   661,  1315, -1672, -1672,   150,  1129,   610, 12144,
   19407, 18616, -1672,  2725, -1672,  5616,   841,  1315, -1672,   257,
     217, -1672, 19407,  1190,  1131, -1672,  1135,  1205, -1672, -1672,
   -1672, 14796, 17365,   880, 17365,    86,  1316,  1250,   202, -1672,
     687,   205, -1672, -1672, 18230, 15000, 19407, 19501, -1672, -1672,
   -1672,  3390, -1672, -1672, -1672, -1672, -1672, -1672,  1141,    67,
   -1672,  1145,    67,  1150, 15884, 19407, 18664,  1151, 11328,  1163,
    1149, 17365,  1157,  1153, 17365,   929, -1672,   704,   534, 11328,
   15000, -1672, -1672, -1672, -1672, -1672, -1672,  1217,  1160,  1347,
    1272,  4033,  4033,  4033,  4033,  4033,  4033,  1215, -1672, 19008,
     105,  4033, -1672, -1672, -1672, 18230, 19407,  1165, -1672,    23,
    1342,  1299, 10716, -1672, -1672, -1672,  1178, 15000,  1152,   610,
   17918, 17585,  1180, 17710,  7452,   644,  1181, 15000,   144,   319,
   -1672,  1196, -1672, 17365, 16806, -1672,  1248, -1672, -1672, 17317,
    1349,  1185, 17710, -1672, 17710, -1672,  1189,  1192,  1384, 18720,
    1198, 19501,  1394,  1200,  1201,  1204,  1273,  1403,  1216, -1672,
   -1672, -1672, 18768,  1219,  1406, 19546, 19634, 11308, 17710, 19455,
   13959, 14366,  3100, 13752, 16411, 16607, 16607, 16607, 16607,  2909,
    2909,  2909,  2909,  2909,  1158,  1158,   523,   523,   523,  1239,
    1239,  1239,  1239, -1672,  1220, -1672,  1224,  1233,  1234,  1237,
   -1672, -1672, 19501, 16806, 17365, 17365, -1672,   661,  4446,   769,
   -1672, 17918, -1672, -1672, 19671, 15000,  1228, -1672,  1243,  1685,
   -1672,   125, 15000, -1672, -1672, -1672, 15000, -1672, 15000, -1672,
     880, 13572,  1245,   218,   752,   218,   203, -1672, -1672,   135,
    1411,  1359, 15000, -1672,  1251,   610, 19407,  1205,  1247, -1672,
    1252,    67, 15000, 11328,  1254, -1672, -1672,   841, -1672, -1672,
    1255,  1249,  1256, -1672,  1257,  4033, -1672,  4033, -1672, -1672,
    1258,  1253,  1449,  1322,  1259, -1672,  1452,  1260,  1262,  1264,
   -1672,  1325,  1271,  1463, -1672, -1672,   610, -1672,  1441, -1672,
    1276, -1672, -1672,  1286,  1288,   159, -1672, -1672, 19501,  1289,
    1293, -1672,  3146, -1672, -1672, -1672, -1672, -1672, -1672,  1356,
   17365, -1672, 17365, -1672, 19501, 18824, -1672, -1672, 17710, -1672,
   17710, -1672, 17710, -1672, -1672, -1672, -1672, 17710, 19008, -1672,
   -1672, 17710, -1672, 17710, -1672, 11716, 17710,  1295,  7656, -1672,
   -1672, -1672, -1672,   661, -1672, -1672, -1672, -1672,   636, 16060,
    4446,  1386, -1672,  3338,  1330,  4080, -1672, -1672, -1672,   848,
   17239,   124,   126,  1302,   841,  1080,   161, 19407, -1672, -1672,
   -1672,  1344, 12332, 12740, 19407, -1672,  3339, -1672,  6024,   356,
    1498,  1430, 15000, -1672, 19407, 11328,  1396,  1205,  1716,  1205,
    1318, 19407,  1321, -1672,  1776,  1320,  2090, -1672, -1672,    67,
   -1672, -1672,  1385, -1672, -1672,  4033, -1672,  4033, -1672,  4033,
   -1672, -1672, -1672, -1672,  4033, -1672, 19008, -1672,  2188, -1672,
    7860, -1672, -1672, -1672, -1672, 10308, -1672, -1672, -1672,  6228,
   17365, -1672,  1327, 17710, 18872, 19501, 19501, 19501,  1389, 19501,
   18928, 11716, -1672, -1672,   661,  4446,  4446, 16806, -1672,  1513,
   16912,    81, -1672, 16060,   841, 16132, -1672,  1346, -1672,   128,
    1329,   129, -1672, 16415, -1672, -1672, -1672,   130, -1672, -1672,
    1336, -1672,  1335, -1672,  1455,   677, -1672, 16238, -1672, 16238,
   -1672, -1672,  1521,   848, -1672, 15529, -1672, -1672, -1672, -1672,
    2862,  1525,  1457, 15000, -1672, 19407,  1343,  1345,  1205,  1348,
   -1672,  1396,  1205, -1672, -1672, -1672, -1672,  2298,  1350,  4033,
    1409, -1672, -1672, -1672,  1410, -1672,  6228, 10512, 10308, -1672,
   -1672, -1672,  6228, -1672, -1672, 19501, 17710, 17710, 17710,  8064,
    1351,  1352, -1672, 17710, -1672,  4446, -1672, -1672, -1672, -1672,
   -1672, 17365,   674,  3338, -1672, -1672, -1672, -1672, -1672, -1672,
   -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672,
   -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672,
   -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672,
   -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672,
   -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672,
   -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672,
   -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672,
   -1672, -1672, -1672, -1672, -1672, -1672, -1672,   179, -1672,  1330,
   -1672, -1672, -1672, -1672, -1672,   139,   381, -1672,  1529,   133,
   16606,  1455,  1535, -1672, 17365,   677, -1672, -1672,  1354,  1542,
   15000, -1672, 19407, -1672,   169,  1360, 17365,   574,  1205,  1348,
   15706, -1672,  1205, -1672,  4033,  4033, -1672, -1672, -1672, -1672,
    8268, 19501, 19501, 19501, -1672, -1672, -1672, 19501, -1672,  2080,
    1552,  1555,  1364, -1672, -1672, 17710, 16415, 16415,  1500, -1672,
    1336,  1336,   605, -1672, -1672, -1672, 17710,  1483, -1672,  1387,
    1374,   136, 17710, -1672, 16606, -1672, 17710, 19407,  1492, -1672,
    1567, -1672,  1568, -1672,   547, -1672, -1672, -1672,  1379,   574,
   -1672,   574, -1672, -1672,  8472,  1381,  1466, -1672,  1485,  1424,
   -1672, -1672,  1487, 17365,  1407,   674, -1672, -1672, 19501, -1672,
   -1672,  1418, -1672,  1556, -1672, -1672, -1672, -1672, 19501,  1579,
     690, -1672, -1672, 19501,  1398, 19501, -1672,   372,  1400,  8676,
   17365, -1672, 17365, -1672,  8880, -1672, -1672, -1672,  1401, -1672,
    1404,  1425, 16806,  1080,  1422, -1672, -1672, -1672, 17710,  1423,
      69, -1672,  1530, -1672, -1672, -1672, -1672, -1672, -1672,  9084,
   -1672,  4446,   984, -1672,  1436, 16806,   867, -1672, 19501, -1672,
    1416,  1607,   681,    69, -1672, -1672,  1534, -1672,  4446,  1420,
   -1672,  1205,    80, -1672, -1672, -1672, -1672, 17365, -1672,  1428,
    1431,   137, -1672,  1348,   681,   171,  1205,  1432, -1672,   581,
   17365,   357,  1606,  1539,  1348, -1672, -1672, -1672, -1672,   176,
    1616,  1548, 15000, -1672,   581,  9288,  9492,   416,  1619,  1551,
   15000, -1672, 19407, -1672, -1672, -1672,  1621,  1553, 15000, -1672,
   19407, 15000, -1672, 19407, 19407
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   202,   462,     0,   894,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   986,
     974,     0,   760,     0,   766,   767,   768,    29,   832,   962,
     963,   169,   170,   769,     0,   150,     0,     0,     0,     0,
      30,     0,     0,     0,     0,   221,     0,     0,     0,     0,
       0,     0,   431,   432,   433,   430,   429,   428,     0,     0,
       0,     0,   250,     0,     0,     0,    37,    38,    40,    41,
      39,   773,   775,   776,   770,   771,     0,     0,     0,   777,
     772,     0,   743,    32,    33,    34,    36,    35,     0,   774,
       0,     0,     0,     0,   778,   434,   571,    31,     0,   168,
     138,     0,   761,     0,     0,     4,   124,   126,   831,     0,
     742,     0,     6,     0,     0,   220,     7,     9,     8,    10,
       0,     0,   426,     0,   476,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   532,   474,   951,   952,   553,   547,
     548,   549,   550,   551,   552,   457,   556,     0,   456,   922,
     744,   751,     0,   834,   546,   425,   925,   926,   938,   475,
       0,     0,     0,   478,   477,   923,   924,   921,   958,   961,
     536,   833,    11,   431,   432,   433,     0,     0,    36,     0,
     124,   220,     0,  1026,   475,  1027,     0,  1029,  1030,   555,
     470,     0,   463,   468,     0,     0,   518,   519,   520,   521,
      29,   962,   769,   746,    37,    38,    40,    41,    39,     0,
       0,  1050,   944,   744,     0,   745,   497,     0,   499,   537,
     538,   539,   540,   541,   542,   543,   545,     0,   990,     0,
     841,   756,   240,     0,  1050,   454,   755,   749,     0,   765,
     745,   969,   970,   976,   968,   757,     0,   455,     0,   759,
     544,     0,   203,     0,     0,   459,   203,   148,   461,     0,
       0,   154,   156,     0,     0,   158,     0,    75,    76,    81,
      82,    67,    68,    59,    79,    90,    91,     0,    62,     0,
      66,    74,    72,    93,    85,    84,    57,    80,   100,   101,
      58,    96,    55,    97,    56,    98,    54,   102,    89,    94,
      99,    86,    87,    61,    88,    92,    53,    83,    69,   103,
      77,   105,    70,    60,    47,    48,    49,    50,    51,    52,
      71,   106,   104,   108,    64,    45,    46,    73,  1097,  1098,
      65,  1102,    44,    63,    95,     0,     0,   124,   107,  1041,
    1096,     0,  1099,     0,     0,     0,   160,     0,     0,     0,
     211,     0,     0,     0,     0,     0,     0,     0,     0,   843,
       0,   112,   114,   339,     0,     0,   338,   344,     0,     0,
     251,     0,   254,     0,     0,     0,     0,  1047,   236,   248,
     982,   986,   590,   617,   617,   590,   617,     0,  1011,     0,
     780,     0,     0,     0,  1009,     0,    16,     0,   128,   228,
     242,   249,   647,   583,     0,  1035,   563,   565,   567,   898,
     462,   476,     0,     0,   474,   475,   477,   203,     0,   965,
     762,     0,   763,     0,     0,     0,   200,     0,     0,   130,
     330,     0,    28,     0,     0,     0,     0,     0,   201,   219,
       0,   247,   232,   246,   431,   434,   220,   427,   967,     0,
     914,   190,   191,   192,   193,   194,   196,   197,   199,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   974,     0,   189,
     967,   967,   996,     0,     0,     0,     0,     0,     0,     0,
       0,   424,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   496,   498,   899,   900,     0,
     913,   912,   330,   330,   967,     0,   982,     0,   220,     0,
       0,   162,     0,   896,   891,   841,     0,   476,   474,     0,
     994,     0,   588,   840,   985,   765,   476,   474,   475,   130,
       0,   330,   453,     0,   915,   758,     0,   138,   290,     0,
     570,     0,   165,     0,   203,   460,     0,     0,     0,     0,
       0,   157,   188,   159,  1097,  1098,  1094,  1095,     0,  1101,
    1087,     0,     0,     0,     0,    78,    43,    65,    42,  1042,
     195,   198,   161,   138,     0,   178,   187,     0,     0,     0,
       0,     0,     0,   115,     0,     0,     0,   842,   113,    18,
       0,   109,     0,   340,     0,   163,     0,     0,   164,   252,
     253,  1031,     0,     0,   476,   474,   475,   478,   477,     0,
    1077,   260,     0,   983,     0,     0,     0,     0,   841,   841,
       0,     0,     0,     0,   166,     0,     0,   779,  1010,   832,
       0,     0,  1008,   837,  1007,   127,     5,    13,    14,     0,
     258,     0,     0,   576,     0,     0,   841,     0,   753,     0,
     752,   747,   577,     0,     0,     0,     0,   898,   134,     0,
     843,   897,  1106,   452,   465,   479,   931,   950,   145,   137,
     141,   142,   143,   144,   425,     0,   554,   835,   836,   125,
     841,     0,  1051,     0,     0,     0,   843,   331,     0,     0,
       0,   476,   207,   208,   206,   474,   475,   203,   182,   180,
     181,   183,   559,   222,   256,     0,   966,     0,     0,   502,
     504,   503,   515,     0,     0,   535,   500,   501,   505,   507,
     506,   524,   525,   522,   523,   526,   527,   528,   529,   530,
     516,   517,   509,   510,   508,   511,   512,   514,   531,   513,
       0,     0,  1000,     0,   841,  1034,     0,  1033,  1050,   928,
     238,   230,   244,     0,  1035,   234,   220,     0,   466,   469,
     471,   481,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   902,     0,   901,   904,   927,   908,
    1050,   905,     0,     0,     0,     0,     0,     0,  1028,   464,
     889,   893,   840,   895,   451,   748,     0,   989,     0,   988,
     256,     0,   748,   973,   972,   958,   961,     0,     0,   901,
     904,   971,   905,   473,   292,   294,   134,   574,   573,   458,
       0,   138,   274,   149,   461,     0,     0,     0,     0,   203,
     286,   286,   155,   841,     0,     0,  1086,     0,  1083,   841,
       0,  1057,     0,     0,     0,     0,     0,   839,     0,    37,
      38,    40,    41,    39,     0,     0,   782,   786,   787,   788,
     789,   790,   792,     0,   781,   132,   830,   791,  1050,  1100,
     203,     0,     0,     0,    21,     0,    22,     0,    19,     0,
     110,     0,    20,     0,     0,     0,   121,   843,     0,   119,
     114,   111,   116,     0,   337,   345,   342,     0,     0,  1020,
    1025,  1022,  1021,  1024,  1023,    12,  1075,  1076,     0,   841,
       0,     0,     0,   982,   979,     0,   587,     0,   601,   840,
     589,   840,   616,   604,   610,   613,   607,  1019,  1018,  1017,
       0,  1013,     0,  1014,  1016,   203,     5,     0,     0,     0,
     641,   642,   650,   649,     0,   474,     0,   840,   582,   586,
       0,     0,  1036,     0,   564,     0,   203,  1064,   898,   316,
    1105,     0,     0,     0,   964,   840,  1053,  1049,   332,   333,
     741,   842,   329,     0,     0,     0,     0,   451,     0,     0,
     479,     0,   932,   210,   203,   140,   898,     0,     0,   258,
     561,   224,   910,   911,   534,     0,   624,   625,     0,   622,
     840,   995,     0,     0,   330,   260,     0,   258,     0,     0,
     256,     0,   974,   482,     0,     0,   929,   930,   959,   960,
       0,     0,     0,   877,   848,   849,   850,   857,     0,    37,
      38,    40,    41,    39,     0,     0,   863,   869,   870,   871,
     872,   873,     0,   861,   859,   860,   883,   841,     0,   891,
     993,   992,     0,   258,     0,   916,   764,     0,   296,     0,
     203,   146,   203,     0,   203,     0,     0,     0,     0,   266,
     267,   278,     0,   138,   276,   175,   286,     0,   286,     0,
     840,     0,     0,     0,     0,     0,   840,  1085,  1088,  1056,
     841,  1055,     0,   841,   813,   814,   811,   812,   847,     0,
     841,   839,   594,   619,   619,   594,   619,   585,     0,     0,
    1002,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1091,
     212,     0,   215,   179,     0,     0,     0,   117,     0,   122,
     123,   115,   842,   120,     0,   341,     0,  1032,   167,  1048,
    1077,  1068,  1072,   259,   261,   351,     0,     0,   980,     0,
     592,     0,  1012,     0,    17,   203,  1035,   257,   351,     0,
       0,   748,   579,     0,   754,  1037,     0,  1064,   568,   133,
     135,     0,     0,     0,  1106,     0,   321,   319,   904,   917,
    1050,   904,   918,  1052,     0,     0,   334,   131,   205,   207,
     208,   475,   186,   204,   184,   185,   209,   139,     0,   898,
     255,     0,   898,     0,   533,   999,   998,     0,   330,     0,
       0,     0,     0,     0,     0,   258,   226,   765,   903,   330,
       0,   853,   854,   855,   856,   864,   865,   881,     0,   841,
       0,   877,   598,   621,   621,   598,   621,     0,   852,   885,
       0,   840,   888,   890,   892,     0,   987,     0,   903,     0,
       0,     0,   203,   293,   575,   151,     0,   461,   266,   268,
     982,     0,     0,     0,   203,     0,     0,     0,     0,     0,
     280,     0,  1092,     0,     0,  1078,     0,  1084,  1082,   840,
       0,     0,     0,   784,   840,   838,     0,     0,   841,     0,
       0,   827,   841,     0,     0,     0,     0,   841,     0,   793,
     828,   829,  1006,     0,   841,   796,   798,   797,     0,     0,
     794,   795,   799,   801,   800,   817,   818,   815,   816,   819,
     820,   821,   822,   823,   808,   809,   803,   804,   802,   805,
     806,   807,   810,  1090,     0,   138,     0,     0,     0,     0,
     118,    23,   343,     0,     0,     0,  1069,  1074,     0,   425,
     984,   982,   467,   472,   480,     0,     0,    15,     0,   425,
     653,     0,     0,   655,   648,   651,     0,   646,     0,  1039,
       0,     0,     0,     0,   532,     0,   478,  1065,   572,     0,
     322,     0,     0,   317,     0,   336,   335,  1064,     0,   351,
       0,   898,     0,   330,     0,   956,   351,  1035,   351,  1038,
       0,     0,     0,   483,     0,     0,   867,   840,   876,   858,
       0,     0,   841,     0,     0,   875,   841,     0,     0,     0,
     851,     0,     0,   841,   862,   882,   991,   351,     0,   138,
       0,   289,   275,     0,     0,     0,   265,   171,   279,     0,
       0,   282,     0,   287,   288,   138,   281,  1093,  1079,     0,
       0,  1054,     0,  1104,   846,   845,   783,   602,   840,   593,
       0,   605,   840,   618,   611,   614,   608,     0,   840,   584,
     785,     0,   623,   840,  1001,   825,     0,     0,   203,    24,
      25,    26,    27,  1071,  1066,  1067,  1070,   262,     0,     0,
       0,   432,   423,     0,     0,     0,   237,   350,   352,     0,
     422,     0,     0,     0,  1035,   425,     0,   591,  1015,   347,
     243,   644,     0,     0,   578,   566,   475,   136,   203,     0,
     325,   315,     0,   318,   324,   330,   558,  1064,   425,  1064,
       0,   997,     0,   955,   425,     0,   425,  1040,   351,   898,
     953,   880,   879,   866,   603,   840,   597,     0,   606,   840,
     620,   612,   615,   609,     0,   868,   840,   884,   425,   138,
     203,   147,   152,   173,   269,   203,   277,   283,   138,   285,
       0,  1080,     0,     0,     0,   596,   826,   581,     0,  1005,
    1004,   824,   138,   216,  1073,     0,     0,     0,  1043,     0,
       0,     0,   263,     0,  1035,     0,   388,   384,   390,   743,
      36,     0,   378,     0,   383,   387,   400,     0,   398,   403,
       0,   402,     0,   401,     0,   220,   354,     0,   356,     0,
     357,   358,     0,     0,   981,     0,   645,   643,   654,   652,
       0,   326,     0,     0,   313,   323,     0,     0,  1064,  1058,
     233,   558,  1064,   957,   239,   347,   245,   425,     0,     0,
       0,   600,   874,   887,     0,   241,   291,   203,   203,   138,
     272,   172,   284,  1081,  1103,   844,     0,     0,     0,   203,
       0,     0,   450,     0,  1044,     0,   368,   372,   447,   448,
     382,     0,     0,     0,   363,   703,   704,   702,   705,   706,
     723,   725,   724,   694,   666,   664,   665,   684,   699,   700,
     660,   671,   672,   674,   673,   693,   677,   675,   676,   678,
     679,   680,   681,   682,   683,   685,   686,   687,   688,   689,
     690,   692,   691,   661,   662,   663,   667,   668,   670,   740,
     708,   709,   713,   714,   715,   716,   717,   718,   701,   720,
     710,   711,   712,   695,   696,   697,   698,   721,   722,   726,
     728,   727,   729,   730,   707,   732,   731,   734,   736,   735,
     669,   739,   737,   738,   733,   719,   659,   395,   656,     0,
     364,   416,   417,   415,   408,     0,   409,   365,   442,     0,
       0,     0,     0,   446,     0,   220,   229,   346,     0,     0,
       0,   314,   328,   954,     0,     0,     0,     0,  1064,  1058,
       0,   235,  1064,   878,     0,     0,   138,   270,   153,   174,
     203,   595,   580,  1003,   214,   366,   367,   445,   264,     0,
     841,   841,     0,   391,   379,     0,     0,     0,   397,   399,
       0,     0,   404,   411,   412,   410,     0,     0,   353,  1045,
       0,     0,     0,   449,     0,   348,     0,   327,     0,   639,
     843,   134,   843,  1060,     0,   418,   134,   223,     0,     0,
     231,     0,   599,   886,   203,     0,   176,   369,   124,     0,
     370,   371,     0,   840,     0,   840,   393,   389,   394,   657,
     658,     0,   380,   413,   414,   406,   407,   405,   443,   440,
    1077,   359,   355,   444,     0,   349,   640,   842,     0,   203,
     842,  1059,     0,  1063,   203,   134,   225,   227,     0,   273,
       0,   218,     0,   425,     0,   385,   392,   396,     0,     0,
     898,   361,     0,   637,   557,   560,  1061,  1062,   419,   203,
     271,     0,     0,   177,   376,     0,   424,   386,   441,  1046,
       0,   843,   436,   898,   638,   562,     0,   217,     0,     0,
     375,  1064,   898,   300,   439,   438,   437,  1106,   435,     0,
       0,     0,   374,  1058,   436,     0,  1064,     0,   373,     0,
    1106,     0,   305,   303,  1058,   138,   420,   134,   360,     0,
     306,     0,     0,   301,     0,   203,   203,     0,   309,   299,
       0,   302,   308,   362,   213,   421,   310,     0,     0,   297,
     307,     0,   298,   312,   311
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1672, -1672, -1672,  -589, -1672, -1672, -1672,   531,  -444,   -47,
     495, -1672,  -243,  -516, -1672, -1672,   455,  1244,  1704, -1672,
    2347, -1672,  -798, -1672,  -531, -1672,  -700,    38, -1672, -1672,
   -1672, -1672, -1672, -1672, -1672, -1672,  -903, -1672, -1672,  -910,
    -280, -1672, -1672, -1672,  -344, -1672, -1672,  -166,    29,    30,
   -1672, -1672, -1672, -1672, -1672, -1672,    39, -1672, -1672, -1672,
   -1672, -1672, -1672,    46, -1672, -1672,  1132,  1136,  1137,   -97,
    -741,  -939,   607,   676,  -351,   342, -1008, -1672,   -57, -1672,
   -1672, -1672, -1672,  -775,   163, -1672, -1672, -1672, -1672,  -341,
   -1672,  -605, -1672,  -466, -1672, -1672,  1032, -1672,   -38, -1672,
   -1672, -1094, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672,
   -1672, -1672,   -75, -1672,     9, -1672, -1672, -1672, -1672, -1672,
    -160, -1672,   116, -1090, -1672, -1406,  -368, -1672,  -163,   153,
    -116,  -347, -1672,  -159, -1672, -1672, -1672,   127,   -91,   -79,
      48,  -781,   -67, -1672, -1672,    13, -1672,    61,  -380, -1672,
      11,    -5,   -72,   -80,   -20, -1672, -1672, -1672, -1672, -1672,
   -1672, -1672, -1672, -1672,  -633,  -913, -1672, -1672, -1672, -1672,
   -1672,   263,  1279, -1672,   541, -1672,   392, -1672, -1672, -1672,
   -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672,
   -1672, -1672, -1672,   227,  -493,  -579, -1672, -1672, -1672, -1672,
   -1672,   466, -1672, -1672, -1672, -1672, -1672, -1672, -1672, -1672,
   -1082,  -358,  2639,    41, -1672,   394,  -436, -1672, -1672,  -495,
    3513,  3269, -1672,  -169, -1672, -1672,   549,     8,  -660, -1672,
   -1672,   627,   402,   412, -1672,   403, -1672, -1672, -1672, -1672,
   -1672,   608, -1672, -1672, -1672,    10,  -923,  -151,  -450,  -438,
   -1672,   698,  -133, -1672, -1672,    55,    57,   659,   -65, -1672,
   -1672,   226,   -77, -1672,  -339,    54,  -378,   184,  -301, -1672,
   -1672,  -491,  1301, -1672, -1672, -1672, -1672, -1672,   713,   473,
   -1672, -1672, -1672,  -335,  -744, -1672,  1265, -1354,  -199,    51,
    -175,   -37,   827, -1672, -1671, -1672,  -254, -1118, -1322,  -240,
     168, -1672,   519,   595, -1672, -1672, -1672, -1672,   552, -1672,
     160, -1178
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   115,   966,   666,   190,   348,   778,
     368,   369,   370,   371,   917,   918,   919,   117,   118,   119,
     120,   121,   986,  1219,   427,  1014,   699,   700,   574,   266,
    1707,   580,  1615,  1708,  1961,   902,   123,   124,   719,   720,
     728,   361,   603,  1916,  1173,  1385,  1983,   450,   191,   701,
    1017,  1253,  1452,   127,   669,  1036,   702,   734,  1040,   641,
    1035,   245,   555,   703,   670,  1037,   452,   388,   410,   130,
    1019,   969,   942,  1193,  1641,  1312,  1099,  1858,  1711,   853,
    1105,   579,   862,  1107,  1495,   845,  1088,  1091,  1301,  1990,
    1991,   689,   690,   715,   716,   375,   376,   378,  1675,  1837,
    1838,  1399,  1547,  1664,  1831,  1970,  1993,  1869,  1920,  1921,
    1922,  1651,  1652,  1653,  1654,  1871,  1872,  1878,  1932,  1657,
    1658,  1662,  1824,  1825,  1826,  1907,  2028,  1548,  1549,   192,
     132,  2007,  2008,  1829,  1551,  1552,  1553,  1554,   133,   134,
     575,   576,   135,   136,   137,   138,   139,   140,   141,   142,
     259,   143,   144,   145,  1688,   146,  1016,  1252,   147,   686,
     687,   688,   263,   419,   570,   675,   676,  1347,   677,  1348,
     148,   149,   647,   648,  1337,  1338,  1461,  1462,   150,   887,
    1067,   151,   888,  1068,   152,   889,  1069,   153,   890,  1070,
     154,   891,  1071,   650,  1340,  1464,   155,   892,   156,   157,
    1900,   158,   671,  1677,   672,  1209,   974,  1417,  1414,  1817,
    1818,   159,   160,   161,   248,   162,   249,   260,   431,   562,
     163,  1341,  1342,   896,   897,   164,  1129,   996,   618,  1130,
    1074,  1275,  1075,  1465,  1466,  1278,  1279,  1077,  1472,  1473,
    1078,   821,   545,   204,   205,   704,   692,   529,  1228,  1229,
     809,   810,   460,   166,   251,   167,   168,   194,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   737,   255,
     256,   644,   239,   240,   773,   774,  1353,  1354,   403,   404,
     960,   180,   632,   181,   685,   182,   351,  1839,  1890,   389,
     439,   710,   711,  1122,  1847,  1902,  1903,  1223,  1396,   938,
    1397,   939,   940,   868,   869,   870,   352,   353,   899,   589,
    1640,   991
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     193,   195,   510,   197,   198,   199,   200,   202,   203,   349,
     206,   207,   208,   209,   165,   457,   229,   230,   231,   232,
     233,   234,   235,   236,   238,   537,   257,  1015,   424,   421,
     989,   426,   444,   125,   126,   681,   844,   530,   531,   265,
    1039,   422,   122,   128,   262,   816,  1429,   273,  1092,   276,
     129,   445,   359,   984,   362,   678,  1002,   267,   446,   777,
     830,   768,   271,   723,  1213,  1224,   812,   813,   509,   559,
     357,   247,   900,  1095,   680,  1536,   457,   965,   682,   807,
    1251,   265,   985,  1221,   254,   252,  1109,   253,  1308,  1083,
    1725,   808,   264,    14,  1238,   837,  1243,   -78,  1262,  1420,
     916,   921,   -78,   423,   563,   858,   -43,   441,   571,   424,
     421,   -43,   426,   840,  1409,    14,   860,    14,   358,   624,
     608,   610,   612,   564,   615,   841,   -42,   627,    14,  1225,
      14,   -42,   571,  1667,   397,  1669,   655,  -381,  1733,  1819,
     548,   547,  1887,   426,  1297,  1887,  1725,  -451,  1880,   411,
     927,   571,   944,   414,   415,  1004,   196,   131,   944,   944,
     456,   546,   458,   557,   556,   729,   730,   731,   944,  1430,
     944,  -935,   458,  1569,  1226,  1881,  1493,   604,  1909,  1561,
    1139,   453,   835,     3,   423,  1639,  1474,   620,   258,  1221,
     377,   540,   360,  1875,   527,   528, -1050,   936,   937,   770,
     771,   527,   528,  1287,  -934,   211,    40,  1898,  -975,  2021,
    1168,  1876,   527,   528,  2037,   423,   354,   438,  1570,  1140,
     261,   538,   909,  -569,  1346,   534,  -937,   527,   528,   400,
    1877,   438,  -978,   814,  -626, -1050,  1350,  -977,   423,  -842,
    -636,   605,  -633,  -842,  -936,   227,   227,   656,   554,   373,
     621,  -932,  1899,   566,  2022,  -939,   566,  1183,  -634,  2038,
     964,  1431,  -752,   265,   577,  -320,  1227,  -304,  -746,   534,
     588,  1288,  -919,  -633,  -933,  -920,  -320,  -451,  -842,  -940,
    1726,  1727,   222,   222,   861,   910,  1220,   -78,   458,  1728,
     735,   412,   459,  -633,   859,   429,   -43,   442,   572,  1265,
     568,  -935,   459,  1486,   573,   635,   634,   599,   638,   625,
    1571,  1422,  1536,  1832,  1247,  1833,   -42,   628,   511,  1576,
    1094,   971,   654,  1668,   111,  1670,  1451,  -381,  1734,  1820,
    -840,  1315,  1888,  1319,  -934,  1942,  2018,  1882,  -975,   928,
     929,   945,  2019,  1494,  1005,  1578,  2023,  1050,  1400,   447,
     200,  2039,  1584,  2034,  1586,   535,  -937,  1614,   392,  1674,
    -747,  -944,  -978,   725,   721,  -753,   426,  -977,   374,   533,
    -754,  -942,   817,   620,  -936,   268,  1471,  1205,  -946,   265,
     423,  -932,  -949,  1608,  1415,  -939,   238,   646,   265,   265,
     646,   265,  1220,   437,   349,   457,   660,  1179,  1180,   535,
     533,  -943,  -919,   727,  -933,  -920,  1034,  1317,  1318,  -940,
    1972,  1248,  1883,   224,   224,   202,   269,  1416,  1568,   270,
     533,   933,   383,   705,  1410,  1317,  1318,   667,   668,   691,
     416,  1884,   633,   972,  1885,   717,   437,  1411,   724,  1681,
    2030,   649,   649,   384,   649,   788,   385,   586,   973,   587,
     211,    40,  1196,   736,   738,  1973,   125,  1412,  -635,  1689,
     393,  1691,  1408,   227,   739,   740,   741,   742,   744,   745,
     746,   747,   748,   749,   750,   751,   752,   753,   754,   755,
     756,   757,   758,   759,   760,   761,   762,   763,   764,   765,
     766,   767,   386,   769,  1697,   736,   736,   772,   394,  2046,
     222,   733,  1320,  1956,   722,  1957,   592,   791,   792,   793,
     794,   795,   796,   797,   798,   799,   800,   801,   802,   803,
    1496,   527,   528,   527,   528,   789,  1483,   717,   717,   736,
     815,  1682,  2031,   909,   791,   116,   777,   819,   247,   786,
     417,   510,   992,  1231,   993,  -745,   827,   418,   829,  -748,
   -1050,   254,   252,   823,   253,  1232,   717,   411,   783,   784,
     453,  1952,   390,   398,   848,   381,   849,   428,  1259,   111,
    1845,   438,  1314,   790,  1849,   382,  1164,  1165,  1166,   379,
     131,  -945,  1953,  -947,   274,  -947, -1050,   347,   380, -1050,
    1428,  2047,  1167,  -108,   975,   398,   437,   391,   712,   681,
     227,   354,   662,  1033,   387,  1628,  1267,   509,  -108,   227,
     395,   637,   852,   398,   834,   398,   227,   923,   396,   678,
    1041,   651,   662,   653,  1240,   398,  1240,   409,   227,   387,
     847,   224,   399,   387,   387,  1045,  1935,   222,   680,   401,
     402,  1343,   682,  1345,  1438,   398,   222,  1440,   412,  1394,
    1395,  1073,   662,   222,   436,  1936,   950,   952,  1937,   413,
    1174,   387,  1175,   169,  1176,   222,   916,  -107,  1178,   423,
    -906,   401,   402,   992,   993,  1316,  1317,  1318,   226,   228,
    1084,   993,  -107,   437,   978,  -906,   440,   779,   657,   401,
     402,   401,   402,  1704,   443,  1351,   497,   691,  1085,  -909,
     400,   401,   402,  1585,   448,  1467,  -124,  1469,   498,   999,
    -124,   559,   398,   811,  -909,   449,  -907,  -627,   708,   430,
     663,   401,   402,  1169,   461,  1242,   462,  -124,  1244,  1245,
    1908,  -907,   398,   779,  1911,   398,   544,   707,  1024,   433,
     936,   937,   662,   463,   836,   527,   528,   842,   863,  1490,
    1317,  1318,    34,    35,    36,   464,   125,  1089,  1090,   681,
    1299,  1300,   227,  1394,  1395,   212,   465,   425,   224,  1635,
    1636,  1032,  1453,  1905,  1906,  1538,   466,   224,   467,   678,
    2026,  2027,  1031,   468,   224,  -628,  1021,  1565,   401,   402,
    1933,  1934,  1444,   116,  1929,  1930,   224,   116,   680,   222,
    1044,   578,   682,  1454,  2004,  2005,  2006,   679,   401,   402,
    1672,   401,   402,  -629,   494,   495,   496,    14,   497,  1485,
      81,    82,    83,    84,    85,   432,   434,   435,  -630,  2015,
     498,   219,  -631,  1087,  1120,  1123,  1580,    89,    90,   503,
     500,  1240,  2029,   981,   982,   501,   502,   532,   425,   265,
    -941,    99,   990,  -632,  1528,  -746,   405,   511,   607,   609,
     611,  1093,   614,   536,   541,   104,   543,   498,  1015,   438,
     549,  1111,   658,   125,   552,   553,   664,  1117,  -744,   425,
     131,  1539,   560,  2013,  1073,   598,  -945,  1540,   569,   454,
    1541,   185,    65,    66,    67,  1542,   550,  1104,  2024,   533,
    1729,   561,   558,   658,    55,   664,   658,   664,   664,   582,
    1556,   681, -1089,   454,   184,   185,    65,    66,    67,   590,
     593,   169,   600,   601,   594,   169,   616,   617,   619,   125,
     224,   678,   629,   626,   630,   639,   640,  1543,  1544,   227,
    1545,   683,  1200,   684,  1201,   693,   849,  1191,  1610,   706,
     680,   694,   727,   695,   682,   697,  -129,  1203,   116,    55,
     822,   455,   732,   820,  1619,   657,   824,   850,   709,   825,
    1546,  1212,   347,   571,   831,   165,   222,  1582,   832,   857,
     854,   387,   588,   871,  1698,   455,   872,   454,   184,   185,
      65,    66,    67,   901,   125,   126,  1236,   131,   691,   724,
     926,   724,   903,   122,   128,   904,   791,   930,   905,   906,
     908,   129,   907,   227,   911,   125,   941,   912,   931,   934,
    1254,   935,   943,  1255,   946,  1256,   691,   948,   949,   717,
     951,   712,   712,   623,   598,   387,   781,   387,   387,   387,
     387,   723,   631,   125,   636,   962,  1221,  1992,   953,   643,
     222,  1221,   227,   131,   227,  1434,   954,   955,   956,   455,
     806,   661,   967,   968,  -769,  1239,  2000,  1239,   970,   976,
    1992,   977,   790,   979,   980,   983,  1221,  1296,  1706,  2014,
     598,   988,   227,   247,   987,  1292,   169,  1712,   995,   222,
     997,   222,  1266,  1000,   839,  1638,   254,   252,  1001,   253,
    1302,  1719,   726,  1949,  1003,   116,  1006,   224,  1954,  1686,
    1007,  1018,  1073,  1073,  1073,  1073,  1073,  1073,   131,   222,
    1008,   125,  1073,   125,  1009,   898,  1010,  1206,  1303,  1022,
    1423,  1331,  1402,  1221,  1026,  1027,  1029,  1030,  1335,   131,
     681,  1038,  1046,  1216,   729,   730,   731,  1047,  -750,  1424,
    1048,   922,   709,   227,  1020,  1233,  1425,  1979,  1086,  1096,
     678,  1106,  1108,  1110,  1214,  1114,  1115,   131,  1116,   227,
     227,  1118,  1132,  1131,  1133,  1134,   811,   842,  1860,   680,
    1135,   224,  1136,   682,  1172,  1137,   959,   961,  1182,  1186,
     222,  1184,  1188,  1189,  1404,   643,  1724,  1190,  1263,  1638,
     454,   184,   185,    65,    66,    67,   222,   222,  1161,  1162,
    1163,  1164,  1165,  1166,  1195,   165,   724,  1199,  1208,  1202,
     224,  1210,   224,  1638,  1211,  1638,  1215,  1167,   681,  2036,
    1436,  1638,  1076,   169,   125,   126,  1217,  1222,  1249,  1264,
    1948,  1258,  1951,   122,   128,   131,  1261,   131,   678,  1220,
     224,   129,  1269,   717,  1220,  -948,  1270,   864,   116,   691,
    1403,  1280,   691,  1281,   717,  1404,   387,   680,   842,  1282,
    1321,   682,   455,  1289,  1325,  1290,  1283,  1291,  1284,  1220,
    1285,  1286,  1239,  1293,  1310,  1305,  1073,  1458,  1073,  1311,
    1307,  1313,   530,  1323,  1322,  1324,   491,   492,   493,   494,
     495,   496,   265,   497,   372,  1329,  1330,   210,  1167,  1334,
    1478,  1333,  1492,  1384,  1386,   498,  1387,   865,  1388,  1389,
    1391,   224,   227,   227,  1398,  1914,  1401,  1418,  1034,    50,
    1432,  2003,   407,  1433,  1419,   408,  1220,   224,   224,  1437,
    1481,   424,   421,   125,   426,  1439,  1509,  1441,  1443,  1446,
    1513,  1066,  1449,  1079,  1455,  1519,  1457,  1448,   131,   222,
     222,  1445,  1524,  1057,  1456,  1477,   214,   215,   216,   217,
     218,   679,   998,  1470,  1479,   116,  1480,  1482,  1487,  1497,
    1491,  1502,  1427,  1503,   990,  1500,   169,  1506,   187,  1102,
     116,    91,  1673,  1508,    93,    94,  1507,    95,   188,    97,
    1557,   866,  1511,  1512,  1514,  1515,  1638,  1562,  1516,  1555,
    1517,  1563,  1518,  1564,  1520,  1523,   724,   210,  1527,  1555,
    1522,  1447,   107,  1529,  1450,  1572,  1073,  1574,  1073,  1558,
    1073,   116,  1530,  1531,   457,  1073,  1532,  1581,   717,    50,
    1177,   709,  1573,  1559,  1567,  1577,  1043,  1426,  1575,  1588,
    1579,   691,  1583,  1589,  1587,  1590,  1593,  1594,  1595,  1597,
     227,  1599,  1604,  1598,  1601,  1277,  1602,   131,  1603,  1605,
    1596,  1192,  1606,  1609,  1600,  1611,   214,   215,   216,   217,
     218,  1607,   722,  1498,  1612,  1080,  1613,  1081,  1616,  1233,
     224,   224,  1617,  1620,  2035,  1632,   116,   222,  1643,  1830,
    1656,  1671,  1821,   169,    93,    94,  1822,    95,   188,    97,
    1676,   598,  1683,  1684,  1687,  1100,  1692,   116,   169,  1693,
    1695,   227,  1699,   806,   839,  1714,  1717,  1723,  1731,  1732,
    1073,   679,   107,  1660,  1827,  1834,   227,   227,  1828,  1840,
    1841,  1843,  1844,  1886,  1846,   116,  1854,  1855,  1852,  1892,
    1865,  1866,  1550,  1895,  1534,  1535,  1896,   125,   222,   169,
    1901,  1923,  1550,  1680,  1925,  1927,  1939,  1685,  1931,  1940,
     717,  1941,   387,   222,   222,  1946,  1947,  1950,  1665,  1955,
    1959,  1960,  1274,  1274,  1066,  1962,  1187,  -377,  1555,  1963,
    1965,  1967,  1881,  1968,  1555,  1971,  1555,   125,  1974,   691,
    1980,  1981,   643,  1198,  1982,  1987,  1989,   372,   372,   372,
     613,   372,  1998,  1994,  2001,   839,  2002,  2010,  1555,  2012,
    2032,   116,  2033,   116,   169,   116,  2016,   227,   224,  2017,
    2040,  2041,  2025,  2048,  2049,  2051,  2052,  1390,  1997,   125,
     780,   785,  1260,   782,  1207,   169,  1326,  2011,   125,   665,
    1484,  1859,  2009,  1710,  1730,  1618,   924,  1850,  1874,  1879,
    1621,  1663,  1622,  1241,   222,  1241,  2043,  2020,  1644,  1894,
     598,  1848,  1891,   169,   652,  1413,  1344,  1468,  1842,  1276,
    1336,   131,  1459,   679,  1460,  1073,  1073,  1294,  1722,   224,
    1230,  1538,   645,  1277,  1463,  1944,  1976,  1463,  1121,   898,
    1969,  1634,  1835,  1475,   224,   224,   718,  1555,   511,  1393,
    1666,  1328,     0,     0,   454,    63,    64,    65,    66,    67,
    1383,   131,  1538,   221,   221,    72,   504,     0,     0,     0,
       0,  1550,     0,    14,   244,   125,   116,  1550,     0,  1550,
       0,   125,     0,     0,     0,  1857,  1710,     0,   125,   169,
       0,   169,     0,   169,     0,  1100,  1309,     0,     0,     0,
     244,  1550,     0,   131,    14,     0,     0,     0,   506,     0,
       0,     0,   131,     0,     0,     0,     0,     0,     0,     0,
    1713,     0,  1538,  1889,     0,     0,   455,     0,     0,     0,
       0,     0,     0,     0,     0,   224,     0,  1539,     0,     0,
    1985,     0,     0,  1540,     0,   454,  1541,   185,    65,    66,
      67,  1542,  1066,  1066,  1066,  1066,  1066,  1066,     0,     0,
       0,     0,  1066,     0,    14,     0,     0,     0,  1539,     0,
       0,     0,     0,   116,  1540,  1897,   454,  1541,   185,    65,
      66,    67,  1542,     0,     0,   116,     0,  1889,     0,     0,
    1550,     0,     0,  1543,  1544,  1499,  1545,     0,     0,   131,
     920,   920,     0,     0,   169,   131,     0,  1591,     0,  1592,
     457,     0,   131,     0,     0,     0,     0,   455,  1924,  1926,
    1241,     0,     0,     0,  1543,  1544,  1560,  1545,  1539,   125,
       0,  1870,     0,  1435,  1540,     0,   454,  1541,   185,    65,
      66,    67,  1542,     0,     0,     0,     0,     0,   455,     0,
       0,     0,   679,     0,     0,     0,     0,  1690,     0,     0,
       0,     0,     0,     0,  1533,     0,   512,   513,   514,   515,
     516,   517,   518,   519,   520,   521,   522,   523,   524,     0,
       0,   221,     0,   125,  1543,  1544,     0,  1545,     0,     0,
       0,     0,     0,     0,  1476,     0,     0,     0,     0,     0,
       0,   169,     0,     0,     0,     0,     0,     0,   455,   643,
    1100,   525,   526,   169,     0,     0,     0,  1694,   125,     0,
     691,     0,     0,   125,     0,     0,  1066,     0,  1066,     0,
       0,   244,     0,   244,  1893,     0,     0,     0,     0,     0,
     679,     0,     0,   691,     0,     0,  1904,  1700,   125,  1701,
       0,  1702,   691,   131,     0,     0,  1703,   454,    63,    64,
      65,    66,    67,     0,     0,     0,     0,  2042,    72,   504,
       0,     0,     0,     0,     0,  2050,     0,     0,     0,     0,
       0,     0,     0,  2053,     0,     0,  2054,   527,   528,     0,
     244,     0,     0,     0,     0,     0,     0,     0,     0,   116,
     643,     0,     0,     0,   125,   125,     0,   131,     0,   505,
     347,   506,     0,     0,     0,     0,  1661,     0,   221,     0,
    1566,     0,     0,  1964,   507,     0,   508,   221,     0,   455,
       0,     0,     0,     0,   221,     0,  1538,     0,     0,   116,
       0,     0,   131,     0,     0,     0,   221,   131,     0,     0,
    1904,  1853,  1977,     0,     0,     0,  1986,   221,     0,     0,
       0,     0,     0,     0,     0,     0,  1066,     0,  1066,     0,
    1066,     0,   131,     0,     0,  1066,     0,     0,    14,     0,
       0,   116,   244,     0,     0,   244,   116,   920,     0,   920,
     116,   920,     0,     0,     0,   920,     0,   920,   920,  1181,
       0,   210,     0,     0,     0,     0,     0,   990,   387,     0,
       0,   598,     0,     0,   347,     0,     0,     0,     0,     0,
     990,     0,     0,    50,  1816,     0,     0,   169,   131,   131,
       0,  1823,     0,     0,  1538,     0,     0,     0,   347,     0,
     347,   244,  1539,     0,     0,     0,   347,     0,  1540,     0,
     454,  1541,   185,    65,    66,    67,  1542,     0,     0,     0,
     214,   215,   216,   217,   218,     0,     0,   169,     0,     0,
    1066,     0,     0,     0,     0,     0,    14,   116,   116,   116,
     221,     0,   187,   116,     0,    91,     0,     0,    93,    94,
     116,    95,   188,    97,     0,     0,     0,     0,  1543,  1544,
       0,  1545,     0,     0,     0,     0,  1912,  1913,     0,   169,
       0,     0,     0,     0,   169,     0,   107,     0,   169,     0,
       0,  1917,   455,     0,     0,     0,     0,     0,     0,     0,
       0,  1696,   244,     0,   244,     0,     0,   886,     0,     0,
    1539,     0,     0,     0,  1538,     0,  1540,     0,   454,  1541,
     185,    65,    66,    67,  1542,     0,     0,   539,   513,   514,
     515,   516,   517,   518,   519,   520,   521,   522,   523,   524,
     886,   539,   513,   514,   515,   516,   517,   518,   519,   520,
     521,   522,   523,   524,     0,     0,    14,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1543,  1544,     0,  1545,
       0,   598,   525,   526,     0,   169,   169,   169,     0,     0,
       0,   169,     0,   210,     0,     0,   525,   526,   169,     0,
     455,   347,     0,     0,     0,  1066,  1066,   244,   244,  1705,
       0,   116,     0,     0,     0,    50,   244,     0,     0,     0,
    1918,     0,     0,   350,     0,     0,     0,  1816,  1816,     0,
    1539,  1823,  1823,     0,     0,     0,  1540,   221,   454,  1541,
     185,    65,    66,    67,  1542,   598,   920,     0,     0,     0,
       0,     0,   214,   215,   216,   217,   218,     0,   527,   528,
       0,     0,     0,     0,     0,   116,     0,     0,     0,     0,
       0,     0,   527,   528,     0,     0,     0,   405,     0,     0,
      93,    94,     0,    95,   188,    97,  1543,  1544,     0,  1545,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     116,     0,     0,     0,     0,   116,     0,     0,   107,     0,
     455,   221,   406,  1984,     0,     0,     0,     0,     0,  1851,
       0,   696,     0,     0,     0,     0,     0,     0,     0,     0,
     116,     0,  1141,  1142,  1143,   833,  1999,     0,     0,   169,
       0,     0,     0,     0,   244,     0,     0,     0,     0,     0,
     221,     0,   221,  1144,     0,     0,  1145,  1146,  1147,  1148,
    1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,
    1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,     0,     0,
     221,   886,     0,     0,     0,     0,   116,   116,   244,     0,
       0,  1167,     0,   169,     0,   244,   244,   886,   886,   886,
     886,   886,     0,     0,     0,     0,     0,     0,     0,   886,
   -1090, -1090, -1090, -1090, -1090,   489,   490,   491,   492,   493,
     494,   495,   496,     0,   497,   244,     0,     0,   169,     0,
       0,     0,     0,   169,     0,     0,   498,     0,   539,   513,
     514,   515,   516,   517,   518,   519,   520,   521,   522,   523,
     524,   221,     0,     0,   350,     0,   350,     0,   169,     0,
       0,     0,     0,     0,     0,   244,     0,   221,   221,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   223,   223,
       0,     0,     0,   525,   526,     0,     0,     0,     0,   246,
       0,   244,   244,     0,     0,     0,     0,     0,     0,     0,
       0,   221,     0,     0,     0,     0,     0,   244,     0,     0,
       0,     0,     0,   350,   169,   169,     0,     0,     0,   244,
    1349,     0,     0,     0,     0,     0,     0,   886,   539,   513,
     514,   515,   516,   517,   518,   519,   520,   521,   522,   523,
     524,     0,   244,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   469,   470,   471,     0,   527,
     528,     0,   244,     0,     0,     0,   244,     0,     0,     0,
       0,     0,     0,   525,   526,   472,   473,   244,   474,   475,
     476,   477,   478,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,     0,   497,     0,     0,   350,     0,     0,   350,     0,
       0,     0,     0,     0,   498,     0,     0,     0,     0,     0,
     221,   221,   932,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   244,     0,     0,     0,   244,     0,
     244,     0,     0,     0,     0,     0,     0,     0,     0,   527,
     528,     0,     0,     0,     0,   886,   886,   886,   886,   886,
     886,   221,     0,     0,   886,   886,   886,   886,   886,   886,
     886,   886,   886,   886,   886,   886,   886,   886,   886,   886,
     886,   886,   886,   886,   886,   886,   886,   886,   886,   886,
     886,   886,   469,   470,   471,     0,   223,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     886,     0,   472,   473,     0,   474,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,     0,   497,
     210,     0,     0,     0,     0,     0,   244,     0,   244,  1406,
       0,   498,     0,     0,     0,   350,     0,   867,   221,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
     363,   364, -1090, -1090, -1090, -1090, -1090,  1159,  1160,  1161,
    1162,  1163,  1164,  1165,  1166,   244,     0,     0,   244,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1167,   214,
     215,   216,   217,   218,     0,   244,   244,   244,   244,   244,
     244,     0,     0,   221,     0,   244,     0,     0,     0,   221,
       0,   365,     0,     0,   366,     0,     0,    93,    94,     0,
      95,   188,    97,   223,   221,   221,     0,   886,     0,     0,
       0,     0,   223,     0,     0,     0,   367,   244,     0,   223,
     350,   350,     0,   244,     0,   107,   886,     0,   886,   350,
       0,   223,     0,     0,   469,   470,   471,     0,     0,     0,
       0,     0,   223,     0,     0,     0,     0,     0,     0,     0,
       0,   499,   886,  1023,   472,   473,     0,   474,   475,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
       0,   497,     0,     0,     0,     0,     0,     0,   244,   244,
       0,     0,   244,   498,     0,   221,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1011,   513,
     514,   515,   516,   517,   518,   519,   520,   521,   522,   523,
     524,     0,     0,     0,     0,     0,   246,  1148,  1149,  1150,
    1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,
    1161,  1162,  1163,  1164,  1165,  1166,   469,   470,   471,   244,
       0,   244,     0,   525,   526,     0,     0,     0,     0,  1167,
       0,     0,     0,     0,     0,   223,   472,   473,  1493,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,     0,   497,   244,     0,   244,     0,     0,     0,
       0,  1113,   886,     0,   886,   498,   886,     0,   350,   350,
       0,   886,   221,     0,     0,   886,     0,   886,     0,     0,
     886,     0,   893,     0,     0,   963,     0,     0,     0,   527,
     528,     0,     0,   244,   244,     0,     0,   244,   469,   470,
     471,     0,     0,     0,   244,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   893,     0,     0,   472,   473,
       0,   474,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,     0,   497,     0,     0,     0,   244,
       0,   244,  1012,   244,     0,     0,     0,   498,   244,     0,
     221,     0,     0,     0,   350,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   244,     0,     0,   886,     0,     0,
     350,     0,     0,     0,     0,     0,     0,     0,     0,   244,
     244,     0,   350,     0,     0,  1494,     0,   244,     0,   244,
       0,     0,   223,  1011,   513,   514,   515,   516,   517,   518,
     519,   520,   521,   522,   523,   524,     0,     0,     0,  1645,
       0,   244,     0,   244,     0,     0,     0,     0,     0,   244,
       0,     0,     0,     0,     0,   350,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   525,   526,
       0,     0,     0,   244,  1011,   513,   514,   515,   516,   517,
     518,   519,   520,   521,   522,   523,   524,     0,     0,   210,
     886,   886,   886,     0,     0,     0,   223,   886,     0,   244,
     210,     0,     0,     0,     0,   244,     0,   244,     0,   994,
       0,    50,     0,     0,     0,     0,     0,     0,     0,   525,
     526,     0,    50,     0,     0,     0,     0,   350,     0,  1072,
       0,   350,     0,   867,  1646,   223,     0,   223,     0,     0,
       0,     0,     0,     0,   527,   528,     0,  1647,   214,   215,
     216,   217,   218,  1648,     0,     0,     0,     0,     0,   214,
     215,   216,   217,   218,     0,   223,   893,     0,     0,     0,
     187,     0,     0,    91,  1649,     0,    93,    94,     0,    95,
    1650,    97,   893,   893,   893,   893,   893,    93,    94,     0,
      95,   188,    97,     0,   893,   527,   528,     0,     0,     0,
       0,     0,   225,   225,   107,     0,     0,   696,   244,     0,
    1171,     0,     0,   250,     0,   107,   732,     0,     0,   864,
     244,     0,     0,     0,   244,     0,     0,     0,   244,   244,
       0,     0,     0,     0,     0,     0,   223,     0,     0,   350,
       0,   350,     0,   244,     0,     0,     0,     0,     0,   886,
    1194,     0,   223,   223,     0,     0,     0,     0,     0,     0,
     886,     0,     0,     0,     0,     0,   886,     0,     0,   210,
     886,     0,     0,     0,     0,     0,     0,  1194,   350,   865,
       0,   350,     0,     0,     0,     0,   223,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,   244,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   893,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   244,     0,   244,  1250,   214,   215,
     216,   217,   218,     0,     0,     0,     0,     0,     0,     0,
     350,     0,   886,     0,     0,     0,   350,     0,     0,     0,
     187,   246,     0,    91,     0,   244,    93,    94,     0,    95,
     188,    97,  1072,  1327,     0,     0,     0,     0,     0,     0,
       0,     0,   244,     0,     0,     0,     0,     0,     0,     0,
       0,   244,     0,     0,   107,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   244,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   223,   223,     0,     0,     0,
       0,   350,   350,     0,     0,     0,     0,     0,     0,     0,
     225,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     893,   893,   893,   893,   893,   893,   223,     0,     0,   893,
     893,   893,   893,   893,   893,   893,   893,   893,   893,   893,
     893,   893,   893,   893,   893,   893,   893,   893,   893,   893,
     893,   893,   893,   893,   893,   893,   893,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   893,   497,     0,   287,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   498,     0,
       0,     0,     0,     0,     0,     0,     0,   350,     0,   350,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   895,     0,     0,   289,     0,     0,     0,     0,
       0,     0,     0,   223,     0,     0,     0,     0,   210,     0,
       0,     0,     0,     0,     0,     0,   350,   225,     0,     0,
       0,  1271,  1272,  1273,   210,   925,   225,   350,     0,     0,
      50,     0,     0,   225,     0,     0,     0,     0,   591,     0,
       0,     0,     0,     0,     0,   225,    50,     0,     0,     0,
    1072,  1072,  1072,  1072,  1072,  1072,   250,     0,   223,     0,
    1072,     0,     0,     0,   223,     0,   584,   214,   215,   216,
     217,   218,   585,     0,     0,     0,     0,     0,     0,   223,
     223,     0,   893,   214,   215,   216,   217,   218,     0,   187,
       0,     0,    91,   341,     0,    93,    94,   350,    95,   188,
      97,   893,     0,   893,     0,     0,     0,     0,     0,     0,
       0,    93,    94,   345,    95,   188,    97,     0,     0,     0,
     350,     0,     0,   107,   346,     0,     0,   893,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   107,
     250,     0,     0,     0,   350,     0,   350,     0,     0,     0,
       0,     0,   350,     0,     0,     0,     0,   469,   470,   471,
       0,     0,     0,     0,     0,     0,     0,  1537,     0,     0,
     223,     0,     0,     0,     0,     0,     0,   472,   473,   225,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,     0,   497,     0,     0,     0,   350,     0,
       0,     0,     0,  1051,  1052,     0,   498,     0,     0,     0,
       0,     0,     0,     0,  1072,     0,  1072,     0,     0,     0,
       0,     0,     0,  1053,     0,     0,   894,     0,     0,     0,
       0,  1054,  1055,  1056,   210,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1057,     0,  1101,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,     0,   894,
       0,     0,  1124,  1125,  1126,  1127,  1128,   893,     0,   893,
       0,   893,     0,     0,  1138,     0,   893,   223,     0,     0,
     893,   210,   893,     0,     0,   893,     0,     0,     0,     0,
       0,     0,  1058,  1059,  1060,  1061,  1062,  1063,     0,  1642,
       0,   350,  1655,    50,     0,     0,     0,     0,     0,     0,
    1064,     0,     0,   350,     0,   187,     0,   350,    91,    92,
       0,    93,    94,     0,    95,   188,    97,     0,     0,  1659,
       0,     0,     0,     0,     0,     0,  1919,     0,  1023,  1065,
     214,   215,   216,   217,   218,     0,   225,     0,     0,   107,
       0,     0,     0,     0,  1072,     0,  1072,     0,  1072,     0,
       0,     0,     0,  1072,     0,   223,     0,     0,    93,    94,
       0,    95,   188,    97,     0,     0,     0,     0,     0,     0,
       0,     0,   893,     0,     0,     0,     0,     0,     0,     0,
     350,     0,  1237,     0,  1720,  1721,   107,  1660,     0,     0,
       0,   469,   470,   471,  1655,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   350,     0,   350,
     225,   472,   473,     0,   474,   475,   476,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,     0,   497,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1072,   225,
     498,   225,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   350,   893,   893,   893,     0,     0,
       0,     0,   893,     0,  1868,     0,     0,   350,     0,   225,
     894,     0,  1655,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   210,     0,     0,   894,   894,   894,   894,
     894,     0,     0,     0,     0,     0,     0,     0,   894,     0,
    1128,  1339,     0,     0,  1339,    50,     0,     0,     0,  1352,
    1355,  1356,  1357,  1359,  1360,  1361,  1362,  1363,  1364,  1365,
    1366,  1367,  1368,  1369,  1370,  1371,  1372,  1373,  1374,  1375,
    1376,  1377,  1378,  1379,  1380,  1381,  1382,     0,     0,     0,
     225,     0,   214,   215,   216,   217,   218,     0,     0,     0,
       0,     0,     0,     0,     0,  1392,   225,   225,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      93,    94,  1049,    95,   188,    97,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     250,     0,     0,  1072,  1072,     0,     0,     0,   107,  1020,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   893,     0,   894,     0,     0,     0,
       0,     0,     0,     0,     0,   893,     0,   210,     0,     0,
       0,   893,     0,     0,     0,   893,   469,   470,   471,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,   250,   472,   473,     0,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,  1488,   497,     0,     0,   214,   215,   216,   217,
     218,     0,     0,     0,     0,   498,     0,     0,     0,     0,
       0,  1504,     0,  1505,     0,     0,     0,   893,   187,   225,
     225,    91,    92,     0,    93,    94,     0,    95,   188,    97,
    1996,     0,     0,     0,     0,     0,     0,  1525,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1642,     0,     0,
       0,     0,   107,     0,   894,   894,   894,   894,   894,   894,
     250,     0,     0,   894,   894,   894,   894,   894,   894,   894,
     894,   894,   894,   894,   894,   894,   894,   894,   894,   894,
     894,   894,   894,   894,   894,   894,   894,   894,   894,   894,
     894,   469,   470,   471,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   894,
       0,   472,   473,     0,   474,   475,   476,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,  1185,   497,   469,
     470,   471,     0,     0,     0,     0,     0,     0,     0,     0,
     498,     0,     0,     0,     0,     0,     0,   225,     0,   472,
     473,     0,   474,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,     0,   497,  1624,     0,  1625,
       0,  1626,     0,     0,     0,     0,  1627,     0,   498,   210,
    1629,     0,  1630,     0,     0,  1631,     0,     0,     0,     0,
       0,     0,   250,     0,     0,     0,     0,     0,   225,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,   913,
     914,     0,     0,   225,   225,     0,   894,     0,     0,   277,
     278,     0,   279,   280,     0,     0,   281,   282,   283,   284,
       0,     0,     0,     0,     0,   894,     0,   894,   214,   215,
     216,   217,   218,     0,   285,   286,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   894,  1257,   915,     0,     0,    93,    94,     0,    95,
     188,    97,     0,   288,     0,     0,     0,     0,     0,     0,
       0,     0,  1715,     0,     0,     0,     0,   290,   291,   292,
     293,   294,   295,   296,   107,     0,     0,   210,     0,   211,
      40,     0,     0,     0,   225,     0,     0,   818,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,    50,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,     0,   332,     0,   775,   334,   335,
     336,     0,     0,     0,   337,   595,   214,   215,   216,   217,
     218,   596,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1861,  1862,  1863,   597,     0,
       0,     0,  1867,     0,    93,    94,     0,    95,   188,    97,
     342,     0,   343,     0,     0,   344,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   469,   470,   471,
       0,   894,   107,   894,     0,   894,   776,     0,   111,     0,
     894,   250,     0,     0,   894,     0,   894,   472,   473,   894,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,     0,   497,     0,     0,     0,   469,   470,
     471,     0,     0,     0,     0,     0,   498,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   472,   473,
       0,   474,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,     0,   497,     0,     0,     0,   250,
       0,     0,     0,     0,   210,     0,   957,   498,   958,     0,
       0,     0,     0,     0,     0,     0,   894,     0,     0,     0,
       0,     0,     0,     0,  1928,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,  1938,     0,     0,     0,     0,
       0,  1943,     0,     0,     0,  1945,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   214,   215,   216,   217,   218,     0,     0,
       0,     0,     0,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,  1268,     0,
       0,    93,    94,     0,    95,   188,    97,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   894,
     894,   894,     0,     0,     0,     0,   894,  1988,    14,   107,
      15,    16,     0,     0,     0,  1873,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,  1298,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,    56,    57,    58,     0,    59,  -203,    60,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,    88,    89,    90,    91,    92,     0,    93,    94,
       0,    95,    96,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,   102,     0,   103,
       0,   104,   105,   106,     0,     0,   107,   108,   894,   109,
     110,     0,   111,   112,     0,   113,   114,     0,     0,   894,
       0,     0,     0,     0,     0,   894,     0,     0,     0,   894,
       0,     0,     0,     0,     0,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1966,    11,
      12,    13,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,   894,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
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
       0,   103,     0,   104,   105,   106,     0,     0,   107,   108,
       0,   109,   110,  1204,   111,   112,     0,   113,   114,     5,
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
      51,     0,     0,     0,    52,    53,    54,    55,    56,    57,
      58,     0,    59,     0,    60,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,    88,    89,
      90,    91,    92,     0,    93,    94,     0,    95,    96,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,   102,     0,   103,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,  1407,   111,   112,
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
      54,    55,     0,    57,    58,     0,    59,     0,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   187,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   188,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   108,     0,   109,
     110,   698,   111,   112,     0,   113,   114,     5,     6,     7,
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
      59,     0,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   187,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   188,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,  1013,   111,   112,     0,   113,
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
       0,    57,    58,     0,    59,  -203,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     187,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     188,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
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
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   187,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   188,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   108,
       0,   109,   110,  1170,   111,   112,     0,   113,   114,     5,
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
      58,     0,    59,     0,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,  1218,   111,   112,
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
      54,    55,     0,    57,    58,     0,    59,     0,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   187,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   188,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   108,     0,   109,
     110,  1246,   111,   112,     0,   113,   114,     5,     6,     7,
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
      59,     0,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   187,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   188,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,  1304,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,  1306,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     187,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     188,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
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
      48,     0,    49,  1489,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   187,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   188,    97,    98,     0,     0,    99,
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
      58,     0,    59,     0,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,  1633,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,  -295,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   187,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   188,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   108,     0,   109,
     110,     0,   111,   112,     0,   113,   114,     5,     6,     7,
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
      59,     0,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   187,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   188,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,  1864,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,  1915,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     187,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     188,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
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
       0,     0,     0,    43,    44,    45,    46,     0,    47,  1958,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   187,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   188,    97,    98,     0,     0,    99,
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
      58,     0,    59,     0,     0,    61,    62,    63,    64,    65,
      66,    67,     0,    68,    69,    70,     0,    72,    73,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
      98,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,  1975,   111,   112,
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
      54,    55,     0,    57,    58,     0,    59,     0,     0,    61,
      62,    63,    64,    65,    66,    67,     0,    68,    69,    70,
       0,    72,    73,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   187,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   188,    97,    98,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   108,     0,   109,
     110,  1978,   111,   112,     0,   113,   114,     5,     6,     7,
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
      59,     0,     0,    61,    62,    63,    64,    65,    66,    67,
       0,    68,    69,    70,     0,    72,    73,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   187,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   188,    97,    98,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,  1995,   111,   112,     0,   113,
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
       0,    57,    58,     0,    59,     0,     0,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,     0,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     187,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     188,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   108,     0,   109,   110,  2044,
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
       0,    61,    62,    63,    64,    65,    66,    67,     0,    68,
      69,    70,     0,    72,    73,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   187,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   188,    97,    98,     0,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   108,
       0,   109,   110,  2045,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,   567,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
      33,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,    41,    42,     0,     0,     0,    43,    44,    45,
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
       0,     0,   107,   108,     0,   109,   110,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,   851,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    47,     0,    48,     0,
      49,     0,     0,    50,    51,     0,     0,     0,    52,    53,
      54,    55,     0,    57,    58,     0,    59,     0,     0,    61,
      62,   184,   185,    65,    66,    67,     0,    68,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,    86,     0,     0,    87,     0,     0,
       0,     0,   187,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   188,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   108,     0,   109,
     110,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,  1103,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
      41,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      47,     0,    48,     0,    49,     0,     0,    50,    51,     0,
       0,     0,    52,    53,    54,    55,     0,    57,    58,     0,
      59,     0,     0,    61,    62,   184,   185,    65,    66,    67,
       0,    68,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,    86,     0,
       0,    87,     0,     0,     0,     0,   187,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   188,    97,     0,     0,
       0,    99,     0,     0,   100,     0,     0,     0,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   108,     0,   109,   110,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
    1709,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,     0,    48,     0,    49,     0,
       0,    50,    51,     0,     0,     0,    52,    53,    54,    55,
       0,    57,    58,     0,    59,     0,     0,    61,    62,   184,
     185,    65,    66,    67,     0,    68,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
     187,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     188,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   108,     0,   109,   110,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,  1856,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,    33,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,    41,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    47,     0,
      48,     0,    49,     0,     0,    50,    51,     0,     0,     0,
      52,    53,    54,    55,     0,    57,    58,     0,    59,     0,
       0,    61,    62,   184,   185,    65,    66,    67,     0,    68,
      69,    70,     0,     0,     0,     0,     0,     0,     0,     0,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,     0,     0,     0,    86,     0,     0,    87,
       0,     0,     0,     0,   187,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   188,    97,     0,     0,     0,    99,
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
      58,     0,    59,     0,     0,    61,    62,   184,   185,    65,
      66,    67,     0,    68,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
      86,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   108,     0,   109,   110,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   355,   420,    13,
       0,     0,     0,     0,     0,     0,     0,     0,   787,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
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
       0,   104,   105,   106,     0,     0,   107,   108,     0,     0,
       0,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   355,     0,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
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
     107,   189,     0,   356,     0,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,  1144,
       0,    10,  1145,  1146,  1147,  1148,  1149,  1150,  1151,  1152,
    1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,
    1163,  1164,  1165,  1166,     0,     0,   713,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1167,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,   183,   184,
     185,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   186,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     187,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     188,    97,     0,   714,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   189,     0,     0,     0,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   355,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
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
       0,     0,   846,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
    1145,  1146,  1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,
    1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,
    1165,  1166,     0,     0,  1234,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1167,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,     0,   183,   184,   185,    65,
      66,    67,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   186,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
       0,  1235,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   189,     0,     0,     0,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   355,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   787,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
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
       0,   104,   105,   106,     0,     0,   107,   189,     0,     0,
       0,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   355,   420,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
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
     107,   108,   469,   470,   471,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   472,   473,     0,   474,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,     0,   497,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,   498,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,   201,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,   183,   184,
     185,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   186,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     187,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     188,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,  1678,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   189,     0,     0,     0,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   476,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,     0,   497,     0,
     237,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     498,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
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
     469,   470,   471,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     472,   473,     0,   474,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,     0,   497,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,   498,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,     0,   183,   184,   185,    65,
      66,    67,     0,     0,    69,    70,     0,     0,     0,     0,
       0,     0,     0,     0,   186,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,     0,     0,     0,
       0,     0,     0,    87,     0,     0,     0,     0,   187,    89,
      90,    91,    92,     0,    93,    94,     0,    95,   188,    97,
       0,     0,     0,    99,     0,     0,   100,     0,     0,     0,
       0,  1679,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   189,     0,   272,   470,   471,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,   472,   473,     0,   474,   475,
     476,   477,   478,   479,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,     0,   497,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,     0,   498,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
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
       0,   104,   105,   106,     0,     0,   107,   189,     0,   275,
       0,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   420,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
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
     107,   108,   469,   470,   471,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   472,   473,     0,   474,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,     0,   497,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,   498,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,   183,   184,
     185,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   186,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     187,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     188,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,   499,     0,     0,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   189,   565,     0,     0,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   355,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
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
       0,     0,     0,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
    1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,
    1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,   743,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1167,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
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
       0,     0,   107,   189,     0,     0,     0,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,  1146,  1147,  1148,  1149,  1150,  1151,
    1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,
    1162,  1163,  1164,  1165,  1166,     0,     0,     0,   787,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1167,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
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
       0,   104,   105,   106,     0,     0,   107,   189,     0,     0,
       0,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,     0,   497,
       0,     0,   826,     0,     0,     0,     0,     0,     0,     0,
       0,   498,     0,     0,    15,    16,     0,     0,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,    38,    39,    40,     0,
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
     107,   189,     0,     0,     0,     0,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,  1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,
    1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,
    1165,  1166,     0,     0,     0,     0,   828,     0,     0,     0,
       0,     0,     0,     0,     0,  1167,     0,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,     0,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    43,
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
     105,   106,     0,     0,   107,   189,     0,     0,     0,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,     0,   497,     0,     0,     0,
    1295,     0,     0,     0,     0,     0,     0,     0,   498,     0,
       0,     0,    15,    16,     0,     0,     0,     0,    17,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,     0,    29,    30,    31,    32,     0,     0,     0,     0,
      34,    35,    36,    37,    38,    39,    40,     0,     0,     0,
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
       0,     0,     0,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   355,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,    16,     0,     0,
       0,     0,    17,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,    30,    31,    32,
       0,     0,     0,     0,    34,    35,    36,    37,    38,    39,
      40,     0,     0,     0,     0,     0,     0,    43,     0,     0,
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
       0,     0,   107,  1421,   469,   470,   471,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   472,   473,     0,   474,   475,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
       0,   497,     0,     0,     0,     0,     0,     0,     0,     0,
      15,    16,     0,   498,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,     0,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,     0,
     183,   184,   185,    65,    66,    67,     0,     0,    69,    70,
       0,     0,     0,     0,     0,     0,     0,     0,   186,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,   187,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   188,    97,     0,     0,     0,    99,     0,     0,
     100,     0,     0,   581,     0,     0,   101,     0,     0,     0,
       0,   104,   105,   106,     0,     0,   107,   189,   469,   470,
     471,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,   855,     0,    10,   472,   473,
       0,   474,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,     0,   497,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,   498,     0,     0,
      17,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,     0,     0,
       0,     0,    34,    35,    36,    37,   659,    39,    40,     0,
     856,     0,     0,     0,     0,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,     0,   183,   184,   185,    65,    66,    67,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   186,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   187,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   188,    97,     0,   277,
     278,    99,   279,   280,   100,     0,   281,   282,   283,   284,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   189,     0,     0,   285,   286,   111,   112,     0,   113,
     114,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,     0,   497,
       0,     0,     0,   288,     0,     0,     0,     0,     0,     0,
       0,   498,     0,     0,     0,     0,     0,   290,   291,   292,
     293,   294,   295,   296,     0,     0,     0,   210,     0,   211,
      40,     0,     0,     0,     0,     0,     0,     0,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,    50,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   210,   332,     0,   333,   334,   335,
     336,     0,     0,     0,   337,   595,   214,   215,   216,   217,
     218,   596,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,   277,   278,     0,   279,   280,     0,   597,   281,
     282,   283,   284,     0,    93,    94,     0,    95,   188,    97,
     342,     0,   343,     0,     0,   344,     0,   285,   286,     0,
     287,     0,     0,   214,   215,   216,   217,   218,     0,     0,
       0,     0,   107,     0,     0,     0,   776,     0,   111,     0,
       0,     0,     0,     0,     0,     0,   288,     0,     0,   451,
       0,    93,    94,     0,    95,   188,    97,   289,     0,     0,
     290,   291,   292,   293,   294,   295,   296,     0,     0,     0,
     210,     0,     0,     0,     0,     0,     0,     0,     0,   107,
       0,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,    50,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,     0,   332,     0,
       0,   334,   335,   336,     0,     0,     0,   337,   338,   214,
     215,   216,   217,   218,   339,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   340,     0,     0,    91,   341,     0,    93,    94,     0,
      95,   188,    97,   342,     0,   343,     0,     0,   344,   277,
     278,     0,   279,   280,     0,   345,   281,   282,   283,   284,
       0,     0,     0,     0,     0,   107,   346,     0,     0,     0,
    1836,     0,     0,     0,   285,   286,     0,   287,   473,     0,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   288,   497,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   289,     0,   498,   290,   291,   292,
     293,   294,   295,   296,     0,     0,     0,   210,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,    50,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,     0,   332,     0,     0,   334,   335,
     336,     0,     0,     0,   337,   338,   214,   215,   216,   217,
     218,   339,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   340,     0,
       0,    91,   341,     0,    93,    94,     0,    95,   188,    97,
     342,     0,   343,     0,     0,   344,   277,   278,     0,   279,
     280,     0,   345,   281,   282,   283,   284,     0,     0,     0,
       0,     0,   107,   346,     0,     0,     0,  1910,     0,     0,
       0,   285,   286,     0,   287,     0,     0,   474,   475,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     288,   497,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   289,     0,   498,   290,   291,   292,   293,   294,   295,
     296,     0,     0,     0,   210,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,    50,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,     0,   332,     0,   333,   334,   335,   336,     0,     0,
       0,   337,   338,   214,   215,   216,   217,   218,   339,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   340,     0,     0,    91,   341,
       0,    93,    94,     0,    95,   188,    97,   342,     0,   343,
       0,     0,   344,   277,   278,     0,   279,   280,     0,   345,
     281,   282,   283,   284,     0,     0,     0,     0,     0,   107,
     346,     0,     0,     0,     0,     0,     0,     0,   285,   286,
       0,   287, -1090, -1090, -1090, -1090,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,     0,
     497,     0,     0,     0,     0,     0,     0,   288,     0,     0,
       0,     0,   498,     0,     0,     0,     0,     0,   289,     0,
       0,   290,   291,   292,   293,   294,   295,   296,     0,     0,
       0,   210,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,    50,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,     0,   332,
       0,     0,   334,   335,   336,     0,     0,     0,   337,   338,
     214,   215,   216,   217,   218,   339,     0,     0,     0,     0,
       0,     0,     0,   210,     0,     0,     0,     0,     0,     0,
       0,     0,   340,     0,     0,    91,   341,     0,    93,    94,
       0,    95,   188,    97,   342,    50,   343,     0,     0,   344,
       0,   277,   278,     0,   279,   280,   345,  1637,   281,   282,
     283,   284,     0,     0,     0,     0,   107,   346,  1646,     0,
       0,     0,     0,     0,     0,     0,   285,   286,     0,   287,
       0,  1647,   214,   215,   216,   217,   218,  1648,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   187,   288,     0,    91,    92,     0,
      93,    94,     0,    95,  1650,    97,   289,     0,     0,   290,
     291,   292,   293,   294,   295,   296,     0,     0,     0,   210,
       0,     0,     0,     0,     0,     0,     0,     0,   107,     0,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,    50,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,     0,   332,     0,     0,
     334,   335,   336,     0,     0,     0,   337,   338,   214,   215,
     216,   217,   218,   339,     0,     0,   210,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1112,     0,     0,     0,
     340,     0,     0,    91,   341,     0,    93,    94,    50,    95,
     188,    97,   342,     0,   343,     0,     0,   344,  1735,  1736,
    1737,  1738,  1739,     0,   345,  1740,  1741,  1742,  1743,     0,
       0,     0,     0,     0,   107,   346,     0,     0,     0,     0,
       0,     0,  1744,  1745,  1746,   214,   215,   216,   217,   218,
    1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,
    1160,  1161,  1162,  1163,  1164,  1165,  1166,   187,     0,     0,
      91,     0,  1747,    93,    94,     0,    95,   188,    97,     0,
    1167,     0,     0,     0,     0,     0,  1748,  1749,  1750,  1751,
    1752,  1753,  1754,     0,     0,     0,   210,     0,     0,     0,
       0,   107,     0,     0,     0,     0,     0,  1755,  1756,  1757,
    1758,  1759,  1760,  1761,  1762,  1763,  1764,  1765,    50,  1766,
    1767,  1768,  1769,  1770,  1771,  1772,  1773,  1774,  1775,  1776,
    1777,  1778,  1779,  1780,  1781,  1782,  1783,  1784,  1785,  1786,
    1787,  1788,  1789,  1790,  1791,  1792,  1793,  1794,  1795,  1796,
       0,     0,     0,  1797,  1798,   214,   215,   216,   217,   218,
       0,  1799,  1800,  1801,  1802,  1803,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1804,  1805,  1806,
       0,   210,     0,    93,    94,     0,    95,   188,    97,  1807,
       0,  1808,  1809,     0,  1810,     0,     0,     0,     0,     0,
       0,  1811,  1812,    50,  1813,     0,  1814,  1815,     0,   277,
     278,   107,   279,   280,     0,     0,   281,   282,   283,   284,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   285,   286,     0,     0,     0,     0,
     214,   215,   216,   217,   218,     0, -1090, -1090, -1090, -1090,
    1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,
    1164,  1165,  1166,   288,     0,   366,     0,     0,    93,    94,
       0,    95,   188,    97,     0,     0,  1167,   290,   291,   292,
     293,   294,   295,   296,     0,     0,     0,   210,     0,     0,
       0,     0,     0,     0,     0,     0,   107,     0,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,    50,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   210,   332,     0,   333,   334,   335,
     336,     0,     0,     0,   337,   595,   214,   215,   216,   217,
     218,   596,     0,     0,     0,     0,    50,     0,     0,     0,
       0,     0,   277,   278,     0,   279,   280,     0,   597,   281,
     282,   283,   284,     0,    93,    94,     0,    95,   188,    97,
     342,     0,   343,     0,     0,   344,     0,   285,   286,     0,
       0,     0,     0,   214,   215,   216,   217,   218,     0,     0,
       0,     0,   107,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   288,     0,   915,     0,
       0,    93,    94,     0,    95,   188,    97,     0,     0,     0,
     290,   291,   292,   293,   294,   295,   296,     0,     0,     0,
     210,     0,     0,     0,     0,     0,     0,     0,     0,   107,
       0,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,    50,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   210,   332,     0,
    1350,   334,   335,   336,     0,     0,     0,   337,   595,   214,
     215,   216,   217,   218,   596,     0,     0,     0,     0,    50,
       0,     0,     0,     0,     0,   277,   278,     0,   279,   280,
       0,   597,   281,   282,   283,   284,     0,    93,    94,     0,
      95,   188,    97,   342,     0,   343,     0,     0,   344,     0,
     285,   286,     0,     0,     0,     0,   214,   215,   216,   217,
     218,     0,     0,     0,     0,   107,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   288,
       0,     0,     0,     0,    93,    94,     0,    95,   188,    97,
       0,     0,     0,   290,   291,   292,   293,   294,   295,   296,
       0,     0,     0,   210,     0,     0,     0,     0,     0,     0,
       0,     0,   107,     0,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,    50,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
       0,   332,     0,     0,   334,   335,   336,     0,     0,     0,
     337,   595,   214,   215,   216,   217,   218,   596,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   597,     0,     0,     0,     0,     0,
      93,    94,     0,    95,   188,    97,   342,     0,   343,     0,
       0,   344,   469,   470,   471,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   107,     0,
       0,     0,   472,   473,     0,   474,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,     0,   497,
     469,   470,   471,     0,     0,     0,     0,     0,     0,     0,
       0,   498,     0,     0,     0,     0,     0,     0,     0,     0,
     472,   473,     0,   474,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,     0,   497,   469,   470,
     471,     0,     0,     0,     0,     0,     0,     0,     0,   498,
       0,     0,     0,   287,     0,     0,     0,     0,   472,   473,
       0,   474,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,     0,   497,     0,     0,     0,     0,
     289,     0,     0,     0,     0,     0,     0,   498,     0,     0,
       0,     0,     0,   210,     0,     0,     0,     0,     0,   469,
     470,   471,     0,     0,     0,     0,     0,     0,     0,     0,
     287,     0,     0,     0,     0,    50,     0,     0,     0,   472,
     473,   583,   474,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,     0,   497,   289,     0,     0,
       0,   584,   214,   215,   216,   217,   218,   585,   498,     0,
     210,     0,     0,     0,     0,     0,     0,     0,     0,   602,
       0,     0,     0,     0,   187,     0,     0,    91,   341,     0,
      93,    94,    50,    95,   188,    97,     0,  1119,   287,     0,
    -424,     0,     0,     0,     0,     0,     0,     0,   345,   454,
     184,   185,    65,    66,    67,     0,     0,     0,   107,   346,
       0,     0,     0,     0,     0,     0,     0,   606,   584,   214,
     215,   216,   217,   218,   585,   289,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   287,     0,   210,     0,
       0,   187,     0,     0,    91,   341,     0,    93,    94,     0,
      95,   188,    97,     0,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,   345,     0,     0,     0,     0,
       0,   455,     0,   289,     0,   107,   346,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   210,   843,     0,     0,
       0,     0,     0,     0,     0,     0,   584,   214,   215,   216,
     217,   218,   585,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   187,
       0,     0,    91,   341,     0,    93,    94,     0,    95,   188,
      97,     0,  1501,     0,     0,     0,     0,  1358,     0,     0,
       0,     0,     0,   345,   584,   214,   215,   216,   217,   218,
     585,     0,     0,   107,   346,   873,   874,     0,     0,     0,
       0,   875,     0,   876,     0,     0,     0,   187,     0,     0,
      91,   341,     0,    93,    94,   877,    95,   188,    97,     0,
       0,     0,     0,    34,    35,    36,   210,     0,     0,     0,
       0,   345,     0,     0,     0,     0,   212,  1141,  1142,  1143,
       0,   107,   346,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1144,     0,
       0,  1145,  1146,  1147,  1148,  1149,  1150,  1151,  1152,  1153,
    1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,
    1164,  1165,  1166,     0,   878,   879,   880,   881,   882,   883,
       0,    81,    82,    83,    84,    85,  1167,     0,     0,     0,
       0,     0,   219,  1097,     0,     0,     0,   187,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   188,    97,     0,
       0,     0,    99,     0,     0,     0,     0,     0,     0,     0,
       0,   884,     0,     0,     0,    29,   104,     0,     0,     0,
       0,   107,   885,    34,    35,    36,   210,     0,   211,    40,
       0,     0,     0,     0,     0,     0,   212,     0,     0,     0,
       0,     0,     0,     0,  1332,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   213,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1098,    75,   214,   215,   216,   217,   218,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,   219,     0,     0,     0,     0,   187,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   188,    97,     0,
     873,   874,    99,     0,     0,     0,   875,     0,   876,     0,
       0,     0,     0,     0,     0,     0,   104,     0,     0,     0,
     877,   107,   220,     0,     0,     0,     0,   111,    34,    35,
      36,   210,     0,     0,     0,     0,   469,   470,   471,     0,
       0,   212,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    50,     0,     0,   472,   473,     0,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,     0,   497,     0,     0,     0,     0,     0,   878,
     879,   880,   881,   882,   883,   498,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,   219,     0,     0,
       0,     0,   187,    89,    90,    91,    92,     0,    93,    94,
       0,    95,   188,    97,    29,     0,     0,    99,     0,     0,
       0,     0,    34,    35,    36,   210,   884,   211,    40,     0,
       0,   104,     0,     0,     0,   212,   107,   885,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    50,     0,     0,
       0,     0,     0,   542,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   213,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,   214,   215,   216,   217,   218,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,   219,     0,     0,     0,     0,   187,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   188,    97,    29,     0,
       0,    99,     0,     0,     0,     0,    34,    35,    36,   210,
       0,   211,    40,     0,     0,   104,     0,     0,     0,   212,
     107,   220,     0,     0,   622,     0,   111,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   213,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   642,    75,   214,   215,
     216,   217,   218,     0,    81,    82,    83,    84,    85,     0,
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
       0,     0,   210,     0,   211,    40,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   214,
     215,   216,   217,   218,    50,    81,    82,    83,    84,    85,
       0,     0,     0,     0,     0,     0,   219,   210,     0,   211,
      40,   187,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   188,    97,     0,     0,     0,    99,     0,     0,    50,
       0,   214,   215,   216,   217,   218,     0,     0,     0,     0,
     104,     0,     0,     0,     0,   107,   243,     0,     0,     0,
       0,   111,     0,     0,     0,     0,     0,   804,     0,    93,
      94,     0,    95,   188,    97,     0,   214,   215,   216,   217,
     218,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   107,     0,     0,
       0,   805,   804,   111,    93,    94,     0,    95,   188,    97,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   469,
     470,   471,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   107,     0,     0,     0,   838,     0,   111,   472,
     473,  1025,   474,   475,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,     0,   497,   469,   470,   471,
       0,     0,     0,     0,     0,     0,     0,     0,   498,     0,
       0,     0,     0,     0,     0,     0,     0,   472,   473,     0,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,     0,   497,  1141,  1142,  1143,     0,     0,
       0,     0,     0,     0,     0,     0,   498,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1144,  1526,     0,  1145,
    1146,  1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,
    1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,
    1166,  1141,  1142,  1143,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1167,     0,     0,     0,     0,     0,
       0,     0,  1144,     0,     0,  1145,  1146,  1147,  1148,  1149,
    1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,
    1160,  1161,  1162,  1163,  1164,  1165,  1166,  1142,  1143,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1167,     0,     0,     0,     0,     0,     0,  1144,     0,     0,
    1145,  1146,  1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,
    1155,  1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,
    1165,  1166,   471,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1167,     0,     0,     0,     0,
     472,   473,     0,   474,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,  1143,   497,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   498,
       0,     0,     0,     0,     0,  1144,     0,     0,  1145,  1146,
    1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,
    1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   472,   473,  1167,   474,   475,   476,   477,   478,   479,
     480,   481,   482,   483,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,     0,   497,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     498
};

static const yytype_int16 yycheck[] =
{
       5,     6,   165,     8,     9,    10,    11,    12,    13,    56,
      15,    16,    17,    18,     4,   131,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   191,    31,   727,   108,   108,
     690,   108,   123,     4,     4,   413,   567,   170,   171,    44,
     784,   108,     4,     4,    33,   536,  1224,    52,   846,    54,
       4,   123,    57,   686,    59,   413,   716,    46,   123,   503,
     555,   497,    51,   443,   977,   988,   532,   533,   165,   244,
      57,    30,   603,   854,   413,  1397,   192,   666,   413,   529,
    1019,    86,   687,   986,    30,    30,   861,    30,  1096,   830,
       9,   529,    44,    48,  1004,   561,  1006,     9,  1037,  1217,
     616,   617,    14,   108,   255,     9,     9,     9,     9,   189,
     189,    14,   189,   563,  1208,    48,    32,    48,    57,     9,
     363,   364,   365,   256,   367,   563,     9,     9,    48,    38,
      48,    14,     9,     9,    86,     9,    70,     9,     9,     9,
     220,   220,     9,   220,  1083,     9,     9,    70,     9,    98,
       9,     9,     9,   102,   103,     9,   197,     4,     9,     9,
     131,    90,    70,   243,   243,   445,   446,   447,     9,    83,
       9,    70,    70,    38,    83,    36,    32,   115,  1849,    54,
     161,   130,   560,     0,   189,  1539,    81,   102,   197,  1092,
      83,   196,   200,    14,   135,   136,   161,    50,    51,   500,
     501,   135,   136,    90,    70,    83,    84,    38,    70,    38,
     161,    32,   135,   136,    38,   220,    56,   182,    83,   200,
     197,   192,   102,     8,  1137,    70,    70,   135,   136,   158,
      51,   182,    70,   534,    70,   200,   131,    70,   243,   194,
      70,   179,    70,   198,    70,    19,    20,   398,   240,    83,
     165,    70,    83,   258,    83,    70,   261,   917,    70,    83,
     201,   175,   161,   268,   269,   198,   175,   198,   161,    70,
     182,   158,    70,    70,    70,    70,   194,   200,   198,    70,
     199,   200,    19,    20,   200,   165,   986,   199,    70,  1643,
     456,   166,   200,    70,   198,   111,   199,   199,   199,  1040,
     262,   200,   200,  1311,   266,   385,   385,   354,   385,   199,
     175,  1221,  1634,  1667,  1014,  1669,   199,   199,   165,  1437,
     851,    54,   199,   199,   202,   199,  1265,   199,   199,   199,
     183,  1106,   199,  1108,   200,   199,   199,   198,   200,   198,
     198,   198,  2013,   199,   198,  1439,   175,   198,   198,   123,
     355,   175,  1446,  2024,  1448,   200,   200,   198,    70,   198,
     161,   197,   200,   443,   443,   161,   443,   200,   202,   197,
     161,   197,   538,   102,   200,   197,  1289,   966,   197,   384,
     385,   200,   197,  1477,   167,   200,   391,   392,   393,   394,
     395,   396,  1092,   165,   441,   511,   401,   913,   914,   200,
     197,   197,   200,   200,   200,   200,   197,   106,   107,   200,
      38,  1016,    31,    19,    20,   420,   197,   200,   200,   197,
     197,   198,   197,   428,   167,   106,   107,   199,   200,   419,
      83,    50,   384,   166,    53,   440,   165,   180,   443,    83,
      83,   393,   394,   197,   396,   512,   197,   287,   181,   289,
      83,    84,   943,   458,   459,    83,   427,   200,    70,  1577,
      70,  1579,  1206,   237,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   197,   498,  1588,   500,   501,   502,    70,    83,
     237,   450,   201,  1909,   443,  1911,   346,   512,   513,   514,
     515,   516,   517,   518,   519,   520,   521,   522,   523,   524,
     201,   135,   136,   135,   136,   512,  1307,   532,   533,   534,
     535,   175,   175,   102,   539,     4,   980,   542,   497,   510,
     193,   704,   693,   993,   695,   161,   551,   200,   553,   161,
     161,   497,   497,   545,   497,   993,   561,   506,   507,   508,
     509,    14,   197,    83,   569,   121,   571,   200,  1034,   202,
    1688,   182,  1103,   512,  1692,   131,    53,    54,    55,   122,
     427,   197,  1904,   197,    53,   197,   197,    56,   131,   200,
    1223,   175,    69,   182,   674,    83,   165,   197,   438,   977,
     374,   441,    90,   778,    73,  1518,  1042,   704,   197,   383,
      70,   385,   574,    83,   560,    83,   390,   622,    70,   977,
     786,   394,    90,   396,  1004,    83,  1006,    96,   402,    98,
     569,   237,    90,   102,   103,   810,    31,   374,   977,   159,
     160,  1134,   977,  1136,  1249,    83,   383,  1252,   166,   102,
     103,   820,    90,   390,    32,    50,   648,   649,    53,   197,
     903,   130,   905,     4,   907,   402,  1182,   182,   911,   674,
     182,   159,   160,   824,   825,   105,   106,   107,    19,    20,
     831,   832,   197,   165,   676,   197,   197,   503,   158,   159,
     160,   159,   160,  1606,   197,  1139,    57,   687,   831,   182,
     158,   159,   160,  1447,   117,  1284,   161,  1286,    69,   714,
     165,   886,    83,   529,   197,    38,   182,    70,   206,    90,
     158,   159,   160,   898,   199,  1005,   199,   182,  1008,  1009,
    1848,   197,    83,   549,  1852,    83,   205,   205,   743,    90,
      50,    51,    90,   199,   560,   135,   136,   563,   588,   105,
     106,   107,    78,    79,    80,   199,   727,    75,    76,  1137,
      75,    76,   536,   102,   103,    91,   199,   108,   374,   133,
     134,   776,  1267,   199,   200,     6,   199,   383,   199,  1137,
     199,   200,   774,   199,   390,    70,   735,  1420,   159,   160,
    1880,  1881,  1258,   262,  1876,  1877,   402,   266,  1137,   536,
     805,   270,  1137,  1269,   123,   124,   125,   413,   159,   160,
    1554,   159,   160,    70,    53,    54,    55,    48,    57,  1310,
     146,   147,   148,   149,   150,   112,   113,   114,    70,  2007,
      69,   157,    70,   838,   871,   872,  1441,   163,   164,   161,
      70,  1221,  2020,   683,   684,    70,   200,   197,   189,   854,
     197,   177,   692,    70,  1385,   161,   165,   704,   363,   364,
     365,   850,   367,   197,   199,   191,    49,    69,  1568,   182,
     161,   863,   399,   844,   204,     9,   403,   869,   161,   220,
     727,   112,   161,  2001,  1053,   354,   197,   118,     8,   120,
     121,   122,   123,   124,   125,   126,   237,   859,  2016,   197,
    1644,   197,   243,   430,   111,   432,   433,   434,   435,   199,
    1401,  1289,   161,   120,   121,   122,   123,   124,   125,   197,
      14,   262,   199,   199,   161,   266,   200,     9,   199,   900,
     536,  1289,   131,    14,   131,   198,   182,   168,   169,   713,
     171,    14,   947,   102,   949,   198,   951,   939,  1479,   203,
    1289,   198,   200,   198,  1289,   198,   197,   962,   427,   111,
       9,   192,   197,   197,  1495,   158,   198,    94,   437,   198,
     201,   976,   441,     9,   198,   965,   713,  1443,   198,    14,
     199,   450,   182,   197,  1589,   192,     9,   120,   121,   122,
     123,   124,   125,   197,   965,   965,  1001,   844,   988,  1004,
      83,  1006,   200,   965,   965,   199,  1011,   198,   200,   199,
     199,   965,   200,   787,   200,   986,   133,   199,   198,   198,
    1025,   199,   197,  1028,   198,  1030,  1016,   204,     9,  1034,
       9,   871,   872,   374,   503,   504,   505,   506,   507,   508,
     509,  1421,   383,  1014,   385,    70,  1949,  1970,   204,   390,
     787,  1954,   826,   900,   828,  1230,   204,   204,   204,   192,
     529,   402,    32,   134,   161,  1004,   199,  1006,   181,   137,
    1993,     9,  1011,   198,   161,    14,  1979,  1082,  1609,  2002,
     549,     9,   856,  1042,   194,  1077,   427,  1618,     9,   826,
     183,   828,  1041,   198,   563,  1539,  1042,  1042,     9,  1042,
    1089,  1632,   443,  1901,    14,   574,     9,   713,  1906,  1575,
     198,   133,  1281,  1282,  1283,  1284,  1285,  1286,   965,   856,
     198,  1092,  1291,  1094,   198,   594,   198,   967,  1090,   204,
    1221,  1123,  1199,  2036,   204,   204,   201,     9,  1130,   986,
    1518,    14,   198,   983,  1424,  1425,  1426,   198,   161,  1221,
     204,   620,   621,   927,   197,   995,  1221,  1955,   198,   102,
    1518,   199,   199,     9,   980,   137,   161,  1014,     9,   943,
     944,   198,    70,   197,    70,    70,   992,   993,  1709,  1518,
      70,   787,    70,  1518,   200,   197,   655,   656,     9,    14,
     927,   201,   199,   183,  1199,   536,  1640,     9,  1038,  1643,
     120,   121,   122,   123,   124,   125,   943,   944,    50,    51,
      52,    53,    54,    55,   200,  1205,  1221,    14,   200,   204,
     826,    14,   828,  1667,   198,  1669,   199,    69,  1606,  2027,
    1235,  1675,   820,   574,  1205,  1205,   194,    32,   197,    14,
    1900,   197,  1902,  1205,  1205,  1092,    32,  1094,  1606,  1949,
     856,  1205,   197,  1258,  1954,   197,    14,    31,   727,  1249,
    1199,    52,  1252,   197,  1269,  1270,   735,  1606,  1084,    70,
    1110,  1606,   192,   197,  1114,   161,    70,     9,    70,  1979,
      70,    70,  1221,   198,   197,   199,  1455,  1279,  1457,   137,
     199,    14,  1425,   137,   183,   161,    50,    51,    52,    53,
      54,    55,  1307,    57,    60,     9,   198,    81,    69,     9,
    1299,   204,  1317,    83,   201,    69,   201,    91,   201,   201,
     199,   927,  1096,  1097,     9,  1856,   197,   137,   197,   103,
      14,  1991,    88,    83,   199,    91,  2036,   943,   944,   198,
    1302,  1421,  1421,  1314,  1421,   200,  1338,   197,   197,   200,
    1342,   820,   199,   822,   137,  1347,     9,   200,  1205,  1096,
    1097,   198,  1354,    91,   204,   200,   140,   141,   142,   143,
     144,   977,   713,   158,    32,   844,    77,   199,   198,   183,
     199,    32,  1222,   198,  1224,   137,   727,   198,   162,   858,
     859,   165,  1555,     9,   168,   169,   204,   171,   172,   173,
    1405,   175,   204,     9,   204,   204,  1850,  1412,   204,  1399,
     137,  1416,     9,  1418,   198,     9,  1421,    81,   198,  1409,
     201,  1261,   196,   199,  1264,    14,  1595,  1432,  1597,   201,
    1599,   900,   199,   199,  1550,  1604,   199,  1442,  1443,   103,
     909,   910,    83,   200,   199,   198,   787,  1221,   197,   200,
     198,  1441,   198,   197,   199,   198,   198,   204,     9,   137,
    1234,     9,   137,   204,   204,  1053,   204,  1314,   204,   198,
    1462,   940,     9,    32,  1466,   199,   140,   141,   142,   143,
     144,  1473,  1421,  1323,   198,   826,   198,   828,   199,  1329,
    1096,  1097,   199,   137,  2025,   200,   965,  1234,   112,  1665,
     170,   199,   166,   844,   168,   169,   170,   171,   172,   173,
     166,   980,    14,    83,   118,   856,   198,   986,   859,   198,
     200,  1295,   137,   992,   993,   198,   137,    14,   182,   200,
    1699,  1137,   196,   197,   199,    14,  1310,  1311,    83,    14,
      83,   198,   197,    14,   196,  1014,   137,   137,   198,    14,
     199,   199,  1399,   199,  1394,  1395,    14,  1528,  1295,   900,
     200,     9,  1409,  1568,     9,   201,    83,  1572,    68,   182,
    1575,   197,  1041,  1310,  1311,    83,     9,     9,  1549,   200,
     199,   115,  1051,  1052,  1053,   161,   927,   102,  1578,   102,
     183,   173,    36,    14,  1584,   197,  1586,  1568,   198,  1589,
     199,   197,   943,   944,   179,   183,   183,   363,   364,   365,
     366,   367,   176,    83,   198,  1084,     9,    83,  1608,   199,
      14,  1090,    83,  1092,   965,  1094,   198,  1401,  1234,   198,
      14,    83,   200,    14,    83,    14,    83,  1182,  1982,  1610,
     504,   509,  1035,   506,   968,   986,  1115,  1998,  1619,   405,
    1308,  1708,  1993,  1615,  1645,  1492,   624,  1695,  1733,  1819,
    1500,  1545,  1502,  1004,  1401,  1006,  2034,  2014,  1541,  1835,
    1139,  1691,  1831,  1014,   395,  1209,  1135,  1285,  1683,  1052,
    1131,  1528,  1280,  1289,  1281,  1854,  1855,  1079,  1637,  1295,
     992,     6,   391,  1281,  1282,  1894,  1950,  1285,   871,  1168,
    1940,  1533,  1673,  1291,  1310,  1311,   441,  1697,  1555,  1190,
    1550,  1116,    -1,    -1,   120,   121,   122,   123,   124,   125,
    1168,  1568,     6,    19,    20,   131,   132,    -1,    -1,    -1,
      -1,  1578,    -1,    48,    30,  1706,  1205,  1584,    -1,  1586,
      -1,  1712,    -1,    -1,    -1,  1707,  1708,    -1,  1719,  1090,
      -1,  1092,    -1,  1094,    -1,  1096,  1097,    -1,    -1,    -1,
      56,  1608,    -1,  1610,    48,    -1,    -1,    -1,   174,    -1,
      -1,    -1,  1619,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1620,    -1,     6,  1830,    -1,    -1,   192,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1401,    -1,   112,    -1,    -1,
    1963,    -1,    -1,   118,    -1,   120,   121,   122,   123,   124,
     125,   126,  1281,  1282,  1283,  1284,  1285,  1286,    -1,    -1,
      -1,    -1,  1291,    -1,    48,    -1,    -1,    -1,   112,    -1,
      -1,    -1,    -1,  1302,   118,  1840,   120,   121,   122,   123,
     124,   125,   126,    -1,    -1,  1314,    -1,  1894,    -1,    -1,
    1697,    -1,    -1,   168,   169,  1324,   171,    -1,    -1,  1706,
     616,   617,    -1,    -1,  1205,  1712,    -1,  1455,    -1,  1457,
    1986,    -1,  1719,    -1,    -1,    -1,    -1,   192,  1870,  1871,
    1221,    -1,    -1,    -1,   168,   169,   201,   171,   112,  1860,
      -1,  1731,    -1,  1234,   118,    -1,   120,   121,   122,   123,
     124,   125,   126,    -1,    -1,    -1,    -1,    -1,   192,    -1,
      -1,    -1,  1518,    -1,    -1,    -1,    -1,   201,    -1,    -1,
      -1,    -1,    -1,    -1,  1393,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,   237,    -1,  1914,   168,   169,    -1,   171,    -1,    -1,
      -1,    -1,    -1,    -1,  1295,    -1,    -1,    -1,    -1,    -1,
      -1,  1302,    -1,    -1,    -1,    -1,    -1,    -1,   192,  1310,
    1311,    59,    60,  1314,    -1,    -1,    -1,   201,  1949,    -1,
    1970,    -1,    -1,  1954,    -1,    -1,  1455,    -1,  1457,    -1,
      -1,   287,    -1,   289,  1834,    -1,    -1,    -1,    -1,    -1,
    1606,    -1,    -1,  1993,    -1,    -1,  1846,  1595,  1979,  1597,
      -1,  1599,  2002,  1860,    -1,    -1,  1604,   120,   121,   122,
     123,   124,   125,    -1,    -1,    -1,    -1,  2032,   131,   132,
      -1,    -1,    -1,    -1,    -1,  2040,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  2048,    -1,    -1,  2051,   135,   136,    -1,
     346,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1528,
    1401,    -1,    -1,    -1,  2035,  2036,    -1,  1914,    -1,   172,
    1539,   174,    -1,    -1,    -1,    -1,  1545,    -1,   374,    -1,
    1421,    -1,    -1,  1923,   187,    -1,   189,   383,    -1,   192,
      -1,    -1,    -1,    -1,   390,    -1,     6,    -1,    -1,  1568,
      -1,    -1,  1949,    -1,    -1,    -1,   402,  1954,    -1,    -1,
    1950,  1699,  1952,    -1,    -1,    -1,  1963,   413,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1595,    -1,  1597,    -1,
    1599,    -1,  1979,    -1,    -1,  1604,    -1,    -1,    48,    -1,
      -1,  1610,   438,    -1,    -1,   441,  1615,   903,    -1,   905,
    1619,   907,    -1,    -1,    -1,   911,    -1,   913,   914,   915,
      -1,    81,    -1,    -1,    -1,    -1,    -1,  2007,  1637,    -1,
      -1,  1640,    -1,    -1,  1643,    -1,    -1,    -1,    -1,    -1,
    2020,    -1,    -1,   103,  1653,    -1,    -1,  1528,  2035,  2036,
      -1,  1660,    -1,    -1,     6,    -1,    -1,    -1,  1667,    -1,
    1669,   497,   112,    -1,    -1,    -1,  1675,    -1,   118,    -1,
     120,   121,   122,   123,   124,   125,   126,    -1,    -1,    -1,
     140,   141,   142,   143,   144,    -1,    -1,  1568,    -1,    -1,
    1699,    -1,    -1,    -1,    -1,    -1,    48,  1706,  1707,  1708,
     536,    -1,   162,  1712,    -1,   165,    -1,    -1,   168,   169,
    1719,   171,   172,   173,    -1,    -1,    -1,    -1,   168,   169,
      -1,   171,    -1,    -1,    -1,    -1,  1854,  1855,    -1,  1610,
      -1,    -1,    -1,    -1,  1615,    -1,   196,    -1,  1619,    -1,
      -1,   201,   192,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   201,   588,    -1,   590,    -1,    -1,   593,    -1,    -1,
     112,    -1,    -1,    -1,     6,    -1,   118,    -1,   120,   121,
     122,   123,   124,   125,   126,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     626,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    48,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   168,   169,    -1,   171,
      -1,  1830,    59,    60,    -1,  1706,  1707,  1708,    -1,    -1,
      -1,  1712,    -1,    81,    -1,    -1,    59,    60,  1719,    -1,
     192,  1850,    -1,    -1,    -1,  1854,  1855,   683,   684,   201,
      -1,  1860,    -1,    -1,    -1,   103,   692,    -1,    -1,    -1,
    1869,    -1,    -1,    56,    -1,    -1,    -1,  1876,  1877,    -1,
     112,  1880,  1881,    -1,    -1,    -1,   118,   713,   120,   121,
     122,   123,   124,   125,   126,  1894,  1182,    -1,    -1,    -1,
      -1,    -1,   140,   141,   142,   143,   144,    -1,   135,   136,
      -1,    -1,    -1,    -1,    -1,  1914,    -1,    -1,    -1,    -1,
      -1,    -1,   135,   136,    -1,    -1,    -1,   165,    -1,    -1,
     168,   169,    -1,   171,   172,   173,   168,   169,    -1,   171,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1949,    -1,    -1,    -1,    -1,  1954,    -1,    -1,   196,    -1,
     192,   787,   200,  1962,    -1,    -1,    -1,    -1,    -1,   201,
      -1,   198,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1979,    -1,    10,    11,    12,   198,  1985,    -1,    -1,  1860,
      -1,    -1,    -1,    -1,   820,    -1,    -1,    -1,    -1,    -1,
     826,    -1,   828,    31,    -1,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
     856,   857,    -1,    -1,    -1,    -1,  2035,  2036,   864,    -1,
      -1,    69,    -1,  1914,    -1,   871,   872,   873,   874,   875,
     876,   877,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   885,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,   901,    -1,    -1,  1949,    -1,
      -1,    -1,    -1,  1954,    -1,    -1,    69,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,   927,    -1,    -1,   287,    -1,   289,    -1,  1979,    -1,
      -1,    -1,    -1,    -1,    -1,   941,    -1,   943,   944,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    20,
      -1,    -1,    -1,    59,    60,    -1,    -1,    -1,    -1,    30,
      -1,   967,   968,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   977,    -1,    -1,    -1,    -1,    -1,   983,    -1,    -1,
      -1,    -1,    -1,   346,  2035,  2036,    -1,    -1,    -1,   995,
     198,    -1,    -1,    -1,    -1,    -1,    -1,  1003,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,  1018,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,   135,
     136,    -1,  1038,    -1,    -1,    -1,  1042,    -1,    -1,    -1,
      -1,    -1,    -1,    59,    60,    30,    31,  1053,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,   438,    -1,    -1,   441,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
    1096,  1097,   198,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1110,    -1,    -1,    -1,  1114,    -1,
    1116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   135,
     136,    -1,    -1,    -1,    -1,  1131,  1132,  1133,  1134,  1135,
    1136,  1137,    -1,    -1,  1140,  1141,  1142,  1143,  1144,  1145,
    1146,  1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,
    1156,  1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,
    1166,  1167,    10,    11,    12,    -1,   237,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1186,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      81,    -1,    -1,    -1,    -1,    -1,  1222,    -1,  1224,   204,
      -1,    69,    -1,    -1,    -1,   588,    -1,   590,  1234,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,   112,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,  1261,    -1,    -1,  1264,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,   140,
     141,   142,   143,   144,    -1,  1281,  1282,  1283,  1284,  1285,
    1286,    -1,    -1,  1289,    -1,  1291,    -1,    -1,    -1,  1295,
      -1,   162,    -1,    -1,   165,    -1,    -1,   168,   169,    -1,
     171,   172,   173,   374,  1310,  1311,    -1,  1313,    -1,    -1,
      -1,    -1,   383,    -1,    -1,    -1,   187,  1323,    -1,   390,
     683,   684,    -1,  1329,    -1,   196,  1332,    -1,  1334,   692,
      -1,   402,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,   413,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   199,  1358,   201,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,  1394,  1395,
      -1,    -1,  1398,    69,    -1,  1401,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,    -1,    -1,    -1,   497,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    10,    11,    12,  1455,
      -1,  1457,    -1,    59,    60,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,   536,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,  1500,    -1,  1502,    -1,    -1,    -1,
      -1,   864,  1508,    -1,  1510,    69,  1512,    -1,   871,   872,
      -1,  1517,  1518,    -1,    -1,  1521,    -1,  1523,    -1,    -1,
    1526,    -1,   593,    -1,    -1,   201,    -1,    -1,    -1,   135,
     136,    -1,    -1,  1539,  1540,    -1,    -1,  1543,    10,    11,
      12,    -1,    -1,    -1,  1550,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   626,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,  1595,
      -1,  1597,   198,  1599,    -1,    -1,    -1,    69,  1604,    -1,
    1606,    -1,    -1,    -1,   967,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1620,    -1,    -1,  1623,    -1,    -1,
     983,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1635,
    1636,    -1,   995,    -1,    -1,   199,    -1,  1643,    -1,  1645,
      -1,    -1,   713,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,    -1,    -1,    31,
      -1,  1667,    -1,  1669,    -1,    -1,    -1,    -1,    -1,  1675,
      -1,    -1,    -1,    -1,    -1,  1038,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,
      -1,    -1,    -1,  1699,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    81,
    1716,  1717,  1718,    -1,    -1,    -1,   787,  1723,    -1,  1725,
      81,    -1,    -1,    -1,    -1,  1731,    -1,  1733,    -1,   201,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,
      60,    -1,   103,    -1,    -1,    -1,    -1,  1110,    -1,   820,
      -1,  1114,    -1,  1116,   126,   826,    -1,   828,    -1,    -1,
      -1,    -1,    -1,    -1,   135,   136,    -1,   139,   140,   141,
     142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,   140,
     141,   142,   143,   144,    -1,   856,   857,    -1,    -1,    -1,
     162,    -1,    -1,   165,   166,    -1,   168,   169,    -1,   171,
     172,   173,   873,   874,   875,   876,   877,   168,   169,    -1,
     171,   172,   173,    -1,   885,   135,   136,    -1,    -1,    -1,
      -1,    -1,    19,    20,   196,    -1,    -1,   198,  1834,    -1,
     901,    -1,    -1,    30,    -1,   196,   197,    -1,    -1,    31,
    1846,    -1,    -1,    -1,  1850,    -1,    -1,    -1,  1854,  1855,
      -1,    -1,    -1,    -1,    -1,    -1,   927,    -1,    -1,  1222,
      -1,  1224,    -1,  1869,    -1,    -1,    -1,    -1,    -1,  1875,
     941,    -1,   943,   944,    -1,    -1,    -1,    -1,    -1,    -1,
    1886,    -1,    -1,    -1,    -1,    -1,  1892,    -1,    -1,    81,
    1896,    -1,    -1,    -1,    -1,    -1,    -1,   968,  1261,    91,
      -1,  1264,    -1,    -1,    -1,    -1,   977,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,  1923,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1003,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1950,    -1,  1952,  1018,   140,   141,
     142,   143,   144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1323,    -1,  1968,    -1,    -1,    -1,  1329,    -1,    -1,    -1,
     162,  1042,    -1,   165,    -1,  1981,   168,   169,    -1,   171,
     172,   173,  1053,   175,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1998,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  2007,    -1,    -1,   196,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  2020,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1096,  1097,    -1,    -1,    -1,
      -1,  1394,  1395,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     237,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1131,  1132,  1133,  1134,  1135,  1136,  1137,    -1,    -1,  1140,
    1141,  1142,  1143,  1144,  1145,  1146,  1147,  1148,  1149,  1150,
    1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,
    1161,  1162,  1163,  1164,  1165,  1166,  1167,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,  1186,    57,    -1,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1500,    -1,  1502,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   593,    -1,    -1,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1234,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1539,   374,    -1,    -1,
      -1,    78,    79,    80,    81,   626,   383,  1550,    -1,    -1,
     103,    -1,    -1,   390,    -1,    -1,    -1,    -1,   111,    -1,
      -1,    -1,    -1,    -1,    -1,   402,   103,    -1,    -1,    -1,
    1281,  1282,  1283,  1284,  1285,  1286,   413,    -1,  1289,    -1,
    1291,    -1,    -1,    -1,  1295,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,    -1,    -1,    -1,    -1,    -1,  1310,
    1311,    -1,  1313,   140,   141,   142,   143,   144,    -1,   162,
      -1,    -1,   165,   166,    -1,   168,   169,  1620,   171,   172,
     173,  1332,    -1,  1334,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   168,   169,   186,   171,   172,   173,    -1,    -1,    -1,
    1643,    -1,    -1,   196,   197,    -1,    -1,  1358,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   196,
     497,    -1,    -1,    -1,  1667,    -1,  1669,    -1,    -1,    -1,
      -1,    -1,  1675,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1398,    -1,    -1,
    1401,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,   536,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,  1731,    -1,
      -1,    -1,    -1,    50,    51,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1455,    -1,  1457,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    -1,    -1,   593,    -1,    -1,    -1,
      -1,    78,    79,    80,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    91,    -1,   857,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,   626,
      -1,    -1,   873,   874,   875,   876,   877,  1508,    -1,  1510,
      -1,  1512,    -1,    -1,   885,    -1,  1517,  1518,    -1,    -1,
    1521,    81,  1523,    -1,    -1,  1526,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   144,    -1,  1540,
      -1,  1834,  1543,   103,    -1,    -1,    -1,    -1,    -1,    -1,
     157,    -1,    -1,  1846,    -1,   162,    -1,  1850,   165,   166,
      -1,   168,   169,    -1,   171,   172,   173,    -1,    -1,   129,
      -1,    -1,    -1,    -1,    -1,    -1,  1869,    -1,   201,   186,
     140,   141,   142,   143,   144,    -1,   713,    -1,    -1,   196,
      -1,    -1,    -1,    -1,  1595,    -1,  1597,    -1,  1599,    -1,
      -1,    -1,    -1,  1604,    -1,  1606,    -1,    -1,   168,   169,
      -1,   171,   172,   173,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1623,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1923,    -1,  1003,    -1,  1635,  1636,   196,   197,    -1,    -1,
      -1,    10,    11,    12,  1645,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1950,    -1,  1952,
     787,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1699,   826,
      69,   828,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  2007,  1716,  1717,  1718,    -1,    -1,
      -1,    -1,  1723,    -1,  1725,    -1,    -1,  2020,    -1,   856,
     857,    -1,  1733,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,   873,   874,   875,   876,
     877,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   885,    -1,
    1131,  1132,    -1,    -1,  1135,   103,    -1,    -1,    -1,  1140,
    1141,  1142,  1143,  1144,  1145,  1146,  1147,  1148,  1149,  1150,
    1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,
    1161,  1162,  1163,  1164,  1165,  1166,  1167,    -1,    -1,    -1,
     927,    -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1186,   943,   944,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     168,   169,   201,   171,   172,   173,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     977,    -1,    -1,  1854,  1855,    -1,    -1,    -1,   196,   197,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1875,    -1,  1003,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1886,    -1,    81,    -1,    -1,
      -1,  1892,    -1,    -1,    -1,  1896,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,  1042,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,  1313,    57,    -1,    -1,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,  1332,    -1,  1334,    -1,    -1,    -1,  1968,   162,  1096,
    1097,   165,   166,    -1,   168,   169,    -1,   171,   172,   173,
    1981,    -1,    -1,    -1,    -1,    -1,    -1,  1358,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1998,    -1,    -1,
      -1,    -1,   196,    -1,  1131,  1132,  1133,  1134,  1135,  1136,
    1137,    -1,    -1,  1140,  1141,  1142,  1143,  1144,  1145,  1146,
    1147,  1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,
    1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
    1167,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1186,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,   201,    57,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,  1234,    -1,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,  1508,    -1,  1510,
      -1,  1512,    -1,    -1,    -1,    -1,  1517,    -1,    69,    81,
    1521,    -1,  1523,    -1,    -1,  1526,    -1,    -1,    -1,    -1,
      -1,    -1,  1289,    -1,    -1,    -1,    -1,    -1,  1295,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
     112,    -1,    -1,  1310,  1311,    -1,  1313,    -1,    -1,     3,
       4,    -1,     6,     7,    -1,    -1,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,  1332,    -1,  1334,   140,   141,
     142,   143,   144,    -1,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1358,   201,   165,    -1,    -1,   168,   169,    -1,   171,
     172,   173,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1623,    -1,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,   196,    -1,    -1,    81,    -1,    83,
      84,    -1,    -1,    -1,  1401,    -1,    -1,   198,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,    -1,   129,    -1,   131,   132,   133,
     134,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
     144,   145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1716,  1717,  1718,   162,    -1,
      -1,    -1,  1723,    -1,   168,   169,    -1,   171,   172,   173,
     174,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,  1508,   196,  1510,    -1,  1512,   200,    -1,   202,    -1,
    1517,  1518,    -1,    -1,  1521,    -1,  1523,    30,    31,  1526,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,  1606,
      -1,    -1,    -1,    -1,    81,    -1,    83,    69,    85,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1623,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1875,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1886,    -1,    -1,    -1,    -1,
      -1,  1892,    -1,    -1,    -1,  1896,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,   201,    -1,
      -1,   168,   169,    -1,   171,   172,   173,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1716,
    1717,  1718,    -1,    -1,    -1,    -1,  1723,  1968,    48,   196,
      50,    51,    -1,    -1,    -1,  1732,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,   201,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    -1,    96,    -1,    98,    -1,
     100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,
     110,   111,   112,   113,   114,    -1,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,    -1,   127,   128,   129,
     130,   131,   132,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,   141,   142,   143,   144,    -1,   146,   147,   148,   149,
     150,    -1,    -1,    -1,   154,    -1,    -1,   157,    -1,    -1,
      -1,    -1,   162,   163,   164,   165,   166,    -1,   168,   169,
      -1,   171,   172,   173,   174,    -1,    -1,   177,    -1,    -1,
     180,    -1,    -1,    -1,    -1,    -1,   186,   187,    -1,   189,
      -1,   191,   192,   193,    -1,    -1,   196,   197,  1875,   199,
     200,    -1,   202,   203,    -1,   205,   206,    -1,    -1,  1886,
      -1,    -1,    -1,    -1,    -1,  1892,    -1,    -1,    -1,  1896,
      -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1925,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,  1968,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    87,
      -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,
      98,    -1,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,
     108,   109,   110,   111,   112,   113,   114,    -1,   116,    -1,
     118,   119,   120,   121,   122,   123,   124,   125,    -1,   127,
     128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,   141,   142,   143,   144,    -1,   146,   147,
     148,   149,   150,    -1,    -1,    -1,   154,    -1,    -1,   157,
      -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,
     168,   169,    -1,   171,   172,   173,   174,    -1,    -1,   177,
      -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,   186,   187,
      -1,   189,    -1,   191,   192,   193,    -1,    -1,   196,   197,
      -1,   199,   200,   201,   202,   203,    -1,   205,   206,     3,
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
     104,    -1,    -1,    -1,   108,   109,   110,   111,   112,   113,
     114,    -1,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,   125,    -1,   127,   128,   129,   130,   131,   132,    -1,
      -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
     144,    -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,
     154,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,
     164,   165,   166,    -1,   168,   169,    -1,   171,   172,   173,
     174,    -1,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,
      -1,    -1,   186,   187,    -1,   189,    -1,   191,   192,   193,
      -1,    -1,   196,   197,    -1,   199,   200,   201,   202,   203,
      -1,   205,   206,     3,     4,     5,     6,     7,    -1,    -1,
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
     110,   111,    -1,   113,   114,    -1,   116,    -1,    -1,   119,
     120,   121,   122,   123,   124,   125,    -1,   127,   128,   129,
      -1,   131,   132,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,   141,   142,   143,   144,    -1,   146,   147,   148,   149,
     150,    -1,    -1,    -1,   154,    -1,    -1,   157,    -1,    -1,
      -1,    -1,   162,   163,   164,   165,   166,    -1,   168,   169,
      -1,   171,   172,   173,   174,    -1,    -1,   177,    -1,    -1,
     180,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,
      -1,   191,   192,   193,    -1,    -1,   196,   197,    -1,   199,
     200,   201,   202,   203,    -1,   205,   206,     3,     4,     5,
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
     116,    -1,    -1,   119,   120,   121,   122,   123,   124,   125,
      -1,   127,   128,   129,    -1,   131,   132,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,   142,   143,   144,    -1,
     146,   147,   148,   149,   150,    -1,    -1,    -1,   154,    -1,
      -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,
     166,    -1,   168,   169,    -1,   171,   172,   173,   174,    -1,
      -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,
     186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,    -1,
     196,   197,    -1,   199,   200,   201,   202,   203,    -1,   205,
     206,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,   113,   114,    -1,   116,   117,    -1,   119,   120,   121,
     122,   123,   124,   125,    -1,   127,   128,   129,    -1,   131,
     132,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,   144,    -1,   146,   147,   148,   149,   150,    -1,
      -1,    -1,   154,    -1,    -1,   157,    -1,    -1,    -1,    -1,
     162,   163,   164,   165,   166,    -1,   168,   169,    -1,   171,
     172,   173,   174,    -1,    -1,   177,    -1,    -1,   180,    -1,
      -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,
     192,   193,    -1,    -1,   196,   197,    -1,   199,   200,    -1,
     202,   203,    -1,   205,   206,     3,     4,     5,     6,     7,
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
      -1,   119,   120,   121,   122,   123,   124,   125,    -1,   127,
     128,   129,    -1,   131,   132,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,   141,   142,   143,   144,    -1,   146,   147,
     148,   149,   150,    -1,    -1,    -1,   154,    -1,    -1,   157,
      -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,
     168,   169,    -1,   171,   172,   173,   174,    -1,    -1,   177,
      -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,   186,    -1,
      -1,    -1,    -1,   191,   192,   193,    -1,    -1,   196,   197,
      -1,   199,   200,   201,   202,   203,    -1,   205,   206,     3,
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
     114,    -1,   116,    -1,    -1,   119,   120,   121,   122,   123,
     124,   125,    -1,   127,   128,   129,    -1,   131,   132,    -1,
      -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
     144,    -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,
     154,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,
     164,   165,   166,    -1,   168,   169,    -1,   171,   172,   173,
     174,    -1,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,
      -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,   193,
      -1,    -1,   196,   197,    -1,   199,   200,   201,   202,   203,
      -1,   205,   206,     3,     4,     5,     6,     7,    -1,    -1,
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
     110,   111,    -1,   113,   114,    -1,   116,    -1,    -1,   119,
     120,   121,   122,   123,   124,   125,    -1,   127,   128,   129,
      -1,   131,   132,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,   141,   142,   143,   144,    -1,   146,   147,   148,   149,
     150,    -1,    -1,    -1,   154,    -1,    -1,   157,    -1,    -1,
      -1,    -1,   162,   163,   164,   165,   166,    -1,   168,   169,
      -1,   171,   172,   173,   174,    -1,    -1,   177,    -1,    -1,
     180,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,
      -1,   191,   192,   193,    -1,    -1,   196,   197,    -1,   199,
     200,   201,   202,   203,    -1,   205,   206,     3,     4,     5,
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
     116,    -1,    -1,   119,   120,   121,   122,   123,   124,   125,
      -1,   127,   128,   129,    -1,   131,   132,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,   142,   143,   144,    -1,
     146,   147,   148,   149,   150,    -1,    -1,    -1,   154,    -1,
      -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,
     166,    -1,   168,   169,    -1,   171,   172,   173,   174,    -1,
      -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,
     186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,    -1,
     196,   197,    -1,   199,   200,   201,   202,   203,    -1,   205,
     206,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,
      92,    93,    94,    95,    96,    -1,    98,    -1,   100,    -1,
      -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,   111,
      -1,   113,   114,    -1,   116,    -1,    -1,   119,   120,   121,
     122,   123,   124,   125,    -1,   127,   128,   129,    -1,   131,
     132,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,   144,    -1,   146,   147,   148,   149,   150,    -1,
      -1,    -1,   154,    -1,    -1,   157,    -1,    -1,    -1,    -1,
     162,   163,   164,   165,   166,    -1,   168,   169,    -1,   171,
     172,   173,   174,    -1,    -1,   177,    -1,    -1,   180,    -1,
      -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,
     192,   193,    -1,    -1,   196,   197,    -1,   199,   200,    -1,
     202,   203,    -1,   205,   206,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    86,    87,
      -1,    -1,    -1,    91,    92,    93,    94,    -1,    96,    -1,
      98,    -1,   100,   101,    -1,   103,   104,    -1,    -1,    -1,
     108,   109,   110,   111,    -1,   113,   114,    -1,   116,    -1,
      -1,   119,   120,   121,   122,   123,   124,   125,    -1,   127,
     128,   129,    -1,   131,   132,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,   141,   142,   143,   144,    -1,   146,   147,
     148,   149,   150,    -1,    -1,    -1,   154,    -1,    -1,   157,
      -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,
     168,   169,    -1,   171,   172,   173,   174,    -1,    -1,   177,
      -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,   186,    -1,
      -1,    -1,    -1,   191,   192,   193,    -1,    -1,   196,   197,
      -1,   199,   200,    -1,   202,   203,    -1,   205,   206,     3,
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
     114,    -1,   116,    -1,    -1,   119,   120,   121,   122,   123,
     124,   125,    -1,   127,   128,   129,    -1,   131,   132,    -1,
      -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
     144,    -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,
     154,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,
     164,   165,   166,    -1,   168,   169,    -1,   171,   172,   173,
     174,    -1,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,
      -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,   193,
      -1,    -1,   196,   197,    -1,   199,   200,   201,   202,   203,
      -1,   205,   206,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    77,    78,    79,
      80,    81,    82,    83,    84,    -1,    86,    87,    -1,    -1,
      -1,    91,    92,    93,    94,    -1,    96,    -1,    98,    -1,
     100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,
     110,   111,    -1,   113,   114,    -1,   116,    -1,    -1,   119,
     120,   121,   122,   123,   124,   125,    -1,   127,   128,   129,
      -1,   131,   132,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,   141,   142,   143,   144,    -1,   146,   147,   148,   149,
     150,    -1,    -1,    -1,   154,    -1,    -1,   157,    -1,    -1,
      -1,    -1,   162,   163,   164,   165,   166,    -1,   168,   169,
      -1,   171,   172,   173,   174,    -1,    -1,   177,    -1,    -1,
     180,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,
      -1,   191,   192,   193,    -1,    -1,   196,   197,    -1,   199,
     200,    -1,   202,   203,    -1,   205,   206,     3,     4,     5,
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
     116,    -1,    -1,   119,   120,   121,   122,   123,   124,   125,
      -1,   127,   128,   129,    -1,   131,   132,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,   142,   143,   144,    -1,
     146,   147,   148,   149,   150,    -1,    -1,    -1,   154,    -1,
      -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,
     166,    -1,   168,   169,    -1,   171,   172,   173,   174,    -1,
      -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,
     186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,    -1,
     196,   197,    -1,   199,   200,   201,   202,   203,    -1,   205,
     206,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,    91,
      92,    93,    94,    -1,    96,    -1,    98,    99,   100,    -1,
      -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,   111,
      -1,   113,   114,    -1,   116,    -1,    -1,   119,   120,   121,
     122,   123,   124,   125,    -1,   127,   128,   129,    -1,   131,
     132,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,   144,    -1,   146,   147,   148,   149,   150,    -1,
      -1,    -1,   154,    -1,    -1,   157,    -1,    -1,    -1,    -1,
     162,   163,   164,   165,   166,    -1,   168,   169,    -1,   171,
     172,   173,   174,    -1,    -1,   177,    -1,    -1,   180,    -1,
      -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,
     192,   193,    -1,    -1,   196,   197,    -1,   199,   200,    -1,
     202,   203,    -1,   205,   206,     3,     4,     5,     6,     7,
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
      -1,   119,   120,   121,   122,   123,   124,   125,    -1,   127,
     128,   129,    -1,   131,   132,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,   141,   142,   143,   144,    -1,   146,   147,
     148,   149,   150,    -1,    -1,    -1,   154,    -1,    -1,   157,
      -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,
     168,   169,    -1,   171,   172,   173,   174,    -1,    -1,   177,
      -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,   186,    -1,
      -1,    -1,    -1,   191,   192,   193,    -1,    -1,   196,   197,
      -1,   199,   200,    -1,   202,   203,    -1,   205,   206,     3,
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
     114,    -1,   116,    -1,    -1,   119,   120,   121,   122,   123,
     124,   125,    -1,   127,   128,   129,    -1,   131,   132,    -1,
      -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
     144,    -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,
     154,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,
     164,   165,   166,    -1,   168,   169,    -1,   171,   172,   173,
     174,    -1,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,
      -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,   193,
      -1,    -1,   196,   197,    -1,   199,   200,   201,   202,   203,
      -1,   205,   206,     3,     4,     5,     6,     7,    -1,    -1,
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
     110,   111,    -1,   113,   114,    -1,   116,    -1,    -1,   119,
     120,   121,   122,   123,   124,   125,    -1,   127,   128,   129,
      -1,   131,   132,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,   141,   142,   143,   144,    -1,   146,   147,   148,   149,
     150,    -1,    -1,    -1,   154,    -1,    -1,   157,    -1,    -1,
      -1,    -1,   162,   163,   164,   165,   166,    -1,   168,   169,
      -1,   171,   172,   173,   174,    -1,    -1,   177,    -1,    -1,
     180,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,
      -1,   191,   192,   193,    -1,    -1,   196,   197,    -1,   199,
     200,   201,   202,   203,    -1,   205,   206,     3,     4,     5,
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
     116,    -1,    -1,   119,   120,   121,   122,   123,   124,   125,
      -1,   127,   128,   129,    -1,   131,   132,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,   142,   143,   144,    -1,
     146,   147,   148,   149,   150,    -1,    -1,    -1,   154,    -1,
      -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,
     166,    -1,   168,   169,    -1,   171,   172,   173,   174,    -1,
      -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,
     186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,    -1,
     196,   197,    -1,   199,   200,   201,   202,   203,    -1,   205,
     206,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,   113,   114,    -1,   116,    -1,    -1,   119,   120,   121,
     122,   123,   124,   125,    -1,   127,   128,   129,    -1,   131,
     132,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,   144,    -1,   146,   147,   148,   149,   150,    -1,
      -1,    -1,   154,    -1,    -1,   157,    -1,    -1,    -1,    -1,
     162,   163,   164,   165,   166,    -1,   168,   169,    -1,   171,
     172,   173,   174,    -1,    -1,   177,    -1,    -1,   180,    -1,
      -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,
     192,   193,    -1,    -1,   196,   197,    -1,   199,   200,   201,
     202,   203,    -1,   205,   206,     3,     4,     5,     6,     7,
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
      -1,   119,   120,   121,   122,   123,   124,   125,    -1,   127,
     128,   129,    -1,   131,   132,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,   141,   142,   143,   144,    -1,   146,   147,
     148,   149,   150,    -1,    -1,    -1,   154,    -1,    -1,   157,
      -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,
     168,   169,    -1,   171,   172,   173,   174,    -1,    -1,   177,
      -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,   186,    -1,
      -1,    -1,    -1,   191,   192,   193,    -1,    -1,   196,   197,
      -1,   199,   200,   201,   202,   203,    -1,   205,   206,     3,
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
     114,    -1,   116,    -1,    -1,   119,   120,   121,   122,   123,
     124,   125,    -1,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
     144,    -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,
     154,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,
     164,   165,   166,    -1,   168,   169,    -1,   171,   172,   173,
      -1,    -1,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,
      -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,   193,
      -1,    -1,   196,   197,    -1,   199,   200,    -1,   202,   203,
      -1,   205,   206,     3,     4,     5,     6,     7,    -1,    -1,
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
     110,   111,    -1,   113,   114,    -1,   116,    -1,    -1,   119,
     120,   121,   122,   123,   124,   125,    -1,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,   141,   142,   143,   144,    -1,   146,   147,   148,   149,
     150,    -1,    -1,    -1,   154,    -1,    -1,   157,    -1,    -1,
      -1,    -1,   162,   163,   164,   165,   166,    -1,   168,   169,
      -1,   171,   172,   173,    -1,    -1,    -1,   177,    -1,    -1,
     180,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,
      -1,   191,   192,   193,    -1,    -1,   196,   197,    -1,   199,
     200,    -1,   202,   203,    -1,   205,   206,     3,     4,     5,
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
     116,    -1,    -1,   119,   120,   121,   122,   123,   124,   125,
      -1,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,   142,   143,   144,    -1,
     146,   147,   148,   149,   150,    -1,    -1,    -1,   154,    -1,
      -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,
     166,    -1,   168,   169,    -1,   171,   172,   173,    -1,    -1,
      -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,
     186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,    -1,
     196,   197,    -1,   199,   200,    -1,   202,   203,    -1,   205,
     206,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,   113,   114,    -1,   116,    -1,    -1,   119,   120,   121,
     122,   123,   124,   125,    -1,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,   144,    -1,   146,   147,   148,   149,   150,    -1,
      -1,    -1,   154,    -1,    -1,   157,    -1,    -1,    -1,    -1,
     162,   163,   164,   165,   166,    -1,   168,   169,    -1,   171,
     172,   173,    -1,    -1,    -1,   177,    -1,    -1,   180,    -1,
      -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,
     192,   193,    -1,    -1,   196,   197,    -1,   199,   200,    -1,
     202,   203,    -1,   205,   206,     3,     4,     5,     6,     7,
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
      -1,   119,   120,   121,   122,   123,   124,   125,    -1,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,   141,   142,   143,   144,    -1,   146,   147,
     148,   149,   150,    -1,    -1,    -1,   154,    -1,    -1,   157,
      -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,
     168,   169,    -1,   171,   172,   173,    -1,    -1,    -1,   177,
      -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,   186,    -1,
      -1,    -1,    -1,   191,   192,   193,    -1,    -1,   196,   197,
      -1,   199,   200,    -1,   202,   203,    -1,   205,   206,     3,
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
     114,    -1,   116,    -1,    -1,   119,   120,   121,   122,   123,
     124,   125,    -1,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
     144,    -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,
     154,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,
     164,   165,   166,    -1,   168,   169,    -1,   171,   172,   173,
      -1,    -1,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,
      -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,   193,
      -1,    -1,   196,   197,    -1,   199,   200,    -1,   202,   203,
      -1,   205,   206,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     120,   121,   122,   123,   124,   125,    -1,    -1,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,   141,   142,   143,   144,    -1,   146,   147,   148,   149,
     150,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,
      -1,    -1,   162,   163,   164,   165,   166,    -1,   168,   169,
      -1,   171,   172,   173,    -1,    -1,    -1,   177,    -1,    -1,
     180,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,
      -1,   191,   192,   193,    -1,    -1,   196,   197,    -1,    -1,
      -1,    -1,   202,   203,    -1,   205,   206,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   120,   121,   122,   123,   124,   125,
      -1,    -1,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,   142,   143,   144,    -1,
     146,   147,   148,   149,   150,    -1,    -1,    -1,    -1,    -1,
      -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,
     166,    -1,   168,   169,    -1,   171,   172,   173,    -1,    -1,
      -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,
     186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,    -1,
     196,   197,    -1,   199,    -1,    -1,   202,   203,    -1,   205,
     206,     3,     4,     5,     6,     7,    -1,    -1,    -1,    31,
      -1,    13,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,   121,
     122,   123,   124,   125,    -1,    -1,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,   144,    -1,   146,   147,   148,   149,   150,    -1,
      -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,
     162,   163,   164,   165,   166,    -1,   168,   169,    -1,   171,
     172,   173,    -1,   175,    -1,   177,    -1,    -1,   180,    -1,
      -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,
     192,   193,    -1,    -1,   196,   197,    -1,    -1,    -1,    -1,
     202,   203,    -1,   205,   206,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   120,   121,   122,   123,   124,   125,    -1,    -1,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,   141,   142,   143,   144,    -1,   146,   147,
     148,   149,   150,    -1,    -1,    -1,    -1,    -1,    -1,   157,
      -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,
     168,   169,    -1,   171,   172,   173,    -1,    -1,    -1,   177,
      -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,   186,    -1,
      -1,    -1,    -1,   191,   192,   193,    -1,    -1,   196,   197,
      -1,    -1,   200,    -1,   202,   203,    -1,   205,   206,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   120,   121,   122,   123,
     124,   125,    -1,    -1,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
     144,    -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,
      -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,
     164,   165,   166,    -1,   168,   169,    -1,   171,   172,   173,
      -1,   175,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,
      -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,   193,
      -1,    -1,   196,   197,    -1,    -1,    -1,    -1,   202,   203,
      -1,   205,   206,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     120,   121,   122,   123,   124,   125,    -1,    -1,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,   141,   142,   143,   144,    -1,   146,   147,   148,   149,
     150,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,
      -1,    -1,   162,   163,   164,   165,   166,    -1,   168,   169,
      -1,   171,   172,   173,    -1,    -1,    -1,   177,    -1,    -1,
     180,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,
      -1,   191,   192,   193,    -1,    -1,   196,   197,    -1,    -1,
      -1,    -1,   202,   203,    -1,   205,   206,     3,     4,     5,
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
      -1,    -1,    -1,    -1,   120,   121,   122,   123,   124,   125,
      -1,    -1,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,   142,   143,   144,    -1,
     146,   147,   148,   149,   150,    -1,    -1,    -1,    -1,    -1,
      -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,
     166,    -1,   168,   169,    -1,   171,   172,   173,    -1,    -1,
      -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,
     186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,    -1,
     196,   197,    10,    11,    12,    -1,   202,   203,    -1,   205,
     206,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    69,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,   108,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,   121,
     122,   123,   124,   125,    -1,    -1,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,   144,    -1,   146,   147,   148,   149,   150,    -1,
      -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,
     162,   163,   164,   165,   166,    -1,   168,   169,    -1,   171,
     172,   173,    -1,    -1,    -1,   177,    -1,    -1,   180,    -1,
      -1,    -1,    -1,   201,   186,    -1,    -1,    -1,    -1,   191,
     192,   193,    -1,    -1,   196,   197,    -1,    -1,    -1,    -1,
     202,   203,    -1,   205,   206,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   120,   121,   122,   123,   124,   125,    -1,    -1,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,   141,   142,   143,   144,    -1,   146,   147,
     148,   149,   150,    -1,    -1,    -1,    -1,    -1,    -1,   157,
      -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,
     168,   169,    -1,   171,   172,   173,    -1,    -1,    -1,   177,
      -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,   186,    -1,
      -1,    -1,    -1,   191,   192,   193,    -1,    -1,   196,   197,
      10,    11,    12,    -1,   202,   203,    -1,   205,   206,     3,
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
      -1,    -1,    -1,    -1,    -1,    -1,   120,   121,   122,   123,
     124,   125,    -1,    -1,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
     144,    -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,
      -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,
     164,   165,   166,    -1,   168,   169,    -1,   171,   172,   173,
      -1,    -1,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,
      -1,   201,   186,    -1,    -1,    -1,    -1,   191,   192,   193,
      -1,    -1,   196,   197,    -1,   199,    11,    12,   202,   203,
      -1,   205,   206,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    30,    31,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,    -1,    69,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     120,   121,   122,   123,   124,   125,    -1,    -1,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,   141,   142,   143,   144,    -1,   146,   147,   148,   149,
     150,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,
      -1,    -1,   162,   163,   164,   165,   166,    -1,   168,   169,
      -1,   171,   172,   173,    -1,    -1,    -1,   177,    -1,    -1,
     180,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,
      -1,   191,   192,   193,    -1,    -1,   196,   197,    -1,   199,
      -1,    -1,   202,   203,    -1,   205,   206,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   120,   121,   122,   123,   124,   125,
      -1,    -1,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,   142,   143,   144,    -1,
     146,   147,   148,   149,   150,    -1,    -1,    -1,    -1,    -1,
      -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,
     166,    -1,   168,   169,    -1,   171,   172,   173,    -1,    -1,
      -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,
     186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,    -1,
     196,   197,    10,    11,    12,    -1,   202,   203,    -1,   205,
     206,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,   121,
     122,   123,   124,   125,    -1,    -1,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,   144,    -1,   146,   147,   148,   149,   150,    -1,
      -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,
     162,   163,   164,   165,   166,    -1,   168,   169,    -1,   171,
     172,   173,    -1,    -1,    -1,   177,    -1,    -1,   180,    -1,
      -1,   199,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,
     192,   193,    -1,    -1,   196,   197,   198,    -1,    -1,    -1,
     202,   203,    -1,   205,   206,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   120,   121,   122,   123,   124,   125,    -1,    -1,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,   141,   142,   143,   144,    -1,   146,   147,
     148,   149,   150,    -1,    -1,    -1,    -1,    -1,    -1,   157,
      -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,
     168,   169,    -1,   171,   172,   173,    -1,    -1,    -1,   177,
      -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,   186,    -1,
      -1,    -1,    -1,   191,   192,   193,    -1,    -1,   196,   197,
      -1,    -1,    -1,    -1,   202,   203,    -1,   205,   206,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    32,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   120,   121,   122,   123,
     124,   125,    -1,    -1,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
     144,    -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,
      -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,
     164,   165,   166,    -1,   168,   169,    -1,   171,   172,   173,
      -1,    -1,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,
      -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,   193,
      -1,    -1,   196,   197,    -1,    -1,    -1,    -1,   202,   203,
      -1,   205,   206,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      50,    51,    -1,    -1,    -1,    -1,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     120,   121,   122,   123,   124,   125,    -1,    -1,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,   141,   142,   143,   144,    -1,   146,   147,   148,   149,
     150,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,
      -1,    -1,   162,   163,   164,   165,   166,    -1,   168,   169,
      -1,   171,   172,   173,    -1,    -1,    -1,   177,    -1,    -1,
     180,    -1,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,
      -1,   191,   192,   193,    -1,    -1,   196,   197,    -1,    -1,
      -1,    -1,   202,   203,    -1,   205,   206,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    50,    51,    -1,    -1,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   120,   121,   122,   123,   124,   125,
      -1,    -1,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,   142,   143,   144,    -1,
     146,   147,   148,   149,   150,    -1,    -1,    -1,    -1,    -1,
      -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,
     166,    -1,   168,   169,    -1,   171,   172,   173,    -1,    -1,
      -1,   177,    -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,
     186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,    -1,
     196,   197,    -1,    -1,    -1,    -1,   202,   203,    -1,   205,
     206,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    -1,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    91,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,   121,
     122,   123,   124,   125,    -1,    -1,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,   144,    -1,   146,   147,   148,   149,   150,    -1,
      -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,
     162,   163,   164,   165,   166,    -1,   168,   169,    -1,   171,
     172,   173,    -1,    -1,    -1,   177,    -1,    -1,   180,    -1,
      -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,   191,
     192,   193,    -1,    -1,   196,   197,    -1,    -1,    -1,    -1,
     202,   203,    -1,   205,   206,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    50,    51,    -1,    -1,    -1,    -1,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      78,    79,    80,    81,    82,    83,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   120,   121,   122,   123,   124,   125,    -1,    -1,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     138,   139,   140,   141,   142,   143,   144,    -1,   146,   147,
     148,   149,   150,    -1,    -1,    -1,    -1,    -1,    -1,   157,
      -1,    -1,    -1,    -1,   162,   163,   164,   165,   166,    -1,
     168,   169,    -1,   171,   172,   173,    -1,    -1,    -1,   177,
      -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,   186,    -1,
      -1,    -1,    -1,   191,   192,   193,    -1,    -1,   196,   197,
      -1,    -1,    -1,    -1,   202,   203,    -1,   205,   206,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   120,   121,   122,   123,
     124,   125,    -1,    -1,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
     144,    -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,
      -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,
     164,   165,   166,    -1,   168,   169,    -1,   171,   172,   173,
      -1,    -1,    -1,   177,    -1,    -1,   180,    -1,    -1,    -1,
      -1,    -1,   186,    -1,    -1,    -1,    -1,   191,   192,   193,
      -1,    -1,   196,   197,    10,    11,    12,    -1,   202,   203,
      -1,   205,   206,     3,     4,     5,     6,     7,    -1,    -1,
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
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     120,   121,   122,   123,   124,   125,    -1,    -1,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,   141,   142,   143,   144,    -1,   146,   147,   148,   149,
     150,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,
      -1,    -1,   162,   163,   164,   165,   166,    -1,   168,   169,
      -1,   171,   172,   173,    -1,    -1,    -1,   177,    -1,    -1,
     180,    -1,    -1,   199,    -1,    -1,   186,    -1,    -1,    -1,
      -1,   191,   192,   193,    -1,    -1,   196,   197,    10,    11,
      12,    -1,   202,   203,    -1,   205,   206,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    27,    -1,    13,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    69,    -1,    -1,
      56,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
     102,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   120,   121,   122,   123,   124,   125,
      -1,    -1,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,   140,   141,   142,   143,   144,    -1,
     146,   147,   148,   149,   150,    -1,    -1,    -1,    -1,    -1,
      -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,
     166,    -1,   168,   169,    -1,   171,   172,   173,    -1,     3,
       4,   177,     6,     7,   180,    -1,    10,    11,    12,    13,
     186,    -1,    -1,    -1,    -1,   191,   192,   193,    -1,    -1,
     196,   197,    -1,    -1,    28,    29,   202,   203,    -1,   205,
     206,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,    81,   129,    -1,   131,   132,   133,
     134,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
     144,   145,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,   162,    10,
      11,    12,    13,    -1,   168,   169,    -1,   171,   172,   173,
     174,    -1,   176,    -1,    -1,   179,    -1,    28,    29,    -1,
      31,    -1,    -1,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,   196,    -1,    -1,    -1,   200,    -1,   202,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,   166,
      -1,   168,   169,    -1,   171,   172,   173,    68,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   196,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,    -1,   129,    -1,
      -1,   132,   133,   134,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   162,    -1,    -1,   165,   166,    -1,   168,   169,    -1,
     171,   172,   173,   174,    -1,   176,    -1,    -1,   179,     3,
       4,    -1,     6,     7,    -1,   186,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   196,   197,    -1,    -1,    -1,
     201,    -1,    -1,    -1,    28,    29,    -1,    31,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    57,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    -1,    69,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,    -1,   129,    -1,    -1,   132,   133,
     134,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
     144,   145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   162,    -1,
      -1,   165,   166,    -1,   168,   169,    -1,   171,   172,   173,
     174,    -1,   176,    -1,    -1,   179,     3,     4,    -1,     6,
       7,    -1,   186,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,   196,   197,    -1,    -1,    -1,   201,    -1,    -1,
      -1,    28,    29,    -1,    31,    -1,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      57,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    69,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,    -1,   129,    -1,   131,   132,   133,   134,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,   144,   145,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   162,    -1,    -1,   165,   166,
      -1,   168,   169,    -1,   171,   172,   173,   174,    -1,   176,
      -1,    -1,   179,     3,     4,    -1,     6,     7,    -1,   186,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,   196,
     197,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    29,
      -1,    31,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    68,    -1,
      -1,    71,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,    -1,   129,
      -1,    -1,   132,   133,   134,    -1,    -1,    -1,   138,   139,
     140,   141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   162,    -1,    -1,   165,   166,    -1,   168,   169,
      -1,   171,   172,   173,   174,   103,   176,    -1,    -1,   179,
      -1,     3,     4,    -1,     6,     7,   186,   187,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,   196,   197,   126,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    29,    -1,    31,
      -1,   139,   140,   141,   142,   143,   144,   145,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   162,    57,    -1,   165,   166,    -1,
     168,   169,    -1,   171,   172,   173,    68,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   196,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,    -1,   129,    -1,    -1,
     132,   133,   134,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,   144,   145,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
     162,    -1,    -1,   165,   166,    -1,   168,   169,   103,   171,
     172,   173,   174,    -1,   176,    -1,    -1,   179,     3,     4,
       5,     6,     7,    -1,   186,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,   196,   197,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,   140,   141,   142,   143,   144,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,   162,    -1,    -1,
     165,    -1,    57,   168,   169,    -1,   171,   172,   173,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,   196,    -1,    -1,    -1,    -1,    -1,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
      -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   162,   163,   164,
      -1,    81,    -1,   168,   169,    -1,   171,   172,   173,   174,
      -1,   176,   177,    -1,   179,    -1,    -1,    -1,    -1,    -1,
      -1,   186,   187,   103,   189,    -1,   191,   192,    -1,     3,
       4,   196,     6,     7,    -1,    -1,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    29,    -1,    -1,    -1,    -1,
     140,   141,   142,   143,   144,    -1,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    57,    -1,   165,    -1,    -1,   168,   169,
      -1,   171,   172,   173,    -1,    -1,    69,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   196,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,    81,   129,    -1,   131,   132,   133,
     134,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
     144,   145,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,
      -1,    -1,     3,     4,    -1,     6,     7,    -1,   162,    10,
      11,    12,    13,    -1,   168,   169,    -1,   171,   172,   173,
     174,    -1,   176,    -1,    -1,   179,    -1,    28,    29,    -1,
      -1,    -1,    -1,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,   196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,   165,    -1,
      -1,   168,   169,    -1,   171,   172,   173,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   196,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,    81,   129,    -1,
     131,   132,   133,   134,    -1,    -1,    -1,   138,   139,   140,
     141,   142,   143,   144,   145,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,   162,    10,    11,    12,    13,    -1,   168,   169,    -1,
     171,   172,   173,   174,    -1,   176,    -1,    -1,   179,    -1,
      28,    29,    -1,    -1,    -1,    -1,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,   196,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,
      -1,    -1,    -1,    -1,   168,   169,    -1,   171,   172,   173,
      -1,    -1,    -1,    71,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   196,    -1,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
      -1,   129,    -1,    -1,   132,   133,   134,    -1,    -1,    -1,
     138,   139,   140,   141,   142,   143,   144,   145,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,
     168,   169,    -1,   171,   172,   173,   174,    -1,   176,    -1,
      -1,   179,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   196,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    30,
      31,   199,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    68,    -1,    -1,
      -1,   139,   140,   141,   142,   143,   144,   145,    69,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   199,
      -1,    -1,    -1,    -1,   162,    -1,    -1,   165,   166,    -1,
     168,   169,   103,   171,   172,   173,    -1,   175,    31,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   186,   120,
     121,   122,   123,   124,   125,    -1,    -1,    -1,   196,   197,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   199,   139,   140,
     141,   142,   143,   144,   145,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    81,    -1,
      -1,   162,    -1,    -1,   165,   166,    -1,   168,   169,    -1,
     171,   172,   173,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,
      -1,   192,    -1,    68,    -1,   196,   197,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    81,   198,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
     143,   144,   145,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   162,
      -1,    -1,   165,   166,    -1,   168,   169,    -1,   171,   172,
     173,    -1,   175,    -1,    -1,    -1,    -1,    32,    -1,    -1,
      -1,    -1,    -1,   186,   139,   140,   141,   142,   143,   144,
     145,    -1,    -1,   196,   197,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    -1,    -1,    -1,   162,    -1,    -1,
     165,   166,    -1,   168,   169,    70,   171,   172,   173,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    -1,    -1,    -1,
      -1,   186,    -1,    -1,    -1,    -1,    91,    10,    11,    12,
      -1,   196,   197,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,   139,   140,   141,   142,   143,   144,
      -1,   146,   147,   148,   149,   150,    69,    -1,    -1,    -1,
      -1,    -1,   157,    38,    -1,    -1,    -1,   162,   163,   164,
     165,   166,    -1,   168,   169,    -1,   171,   172,   173,    -1,
      -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   186,    -1,    -1,    -1,    70,   191,    -1,    -1,    -1,
      -1,   196,   197,    78,    79,    80,    81,    -1,    83,    84,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   137,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
      -1,   146,   147,   148,   149,   150,    -1,    -1,    -1,    -1,
      -1,    -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,
     165,   166,    -1,   168,   169,    -1,   171,   172,   173,    -1,
      50,    51,   177,    -1,    -1,    -1,    56,    -1,    58,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   191,    -1,    -1,    -1,
      70,   196,   197,    -1,    -1,    -1,    -1,   202,    78,    79,
      80,    81,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,    69,   146,   147,   148,   149,
     150,    -1,    -1,    -1,    -1,    -1,    -1,   157,    -1,    -1,
      -1,    -1,   162,   163,   164,   165,   166,    -1,   168,   169,
      -1,   171,   172,   173,    70,    -1,    -1,   177,    -1,    -1,
      -1,    -1,    78,    79,    80,    81,   186,    83,    84,    -1,
      -1,   191,    -1,    -1,    -1,    91,   196,   197,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,   137,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   144,    -1,
     146,   147,   148,   149,   150,    -1,    -1,    -1,    -1,    -1,
      -1,   157,    -1,    -1,    -1,    -1,   162,   163,   164,   165,
     166,    -1,   168,   169,    -1,   171,   172,   173,    70,    -1,
      -1,   177,    -1,    -1,    -1,    -1,    78,    79,    80,    81,
      -1,    83,    84,    -1,    -1,   191,    -1,    -1,    -1,    91,
     196,   197,    -1,    -1,   200,    -1,   202,    -1,    -1,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,   144,    -1,   146,   147,   148,   149,   150,    -1,
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
      -1,    -1,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,
     141,   142,   143,   144,   103,   146,   147,   148,   149,   150,
      -1,    -1,    -1,    -1,    -1,    -1,   157,    81,    -1,    83,
      84,   162,   163,   164,   165,   166,    -1,   168,   169,    -1,
     171,   172,   173,    -1,    -1,    -1,   177,    -1,    -1,   103,
      -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
     191,    -1,    -1,    -1,    -1,   196,   197,    -1,    -1,    -1,
      -1,   202,    -1,    -1,    -1,    -1,    -1,   166,    -1,   168,
     169,    -1,   171,   172,   173,    -1,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   196,    -1,    -1,
      -1,   200,   166,   202,   168,   169,    -1,   171,   172,   173,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   196,    -1,    -1,    -1,   200,    -1,   202,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    32,    -1,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    12,    57,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,    69,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69
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
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   138,   139,   146,
     147,   148,   149,   150,   162,   163,   164,   174,   176,   177,
     179,   186,   187,   189,   191,   192,   214,   416,   417,     9,
     199,   166,   170,   214,   329,   330,   331,   199,    83,   340,
     254,   311,   494,   494,    14,   255,   201,   306,   307,   494,
      14,    83,   358,   198,   197,   504,   196,   501,   361,   504,
     305,   201,   198,   440,   137,   137,    32,   234,   284,   285,
     231,   428,   428,   428,   201,   199,   199,   428,   419,   314,
     517,   322,   323,   427,   319,    14,    32,    51,   324,   327,
       9,    36,   198,    31,    50,    53,    14,     9,   199,   216,
     495,   340,    14,   517,   254,   199,    14,   358,    38,    83,
     407,   200,   502,   503,   517,   199,   200,   332,   504,   501,
     201,   504,   440,   440,   231,    99,   250,   201,   214,   227,
     315,   316,   317,     9,   434,     9,   434,   201,   428,   417,
     417,    68,   325,   330,   330,    31,    50,    53,   428,    83,
     182,   197,   199,   428,   495,   428,    83,     9,   435,   229,
       9,   435,    14,   505,   229,   200,   332,   332,    97,   199,
     115,   241,   161,   102,   517,   183,   427,   173,    14,   506,
     312,   197,    38,    83,   198,   201,   503,   517,   201,   229,
     199,   197,   179,   253,   214,   335,   336,   183,   428,   183,
     296,   297,   453,   313,    83,   201,   419,   251,   176,   214,
     199,   198,     9,   435,   123,   124,   125,   338,   339,   296,
      83,   281,   199,   504,   453,   518,   198,   198,   199,   501,
     338,    38,    83,   175,   504,   200,   199,   200,   333,   518,
      83,   175,    14,    83,   501,   231,   229,    38,    83,   175,
      14,    83,   358,   333,   201,   201,    83,   175,    14,    83,
     358,    14,    83,   358,   358
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
     466,   467,   467,   467,   467,   467,   467,   467,   467,   467,
     468,   469,   469,   470,   470,   471,   471,   471,   472,   473,
     473,   473,   474,   474,   474,   474,   475,   475,   476,   476,
     476,   476,   476,   476,   477,   477,   477,   477,   477,   478,
     478,   478,   478,   478,   478,   479,   479,   480,   480,   480,
     480,   480,   480,   480,   480,   481,   481,   482,   482,   482,
     482,   483,   483,   484,   484,   484,   484,   485,   485,   485,
     485,   486,   486,   486,   486,   486,   486,   487,   487,   487,
     488,   488,   488,   488,   488,   488,   488,   488,   488,   488,
     488,   489,   489,   490,   490,   491,   491,   492,   492,   492,
     492,   493,   493,   494,   494,   495,   495,   496,   496,   497,
     497,   498,   498,   499,   500,   500,   500,   500,   501,   501,
     502,   502,   503,   503,   504,   504,   505,   505,   506,   507,
     507,   508,   508,   508,   508,   509,   509,   509,   510,   510,
     510,   510,   511,   511,   512,   512,   512,   512,   513,   514,
     515,   515,   516,   516,   517,   517,   517,   517,   517,   517,
     517,   517,   517,   517,   517,   518,   518
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
#line 752 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
#line 7206 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 755 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 7214 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 762 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 7220 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 763 "hphp.y" /* yacc.c:1646  */
    { }
#line 7226 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 766 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 7232 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 767 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7238 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 768 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7244 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 769 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7250 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 770 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 7256 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 771 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 7262 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 772 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 7270 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 775 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 7277 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 7283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 778 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 779 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 7295 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 780 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 7301 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 781 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7309 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 785 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7318 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 790 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7327 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 795 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7334 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 798 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7341 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 801 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7349 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 805 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7357 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 809 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7365 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 813 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7373 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 817 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useClass);
                                         _p->nns(T_USE); (yyval).reset();}
#line 7381 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 820 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 7388 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 825 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7394 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 826 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 827 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 828 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 829 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7418 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 830 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 831 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 834 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7454 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7466 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 917 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7472 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 919 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7478 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 924 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7484 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 925 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7491 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 931 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7497 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 935 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7503 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 936 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7509 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 938 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7515 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 940 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7521 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 945 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7527 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 946 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7534 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 952 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7540 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 956 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7547 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 958 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7554 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 960 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7561 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 965 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7567 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 967 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7573 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 970 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7579 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 972 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7585 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 973 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7591 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 978 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7600 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 985 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7609 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 993 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7616 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 996 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7623 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 1002 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 7629 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1003 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval)); }
#line 7635 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1008 "hphp.y" /* yacc.c:1646  */
    {
                                         _p->onUsing((yyval), (yyvsp[-2]), true, (yyvsp[-1]), nullptr);
                                       }
#line 7643 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 1015 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7649 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1016 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7655 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1021 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 7661 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1022 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval), (yyval), (yyvsp[0])); }
#line 7668 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1027 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7674 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1028 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7680 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1029 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7686 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1030 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7692 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1033 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7698 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1037 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7704 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1042 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7710 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1043 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7717 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1045 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7725 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1049 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7732 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1052 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7740 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1056 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7747 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1058 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7755 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1061 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7762 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1063 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7770 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1066 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1067 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1068 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7788 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1069 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7794 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1070 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7800 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1071 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7806 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1072 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7812 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1073 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7818 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1074 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7824 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1075 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7830 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1076 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7836 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1077 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 7842 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1078 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 7848 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1079 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 7854 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1080 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 7861 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1084 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7868 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1086 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 7876 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1091 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::LoopSwitch);}
#line 7883 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1093 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 7891 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1097 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 7899 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1106 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1107 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 7911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1110 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false); }
#line 7917 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1111 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 7923 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1113 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7931 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1117 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7939 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1121 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1125 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-2]), false, (yyvsp[-1]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7955 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1129 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7963 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1133 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1138 "hphp.y" /* yacc.c:1646  */
    { _p->onUsing((yyval), (yyvsp[-4]), false, (yyvsp[-2]), &(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);
                                         _p->popLabelScope(); }
#line 7979 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1141 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 7985 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1142 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 7994 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1146 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8000 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1147 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8006 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1148 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8012 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1149 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8018 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1150 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8024 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1151 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8030 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1152 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 8036 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1153 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8042 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1154 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8048 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1155 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 8054 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1156 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 8060 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1157 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 8070 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1179 "hphp.y" /* yacc.c:1646  */
    { _p->pushLabelScope(LS::Using);
                                         _p->onNewLabelScope(false);
                                         (yyval) = (yyvsp[-1]); }
#line 8078 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1185 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1; }
#line 8084 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1186 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8090 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1195 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), nullptr, (yyvsp[-2]));
                                         _p->onExprListElem((yyval), &(yyval), (yyvsp[0])); }
#line 8097 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1197 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0])); }
#line 8103 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 1207 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8109 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 1208 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8115 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1212 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false); }
#line 8121 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1213 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8127 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1222 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 8133 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1223 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8139 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1227 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope(LS::Finally);}
#line 8146 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1229 "hphp.y" /* yacc.c:1646  */
    { _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->popLabelScope();
                                         _p->onCompleteLabelScope(false);}
#line 8154 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1235 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8160 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1236 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8166 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1240 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 8172 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1241 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8178 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1245 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 8184 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1251 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8193 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1258 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8203 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1266 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8212 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1273 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8222 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1281 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 8231 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1287 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8241 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1296 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8248 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1300 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 8254 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1304 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 8261 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1308 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 8267 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1314 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8274 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 8292 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1332 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 8299 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 8317 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1349 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8324 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 1352 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8332 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1357 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 8339 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1360 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8347 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1366 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 8353 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1369 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 8359 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1373 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8366 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1376 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8377 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1384 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 8384 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1387 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 8395 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1395 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8401 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1396 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 8408 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1400 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8414 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1403 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 8420 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1406 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 8426 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1407 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 8432 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1408 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 8440 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1411 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 8446 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1412 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 8452 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1416 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8458 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1417 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8464 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1420 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8470 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1421 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8476 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1424 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8482 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1425 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8488 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1428 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 8494 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1430 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8500 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1433 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 8506 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1435 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8512 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1439 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8518 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1440 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8524 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1443 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8530 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1444 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8536 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1445 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8542 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1449 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8548 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1451 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8554 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1454 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8560 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1456 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8566 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8572 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1461 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8578 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1464 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8584 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1466 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8590 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1470 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8596 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1472 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8603 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1477 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8609 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1478 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8615 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1479 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8621 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1480 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8627 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1485 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8633 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1487 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8639 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1488 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8645 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1491 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8651 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1492 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8657 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1497 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8663 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1498 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8669 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1503 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8675 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1504 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8681 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1507 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8687 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1508 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8693 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1511 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8699 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1512 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8705 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1520 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8712 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1526 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8719 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1532 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8727 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1536 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8733 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1540 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8740 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1545 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8747 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1550 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8755 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1553 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8761 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1559 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8768 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1563 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8775 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1568 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1573 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8789 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1578 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8796 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1583 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8803 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1589 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8810 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1595 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8817 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1603 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 8824 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1608 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 8831 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1613 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 8839 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1617 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8845 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1620 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 8852 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1624 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 8859 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1628 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 8867 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1631 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8873 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1636 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8880 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1639 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8887 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1643 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8894 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1647 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8901 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1651 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8908 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1655 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8915 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1660 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8922 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1665 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1671 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8935 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1672 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8941 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1675 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,false);}
#line 8947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1676 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),true,false);}
#line 8953 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1677 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,true);}
#line 8959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1679 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),false, false);}
#line 8965 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1681 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),false,true);}
#line 8971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1683 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),true, false);}
#line 8977 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1687 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1688 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 8989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1691 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1692 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 9001 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1693 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 9007 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1697 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9013 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1699 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1700 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 9025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1701 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1706 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9037 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1707 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9043 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1710 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9050 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1715 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9056 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1721 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9062 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1722 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9068 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1725 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[0]));}
#line 9074 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1726 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 9081 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1729 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[-1]));}
#line 9087 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1730 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-3]));}
#line 9094 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 356:
#line 1732 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 9101 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1735 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL, true);}
#line 9108 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1737 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9114 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 359:
#line 1740 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9122 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1747 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9132 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1755 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 9140 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1762 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->closeActiveUsings();
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 9150 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1768 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 9156 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1770 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9162 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1772 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 9168 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1774 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 9174 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1776 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 9180 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1777 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 9187 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1780 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 9193 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1783 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9199 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1784 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 9205 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1785 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 9211 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1791 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 9217 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1796 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 9224 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1799 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 9232 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1806 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 9238 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1807 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 9245 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1812 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 9252 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1815 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 9258 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1822 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 9265 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1824 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 9271 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1828 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9277 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1835 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 9295 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1838 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 9306 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1844 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 9312 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1846 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 9318 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1847 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 9324 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1851 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,0,(yyvsp[0]),0);}
#line 9330 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 1853 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),0,(yyvsp[0]),0);}
#line 9336 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1858 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9342 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 394:
#line 1861 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9348 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1862 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 9354 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1866 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 9360 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1867 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 9366 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1871 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 9373 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1874 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 9380 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1879 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 9387 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1884 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 9393 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1885 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 9400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1887 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 9406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1891 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 9412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1892 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 9418 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1893 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 9424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1894 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 9430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1898 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1899 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 9442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1900 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 9448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1901 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 9454 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1902 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 9460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1904 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 9466 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1906 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 9472 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1910 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 9480 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1913 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 9486 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1914 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 9492 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1918 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9498 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1919 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9504 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1923 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9510 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1924 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 9516 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1927 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9522 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1928 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9528 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1931 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9534 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1932 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9540 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1935 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9546 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1937 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9552 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1940 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9558 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1941 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9564 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1942 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9570 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1943 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9576 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1944 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9582 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1945 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9588 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 1946 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9594 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 1950 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9600 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 1951 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9606 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 1954 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9612 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 1955 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9618 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 1956 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9624 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 1960 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9630 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 1962 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9636 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9642 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 1964 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9648 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 1968 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9654 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 1970 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9660 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 1974 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9666 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 1976 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9672 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 1980 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9680 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 1984 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9687 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 1988 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9693 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 1992 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9699 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 1996 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9705 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 1998 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9711 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 1999 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9717 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9723 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 2001 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9729 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 2002 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9735 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 2005 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9741 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 2006 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9747 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 2010 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9753 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 2011 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9759 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 2015 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 9765 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 2016 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 9771 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 9777 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 2018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9783 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 2022 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9789 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 2027 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9795 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 2031 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 9801 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 2035 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9807 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 2039 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 9813 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 2043 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9819 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 2048 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9825 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 2052 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9831 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 2056 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9837 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 2057 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9843 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 2058 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9849 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 2059 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9855 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 2060 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9861 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 2064 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9867 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 2069 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 9873 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 2070 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 9879 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 2071 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 9885 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 2074 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 9891 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 2075 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 9897 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 2076 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 9903 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 2077 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 9909 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 2078 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 9915 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 2079 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 9921 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 2080 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 9927 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 2081 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 9933 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 2082 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 9939 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 2083 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 9945 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 2084 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 9951 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 2085 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 9957 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 2086 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 9963 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 2087 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 9969 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 2088 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 9975 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 2089 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 9981 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 2090 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 9987 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 2091 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 9993 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 2092 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 9999 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 2093 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 10005 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2094 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 10011 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2095 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 10017 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2096 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 10023 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2097 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 10029 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2098 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 10035 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2099 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 10041 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2100 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 10047 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2101 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 10053 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2102 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 10059 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2103 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 10065 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2104 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 10071 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2105 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 10077 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2106 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 10083 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2107 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 10089 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2108 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 10095 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2109 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 10101 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2110 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 10107 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2111 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 10113 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2112 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 10119 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2113 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 10125 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2114 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 10131 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2115 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 10137 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2116 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 10143 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2117 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 10149 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2118 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 10156 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2120 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 10162 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2121 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 10169 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2123 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 10175 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2125 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 10181 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2126 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10187 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2127 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 10193 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2128 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 10199 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2129 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 10205 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2130 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10211 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2131 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 10217 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2132 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 10223 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2133 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 10229 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2134 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 10235 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2135 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 10241 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2136 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 10247 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2137 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 10253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2138 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 10259 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2139 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 10265 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2140 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10271 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2141 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10277 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2142 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2143 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2144 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10295 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2145 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10301 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2146 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10307 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2147 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10313 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2148 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 10319 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2149 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 10325 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2150 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2157 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 10337 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2158 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10343 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2163 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10352 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10364 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2178 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 10373 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10385 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10398 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10413 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2214 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10423 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10438 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2233 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10448 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10465 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10478 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10491 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2267 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 10501 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10514 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2286 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10520 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2287 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 10526 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2289 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10532 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
#line 2293 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10539 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2295 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10545 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
#line 2302 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10551 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
#line 2305 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10557 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
#line 2312 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10563 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2315 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10569 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
#line 2320 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10575 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2321 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10581 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2326 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10587 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2327 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10593 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2331 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval), (yyvsp[-1]), T_ARRAY);}
#line 10599 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2335 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10605 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2336 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10611 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2341 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10617 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2342 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10623 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2347 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10629 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2348 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10635 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2353 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10641 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2354 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10647 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2360 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10653 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2362 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10659 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2367 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10665 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2368 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10671 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2374 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10677 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2376 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10683 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2380 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10689 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2384 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10695 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2388 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10701 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2392 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10707 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2396 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10713 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2400 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10719 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2404 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10725 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2408 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10731 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2412 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10737 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2416 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10743 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2420 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10749 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2424 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10755 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2428 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10761 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2432 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10767 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2436 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10773 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2441 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10779 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2442 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10785 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2447 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10791 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2448 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10797 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2453 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10803 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2454 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10809 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2459 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10817 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2466 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10825 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2473 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10831 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2475 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10837 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2479 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10843 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2480 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10849 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2481 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10855 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2482 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10861 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10867 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2484 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10873 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2485 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10879 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2486 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10885 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2487 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10892 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2489 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10898 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2490 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10904 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2494 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10910 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2495 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 10916 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2496 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 10922 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2497 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 10928 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2504 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 10934 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10948 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 10962 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2529 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 10968 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2530 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 10974 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2535 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10980 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2536 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10986 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2539 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-1]),0,(yyvsp[0]),0);}
#line 10992 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2540 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10998 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2543 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11005 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2547 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 11013 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2550 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11019 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 11031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2560 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11037 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2561 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11043 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2565 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11049 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2567 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 11055 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2569 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 11061 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2573 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11067 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2574 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11073 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2575 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11079 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2576 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11085 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2577 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11091 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2578 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11097 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2579 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11103 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2580 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11109 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2581 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11115 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2582 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11121 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2583 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11127 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2584 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11133 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2585 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11139 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2586 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11145 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2587 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11151 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2588 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11157 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2589 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11163 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2590 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11169 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2591 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11175 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2592 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11181 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2593 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11187 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2594 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11193 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2595 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11199 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2596 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11205 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2597 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11211 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2598 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11217 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2599 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11223 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2600 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11229 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2601 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11235 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2602 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11241 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2603 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11247 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2604 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2605 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11259 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2606 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11265 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2607 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11271 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2608 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11277 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2609 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2610 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2611 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11295 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2612 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11301 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2613 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11307 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2614 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11313 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2615 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11319 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2616 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11325 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2617 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2618 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11337 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2619 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11343 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2620 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11349 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2621 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11355 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2622 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11361 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2623 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11367 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2624 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11373 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2625 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11379 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2626 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11385 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2627 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11391 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2628 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11397 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2629 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11403 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2630 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11409 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2631 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11415 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2632 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11421 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2633 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11427 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2634 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11433 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2635 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11439 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2636 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11445 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2637 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11451 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2638 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11457 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2639 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11463 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2640 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11469 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2641 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11475 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2642 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11481 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2643 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11487 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2644 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11493 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2645 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11499 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2646 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11505 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2647 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11511 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2648 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11517 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2649 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11523 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2650 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11529 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2651 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11535 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2652 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11541 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2653 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11547 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2654 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11553 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2659 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 11559 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2663 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11565 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2664 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11571 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2668 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11577 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2669 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11583 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2670 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11589 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2671 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11596 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2673 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11603 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2677 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11609 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2686 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11615 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2689 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11621 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2690 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11628 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2692 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11635 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2702 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11641 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2706 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11647 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2707 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11653 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2708 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11659 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2712 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11665 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2713 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11671 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2714 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11677 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2718 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11683 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2719 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11689 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2720 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11695 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2724 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11701 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2725 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11707 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2729 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11713 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2730 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11719 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2731 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11725 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2732 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 11732 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2734 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 11738 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2735 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 11744 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2736 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 11750 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2737 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 11756 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2738 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 11762 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2739 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 11768 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2740 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 11774 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2741 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 11780 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2742 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 11786 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2745 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11792 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2747 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11798 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2751 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11804 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2752 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11810 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2754 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11816 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2755 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11822 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2757 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11828 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2758 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11834 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2759 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11840 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2760 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11846 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2761 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11852 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2762 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11858 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2763 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11864 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2764 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11870 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2765 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11876 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2767 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 11882 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2769 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 11888 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2771 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 11894 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2773 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 11900 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2775 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 11906 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2776 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 11912 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2777 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 11918 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2778 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 11924 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2779 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 11930 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2780 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 11936 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2781 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 11942 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2782 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 11948 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2783 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 11954 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2784 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 11960 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2785 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 11966 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2786 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 11972 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2787 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 11978 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2788 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 11984 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2789 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 11990 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2790 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11996 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2791 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12002 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2793 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 12008 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2795 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 12014 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2797 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 12020 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2799 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 12026 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2800 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 12032 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2802 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 12039 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2804 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 12045 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2807 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 12052 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2811 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 12058 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2814 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 12064 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2815 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 12070 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2819 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12076 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2820 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12082 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2826 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12088 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 829:
#line 2832 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 12094 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2833 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12100 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2837 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12106 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 832:
#line 2838 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 12112 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2839 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12118 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2840 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12124 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2841 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 12130 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2842 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 12136 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2844 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 12143 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2849 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12149 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2850 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12155 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2854 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12161 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2855 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12167 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2858 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 12173 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2859 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12179 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2865 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12185 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2867 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12191 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2869 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12197 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2870 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12203 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2874 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 12209 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2875 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 12215 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2876 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 12221 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2879 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 12227 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2881 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 12233 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2884 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 12239 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2885 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 12245 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2886 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 12251 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2887 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12257 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2891 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 12264 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2894 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 12272 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2901 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12278 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2902 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12284 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2905 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 12292 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2908 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 12298 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2909 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 12304 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2910 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 12310 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2912 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12316 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2913 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12322 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2915 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 12328 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2916 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12334 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 2917 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12340 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2918 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12346 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2919 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12352 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2920 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12358 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2925 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12364 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2926 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12370 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 2931 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12376 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 877:
#line 2932 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12382 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 2937 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12388 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 2939 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12394 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 2941 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 2942 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 2946 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 2947 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12418 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 2952 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 2953 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
#line 2958 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 2961 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 12442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
#line 2966 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 2967 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12454 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 2970 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 12460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 2971 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 12467 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 2978 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 12473 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 2980 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 12479 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 2983 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 12485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 2985 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12491 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 2988 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12497 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 2991 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12503 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 2992 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12509 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 2996 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 12515 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 2997 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 12521 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3001 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12527 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3002 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 12533 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3003 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 12539 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3007 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12545 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3012 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 12551 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3017 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12557 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12563 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3022 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12569 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3027 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12575 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3032 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12581 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3033 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12587 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3038 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12593 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3039 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12599 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3041 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12605 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3046 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12611 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3048 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12617 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 12631 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 12645 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 12659 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 12673 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3104 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12679 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3105 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12685 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3106 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12691 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3107 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12697 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3108 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12703 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3109 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12709 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 12723 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3128 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12729 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3130 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12735 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3132 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12741 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3133 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12747 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3137 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12753 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3141 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12759 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3142 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12765 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3143 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12771 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3144 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12777 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 12791 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
#line 3161 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12797 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3163 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12803 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
#line 3167 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12809 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
#line 3172 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12815 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3173 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12821 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 943:
#line 3174 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12827 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
#line 3175 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12833 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3176 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12839 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
#line 3177 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12845 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
#line 3178 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12851 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
#line 3180 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12857 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3182 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12863 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3186 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12869 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3190 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3191 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12881 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3197 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12887 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3201 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12893 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3208 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 12899 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3217 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 12905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3221 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 12911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3225 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12917 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3234 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12923 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3235 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3236 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12935 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3240 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12941 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3241 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 12947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3242 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 12953 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3244 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]) = 1; _p->onIndirectRef((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3249 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12965 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3250 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3261 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12977 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3262 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3263 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3266 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13003 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3277 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 13009 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3278 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13015 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3282 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13021 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3283 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 13027 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3286 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 13041 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3295 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13047 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3299 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 13053 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3300 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 13059 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3302 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 13065 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3303 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 13071 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3304 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 13077 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3305 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 13083 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3310 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13089 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3311 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 13095 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3315 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13101 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3316 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13107 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3317 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13113 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3318 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13119 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3321 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13125 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3323 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 13131 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3324 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 13137 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3325 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 13143 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 995:
#line 3330 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13149 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3331 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13155 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3335 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13161 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
#line 3336 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13167 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3337 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13173 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3338 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13179 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3343 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13185 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3344 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 13191 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3349 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13197 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1004:
#line 3351 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 13203 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1005:
#line 3353 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 13209 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1006:
#line 3354 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 13215 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1007:
#line 3358 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 13221 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1008:
#line 3360 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 13227 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3361 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 13233 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3363 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 13240 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3368 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 13246 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3370 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 13252 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3372 "hphp.y" /* yacc.c:1646  */
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[-2]),
                                           !(yyvsp[-1]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[0])
                                         );
                                       }
#line 13266 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3382 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 13272 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3384 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 13278 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3385 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 13284 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3388 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 13290 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3389 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 13296 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3390 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 13302 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3394 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 13308 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3395 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 13314 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3396 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13320 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3397 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13326 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3398 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13332 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3399 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 13338 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3400 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 13344 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1027:
#line 3401 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 13350 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1028:
#line 3402 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 13356 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1029:
#line 3403 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 13362 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1030:
#line 3404 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 13368 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1031:
#line 3408 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 13374 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1032:
#line 3409 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 13380 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1033:
#line 3414 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13386 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1034:
#line 3416 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 13392 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1037:
#line 3430 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1038:
#line 3435 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 13408 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1039:
#line 3439 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 13416 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1040:
#line 3444 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 13424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1041:
#line 3450 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1042:
#line 3451 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3455 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3456 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 13448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3462 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13454 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3466 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 13460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3472 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 13466 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3476 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 13473 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13479 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3484 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3488 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 13493 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3491 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 13500 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3497 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13506 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3502 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]); }
#line 13512 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1055:
#line 3503 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13518 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1056:
#line 3504 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13524 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1057:
#line 3505 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13530 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3526 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13536 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3527 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 13542 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3536 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 13548 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3547 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13554 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3549 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 13560 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3553 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13566 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3556 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13572 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3560 "hphp.y" /* yacc.c:1646  */
    {}
#line 13578 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1076:
#line 3561 "hphp.y" /* yacc.c:1646  */
    {}
#line 13584 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1077:
#line 3562 "hphp.y" /* yacc.c:1646  */
    {}
#line 13590 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1078:
#line 3568 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13597 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1079:
#line 3573 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13607 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1080:
#line 3582 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13613 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1081:
#line 3588 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13622 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1082:
#line 3596 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 13628 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1083:
#line 3597 "hphp.y" /* yacc.c:1646  */
    { }
#line 13634 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1084:
#line 3603 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 13640 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1085:
#line 3605 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 13646 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1086:
#line 3606 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 13656 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1087:
#line 3611 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 13663 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1088:
#line 3617 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("array"); }
#line 13670 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1089:
#line 3622 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13676 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1090:
#line 3627 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 13684 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1091:
#line 3631 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13690 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1092:
#line 3636 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 13696 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1093:
#line 3638 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 13702 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1094:
#line 3644 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 13709 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1095:
#line 3646 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 13717 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1096:
#line 3649 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13723 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1097:
#line 3650 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13731 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1098:
#line 3653 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13739 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1099:
#line 3656 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13745 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1100:
#line 3659 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 13753 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1101:
#line 3662 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13760 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1102:
#line 3664 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 13769 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1103:
#line 3670 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 13778 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1104:
#line 3676 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 13788 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1105:
#line 3684 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13794 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1106:
#line 3685 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13800 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;


#line 13804 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 3688 "hphp.y" /* yacc.c:1906  */

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}
