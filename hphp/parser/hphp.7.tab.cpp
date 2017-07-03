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
#define YYLAST   18085

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  206
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  301
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1075
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1977

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
     788,   791,   794,   798,   802,   806,   811,   812,   813,   814,
     815,   816,   817,   818,   819,   820,   821,   822,   823,   827,
     828,   829,   830,   831,   832,   833,   834,   835,   836,   837,
     838,   839,   840,   841,   842,   843,   844,   845,   846,   847,
     848,   849,   850,   851,   852,   853,   854,   855,   856,   857,
     858,   859,   860,   861,   862,   863,   864,   865,   866,   867,
     868,   869,   870,   871,   872,   873,   874,   875,   876,   877,
     878,   879,   880,   881,   882,   883,   884,   885,   886,   887,
     888,   889,   893,   897,   898,   902,   903,   908,   910,   915,
     920,   921,   922,   924,   929,   931,   936,   941,   943,   945,
     950,   951,   955,   956,   958,   962,   969,   976,   980,   986,
     988,   991,   992,   993,   994,   997,   998,  1002,  1007,  1007,
    1013,  1013,  1020,  1019,  1025,  1025,  1030,  1031,  1032,  1033,
    1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,  1043,
    1044,  1048,  1046,  1055,  1053,  1060,  1070,  1064,  1074,  1072,
    1076,  1077,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1088,
    1089,  1090,  1091,  1092,  1100,  1100,  1105,  1111,  1115,  1115,
    1123,  1124,  1128,  1129,  1133,  1139,  1137,  1152,  1149,  1165,
    1162,  1179,  1178,  1187,  1185,  1197,  1196,  1215,  1213,  1232,
    1231,  1240,  1238,  1249,  1249,  1256,  1255,  1267,  1265,  1278,
    1279,  1283,  1286,  1289,  1290,  1291,  1294,  1295,  1298,  1300,
    1303,  1304,  1307,  1308,  1311,  1312,  1316,  1317,  1322,  1323,
    1326,  1327,  1328,  1332,  1333,  1337,  1338,  1342,  1343,  1347,
    1348,  1353,  1354,  1360,  1361,  1362,  1363,  1366,  1369,  1371,
    1374,  1375,  1379,  1381,  1384,  1387,  1390,  1391,  1394,  1395,
    1399,  1405,  1411,  1418,  1420,  1425,  1430,  1436,  1440,  1444,
    1448,  1453,  1458,  1463,  1468,  1474,  1483,  1488,  1493,  1499,
    1501,  1505,  1509,  1514,  1518,  1521,  1524,  1528,  1532,  1536,
    1540,  1545,  1553,  1555,  1558,  1559,  1560,  1561,  1563,  1565,
    1570,  1571,  1574,  1575,  1576,  1580,  1581,  1583,  1584,  1588,
    1590,  1593,  1597,  1603,  1605,  1608,  1608,  1612,  1611,  1615,
    1617,  1620,  1623,  1621,  1638,  1634,  1649,  1651,  1653,  1655,
    1657,  1659,  1661,  1665,  1666,  1667,  1670,  1676,  1680,  1686,
    1689,  1694,  1696,  1701,  1706,  1710,  1711,  1715,  1716,  1718,
    1720,  1726,  1727,  1729,  1733,  1734,  1739,  1743,  1744,  1748,
    1749,  1753,  1755,  1761,  1766,  1767,  1769,  1773,  1774,  1775,
    1776,  1780,  1781,  1782,  1783,  1784,  1785,  1787,  1792,  1795,
    1796,  1800,  1801,  1805,  1806,  1809,  1810,  1813,  1814,  1817,
    1818,  1822,  1823,  1824,  1825,  1826,  1827,  1828,  1832,  1833,
    1836,  1837,  1838,  1841,  1843,  1845,  1846,  1849,  1851,  1855,
    1857,  1861,  1865,  1869,  1874,  1875,  1877,  1878,  1879,  1880,
    1883,  1887,  1888,  1892,  1893,  1897,  1898,  1899,  1900,  1904,
    1908,  1913,  1917,  1921,  1925,  1929,  1934,  1935,  1936,  1937,
    1938,  1942,  1944,  1945,  1946,  1949,  1950,  1951,  1952,  1953,
    1954,  1955,  1956,  1957,  1958,  1959,  1960,  1961,  1962,  1963,
    1964,  1965,  1966,  1967,  1968,  1969,  1970,  1971,  1972,  1973,
    1974,  1975,  1976,  1977,  1978,  1979,  1980,  1981,  1982,  1983,
    1984,  1985,  1986,  1987,  1988,  1989,  1990,  1991,  1992,  1994,
    1995,  1997,  1998,  2000,  2001,  2002,  2003,  2004,  2005,  2006,
    2007,  2008,  2009,  2010,  2011,  2012,  2013,  2014,  2015,  2016,
    2017,  2018,  2019,  2020,  2021,  2022,  2023,  2024,  2028,  2032,
    2037,  2036,  2051,  2049,  2067,  2066,  2085,  2084,  2103,  2102,
    2120,  2120,  2135,  2135,  2153,  2154,  2155,  2160,  2162,  2166,
    2170,  2176,  2180,  2186,  2188,  2192,  2194,  2198,  2202,  2203,
    2207,  2209,  2213,  2215,  2219,  2221,  2225,  2228,  2233,  2235,
    2239,  2242,  2247,  2251,  2255,  2259,  2263,  2267,  2271,  2275,
    2279,  2283,  2287,  2291,  2295,  2299,  2303,  2307,  2309,  2313,
    2315,  2319,  2321,  2325,  2332,  2339,  2341,  2346,  2347,  2348,
    2349,  2350,  2351,  2352,  2353,  2354,  2356,  2357,  2361,  2362,
    2363,  2364,  2368,  2374,  2383,  2396,  2397,  2400,  2403,  2406,
    2407,  2410,  2414,  2417,  2420,  2427,  2428,  2432,  2433,  2435,
    2440,  2441,  2442,  2443,  2444,  2445,  2446,  2447,  2448,  2449,
    2450,  2451,  2452,  2453,  2454,  2455,  2456,  2457,  2458,  2459,
    2460,  2461,  2462,  2463,  2464,  2465,  2466,  2467,  2468,  2469,
    2470,  2471,  2472,  2473,  2474,  2475,  2476,  2477,  2478,  2479,
    2480,  2481,  2482,  2483,  2484,  2485,  2486,  2487,  2488,  2489,
    2490,  2491,  2492,  2493,  2494,  2495,  2496,  2497,  2498,  2499,
    2500,  2501,  2502,  2503,  2504,  2505,  2506,  2507,  2508,  2509,
    2510,  2511,  2512,  2513,  2514,  2515,  2516,  2517,  2518,  2519,
    2520,  2524,  2529,  2530,  2534,  2535,  2536,  2537,  2539,  2543,
    2544,  2555,  2556,  2558,  2560,  2572,  2573,  2574,  2578,  2579,
    2580,  2584,  2585,  2586,  2589,  2591,  2595,  2596,  2597,  2598,
    2600,  2601,  2602,  2603,  2604,  2605,  2606,  2607,  2608,  2609,
    2612,  2617,  2618,  2619,  2621,  2622,  2624,  2625,  2626,  2627,
    2628,  2629,  2630,  2631,  2632,  2634,  2636,  2638,  2640,  2642,
    2643,  2644,  2645,  2646,  2647,  2648,  2649,  2650,  2651,  2652,
    2653,  2654,  2655,  2656,  2657,  2658,  2660,  2662,  2664,  2666,
    2667,  2670,  2671,  2675,  2679,  2681,  2685,  2686,  2690,  2696,
    2699,  2703,  2704,  2705,  2706,  2707,  2708,  2709,  2714,  2716,
    2720,  2721,  2724,  2725,  2729,  2732,  2734,  2736,  2740,  2741,
    2742,  2743,  2746,  2750,  2751,  2752,  2753,  2757,  2759,  2766,
    2767,  2768,  2769,  2774,  2775,  2776,  2777,  2779,  2780,  2782,
    2783,  2784,  2785,  2786,  2790,  2792,  2796,  2798,  2801,  2804,
    2806,  2808,  2811,  2813,  2817,  2819,  2822,  2825,  2831,  2833,
    2836,  2837,  2842,  2845,  2849,  2849,  2854,  2857,  2858,  2862,
    2863,  2867,  2868,  2869,  2873,  2878,  2883,  2884,  2888,  2893,
    2898,  2899,  2903,  2904,  2909,  2911,  2916,  2927,  2941,  2953,
    2968,  2969,  2970,  2971,  2972,  2973,  2974,  2984,  2993,  2995,
    2997,  3001,  3002,  3003,  3004,  3005,  3021,  3022,  3024,  3026,
    3033,  3034,  3035,  3036,  3037,  3038,  3039,  3040,  3042,  3047,
    3051,  3052,  3056,  3059,  3066,  3070,  3079,  3086,  3094,  3096,
    3097,  3101,  3102,  3103,  3105,  3110,  3111,  3122,  3123,  3124,
    3125,  3136,  3139,  3142,  3143,  3144,  3145,  3156,  3160,  3161,
    3162,  3164,  3165,  3166,  3170,  3172,  3175,  3177,  3178,  3179,
    3180,  3183,  3185,  3186,  3190,  3192,  3195,  3197,  3198,  3199,
    3203,  3205,  3208,  3211,  3213,  3215,  3219,  3220,  3222,  3223,
    3229,  3230,  3232,  3242,  3244,  3246,  3249,  3250,  3251,  3255,
    3256,  3257,  3258,  3259,  3260,  3261,  3262,  3263,  3264,  3265,
    3269,  3270,  3274,  3276,  3284,  3286,  3290,  3294,  3299,  3303,
    3311,  3312,  3316,  3317,  3323,  3324,  3333,  3334,  3342,  3345,
    3349,  3352,  3357,  3362,  3364,  3365,  3366,  3369,  3371,  3377,
    3378,  3382,  3383,  3387,  3388,  3392,  3393,  3396,  3401,  3402,
    3406,  3409,  3411,  3415,  3421,  3422,  3423,  3427,  3431,  3439,
    3444,  3456,  3458,  3462,  3465,  3467,  3472,  3477,  3483,  3486,
    3491,  3496,  3498,  3505,  3507,  3510,  3511,  3514,  3517,  3518,
    3523,  3525,  3529,  3535,  3545,  3546
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

#define YYPACT_NINF -1594

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1594)))

#define YYTABLE_NINF -1059

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1059)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
   -1594,   335, -1594, -1594,  5455, 13372, 13372,   -25, 13372, 13372,
   13372, 13372, 11139, 13372, -1594, 13372, 13372, 13372, 13372, 16558,
   16558, 13372, 13372, 13372, 13372, 13372, 13372, 13372, 13372, 11342,
   17223, 13372,    -7,    45, -1594, -1594, -1594,   292, -1594,   368,
   -1594, -1594, -1594,   317, 13372, -1594,    45,   210,   223,   229,
   -1594,    45, 11545, 15167, 11748, -1594, 14250, 10124,   242, 13372,
   17464,    77,    57,    54,   474, -1594, -1594, -1594,   249,   281,
     308,   341, -1594, 15167,   350,   424,   411,   485,   559,   563,
     600, -1594, -1594, -1594, -1594, -1594, 13372,   507,  3178, -1594,
   -1594, 15167, -1594, -1594, -1594, -1594, 15167, -1594, 15167, -1594,
     511,   486, 15167, 15167, -1594,   339, -1594, -1594, 11951, -1594,
   -1594,   495,   526,   605,   605, -1594,   655,   529,   224,   517,
   -1594,    95, -1594,   682, -1594, -1594, -1594, -1594, 14497,   515,
   -1594, -1594,   525,   530,   533,   540,   542,   557,   560,   571,
   12138, -1594, -1594, -1594, -1594,   174,   701,   726,   730,   732,
     734, -1594,   748,   764, -1594,    64,   595, -1594,   646,    -2,
   -1594,  1623,    65, -1594, -1594,  3483,    67,   648,   198, -1594,
      93,   148,   651,   153, -1594, -1594,   772, -1594,   693, -1594,
   -1594,   665,   698, -1594, 13372, -1594,   682,   515, 17678,  4008,
   17678, 13372, 17678, 17678, 17942, 17942,   666,  4376, 17678,   819,
   15167,   800,   800,   188,   800, -1594, -1594, -1594, -1594, -1594,
   -1594, -1594, -1594, -1594,    58, 13372,   690, -1594, -1594,   712,
     677,   569,   680,   569,   800,   800,   800,   800,   800,   800,
     800,   800, 16558, 16118,   674,   869,   693, -1594, 13372,   690,
   -1594,   733, -1594,   745,   696, -1594,    99, -1594, -1594, -1594,
     569,    67, -1594, 12154, -1594, -1594, 13372,  8906,   886,   106,
   17678,  9921, -1594, 13372, 13372, 15167, -1594, -1594, 13356,   708,
   -1594, 15442, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594,
   -1594, -1594,  2908, -1594,  2908, -1594, -1594, -1594, -1594, -1594,
   -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594,
   -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594,
   -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594,
   -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594,
   -1594, -1594,    88,   104,   698, -1594, -1594, -1594, -1594,   715,
    2783,   105, -1594, -1594,   756,   882, -1594,   757, 14969, -1594,
     720,   721, 15490, -1594,    50, 15538, 14674, 14674, 15015, 15167,
     722,   913,   731, -1594,    66, -1594, 16146,   107, -1594,   914,
     108,   797, -1594,   803, -1594, 16558, 13372, 13372,   739,   749,
   -1594, -1594, 16249, 11342, 13372, 13372, 13372, 13372, 13372,   111,
     599,   532, -1594, 13575, 16558,   520, -1594, 15167, -1594,   551,
     529, -1594, -1594, -1594, -1594, 17323,   924,   837, -1594, -1594,
   -1594,    84, 13372,   743,   746, 17678,   750,   865,   751,  5658,
   13372, -1594,   491,   740,   627,   491,   285,   469, -1594, 15167,
    2908,   753, 10327, 14250, -1594, -1594,  3244, -1594, -1594, -1594,
   -1594, -1594,   682, -1594, -1594, -1594, -1594, -1594, -1594, -1594,
   -1594, -1594, 13372, 13372, 13372, 13372, 12357, 13372, 13372, 13372,
   13372, 13372, 13372, 13372, 13372, 13372, 13372, 13372, 13372, 13372,
   13372, 13372, 13372, 13372, 13372, 13372, 13372, 13372, 13372, 13372,
   17423, 13372, -1594, 13372, 13372, 13372,  4471, 15167, 15167, 15167,
   15167, 15167, 14497,   835,   779,  4775, 13372, 13372, 13372, 13372,
   13372, 13372, 13372, 13372, 13372, 13372, 13372, 13372, -1594, -1594,
   -1594, -1594,  2188, 13372, 13372, -1594, 10327, 10327, 13372, 13372,
   16249,   754,   682, 12560,  3808, -1594, 13372, -1594,   759,   943,
     799,   769,   770, 13746,   569, 12763, -1594, 12966, -1594,   696,
     774,   775,  2202, -1594,   325, 10327, -1594,  2576, -1594, -1594,
   15608, -1594, -1594, 10530, -1594, 13372, -1594,   860,  9109,   960,
     776, 13559,   966,    83,    74, -1594, -1594, -1594,   804, -1594,
   -1594, -1594,  2908, -1594,  2432,   788,   979, 16043, 15167, -1594,
   -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594,   793, -1594,
   -1594,   802,   794,   807,   798, 15167,   805,    68,    70, 15841,
   15015, -1594, -1594, 15167, 15167, 13372,   569,    77, -1594, 16043,
     926, -1594, -1594, -1594,   569,    92,   127,   810,   813,  2457,
     206,   814,   815,   142,   880,   820,   569,   133,   818, 16727,
     816,  1008,  1011,   821,   822,   823,   824, -1594, 14321, 15167,
   -1594, -1594,   951,  3090,   473, -1594, -1594, -1594,   529, -1594,
   -1594, -1594,   997,   900,   855,   359,   876, 13372,   901,  1031,
     846, -1594,   885, -1594,   137, -1594,  2908,  2908,  1033,   886,
      84, -1594,   857,  1050, -1594,  2908,    71, -1594,   452,   159,
   -1594, -1594, -1594, -1594, -1594, -1594, -1594,   933,  3482, -1594,
   -1594, -1594, -1594,  1052,   883, -1594, 16558, 13372,   870,  1057,
   17678,  1054, -1594, -1594,   938,  4871, 11733, 17861, 17942, 17979,
   13372, 17630, 18016,  4688, 10711, 12537, 12333, 13144, 13742, 13742,
   13742, 13742,  2970,  2970,  2970,  2970,  2970,  1087,  1087,   736,
     736,   736,   188,   188,   188, -1594,   800, 17678,   868,   874,
   16775,   873,  1071,    -1, 13372,   355,   690,   205, -1594, -1594,
   -1594,  1068,   837, -1594,   682, 16352, -1594, -1594, -1594, 17942,
   17942, 17942, 17942, 17942, 17942, 17942, 17942, 17942, 17942, 17942,
   17942, 17942, -1594, 13372,   395, -1594,   143, -1594,   690,   415,
     881,  3623,   893,   895,   890,  3673,   134,   899, -1594, 17678,
    3825, -1594, 15167, -1594,    71,    20, 16558, 17678, 16558, 16831,
     938,    71,   569,   144, -1594,   137,   941,   905, 13372, -1594,
     151, -1594, -1594, -1594,  8703,   578, -1594, -1594, 17678, 17678,
      45, -1594, -1594, -1594, 13372,   996, 15919, 16043, 15167,  9312,
     909,   910, -1594,  1100, 14145,   974, -1594,   953, -1594,  1102,
     917,  4013,  2908, 16043, 16043, 16043, 16043, 16043,   921,  1048,
    1053,  1055,  1061,  1062,   937, 16043,     2, -1594, -1594, -1594,
   -1594, -1594, -1594,   217, -1594, 17772, -1594, -1594,    22, -1594,
    5861, 13792,   923, 15015, -1594, 15015, -1594,    72, -1594, 15167,
   15167, 15015, 15015, -1594,  1125,   936, -1594, -1594, -1594,  3721,
   -1594, 17772,  1133, 16558,   950, -1594, -1594, -1594, -1594, -1594,
   -1594, -1594, -1594, -1594,   967,  1141, 15167, 13792,   952, 16249,
   16455,  1138, -1594, 13372, -1594, 13372, -1594, 13372, -1594, -1594,
   -1594, -1594, -1594, -1594, -1594, -1594,   954, -1594, 13372, -1594,
   -1594,  5049, -1594,  2908, 13792,   955, -1594, -1594, -1594, -1594,
    1139,   958, 13372, 17323, -1594, -1594,  4471,   962, -1594,  2908,
   -1594,   965,  6064,  1134,    59, -1594, -1594,   164,  2188, -1594,
    2576, -1594,  2908, -1594, -1594,   569, 17678, -1594, 10733, -1594,
   16043,    82,   969, 13792,   900, -1594, -1594, 18016, 13372, -1594,
   -1594, 13372, -1594, 13372, -1594,  3900,   972, 10327,   880,  1140,
     900,  2908,  1157,   938, 15167, 17423,   569,  4243,   977, -1594,
   -1594,   161,   984, -1594, -1594,  1168, 13969, 13969,  3825, -1594,
   -1594, -1594,  1131,   989,  1117,  1119,  1120,  1123,  1126,    69,
     999,   596, -1594, -1594, -1594, -1594, -1594,  1037, -1594, -1594,
   -1594, -1594,  1190,  1003,   759,   569,   569, 13169,   900,  2576,
   -1594, -1594,  4644,   660,    45,  9921, -1594,  6267,  1004,  6470,
    1007, 15919, 16558,  1010,  1073,   569, 17772,  1193, -1594, -1594,
   -1594, -1594,   293, -1594,    46,  2908,  1028,  1076,  1059,  2908,
   15167,  3169, -1594, -1594, -1594,  1206, -1594,  1020,  1052,   758,
     758,  1151,  1151, 16983,  1019,  1214, 16043, 16043, 16043, 16043,
   16043, 16043, 17323, 15656, 15121, 16043, 16043, 16043, 16043, 15800,
   16043, 16043, 16043, 16043, 16043, 16043, 16043, 16043, 16043, 16043,
   16043, 16043, 16043, 16043, 16043, 16043, 16043, 16043, 16043, 16043,
   16043, 16043, 16043, 15167, -1594, -1594,  1142, -1594, -1594,  1026,
    1035, -1594, -1594, -1594, 15841, -1594,  1030, -1594, 16043,   569,
   -1594, -1594,   100, -1594,   659,  1224, -1594, -1594,   135,  1040,
     569, 10936, 17678, 16879, -1594,  2754, -1594,  5252,   837,  1224,
   -1594,    12,   219, -1594, 17678,  1101,  1042, -1594,  1044,  1134,
   -1594,  2908,   886,  2908,    81,  1229,  1161,   165, -1594,   690,
     167, -1594, -1594, 16558, 13372, 17678, 17772,  1051,    82, -1594,
    1046,    82,  1056, 18016, 17678, 16935,  1058, 10327,  1060,  1063,
    2908,  1065,  1049,  2908,   900, -1594,   696,   451, 10327, 13372,
   -1594, -1594, -1594, -1594, -1594, -1594,  1113,  1047,  1244,  1164,
    3825,  3825,  3825,  3825,  3825,  3825,  1099, -1594, 17323,   109,
    3825, -1594, -1594, -1594, 16558, 17678,  1066, -1594,    45,  1226,
    1183,  9921, -1594, -1594, -1594,  1072, 13372,  1073,   569, 16249,
   15919,  1064, 16043,  6673,   710,  1075, 13372,    76,   283, -1594,
    1084, -1594,  2908, 15167, -1594,  1135, -1594, -1594, 15722,  1245,
    1081, 16043, -1594, 16043, -1594,  1082,  1085,  1280, 17038,  1088,
   17772,  1281,  1091,  1094,  1095,  1163,  1293,  1106, -1594, -1594,
   -1594, 17086,  1105,  1303, 17817, 17905,  3518, 16043, 17726, 10306,
   11320, 12740, 12942, 14075, 14427, 14427, 14427, 14427,  2044,  2044,
    2044,  2044,  2044,   729,   729,   758,   758,   758,  1151,  1151,
    1151,  1151, -1594,  1121, -1594,  1118,  1122, -1594, -1594, 17772,
   15167,  2908,  2908, -1594,   659, 13792,  1333, -1594, 16249, -1594,
   -1594, 17942, 13372,  1124, -1594,  1128,  1560, -1594,   208, 13372,
   -1594, -1594, -1594, 13372, -1594, 13372, -1594,   886, -1594, -1594,
     166,  1305,  1234, 13372, -1594,  1127,   569, 17678,  1134,  1132,
   -1594,  1136,    82, 13372, 10327,  1144, -1594, -1594,   837, -1594,
   -1594,  1130,  1146,  1153, -1594,  1145,  3825, -1594,  3825, -1594,
   -1594,  1156,  1129,  1312,  1186,  1137, -1594,  1321,  1152,  1154,
    1158, -1594,  1195,  1169,  1326, -1594, -1594,   569, -1594,  1322,
   -1594,  1162, -1594, -1594,  1173,  1174,   136, -1594, -1594, 17772,
    1167,  1176, -1594, 11529, -1594, -1594, -1594, -1594, -1594, -1594,
    1236,  2908, -1594,  2908, -1594, 17772, 17141, -1594, -1594, 16043,
   -1594, 16043, -1594, 16043, -1594, -1594, -1594, -1594, 16043, 17323,
   -1594, -1594, 16043, -1594, 16043, -1594,  3383, 16043,  1177,  6876,
   -1594, -1594,   659, -1594, -1594, -1594, -1594,   633, 14426, 13792,
    1263, -1594,  1143,  1208,  4619, -1594, -1594, -1594,   835, 15645,
     114,   116,  1180,   837,   779,   140, 17678, -1594, -1594, -1594,
    1215,  4951, 11123, 17678, -1594,   265,  1365,  1300, 13372, -1594,
   17678, 10327,  1269,  1134,  1789,  1134,  1191, 17678,  1198, -1594,
    1910,  1188,  2010, -1594, -1594,    82, -1594, -1594,  1261, -1594,
   -1594,  3825, -1594,  3825, -1594,  3825, -1594, -1594, -1594, -1594,
    3825, -1594, 17323, -1594,  2038, -1594,  8703, -1594, -1594, -1594,
   -1594,  9515, -1594, -1594, -1594,  8703,  2908, -1594,  1201, 16043,
   17189, 17772, 17772, 17772,  1264, 17772, 17244,  3383, -1594, -1594,
     659, 13792, 13792, 15167, -1594,  1387, 15273,   102, -1594, 14426,
     837, 14863, -1594,  1221, -1594,   117,  1204,   118, -1594, 14779,
   -1594, -1594, -1594,   119, -1594, -1594,  4354, -1594,  1207, -1594,
    1323,   682, -1594, 14603, -1594, 14603, -1594, -1594,  1390,   835,
   -1594, 13898, -1594, -1594, -1594, -1594,  1395,  1328, 13372, -1594,
   17678,  1217,  1219,  1134,   576, -1594,  1269,  1134, -1594, -1594,
   -1594, -1594,  2133,  1220,  3825,  1282, -1594, -1594, -1594,  1283,
   -1594,  8703,  9718,  9515, -1594, -1594, -1594,  8703, -1594, -1594,
   17772, 16043, 16043, 16043,  7079,  1218,  1222, -1594, 16043, -1594,
   13792, -1594, -1594, -1594, -1594, -1594,  2908,  1471,  1143, -1594,
   -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594,
   -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594,
   -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594,
   -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594,
   -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594,
   -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594,
   -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594,
   -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594,
   -1594,   382, -1594,  1208, -1594, -1594, -1594, -1594, -1594,    86,
     513, -1594,  1407,   120, 14969,  1323,  1409, -1594,  2908,   682,
   -1594, -1594,  1227,  1410, 13372, -1594, 17678, -1594,   129,  1235,
   -1594, -1594, -1594,  1134,   576, 14074, -1594,  1134, -1594,  3825,
    3825, -1594, -1594, -1594, -1594,  7282, 17772, 17772, 17772, -1594,
   -1594, -1594, 17772, -1594,  4064,  1417,  1424,  1237, -1594, -1594,
   16043, 14779, 14779,  1367, -1594,  4354,  4354,   564, -1594, -1594,
   -1594, 16043,  1353, -1594,  1257,  1243,   122, 16043, -1594, 15167,
   -1594, 16043, 17678,  1357, -1594,  1432, -1594,  7485,  1247, -1594,
   -1594,   576, -1594, -1594,  7688,  1246,  1327, -1594,  1341,  1299,
   -1594, -1594,  1360,  2908,  1285,  1471, -1594, -1594, 17772, -1594,
   -1594,  1292, -1594,  1429, -1594, -1594, -1594, -1594, 17772,  1458,
     142, -1594, -1594, 17772,  1277, 17772, -1594,   353,  1279,  7891,
   -1594, -1594, -1594,  1276, -1594,  1284,  1301, 15167,   779,  1295,
   -1594, -1594, -1594, 16043,  1302,    91, -1594,  1400, -1594, -1594,
   -1594,  8094, -1594, 13792,   923, -1594,  1310, 15167,   521, -1594,
   17772, -1594,  1290,  1479,   700,    91, -1594, -1594,  1406, -1594,
   13792,  1296, -1594,  1134,   113, -1594, -1594, -1594, -1594,  2908,
   -1594,  1294,  1298,   124, -1594,  1307,   700,   170,  1134,  1306,
   -1594,  2908,   589,  2908,   310,  1484,  1421,  1307, -1594,  1497,
   -1594,   519, -1594, -1594, -1594,   171,  1493,  1425, 13372, -1594,
     589,  8297,  2908, -1594,  2908, -1594,  8500,   346,  1495,  1427,
   13372, -1594, 17678, -1594, -1594, -1594, -1594, -1594,  1498,  1430,
   13372, -1594, 17678, 13372, -1594, 17678, 17678
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,     0,   435,     0,   864,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   955,
     943,     0,   730,     0,   736,   737,   738,    26,   802,   931,
     932,   159,   160,   739,     0,   140,     0,     0,     0,     0,
      27,     0,     0,     0,     0,   194,     0,     0,     0,     0,
       0,     0,   404,   405,   406,   403,   402,   401,     0,     0,
       0,     0,   223,     0,     0,     0,    34,    35,    37,    38,
      36,   743,   745,   746,   740,   741,     0,     0,     0,   747,
     742,     0,   713,    29,    30,    31,    33,    32,     0,   744,
       0,     0,     0,     0,   748,   407,   542,    28,     0,   158,
     130,     0,   731,     0,     0,     4,   120,   122,   801,     0,
     712,     0,     6,   193,     7,     9,     8,    10,     0,     0,
     399,   448,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   446,   920,   921,   524,   518,   519,   520,   521,   522,
     523,   429,   527,     0,   428,   891,   714,   721,     0,   804,
     517,   398,   894,   895,   906,   447,     0,     0,   450,   449,
     892,   893,   890,   927,   930,   507,   803,    11,   404,   405,
     406,     0,     0,    33,     0,   120,   193,     0,   995,   447,
     996,     0,   998,   999,   526,   443,     0,   436,   441,     0,
       0,   489,   490,   491,   492,    26,   931,   739,   716,    34,
      35,    37,    38,    36,     0,     0,  1019,   913,   714,     0,
     715,   468,     0,   470,   508,   509,   510,   511,   512,   513,
     514,   516,     0,   959,     0,   811,   726,   213,     0,  1019,
     426,   725,   719,     0,   735,   715,   938,   939,   945,   937,
     727,     0,   427,     0,   729,   515,     0,     0,     0,     0,
     432,     0,   138,   434,     0,     0,   144,   146,     0,     0,
     148,     0,    72,    73,    78,    79,    64,    65,    56,    76,
      87,    88,     0,    59,     0,    63,    71,    69,    90,    82,
      81,    54,    77,    97,    98,    55,    93,    52,    94,    53,
      95,    51,    99,    86,    91,    96,    83,    84,    58,    85,
      89,    50,    80,    66,   100,    74,    67,    57,    44,    45,
      46,    47,    48,    49,    68,   102,   101,   104,    61,    42,
      43,    70,  1066,  1067,    62,  1071,    41,    60,    92,     0,
       0,   120,   103,  1010,  1065,     0,  1068,     0,     0,   150,
       0,     0,     0,   184,     0,     0,     0,     0,     0,     0,
       0,   813,     0,   108,   110,   312,     0,     0,   311,   317,
       0,     0,   224,     0,   227,     0,     0,     0,     0,  1016,
     209,   221,   951,   955,   561,   588,   588,   561,   588,     0,
     980,     0,   750,     0,     0,     0,   978,     0,    16,     0,
     124,   201,   215,   222,   618,   554,     0,  1004,   534,   536,
     538,   868,   435,   448,     0,     0,   446,   447,   449,     0,
       0,   934,   732,     0,   733,     0,     0,     0,   183,     0,
       0,   126,   303,     0,    25,   192,     0,   220,   205,   219,
     404,   407,   193,   400,   173,   174,   175,   176,   177,   179,
     180,   182,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     943,     0,   172,   936,   936,   965,     0,     0,     0,     0,
       0,     0,     0,     0,   397,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   467,   469,
     869,   870,     0,   936,     0,   882,   303,   303,   936,     0,
     951,     0,   193,     0,     0,   152,     0,   866,   861,   811,
       0,   448,   446,     0,   963,     0,   559,   810,   954,   735,
     448,   446,   447,   126,     0,   303,   425,     0,   884,   728,
       0,   130,   263,     0,   541,     0,   155,     0,     0,   433,
       0,     0,     0,     0,     0,   147,   171,   149,  1066,  1067,
    1063,  1064,     0,  1070,  1056,     0,     0,     0,     0,    75,
      40,    62,    39,  1011,   178,   181,   151,   130,     0,   168,
     170,     0,     0,     0,     0,     0,     0,   110,   111,     0,
     812,   109,    18,     0,   105,     0,   313,     0,   153,     0,
       0,   154,   225,   226,  1000,     0,     0,   448,   446,   447,
     450,   449,     0,  1046,   233,     0,   952,     0,     0,     0,
       0,   811,   811,     0,     0,     0,     0,   156,     0,     0,
     749,   979,   802,     0,     0,   977,   807,   976,   123,     5,
      13,    14,     0,   231,     0,     0,   547,     0,     0,   811,
       0,   723,     0,   722,   717,   548,     0,     0,     0,     0,
     868,   130,     0,   813,   867,  1075,   424,   438,   503,   900,
     919,   135,   129,   131,   132,   133,   134,   398,     0,   525,
     805,   806,   121,   811,     0,  1020,     0,     0,     0,   813,
     304,     0,   530,   195,   229,     0,   473,   475,   474,   486,
       0,     0,   506,   471,   472,   476,   478,   477,   495,   496,
     493,   494,   497,   498,   499,   500,   501,   487,   488,   480,
     481,   479,   482,   483,   485,   502,   484,   935,     0,     0,
     969,     0,   811,  1003,     0,  1002,  1019,   897,   211,   203,
     217,     0,  1004,   207,   193,     0,   439,   442,   444,   452,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   872,     0,   871,   874,   896,   878,  1019,   875,
       0,     0,     0,     0,     0,     0,     0,     0,   997,   437,
     859,   863,   810,   865,     0,   718,     0,   958,     0,   957,
     229,     0,   718,   942,   941,   927,   930,     0,     0,   871,
     874,   940,   875,   430,   265,   267,   130,   545,   544,   431,
       0,   130,   247,   139,   434,     0,     0,     0,     0,     0,
     259,   259,   145,   811,     0,     0,  1055,     0,  1052,   811,
       0,  1026,     0,     0,     0,     0,     0,   809,     0,    34,
      35,    37,    38,    36,     0,     0,   752,   756,   757,   758,
     759,   760,   762,     0,   751,   128,   800,   761,  1019,  1069,
       0,     0,     0,     0,    20,     0,    21,   111,    19,     0,
     106,     0,     0,   117,   813,     0,   115,   107,   112,     0,
     310,   318,   315,     0,     0,   989,   994,   991,   990,   993,
     992,    12,  1044,  1045,     0,   811,     0,     0,     0,   951,
     948,     0,   558,     0,   572,   810,   560,   810,   587,   575,
     581,   584,   578,   988,   987,   986,     0,   982,     0,   983,
     985,     0,     5,     0,     0,     0,   612,   613,   621,   620,
       0,   446,     0,   810,   553,   557,     0,     0,  1005,     0,
     535,     0,     0,  1033,   868,   289,  1074,     0,     0,   883,
       0,   933,   810,  1022,  1018,   305,   306,   711,   812,   302,
       0,   868,     0,     0,   231,   532,   197,   505,     0,   595,
     596,     0,   593,   810,   964,     0,     0,   303,   233,     0,
     231,     0,     0,   229,     0,   943,   453,     0,     0,   880,
     881,   898,   899,   928,   929,     0,     0,     0,   847,   818,
     819,   820,   827,     0,    34,    35,    37,    38,    36,     0,
       0,   833,   839,   840,   841,   842,   843,     0,   831,   829,
     830,   853,   811,     0,   861,   962,   961,     0,   231,     0,
     885,   734,     0,   269,     0,     0,   136,     0,     0,     0,
       0,     0,     0,     0,   239,   240,   251,     0,   130,   249,
     165,   259,     0,   259,     0,   810,     0,     0,     0,     0,
       0,   810,  1054,  1057,  1025,   811,  1024,     0,   811,   783,
     784,   781,   782,   817,     0,   811,   809,   565,   590,   590,
     565,   590,   556,     0,     0,   971,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1060,   185,     0,   188,   169,     0,
       0,   113,   118,   119,   812,   116,     0,   314,     0,  1001,
     157,  1017,  1046,  1037,  1041,   232,   234,   324,     0,     0,
     949,     0,   563,     0,   981,     0,    17,     0,  1004,   230,
     324,     0,     0,   718,   550,     0,   724,  1006,     0,  1033,
     539,     0,     0,  1075,     0,   294,   292,   874,   886,  1019,
     874,   887,  1021,     0,     0,   307,   127,     0,   868,   228,
       0,   868,     0,   504,   968,   967,     0,   303,     0,     0,
       0,     0,     0,     0,   231,   199,   735,   873,   303,     0,
     823,   824,   825,   826,   834,   835,   851,     0,   811,     0,
     847,   569,   592,   592,   569,   592,     0,   822,   855,     0,
     810,   858,   860,   862,     0,   956,     0,   873,     0,     0,
       0,     0,   266,   546,   141,     0,   434,   239,   241,   951,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   253,
       0,  1061,     0,     0,  1047,     0,  1053,  1051,   810,     0,
       0,     0,   754,   810,   808,     0,     0,   811,     0,     0,
     797,   811,     0,     0,     0,     0,   811,     0,   763,   798,
     799,   975,     0,   811,   766,   768,   767,     0,     0,   764,
     765,   769,   771,   770,   787,   788,   785,   786,   789,   790,
     791,   792,   793,   778,   779,   773,   774,   772,   775,   776,
     777,   780,  1059,     0,   130,     0,     0,   114,    22,   316,
       0,     0,     0,  1038,  1043,     0,   398,   953,   951,   440,
     445,   451,     0,     0,    15,     0,   398,   624,     0,     0,
     626,   619,   622,     0,   617,     0,  1008,     0,  1034,   543,
       0,   295,     0,     0,   290,     0,   309,   308,  1033,     0,
     324,     0,   868,     0,   303,     0,   925,   324,  1004,   324,
    1007,     0,     0,     0,   454,     0,     0,   837,   810,   846,
     828,     0,     0,   811,     0,     0,   845,   811,     0,     0,
       0,   821,     0,     0,   811,   832,   852,   960,   324,     0,
     130,     0,   262,   248,     0,     0,     0,   238,   161,   252,
       0,     0,   255,     0,   260,   261,   130,   254,  1062,  1048,
       0,     0,  1023,     0,  1073,   816,   815,   753,   573,   810,
     564,     0,   576,   810,   589,   582,   585,   579,     0,   810,
     555,   755,     0,   594,   810,   970,   795,     0,     0,     0,
      23,    24,  1040,  1035,  1036,  1039,   235,     0,     0,     0,
     405,   396,     0,     0,     0,   210,   323,   325,     0,   395,
       0,     0,     0,  1004,   398,     0,   562,   984,   320,   216,
     615,     0,     0,   549,   537,     0,   298,   288,     0,   291,
     297,   303,   529,  1033,   398,  1033,     0,   966,     0,   924,
     398,     0,   398,  1009,   324,   868,   922,   850,   849,   836,
     574,   810,   568,     0,   577,   810,   591,   583,   586,   580,
       0,   838,   810,   854,   398,   130,   268,   137,   142,   163,
     242,     0,   250,   256,   130,   258,     0,  1049,     0,     0,
       0,   567,   796,   552,     0,   974,   973,   794,   130,   189,
    1042,     0,     0,     0,  1012,     0,     0,     0,   236,     0,
    1004,     0,   361,   357,   363,   713,    33,     0,   351,     0,
     356,   360,   373,     0,   371,   376,     0,   375,     0,   374,
       0,   193,   327,     0,   329,     0,   330,   331,     0,     0,
     950,     0,   616,   614,   625,   623,   299,     0,     0,   286,
     296,     0,     0,  1033,     0,   206,   529,  1033,   926,   212,
     320,   218,   398,     0,     0,     0,   571,   844,   857,     0,
     214,   264,     0,     0,   130,   245,   162,   257,  1050,  1072,
     814,     0,     0,     0,     0,     0,     0,   423,     0,  1013,
       0,   341,   345,   420,   421,   355,     0,     0,     0,   336,
     674,   675,   673,   676,   677,   694,   696,   695,   665,   637,
     635,   636,   655,   670,   671,   631,   642,   643,   645,   644,
     664,   648,   646,   647,   649,   650,   651,   652,   653,   654,
     656,   657,   658,   659,   660,   661,   663,   662,   632,   633,
     634,   638,   639,   641,   679,   680,   684,   685,   686,   687,
     688,   689,   672,   691,   681,   682,   683,   666,   667,   668,
     669,   692,   693,   697,   699,   698,   700,   701,   678,   703,
     702,   705,   707,   706,   640,   710,   708,   709,   704,   690,
     630,   368,   627,     0,   337,   389,   390,   388,   381,     0,
     382,   338,   415,     0,     0,     0,     0,   419,     0,   193,
     202,   319,     0,     0,     0,   287,   301,   923,     0,     0,
     391,   130,   196,  1033,     0,     0,   208,  1033,   848,     0,
       0,   130,   243,   143,   164,     0,   566,   551,   972,   187,
     339,   340,   418,   237,     0,   811,   811,     0,   364,   352,
       0,     0,     0,   370,   372,     0,     0,   377,   384,   385,
     383,     0,     0,   326,  1014,     0,     0,     0,   422,     0,
     321,     0,   300,     0,   610,   813,   130,     0,     0,   198,
     204,     0,   570,   856,     0,     0,   166,   342,   120,     0,
     343,   344,     0,   810,     0,   810,   366,   362,   367,   628,
     629,     0,   353,   386,   387,   379,   380,   378,   416,   413,
    1046,   332,   328,   417,     0,   322,   611,   812,     0,     0,
     392,   130,   200,     0,   246,     0,   191,     0,   398,     0,
     358,   365,   369,     0,     0,   868,   334,     0,   608,   528,
     531,     0,   244,     0,     0,   167,   349,     0,   397,   359,
     414,  1015,     0,   813,   409,   868,   609,   533,     0,   190,
       0,     0,   348,  1033,   868,   273,   412,   411,   410,  1075,
     408,     0,     0,     0,   347,  1027,   409,     0,  1033,     0,
     346,     0,     0,  1075,     0,   278,   276,  1027,   130,   813,
    1029,     0,   393,   130,   333,     0,   279,     0,     0,   274,
       0,     0,   812,  1028,     0,  1032,     0,     0,   282,   272,
       0,   275,   281,   335,   186,  1030,  1031,   394,   283,     0,
       0,   270,   280,     0,   271,   285,   284
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1594, -1594, -1594,  -576, -1594, -1594, -1594,   169,    49,   -41,
     453, -1594,  -274,  -528, -1594, -1594,   384,   -30,  1441, -1594,
    1436, -1594,  -475, -1594,    34, -1594, -1594, -1594, -1594, -1594,
   -1594, -1594, -1594, -1594, -1594, -1594,  -378, -1594, -1594,  -161,
      56,    29, -1594, -1594, -1594, -1594, -1594, -1594,    31, -1594,
   -1594, -1594, -1594, -1594, -1594,    37, -1594, -1594,  1027,  1034,
    1038,  -110,  -709,  -881,   535,   592,  -382,   282,  -951, -1594,
    -103, -1594, -1594, -1594, -1594,  -742,   112, -1594, -1594, -1594,
   -1594,  -373, -1594,  -620, -1594,  -460, -1594, -1594,   929, -1594,
     -82, -1594, -1594, -1083, -1594, -1594, -1594, -1594, -1594, -1594,
   -1594, -1594, -1594, -1594,  -117, -1594,   -29, -1594, -1594, -1594,
   -1594, -1594,  -200, -1594,    73,  -966, -1594, -1593,  -405, -1594,
    -159,    27,  -120,  -374, -1594,  -207, -1594, -1594, -1594,    90,
     -18,    -6,    52,  -736,   -60, -1594, -1594,    30, -1594,    11,
   -1594, -1594,    -5,   -38,   -63, -1594, -1594, -1594, -1594, -1594,
   -1594, -1594, -1594, -1594,  -625,  -865, -1594, -1594, -1594, -1594,
   -1594,  1932,  1171, -1594,   464, -1594,   332, -1594, -1594, -1594,
   -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594,
   -1594, -1594, -1594,   155,  -391,  -516, -1594, -1594, -1594, -1594,
   -1594,   403, -1594, -1594, -1594, -1594, -1594, -1594, -1594, -1594,
    -946,  -371,  2358,    35, -1594,  2065,  -406, -1594, -1594,  -480,
    3219,  3263, -1594,  -479, -1594, -1594,   479,   189,  -644, -1594,
   -1594,   561,   348,  -395, -1594,   349, -1594, -1594, -1594, -1594,
   -1594,   536, -1594, -1594, -1594,    10,  -891,  -171,  -437,  -426,
   -1594,   613,  -104, -1594, -1594,    36,    39,   538, -1594, -1594,
     179,   -28, -1594,  -363,    23,  -368,   139,   241, -1594, -1594,
    -492,  1194, -1594, -1594, -1594, -1594, -1594,   724,   406, -1594,
   -1594, -1594,  -362,  -716, -1594,  1147, -1183, -1594,    -4,  -193,
      17,   735, -1594,  -364, -1594,  -361,  -501, -1270,  -286,   126,
   -1594,   436,   508, -1594, -1594, -1594, -1594,   458, -1594,   278,
   -1141
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   115,   932,   649,   185,  1564,   746,
     360,   361,   362,   363,   884,   885,   886,   117,   118,   119,
     120,   121,   419,   682,   683,   558,   261,  1632,   564,  1541,
    1633,  1876,   872,   354,   587,  1836,  1128,  1324,  1895,   436,
     186,   684,   972,  1192,  1383,   125,   652,   989,   685,   704,
     993,   624,   988,   240,   539,   686,   653,   990,   438,   380,
     402,   128,   974,   935,   908,  1145,  1567,  1251,  1054,  1783,
    1636,   823,  1060,   563,   832,  1062,  1426,   815,  1043,  1046,
    1240,  1902,  1903,   672,   673,   698,   699,   367,   368,   370,
    1601,  1761,  1762,  1336,  1476,  1590,  1755,  1885,  1905,  1794,
    1840,  1841,  1842,  1577,  1578,  1579,  1580,  1796,  1797,  1803,
    1852,  1583,  1584,  1588,  1748,  1749,  1750,  1772,  1944,  1477,
    1478,   187,   130,  1919,  1920,  1753,  1480,  1481,  1482,  1483,
     131,   254,   559,   560,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,  1613,   142,   971,  1191,   143,   669,
     670,   671,   258,   411,   554,   658,   659,  1286,   660,  1287,
     144,   145,   630,   631,  1276,  1277,  1392,  1393,   146,   857,
    1022,   147,   858,  1023,   148,   859,  1024,   149,   860,  1025,
     150,   861,  1026,   633,  1279,  1395,   151,   862,   152,   153,
    1825,   154,   654,  1603,   655,  1161,   940,  1354,  1351,  1741,
    1742,   155,   156,   157,   243,   158,   244,   255,   423,   546,
     159,  1280,  1281,   866,   867,   160,  1084,   963,   601,  1085,
    1029,  1214,  1030,  1396,  1397,  1217,  1218,  1032,  1403,  1404,
    1033,   791,   529,   199,   200,   687,   675,   512,  1177,  1178,
     777,   778,   959,   162,   246,   163,   164,   189,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   738,   250,   251,
     627,   234,   235,   741,   742,  1292,  1293,   395,   396,   926,
     175,   615,   176,   668,   177,   345,  1763,  1815,   381,   431,
     693,   694,  1077,  1932,  1939,  1940,  1172,  1333,   904,  1334,
     905,   906,   838,   839,   840,   346,   347,   869,   573,  1566,
     957
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     188,   190,   493,   192,   193,   194,   195,   197,   198,   443,
     201,   202,   203,   204,   161,   343,   224,   225,   226,   227,
     228,   229,   230,   231,   233,   521,   252,   257,   786,   955,
     364,   129,  1360,   124,   661,   126,   992,   664,   122,   260,
     262,   127,   663,   665,   950,   266,   543,   268,   414,   271,
     951,   492,   352,   249,   355,   969,   782,   783,   399,   800,
     123,   400,   515,  1173,  1465,   242,   247,   443,   351,   248,
     416,   883,   887,   931,   735,   775,   814,  1346,  1165,   547,
     418,   260,   592,   594,   596,   807,   776,   350,  1050,  1064,
     413,  1038,   828,  1190,   403,  1805,   259,   -75,   406,   407,
    1247,   893,   -75,   415,   433,   342,   830,    14,  1424,  1201,
     810,  1650,   870,   -40,   -39,   555,   607,   610,   -40,   -39,
     555,   811,  1806,  1593,   439,  1595,  -354,  1658,  1743,  1812,
      14,  1812,    14,  1650,  -901,  -904,   555,   513,   389,    14,
     369,   513,   910,   910,   910,   910,   416,   548,   530,   910,
     902,   903,  1256,  1257,   510,   511,   418,  1236,  -715,  1226,
     365,    14, -1019,  -902,  1361,   588,   413,  1823,   603,  -944,
     603,   191,   879,   116,   879,   371,   805,   532,  1347,   415,
    -104,  1829,  1123,   430,   372,   442,   524,   418,   494,   253,
    1405,  1348,   902,   903,  -914,  -104,   952,   531,   222,   222,
     541, -1019,  1174,   430,  1495,   510,   511,   518,  1934,  1957,
     415,  1349,  1824,  -905,  -947,   392,  -916,  -716,  -903,   639,
     540,  -946,   269,   518,  -723,   341,  1227,  1285,   589,  -908,
     604,  -907,   429,   415,   880,  -888,   429,  -889,  1872,  1289,
    1135,   256,   379,   522,  -597,   480,  1259,  1175,   550,  1496,
     421,   550,  -812,  1935,  1958,  1362,  -812,   481,   260,   561,
    -912,  -911,  1490,  -901,  -904,   401,   514,   379,  -604,   572,
     514,   379,   379,   831,  1425,  -909,  -604,  -293,   366,  -293,
     829,   705,  -810,  1807,  1204,  1565,   -75,  1504,  -277,   894,
    1465,   552,  -902,   434,  1510,   557,  1512,   379,  -944,  1417,
    1651,  1652,   -40,   -39,   556,   608,   611,   583,  -722,   637,
    -812,  1028,  1594,  -717,  1596,  -354,  1659,  1744,  1813,  1254,
    1862,  1258,  1930,  1382,   895,  1534,   364,   364,   597,   598,
     911,  1005,  1337,  1540,   348,     3,   519,  1600,  1176,   618,
    1497,  1047,  -905,  -947,  1936,  1959,  1049,  -903,  1606,   621,
    -946,  1187,   519,  1132,  1133,  -915,  1157,  -918,  -908,   617,
    -907,   787,  -607,  1402,  -888,  -724,  -889,   648,   390,   528,
    -913,   260,   415,   404,   443,   645,  -540,  1094,   233,   629,
     260,   260,   629,   260, -1019,  1352,  1653,  -605,   643,  1256,
    1257,  1887,   343,  1946,   517,  1031,  1800,   342,  1255,  1256,
    1257,   987,   517,   899,  -909,   430,   263,   197,   206,    40,
    1756,   222,  1757,   937,  1801,   688,  1095,  1148,  1353,   264,
   -1019,   674,   408, -1019,   538,   265,   116,   700,   616,  1968,
     116,  1622,   703,  1802,   562,   756,  1888,   632,   632,  1607,
     632,   353,  1345,   393,   394,   375,   129,   706,   707,   708,
     709,   711,   712,   713,   714,   715,   716,   717,   718,   719,
     720,   721,   722,   723,   724,   725,   726,   727,   728,   729,
     730,   731,   732,   733,   734,   123,   736,   376,   737,   737,
     740,   384,   342,  1427,  1947,   403,   751,   752,   439,   690,
     759,   760,   761,   762,   763,   764,   765,   766,   767,   768,
     769,   770,   771,   249,   377,   958,   758,   960,   737,   781,
    1414,   700,   700,   737,   785,   242,   247,   582,   759,   248,
    1969,   789,  -606,  1180,   938,   757,   111,  1198,   493,  1028,
     797,   409,   799,  1954,  1181,   745,  -103,   378,   410,   939,
     700,   634,   165,   636,  1808,   222,   382,  1359,   818,   754,
     819,  -103,   390,   986,   222,   385,   620,   221,   223,   645,
     570,   222,   571,  1809,   817,   877,  1810,   804,  1369,   597,
     597,  1371,   661,   222,   390,   664,  -876,   492,   206,    40,
     663,   665,   342,  1253,  1554,   998,   510,   511,   116,  1206,
     390,  -876,   822,   994,   373,  1855,  -879,   391,   692,  1129,
     889,  1130,   341,   390,   374,   379,   883,   510,   511,   390,
     645,  -879,  -718,  1216,  1856,   390,   422,  1857,   576,   941,
     383,  1331,  1332,   958,   960,   747,    55,   393,   394,   386,
    1039,   960,  -877,   387,   440,   179,   180,    65,    66,    67,
     440,   179,   180,    65,    66,    67,   417,  -877,  -916,   393,
     394,   779,   415,  1044,  1045,   582,   379,   749,   379,   379,
     379,   379,  1511,   543,   392,   393,   394,  1629,  1357,   638,
     388,  1955,   747,   930,   691,  1124,   404,   646,   393,   394,
     674,   774,   405,   806,   393,   394,   812,   428,   390,   640,
     393,   394,   966,   429,   420,   425,   111,  1040,  1282,   222,
    1284,   976,   582,   510,   511,   977,   441,  1398,   695,  1400,
     390,   348,   441,   432,   494,   429,   809,   645,   793,  1912,
     435,   661,   417,   444,   664,   739,  1384,   116,   445,   663,
     665,   446,  1494,   510,   511,  1238,  1239,  1375,   447,   985,
     448,  1028,  1028,  1028,  1028,  1028,  1028,   868,  1385,   650,
     651,  1028,  1506,   417,   780,   449,  -120,  1416,   450,   784,
    -120,  1331,  1332,   393,   394,  1561,  1562,  1598,   997,   451,
     534,  -598,   888,   692,  1770,  1771,   542,  -120,  1927,  1116,
    1117,  1118,  1119,  1120,  1121,   393,   394,  1942,  1943,   477,
     478,   479,  1945,   480,   485,   165,  -599,   641,  1122,   165,
    -600,   647,  -601,  1042,  -602,   481,   486,   925,   927,   591,
     593,  1119,  1120,  1121,  1048,  1421,  1256,  1257,   483,   260,
     916,   918,  1916,  1917,  1918,  1216,  1394,  1122,   641,  1394,
     647,   641,   647,   647,   484,  1406,   424,   426,   427,  1853,
    1854,   129,  -603,   597,   516,   597,  1485,  -910,   944,  1459,
     833,   597,   597,  -716,  1654,  1849,  1850,   661,  1075,  1078,
     664,   520,   397,  1059,   525,   663,   665,  1502,   527,   481,
     123,   430,   533,  -914,   379,   222,   517,   536,   537,   523,
     496,   497,   498,   499,   500,   501,   502,   503,   504,   505,
     506,   507,   545,  -714,   553,  1623,   577,   129,   440,   179,
     180,    65,    66,    67,   606,   544,   566,  1028,  1152,  1028,
    1153,   574,   819,   614,  1508,   619, -1058,   578,   584,   585,
     626,   599,   600,  1155,   508,   509,   123,   612,   609,   602,
     623,   984,   644,   613,   222,  1536,   622,  1164,   666,   667,
     676,   161,   689,   677,   947,   948,    55,   678,   680,  -125,
     702,  1545,   792,   956,   820,   790,   640,   165,   129,  1021,
     124,  1034,   126,  1185,   674,   122,   794,   795,   127,   555,
     441,   801,   802,  1193,   824,   222,  1194,   222,  1195,   129,
     827,   674,   700,   116,   841,   572,  1365,   123,   842,   871,
    1205,  1517,   874,  1518,  1904,   745,   876,  1057,   116,   510,
     511,   873,  1614,   878,  1616,   222,   875,   896,   123,   892,
     897,   900,   907,   901,  1904,   912,   909,   915,   249,   914,
     917,   928,  1066,  1926,   919,   920,   921,   922,  1072,   933,
     242,   247,  1235,   934,   248,   936,  -739,   942,  1241,   116,
     943,  1611,  1028,   945,  1028,   946,  1028,   949,  1131,   692,
     953,  1028,   440,    63,    64,    65,    66,    67,   626,   954,
    1631,   962,   679,    72,   487,   964,   968,   967,   970,  1637,
     973,   979,   222,   982,   129,  1144,   129,   980,   661,  1242,
     983,   664,   991,  1644,   999,  1166,   663,   665,   222,   222,
    1001,  1339,  1002,  1003,  1143,   975,   165,   779,  1051,   812,
     116,  -720,  1041,   123,   597,   123,   489,  1061,  1063,  1065,
    1069,  1071,  1769,  1070,  1073,   582,  1774,  1086,  1087,   695,
     695,   116,  1127,  1088,   441,  1089,  1625,   774,  1626,   809,
    1627,  1090,  1091,  1092,  1134,  1628,  1136,   474,   475,   476,
     477,   478,   479,  1290,   480,  1028,  1341,  1138,  1140,  1141,
    1142,  1147,  1151,  1162,  1160,  1163,   481,  1154,  1169,  1785,
    1167,   661,  1340,   379,   664,  1188,  1171,   161,  1197,   663,
     665,  1203,  1200,  1208,  1571,  1213,  1213,  1021,   812,  1367,
    -917,  1868,  1209,  1219,   129,  1220,   124,  1221,   126,  1222,
    1223,   122,   700,  1224,   127,  1228,  1225,  1229,   674,  1230,
    1232,   674,  1244,   700,  1341,  1246,  1249,  1252,   809,  1250,
    1261,  1158,  1262,   123,   116,  1268,   116,  1269,   116,  1263,
    1122,  1231,  1272,  1273,   205,  1323,  1325,  1168,  1328,  1778,
     222,   222,  1409,  1335,   965,  1326,  1338,  1355,   987,  1265,
    1182,   260,  1356,  1363,  1364,  1370,    50,  1380,  1368,  1386,
    1387,  1423,  1372,  1388,  1374,  1012,  1401,  1376,  1410,  1915,
    1411,  1418,  1377,   582,  1379,  1408,  1428,  1270,  1572,  1202,
    1413,  1431,  1828,  1422,  1274,  1412,  1831,  1433,  1434,  1437,
     129,  1573,   209,   210,   211,   212,   213,  1574,  1438,  1439,
    1443,  1442,   868,   996,  1445,  1953,  1827,  1446,  1447,  1448,
    1028,  1028,  1449,  1451,   182,  1453,  1834,    91,  1575,   123,
      93,    94,  1454,    95,  1576,    97,  1460,  1499,  1458,  1498,
    1461,  1521,  1523,  1501,  1487,  1599,   116,  1488,  1513,  1503,
    1525,  1530,  1520,  1505,  1035,  1532,  1036,  1486,   107,  1467,
    1524,  1509,  1516,  1260,  1491,  1514,  1484,  1264,  1492,  1515,
    1493,  1869,   165,  1519,  1535,  1527,  1484,  1528,  1500,   443,
    1537,  1529,   222,  1479,  1055,  1542,  1531,   165,  1507,   700,
    1538,  1539,  1546,  1479,  1543,  1569,  1558,  1582,  1597,  1608,
    1602,    14,   674,  1609,  1832,  1833,  1612,  1620,  1617,  1021,
    1021,  1021,  1021,  1021,  1021,  1618,  1891,  1624,  1639,  1021,
    1642,  1648,  1656,  1657,  1758,  1751,  1752,  1389,   165,  1764,
     116,  1765,  1925,   222,  1767,  1768,  1790,  1777,  1779,  1780,
    1791,  1811,   116,  1817,  1821,  1820,  1843,  1937,   222,   222,
    1754,  1139,  1430,  1845,  1826,  1851,  1859,  1847,  1860,  1861,
    1866,  1867,  1875,  -350,  1874,  1468,  1871,   626,  1150,  1358,
    1469,   956,   440,  1470,   180,    65,    66,    67,  1471,  1877,
     216,   216,  1878,  1951,  1882,  1806,  1440,  1880,  1956,   165,
    1444,   239,  1883,  1886,  1892,  1450,  1889,  1899,  1378,  1894,
    1893,  1381,  1455,  1906,  1901,  1910,   129,  1913,  1914,  1922,
     165,  1928,   344,  1610,  1924,  1929,   700,   239,  1948,  1462,
    1472,  1473,  1931,  1474,  1949,  1938,  1952,  1960,  1961,  1970,
    1971,   494,  1973,  1974,  1484,   123,  1909,   222,  1327,   753,
    1484,   748,  1484,  1199,   441,   674,  1159,   750,  1923,  1415,
    1784,  1479,  1921,  1475,  1591,  1544,   890,  1479,  1775,  1479,
    1429,  1799,  1655,  1804,  1484,  1963,  1182,  1589,  1816,    34,
      35,    36,  1933,  1773,  1283,  1021,  1399,  1021,   635,  1647,
    1570,  1479,   207,   129,  1350,  1275,  1467,  1390,  1215,  1391,
    1233,  1179,   129,  1950,  1884,  1635,  1076,   628,  1330,  1267,
     701,  1322,  1522,   165,     0,   165,  1526,   165,  1560,  1055,
    1248,  1965,   123,  1533,     0,     0,     0,     0,  1819,     0,
       0,   123,     0,  1766,     0,     0,     0,     0,    14,  1463,
    1464,     0,     0,     0,     0,  1649,    81,    82,    83,    84,
      85,     0,     0,     0,     0,     0,     0,   214,   116,     0,
       0,     0,  1484,    89,    90,     0,     0,   341,     0,     0,
       0,     0,     0,  1587,     0,     0,     0,    99,     0,  1479,
       0,     0,     0,     0,     0,  1759,     0,     0,   129,     0,
       0,   104,     0,     0,   129,     0,  1782,  1635,     0,     0,
       0,   129,  1468,   216,     0,     0,     0,  1469,     0,   440,
    1470,   180,    65,    66,    67,  1471,     0,   123,     0,     0,
    1021,     0,  1021,   123,  1021,   165,     0,     0,     0,  1021,
     123,     0,     0,     0,     0,   116,     0,     0,     0,  1547,
     116,  1548,     0,  1814,   116,     0,     0,     0,   344,  1897,
     344,  1366,     0,   239,     0,   239,     0,  1472,  1473,     0,
    1474,     0,   379,     0,     0,   582,     0,     0,   341,     0,
       0,     0,   440,    63,    64,    65,    66,    67,  1740,     0,
       0,   441,     0,    72,   487,  1747,     0,  1592,     0,  1822,
    1489,     0,   341,     0,   341,     0,     0,     0,     0,     0,
     341,     0,  1407,     0,     0,     0,   344,     0,   443,   165,
       0,   239,     0,     0,     0,     0,     0,   626,  1055,     0,
       0,   165,     0,  1021,   488,  1467,   489,     0,     0,     0,
     116,   116,   116,   342,     0,     0,   116,   216,     0,   490,
       0,   491,   129,   116,   441,  1864,   216,     0,     0,     0,
       0,     0,     0,   216,  1638,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   216,     0,    14,     0,     0,
       0,   123,     0,     0,     0,     0,   216,     0,     0,     0,
       0,     0,     0,     0,   129,     0,     0,     0,     0,     0,
       0,   129,     0,     0,     0,     0,   344,     0,     0,   344,
       0,   239,     0,     0,   239,     0,   626,     0,     0,     0,
       0,     0,     0,   123,     0,     0,     0,     0,     0,     0,
     123,     0,     0,     0,     0,   674,   129,     0,     0,     0,
       0,  1468,     0,     0,     0,  1898,  1469,     0,   440,  1470,
     180,    65,    66,    67,  1471,   674,  1467,     0,   129,     0,
       0,   239,     0,   582,   674,   123,     0,     0,     0,     0,
       0,     0,     0,     0,  1795,     0,     0,     0,     0,     0,
       0,     0,     0,  1962,   341,     0,     0,   123,  1021,  1021,
       0,   217,   217,     0,   116,  1972,  1472,  1473,    14,  1474,
       0,   216,     0,  1838,     0,  1975,     0,     0,  1976,     0,
    1740,  1740,     0,     0,  1747,  1747,     0,     0,   129,     0,
     441,     0,     0,   129,  1844,  1846,     0,     0,   379,  1615,
       0,     0,     0,     0,     0,     0,   116,   165,     0,     0,
       0,     0,     0,   116,     0,     0,     0,   123,   344,     0,
     837,     0,   123,   239,     0,   239,  1467,     0,   856,     0,
       0,     0,  1468,     0,     0,     0,     0,  1469,     0,   440,
    1470,   180,    65,    66,    67,  1471,  1818,     0,   116,     0,
       0,     0,     0,     0,  1467,     0,  1896,     0,     0,     0,
     856,     0,     0,     0,     0,     0,     0,     0,    14,     0,
     116,     0,     0,     0,     0,     0,  1911,     0,     0,     0,
       0,     0,     0,     0,   165,     0,     0,  1472,  1473,   165,
    1474,     0,     0,   165,   219,   219,    14, -1059, -1059, -1059,
   -1059, -1059,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
       0,   441,   344,   344,     0,     0,     0,   239,   239,     0,
    1619,   344,     0,  1122,     0,     0,   239,     0,     0,     0,
     116,  1879,  1468,     0,     0,   116,     0,  1469,     0,   440,
    1470,   180,    65,    66,    67,  1471,     0,   216,     0,  1467,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1468,     0,     0,     0,     0,  1469,     0,   440,  1470,   180,
      65,    66,    67,  1471,   217,     0,     0,     0,     0,   165,
     165,   165,     0,     0,     0,   165,     0,  1472,  1473,     0,
    1474,    14,   165,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   216,   956,     0,     0,
       0,   441,     0,     0,     0,  1472,  1473,     0,  1474,  1941,
    1621,   956,     0,     0,     0,     0,   523,   496,   497,   498,
     499,   500,   501,   502,   503,   504,   505,   506,   507,   441,
    1941,   239,  1966,     0,     0,     0,     0,   216,  1630,   216,
       0,     0,     0,     0,     0,  1468,     0,     0,     0,     0,
    1469,     0,   440,  1470,   180,    65,    66,    67,  1471,     0,
       0,   508,   509,     0,     0,     0,     0,   216,   856,   205,
    1068,   206,    40,     0,     0,   239,     0,   344,   344,     0,
       0,     0,   239,   239,   856,   856,   856,   856,   856,     0,
       0,    50,     0,     0,     0,     0,   856,   219,   217,     0,
    1472,  1473,     0,  1474,     0,     0,     0,   217,     0,     0,
       0,     0,   239,     0,   217,     0,     0,     0,     0,     0,
       0,     0,     0,   165,   441,     0,   217,   209,   210,   211,
     212,   213,     0,  1776,   216,     0,   510,   511,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   239,     0,
     216,   216,     0,   772,     0,    93,    94,     0,    95,   183,
      97,     0,     0,     0,     0,   165,     0,     0,     0,   344,
       0,     0,   165,     0,   239,   239,     0,   218,   218,     0,
       0,     0,     0,   107,   216,   344,     0,   773,   241,   111,
     239,     0,     0,     0,     0,     0,     0,     0,   344,   803,
       0,     0,     0,   239,     0,     0,     0,   165,     0,     0,
       0,   856,     0,     0,   239,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   344,     0,   165,
       0,   219,   239,     0,     0,     0,   239,     0,     0,     0,
     219,     0,     0,     0,     0,     0,     0,   219,     0,   239,
       0,     0,   217,     0,     0,     0,     0,     0,     0,   219,
       0,     0,     0,   834,     0,     0,     0,     0,     0,     0,
     662,   523,   496,   497,   498,   499,   500,   501,   502,   503,
     504,   505,   506,   507,     0,     0,     0,     0,     0,   165,
       0,     0,   216,   216,   165,     0,     0,     0,     0,     0,
       0,   344,     0,     0,     0,   344,   239,   837,     0,     0,
     239,     0,   239,   205,     0,     0,   508,   509,     0,     0,
       0,     0,     0,   835,     0,     0,     0,   856,   856,   856,
     856,   856,   856,   216,     0,    50,   856,   856,   856,   856,
     856,   856,   856,   856,   856,   856,   856,   856,   856,   856,
     856,   856,   856,   856,   856,   856,   856,   856,   856,   856,
     856,   856,   856,   856,     0,     0,     0,     0,     0,     0,
       0,   209,   210,   211,   212,   213,     0,     0,     0,   856,
       0,     0,     0,     0,     0,   219,     0,     0,     0,     0,
     218,   510,   511,   182,     0,     0,    91,     0,     0,    93,
      94,     0,    95,   183,    97,     0,   836,   344,     0,   344,
       0,     0,   239,     0,   239,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   216,     0,     0,   107,   217,     0,
       0,     0,     0,     0,     0,     0,   344,     0,     0,   344,
       0,   239,     0,     0,   239,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   898,     0,     0,   205,     0,   206,
      40,   239,   239,   239,   239,   239,   239,     0,     0,   216,
       0,   239,     0,     0,     0,   216,     0,     0,     0,    50,
       0,     0,     0,     0,     0,     0,     0,   217,     0,     0,
     216,   216,     0,   856,     0,     0,     0,     0,   344,     0,
       0,     0,     0,   239,   344,     0,     0,     0,     0,   239,
       0,     0,   856,     0,   856,   209,   210,   211,   212,   213,
       0,     0,     0,     0,   218,     0,     0,     0,   217,     0,
     217,     0,     0,   218,     0,     0,     0,     0,   856,     0,
     218,   772,     0,    93,    94,     0,    95,   183,    97,     0,
       0,     0,   218,     0,     0,     0,     0,     0,   217,     0,
       0,   219,     0,   218,   452,   453,   454,   344,   344,     0,
       0,   107,   239,   239,     0,   808,   239,   111,     0,   216,
       0,     0,     0,     0,   455,   456,     0,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
       0,   480,     0,     0,   282,     0,     0,     0,     0,     0,
     219,     0,     0,   481,     0,   217,     0,   239,     0,   239,
       0,     0,     0,     0,     0,     0,     0,     0,   241,     0,
       0,   217,   217,     0,     0,     0,     0,     0,     0,     0,
       0,   284,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   219,     0,   219,   205,     0,     0,   344,     0,   344,
       0,     0,   239,     0,   239,     0,     0,     0,   218,     0,
     856,     0,   856,     0,   856,     0,    50,     0,     0,   856,
     216,   219,     0,   856,   575,   856,     0,     0,   856,     0,
       0,     0,     0,     0,   344,     0,     0,     0,     0,   239,
     239,     0,     0,   239,     0,   344,     0,     0,     0,     0,
     239,   568,   209,   210,   211,   212,   213,   569,     0,     0,
       0,     0,     0,     0,     0,   863,     0,     0,     0,   282,
       0,     0,     0,     0,   182,     0,     0,    91,   335,     0,
      93,    94,     0,    95,   183,    97,     0,  1343,   219,     0,
       0,     0,   239,     0,   239,     0,   239,   863,   339,     0,
       0,   239,     0,   216,   219,   219,   284,     0,   107,   340,
       0,     0,   344,   217,   217,     0,     0,   239,     0,   205,
     856,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   239,   239,     0,   344,     0,     0,   662,     0,
     239,    50,   239, -1059, -1059, -1059, -1059, -1059,   472,   473,
     474,   475,   476,   477,   478,   479,     0,   480,     0,   344,
       0,   344,     0,     0,   239,     0,   239,   344,     0,   481,
       0,     0,   239,     0,     0,     0,   568,   209,   210,   211,
     212,   213,   569,     0,   218,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   239,     0,     0,     0,   182,
       0,     0,    91,   335,     0,    93,    94,     0,    95,   183,
      97,     0,   856,   856,   856,     0,     0,     0,     0,   856,
       0,   239,   344,   339,     0,     0,     0,   239,     0,   239,
     452,   453,   454,   107,   340,     0,     0,     0,     0,     0,
       0,     0,     0,   218,     0,   217,   219,   219,     0,     0,
     455,   456,     0,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,     0,   480,  1027,     0,
       0,     0,     0,     0,   218,     0,   218,   662,     0,   481,
       0,     0,     0,     0,     0,     0,   217,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   217,   217,     0,   218,   863,     0,     0,     0,     0,
       0,     0,     0,     0,   344,     0,     0,     0,     0,   239,
     834,   863,   863,   863,   863,   863,     0,     0,     0,     0,
       0,   344,     0,   863,     0,     0,   239,     0,     0,     0,
     239,   239,     0,     0,     0,     0,     0,     0,     0,  1126,
    1839,     0,     0,     0,     0,   239,     0,     0,   220,   220,
       0,   856,     0,     0,     0,     0,     0,     0,   219,   245,
     205,   218,   856,     0,     0,     0,     0,     0,   856,   205,
     835,     0,   856,     0,     0,  1146,     0,   218,   218,     0,
     217,     0,    50,     0,     0,     0,     0,     0,     0,   344,
       0,    50,     0,     0,   239,     0,     0,     0,     0,     0,
     929,     0,  1146,   662,     0,     0,     0,     0,     0,   219,
       0,   218,     0,     0,     0,     0,     0,     0,   209,   210,
     211,   212,   213,     0,   219,   219,     0,   209,   210,   211,
     212,   213,     0,     0,   856,   205,     0,     0,   863,     0,
     182,  1189,     0,    91,   239,     0,    93,    94,     0,    95,
     183,    97,   397,  1266,     0,    93,    94,    50,    95,   183,
      97,   239,     0,   241,     0,   344,     0,     0,     0,     0,
     239,     0,     0,     0,   107,     0,  1027,   344,     0,   344,
       0,     0,   239,   107,   239,     0,     0,   398,     0,     0,
       0,     0,     0,   209,   210,   211,   212,   213,   344,     0,
     344,     0,     0,   239,     0,   239,     0,     0,     0,     0,
       0,     0,     0,   219,     0,     0,     0,     0,     0,   218,
     218,    93,    94,     0,    95,   183,    97,  1100,  1101,  1102,
    1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,   107,
     702,     0,     0,     0,   863,   863,   863,   863,   863,   863,
     218,   220,  1122,   863,   863,   863,   863,   863,   863,   863,
     863,   863,   863,   863,   863,   863,   863,   863,   863,   863,
     863,   863,   863,   863,   863,   863,   863,   863,   863,   863,
     863,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   452,   453,   454,     0,   863,   495,   496,   497,
     498,   499,   500,   501,   502,   503,   504,   505,   506,   507,
       0,     0,   455,   456,   662,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,     0,   480,
       0,   218,   508,   509,     0,     0,     0,     0,     0,  1099,
       0,   481,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,  1120,  1121,     0,     0,     0,     0,  1027,  1027,
    1027,  1027,  1027,  1027,     0,   220,   218,  1122,  1027,     0,
       0,     0,   218,     0,   220,     0,     0,   662,     0,     0,
       0,   220,     0,     0,     0,     0,     0,   218,   218,     0,
     863,     0,     0,   220,     0,     0,     0,   510,   511,     0,
       0,     0,     0,     0,   245,     0,     0,     0,     0,   863,
       0,   863,     0,   452,   453,   454,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   455,   456,   863,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,     0,
     480,     0,   961,   452,   453,   454,     0,     0,     0,     0,
       0,     0,   481,  1466,     0,     0,   218,     0,     0,   245,
       0,     0,     0,   455,   456,     0,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,     0,
     480,   452,   453,   454,     0,     0,     0,     0,     0,   220,
       0,     0,   481,     0,  1027,     0,  1027,     0,     0,     0,
       0,   455,   456,     0,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,     0,   480,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     481,     0,     0,     0,     0,     0,   864,   863,     0,   863,
       0,   863,     0,     0,     0,     0,   863,   218,     0,     0,
     863,     0,   863,     0,     0,   863,     0,     0,   452,   453,
     454,     0,     0,  1000,     0,     0,     0,  1568,   864,     0,
    1581,     0,     0,     0,     0,     0,     0,     0,   455,   456,
     865,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,     0,   480,     0,     0,     0,     0,
       0,     0,   891,  1004,     0,  1006,  1007,   481,     0,  1027,
       0,  1027,     0,  1027,     0,     0,     0,     0,  1027,     0,
     218,     0,     0,     0,     0,  1008,     0,     0,     0,     0,
       0,     0,     0,  1009,  1010,  1011,   205,   863,     0,     0,
     452,   453,   454,     0,     0,   220,  1012,     0,     0,  1645,
    1646,  1137,     0,     0,     0,     0,     0,     0,    50,  1581,
     455,   456,     0,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,     0,   480,     0,     0,
       0,     0,     0,  1013,  1014,  1015,  1016,  1017,  1018,   481,
       0,     0,     0,     0,   220,     0,     0,     0,     0,     0,
       0,  1019,  1027,     0,     0,     0,   182,     0,     0,    91,
      92,     0,    93,    94,     0,    95,   183,    97,     0,   863,
     863,   863,     0,     0,     0,   788,   863,     0,  1793,     0,
    1020,     0,     0,     0,     0,   220,  1581,   220,     0,     0,
     107,     0,   523,   496,   497,   498,   499,   500,   501,   502,
     503,   504,   505,   506,   507,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   282,   220,   864,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   864,   864,   864,   864,   864,   508,   509,     0,
       0,     0,     0,     0,   864,     0,     0,     0,     0,     0,
       0,   284,     0,     0,     0,     0,     0,     0,     0,     0,
    1056,     0,     0,     0,   205,     0,     0,     0,     0,     0,
    1196,     0,     0,     0,     0,     0,  1079,  1080,  1081,  1082,
    1083,     0,   220,     0,     0,     0,    50,     0,  1093,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   220,   220,
       0,     0,     0,     0,     0,     0,     0,  1027,  1027,     0,
       0,     0,   510,   511,     0,   205,     0,     0,     0,     0,
       0,   568,   209,   210,   211,   212,   213,   569,   863,     0,
       0,     0,   245,     0,     0,     0,     0,    50,     0,   863,
       0,     0,     0,     0,   182,   863,     0,    91,   335,   863,
      93,    94,     0,    95,   183,    97,     0,  1074,     0,   864,
       0,     0,     0,     0,     0,     0,     0,     0,   339,     0,
       0,     0,     0,   209,   210,   211,   212,   213,   107,   340,
       0,     0,     0,     0,   245,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   182,     0,     0,    91,     0,
       0,    93,    94,  1186,    95,   183,    97,     0,     0,     0,
       0,   863,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1908,     0,   452,   453,   454,     0,     0,     0,   107,
       0,     0,     0,     0,  1837,     0,     0,     0,  1568,     0,
     220,   220,     0,   455,   456,     0,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,     0,
     480,     0,     0,     0,     0,   864,   864,   864,   864,   864,
     864,   245,   481,     0,   864,   864,   864,   864,   864,   864,
     864,   864,   864,   864,   864,   864,   864,   864,   864,   864,
     864,   864,   864,   864,   864,   864,   864,   864,   864,   864,
     864,   864,     0,     0,     0,     0,     0,     0,     0,  1083,
    1278,     0,     0,  1278,     0,     0,     0,   864,  1291,  1294,
    1295,  1296,  1298,  1299,  1300,  1301,  1302,  1303,  1304,  1305,
    1306,  1307,  1308,  1309,  1310,  1311,  1312,  1313,  1314,  1315,
    1316,  1317,  1318,  1319,  1320,  1321,   452,   453,   454,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1329,   220,     0,     0,     0,   455,   456,     0,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,     0,   480,     0,   205,     0,     0,     0,     0,
       0,     0,     0,  1207,     0,   481,     0,   245,     0,     0,
       0,     0,     0,   220,     0,     0,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   220,   220,
       0,   864,     0,     0,   272,   273,     0,   274,   275,     0,
       0,   276,   277,   278,   279,     0,     0,     0,     0,     0,
     864,     0,   864,   209,   210,   211,   212,   213,     0,   280,
     281,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   526,     0,     0,  1419,   864,     0,     0,  1745,
       0,    93,    94,  1746,    95,   183,    97,     0,   283,     0,
       0,     0,     0,     0,  1435,     0,  1436,     0,     0,     0,
       0,     0,   285,   286,   287,   288,   289,   290,   291,   107,
    1586,     0,   205,     0,   206,    40,     0,   220,     0,     0,
    1456,     0,     0,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,    50,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,     0,   326,
       0,   743,   328,   329,   330,     0,     0,     0,   331,   579,
     209,   210,   211,   212,   213,   580,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   581,     0,     0,     0,     0,     0,    93,    94,
       0,    95,   183,    97,   336,     0,   337,     0,     0,   338,
       0,     0,     0,     0,   452,   453,   454,     0,   864,     0,
     864,     0,   864,     0,     0,     0,   107,   864,   245,     0,
     744,   864,   111,   864,   455,   456,   864,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
     205,   480,  1550,     0,  1551,     0,  1552,     0,     0,     0,
       0,  1553,     0,   481,     0,  1555,     0,  1556,     0,     0,
    1557,     0,    50,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,     0,   480,     0,  1585,     0,     0,
       0,   245,     0,     0,     0,     0,     0,   481,   209,   210,
     211,   212,   213,     0,     0,     0,     0,     0,   864,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     5,     6,
       7,     8,     9,     0,     0,     0,    93,    94,    10,    95,
     183,    97,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,   412,    13,     0,     0,     0,     0,     0,
       0,     0,  1640,   755,   107,  1586,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,  1237,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,    38,    39,    40,
     864,   864,   864,     0,     0,     0,    43,   864,     0,     0,
       0,     0,     0,     0,     0,     0,  1798,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,   178,   179,   180,    65,    66,    67,
       0,     0,    69,    70,  1786,  1787,  1788,     0,     0,     0,
       0,  1792,   181,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   182,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   183,    97,     0,     0,
       0,    99,   205,     0,   100,     0,     0,     0,     0,     0,
     101,   452,   453,   454,     0,   104,   105,   106,     0,     0,
     107,   108,     0,     0,    50,     0,   111,   112,     0,   113,
     114,   455,   456,     0,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,     0,   480,     0,
     209,   210,   211,   212,   213,     0,     0,     0,     0,   864,
     481,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     864,     0,     0,     0,     0,     0,   864,     0,    93,    94,
     864,    95,   183,    97,     0,     0,     0,     0,     0,     0,
       0,     0,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,  1848,  1881,     0,   107,   975,     0,     0,
       0,     0,     0,     0,  1858,     0,    11,    12,    13,     0,
    1863,     0,     0,     0,  1865,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,     0,    15,
      16,     0,   864,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,    33,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,    41,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    47,  1900,    48,     0,    49,
       0,  1604,    50,    51,     0,     0,     0,    52,    53,    54,
      55,    56,    57,    58,     0,    59,    60,    61,    62,    63,
      64,    65,    66,    67,     0,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,    86,     0,     0,    87,     0,     0,     0,     0,
      88,    89,    90,    91,    92,     0,    93,    94,     0,    95,
      96,    97,    98,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,     0,   101,   102,     0,   103,     0,   104,
     105,   106,     0,     0,   107,   108,     0,   109,   110,  1156,
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
     109,   110,  1344,   111,   112,     0,   113,   114,     5,     6,
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
     106,     0,     0,   107,   108,     0,   109,   110,   681,   111,
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
     110,  1125,   111,   112,     0,   113,   114,     5,     6,     7,
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
     108,     0,   109,   110,  1170,   111,   112,     0,   113,   114,
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
       0,     0,   107,   108,     0,   109,   110,  1243,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
      15,    16,     0,     0,     0,     0,    17,     0,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,     0,
      29,    30,    31,    32,    33,     0,     0,     0,    34,    35,
      36,    37,    38,    39,    40,     0,    41,    42,     0,     0,
       0,    43,    44,    45,    46,  1245,    47,     0,    48,     0,
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
       0,    48,     0,    49,  1420,     0,    50,    51,     0,     0,
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
       0,   107,   108,     0,   109,   110,  1559,   111,   112,     0,
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
     105,   106,     0,     0,   107,   108,     0,   109,   110,  1789,
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
      48,  1835,    49,     0,     0,    50,    51,     0,     0,     0,
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
     107,   108,     0,   109,   110,  1870,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     0,     0,    17,     0,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    30,
      31,    32,    33,     0,     0,     0,    34,    35,    36,    37,
      38,    39,    40,     0,    41,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    47,  1873,    48,     0,    49,     0,
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
     110,  1890,   111,   112,     0,   113,   114,     5,     6,     7,
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
     108,     0,   109,   110,  1907,   111,   112,     0,   113,   114,
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
       0,     0,   107,   108,     0,   109,   110,  1964,   111,   112,
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
    1967,   111,   112,     0,   113,   114,     5,     6,     7,     8,
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
       0,     0,     0,    11,    12,    13,     0,     0,   551,     0,
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
       0,   821,     0,     0,     0,     0,     0,     0,     0,     0,
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
      12,    13,     0,     0,  1058,     0,     0,     0,     0,     0,
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
       0,     0,    11,    12,    13,     0,     0,  1634,     0,     0,
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
    1781,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,     0,    13,     0,     0,     0,     0,     0,     0,
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
     184,     0,   349,     0,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
    1120,  1121,     0,     0,     0,   696,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1122,     0,    15,    16,     0,
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
       0,   697,     0,    99,     0,     0,   100,     0,     0,     0,
       0,     0,   101,     0,     0,     0,     0,   104,   105,   106,
       0,     0,   107,   184,     0,     0,     0,     0,   111,   112,
       0,   113,   114,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,     0,     0,
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
     104,   105,   106,     0,     0,   107,   184,     0,     0,   816,
       0,   111,   112,     0,   113,   114,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,     0,   480,     0,
       0,  1183,     0,     0,     0,     0,     0,     0,     0,     0,
     481,     0,     0,    15,    16,     0,     0,     0,     0,    17,
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
      93,    94,     0,    95,   183,    97,     0,  1184,     0,    99,
       0,     0,   100,     0,     0,     0,     0,     0,   101,     0,
       0,     0,     0,   104,   105,   106,     0,     0,   107,   184,
       0,     0,     0,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,   412,     0,     0,     0,     0,     0,
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
       0,   107,   108,   452,   453,   454,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   455,   456,     0,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,     0,
     480,     0,     0,     0,     0,     0,     0,     0,     0,    15,
      16,     0,   481,     0,     0,    17,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,    29,
      30,    31,    32,     0,     0,     0,     0,    34,    35,    36,
      37,    38,    39,    40,     0,     0,     0,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,   196,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,   178,   179,
     180,    65,    66,    67,     0,     0,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,   181,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
     182,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     183,    97,     0,     0,     0,    99,     0,     0,   100,     0,
       0,     0,     0,  1605,   101,     0,     0,     0,     0,   104,
     105,   106,     0,     0,   107,   184,     0,     0,     0,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,  1102,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,  1121,     0,     0,     0,     0,
     232,     0,     0,     0,     0,     0,     0,     0,     0,  1122,
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
       0,     0,   104,   105,   106,     0,     0,   107,   184,   452,
     453,   454,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   455,
     456,  1424,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,     0,   480,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,   481,     0,
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
       0,    99,     0,     0,   100,     0,     0,  1425,     0,     0,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   184,     0,   267,   453,   454,   111,   112,     0,   113,
     114,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,   455,   456,     0,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,     0,
     480,     0,     0,     0,     0,     0,     0,     0,    15,    16,
       0,     0,   481,     0,    17,     0,    18,    19,    20,    21,
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
     106,     0,     0,   107,   184,     0,   270,     0,     0,   111,
     112,     0,   113,   114,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   412,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,   104,   105,   106,     0,     0,   107,   108,   452,   453,
     454,     0,   111,   112,     0,   113,   114,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   455,   456,
       0,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,     0,   480,     0,     0,     0,     0,
       0,     0,     0,     0,    15,    16,     0,   481,     0,     0,
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
      99,     0,     0,   100,     0,     0,   482,     0,     0,   101,
       0,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     184,   549,     0,     0,     0,   111,   112,     0,   113,   114,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   710,
     480,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   481,     0,     0,     0,     0,    15,    16,     0,
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
     477,   478,   479,     0,   480,     0,     0,     0,   755,     0,
       0,     0,     0,     0,     0,     0,   481,     0,     0,     0,
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
       9,     0,     0,     0,     0,     0,    10,  1103,  1104,  1105,
    1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,
    1116,  1117,  1118,  1119,  1120,  1121,     0,     0,     0,     0,
       0,   796,     0,     0,     0,     0,     0,     0,     0,  1122,
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
       0,     0,     0,     0,   111,   112,     0,   113,   114,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
    1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,     0,     0,
       0,     0,     0,     0,   798,     0,     0,     0,     0,     0,
       0,  1122,     0,     0,     0,     0,    15,    16,     0,     0,
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
       0,   107,   184,     0,     0,     0,     0,   111,   112,     0,
     113,   114,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   479,
       0,   480,     0,     0,     0,     0,     0,  1234,     0,     0,
       0,     0,     0,   481,     0,     0,     0,     0,     0,    15,
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
     105,   106,     0,     0,   107,   184,   452,   453,   454,     0,
     111,   112,     0,   113,   114,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   455,   456,     0,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,     0,   480,     0,     0,     0,     0,     0,     0,
       0,     0,    15,    16,     0,   481,     0,     0,    17,     0,
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
       0,   100,     0,     0,   565,     0,     0,   101,     0,     0,
       0,     0,   104,   105,   106,     0,     0,   107,   184,   452,
     453,   454,     0,   111,   112,     0,   113,   114,     5,     6,
       7,     8,     9,     0,     0,     0,   825,     0,    10,   455,
     456,     0,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,     0,   480,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,   481,     0,
       0,    17,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,    29,    30,    31,    32,     0,
       0,     0,     0,    34,    35,    36,    37,   642,    39,    40,
       0,   826,     0,     0,     0,     0,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,   178,   179,   180,    65,    66,    67,
       0,     0,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,   181,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    87,     0,     0,     0,     0,   182,    89,    90,    91,
      92,     0,    93,    94,     0,    95,   183,    97,     0,   272,
     273,    99,   274,   275,   100,     0,   276,   277,   278,   279,
     101,     0,     0,     0,     0,   104,   105,   106,     0,     0,
     107,   184,     0,     0,   280,   281,   111,   112,     0,   113,
     114, -1059, -1059, -1059, -1059,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,     0,   480,
       0,     0,     0,   283,     0,     0,     0,     0,     0,     0,
       0,   481,     0,     0,     0,     0,     0,   285,   286,   287,
     288,   289,   290,   291,     0,     0,     0,   205,     0,   206,
      40,     0,     0,     0,     0,     0,     0,     0,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,    50,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   205,   326,     0,   327,   328,   329,   330,
       0,     0,     0,   331,   579,   209,   210,   211,   212,   213,
     580,     0,     0,     0,     0,    50,     0,     0,     0,     0,
       0,   272,   273,     0,   274,   275,     0,   581,   276,   277,
     278,   279,     0,    93,    94,     0,    95,   183,    97,   336,
       0,   337,     0,     0,   338,     0,   280,   281,     0,   282,
       0,   209,   210,   211,   212,   213,     0,     0,     0,     0,
       0,   107,     0,     0,     0,   744,     0,   111,     0,     0,
       0,     0,     0,   182,     0,   283,    91,    92,     0,    93,
      94,     0,    95,   183,    97,     0,   284,     0,     0,   285,
     286,   287,   288,   289,   290,   291,     0,     0,     0,   205,
       0,     0,     0,     0,     0,     0,     0,   107,     0,     0,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,    50,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,     0,   326,     0,     0,   328,
     329,   330,     0,     0,     0,   331,   332,   209,   210,   211,
     212,   213,   333,     0,     0,     0,     0,  1210,  1211,  1212,
     205,     0,     0,     0,     0,     0,     0,     0,     0,   334,
       0,     0,    91,   335,     0,    93,    94,     0,    95,   183,
      97,   336,    50,   337,     0,     0,   338,   272,   273,     0,
     274,   275,     0,   339,   276,   277,   278,   279,     0,     0,
       0,     0,     0,   107,   340,     0,     0,     0,  1760,     0,
       0,     0,   280,   281,     0,   282,     0,     0,   209,   210,
     211,   212,   213,     0,  1105,  1106,  1107,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,
    1121,   283,     0,     0,     0,     0,    93,    94,     0,    95,
     183,    97,   284,     0,  1122,   285,   286,   287,   288,   289,
     290,   291,     0,     0,     0,   205,     0,     0,     0,     0,
       0,     0,     0,     0,   107,     0,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,    50,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,     0,   326,     0,     0,   328,   329,   330,     0,     0,
       0,   331,   332,   209,   210,   211,   212,   213,   333,     0,
       0,     0,     0,     0,     0,     0,   205,     0,     0,     0,
       0,     0,     0,     0,     0,   334,  1067,     0,    91,   335,
       0,    93,    94,     0,    95,   183,    97,   336,    50,   337,
       0,     0,   338,   272,   273,     0,   274,   275,     0,   339,
     276,   277,   278,   279,     0,     0,     0,     0,     0,   107,
     340,     0,     0,     0,  1830,     0,     0,     0,   280,   281,
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
     327,   328,   329,   330,     0,     0,     0,   331,   332,   209,
     210,   211,   212,   213,   333,     0,     0,     0,     0,     0,
       0,     0,   205,     0,   923,     0,   924,     0,     0,     0,
       0,   334,     0,     0,    91,   335,     0,    93,    94,     0,
      95,   183,    97,   336,    50,   337,     0,     0,   338,   272,
     273,     0,   274,   275,     0,   339,   276,   277,   278,   279,
       0,     0,     0,     0,     0,   107,   340,     0,     0,     0,
       0,     0,     0,     0,   280,   281,     0,   282,     0,     0,
     209,   210,   211,   212,   213,     0, -1059, -1059, -1059, -1059,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,   283,     0,     0,     0,     0,    93,    94,
       0,    95,   183,    97,   284,     0,  1122,   285,   286,   287,
     288,   289,   290,   291,     0,     0,     0,   205,     0,     0,
       0,     0,     0,     0,     0,     0,   107,     0,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,    50,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,     0,   326,     0,     0,   328,   329,   330,
       0,     0,     0,   331,   332,   209,   210,   211,   212,   213,
     333,     0,     0,     0,     0,     0,     0,     0,   205,     0,
       0,     0,     0,     0,     0,     0,     0,   334,     0,     0,
      91,   335,     0,    93,    94,     0,    95,   183,    97,   336,
      50,   337,     0,     0,   338,     0,   272,   273,     0,   274,
     275,   339,  1563,   276,   277,   278,   279,     0,     0,     0,
       0,   107,   340,     0,     0,     0,     0,     0,     0,     0,
       0,   280,   281,     0,   282,     0,   209,   210,   211,   212,
     213,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     283,     0,   437,     0,    93,    94,     0,    95,   183,    97,
       0,   284,     0,     0,   285,   286,   287,   288,   289,   290,
     291,     0,     0,     0,   205,     0,     0,     0,     0,     0,
       0,     0,   107,     0,     0,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,    50,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
       0,   326,     0,     0,   328,   329,   330,     0,     0,     0,
     331,   332,   209,   210,   211,   212,   213,   333,     0,     0,
       0,     0,     0,     0,     0,   205,     0,     0,     0,     0,
       0,     0,     0,     0,   334,     0,     0,    91,   335,     0,
      93,    94,     0,    95,   183,    97,   336,    50,   337,     0,
       0,   338,  1660,  1661,  1662,  1663,  1664,     0,   339,  1665,
    1666,  1667,  1668,     0,     0,     0,     0,     0,   107,   340,
       0,     0,     0,     0,     0,     0,  1669,  1670,  1671,     0,
       0,     0,     0,   209,   210,   211,   212,   213,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1672,     0,   359,     0,
       0,    93,    94,     0,    95,   183,    97,     0,     0,     0,
    1673,  1674,  1675,  1676,  1677,  1678,  1679,     0,     0,     0,
     205,     0,     0,     0,     0,     0,     0,     0,     0,   107,
       0,  1680,  1681,  1682,  1683,  1684,  1685,  1686,  1687,  1688,
    1689,  1690,    50,  1691,  1692,  1693,  1694,  1695,  1696,  1697,
    1698,  1699,  1700,  1701,  1702,  1703,  1704,  1705,  1706,  1707,
    1708,  1709,  1710,  1711,  1712,  1713,  1714,  1715,  1716,  1717,
    1718,  1719,  1720,     0,     0,     0,  1721,  1722,   209,   210,
     211,   212,   213,     0,  1723,  1724,  1725,  1726,  1727,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1728,  1729,  1730,     0,   205,     0,    93,    94,     0,    95,
     183,    97,  1731,     0,  1732,  1733,     0,  1734,     0,     0,
       0,     0,     0,     0,  1735,  1736,    50,  1737,     0,  1738,
    1739,     0,   272,   273,   107,   274,   275,     0,     0,   276,
     277,   278,   279,     0,     0,     0,     0,     0,  1572,     0,
       0,     0,     0,     0,     0,     0,     0,   280,   281,     0,
       0,  1573,   209,   210,   211,   212,   213,  1574,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   182,     0,   283,    91,    92,     0,
      93,    94,     0,    95,  1576,    97,     0,     0,     0,     0,
     285,   286,   287,   288,   289,   290,   291,     0,     0,     0,
     205,     0,     0,     0,     0,     0,     0,     0,   107,     0,
       0,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,    50,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   205,   326,     0,   327,
     328,   329,   330,     0,     0,     0,   331,   579,   209,   210,
     211,   212,   213,   580,     0,     0,     0,     0,    50,     0,
       0,     0,     0,     0,   272,   273,     0,   274,   275,     0,
     581,   276,   277,   278,   279,     0,    93,    94,     0,    95,
     183,    97,   336,     0,   337,     0,     0,   338,     0,   280,
     281,     0,     0,     0,   209,   210,   211,   212,   213,     0,
       0,     0,     0,     0,   107,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   283,   595,
       0,     0,    93,    94,     0,    95,   183,    97,     0,     0,
       0,     0,   285,   286,   287,   288,   289,   290,   291,     0,
       0,     0,   205,     0,     0,     0,     0,     0,     0,     0,
     107,     0,     0,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,    50,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   205,   326,
       0,  1289,   328,   329,   330,     0,     0,     0,   331,   579,
     209,   210,   211,   212,   213,   580,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,   272,   273,     0,   274,
     275,     0,   581,   276,   277,   278,   279,     0,    93,    94,
       0,    95,   183,    97,   336,     0,   337,     0,     0,   338,
       0,   280,   281,     0,     0,     0,   209,   210,   211,   212,
     213,     0,     0,     0,     0,     0,   107,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     283,     0,     0,     0,    93,    94,     0,    95,   183,    97,
       0,     0,     0,     0,   285,   286,   287,   288,   289,   290,
     291,     0,     0,     0,   205,     0,     0,     0,     0,     0,
       0,     0,   107,     0,     0,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,    50,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
       0,   326,     0,     0,   328,   329,   330,     0,     0,     0,
     331,   579,   209,   210,   211,   212,   213,   580,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   581,     0,     0,     0,     0,     0,
      93,    94,     0,    95,   183,    97,   336,     0,   337,     0,
       0,   338,   452,   453,   454,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   107,     0,
       0,     0,   455,   456,     0,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,     0,   480,
     452,   453,   454,     0,     0,     0,     0,     0,     0,     0,
       0,   481,     0,     0,     0,     0,     0,     0,     0,     0,
     455,   456,     0,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,     0,   480,   452,   453,
     454,     0,     0,     0,     0,     0,     0,     0,     0,   481,
       0,     0,     0,     0,     0,     0,     0,     0,   455,   456,
       0,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,     0,   480,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   481,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   452,   453,
     454,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   455,   456,
     567,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,     0,   480,  1096,  1097,  1098,     0,
       0,     0,     0,     0,     0,     0,   282,   481,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1099,   586,     0,
    1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
    1120,  1121,     0,   284,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1122,   205,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   590,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    50,     0,
       0,     0,     0,   282,     0,     0,  -397,     0,     0,     0,
       0,     0,     0,     0,   440,   179,   180,    65,    66,    67,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   568,   209,   210,   211,   212,   213,   569,
     284,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   205,     0,   813,   182,     0,     0,    91,
     335,     0,    93,    94,     0,    95,   183,    97,     0,     0,
       0,     0,     0,     0,     0,    50,     0,     0,     0,     0,
     339,     0,  1297,     0,     0,     0,   441,     0,     0,     0,
     107,   340,     0,     0,     0,     0,     0,     0,     0,     0,
     843,   844,     0,  1288,     0,     0,   845,     0,   846,     0,
     568,   209,   210,   211,   212,   213,   569,     0,     0,     0,
     847,     0,     0,     0,     0,     0,     0,     0,    34,    35,
      36,   205,     0,   182,     0,     0,    91,   335,     0,    93,
      94,   207,    95,   183,    97,     0,  1432,     0,     0,     0,
       0,     0,     0,    50,     0,     0,     0,   339,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   107,   340,     0,
       0,     0,   205,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   848,   849,
     850,   851,   852,   853,    50,    81,    82,    83,    84,    85,
       0,     0,   881,   882,     0,     0,   214,  1052,     0,     0,
       0,   182,    89,    90,    91,    92,     0,    93,    94,     0,
      95,   183,    97,     0,     0,     0,    99,     0,     0,     0,
     209,   210,   211,   212,   213,   854,     0,     0,     0,    29,
     104,     0,     0,     0,     0,   107,   855,    34,    35,    36,
     205,     0,   206,    40,     0,   595,     0,     0,    93,    94,
     207,    95,   183,    97,     0,     0,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   107,     0,   208,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1053,    75,   209,   210,
     211,   212,   213,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   214,     0,     0,     0,     0,
     182,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     183,    97,     0,   843,   844,    99,     0,     0,     0,   845,
       0,   846,     0,     0,     0,     0,     0,     0,     0,   104,
       0,     0,     0,   847,   107,   215,     0,     0,     0,     0,
     111,    34,    35,    36,   205,     0,     0,     0,   452,   453,
     454,     0,     0,     0,   207,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    50,     0,   455,   456,
       0,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,     0,   480,     0,     0,     0,     0,
       0,   848,   849,   850,   851,   852,   853,   481,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,   214,
       0,     0,     0,     0,   182,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   183,    97,    29,     0,     0,    99,
       0,     0,     0,     0,    34,    35,    36,   205,   854,   206,
      40,     0,     0,   104,     0,     0,     0,   207,   107,   855,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    50,
       0,     0,     0,     0,   535,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   208,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    75,   209,   210,   211,   212,   213,
       0,    81,    82,    83,    84,    85,     0,     0,     0,     0,
       0,     0,   214,     0,     0,     0,     0,   182,    89,    90,
      91,    92,     0,    93,    94,     0,    95,   183,    97,    29,
       0,     0,    99,     0,     0,     0,     0,    34,    35,    36,
     205,     0,   206,    40,     0,     0,   104,     0,     0,     0,
     207,   107,   215,     0,     0,   605,     0,   111,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   208,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   625,    75,   209,   210,
     211,   212,   213,     0,    81,    82,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   214,     0,     0,     0,     0,
     182,    89,    90,    91,    92,     0,    93,    94,     0,    95,
     183,    97,    29,   995,     0,    99,     0,     0,     0,     0,
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
       0,     0,  1149,    75,   209,   210,   211,   212,   213,     0,
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
      97,     0,     0,     0,    99,     0,     0,   452,   453,   454,
       0,     0,     0,     0,     0,     0,     0,     0,   104,     0,
       0,     0,     0,   107,   215,     0,     0,   455,   456,   111,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   479,     0,   480,   452,   453,   454,     0,     0,
       0,     0,     0,     0,     0,     0,   481,     0,     0,     0,
       0,     0,     0,     0,     0,   455,   456,     0,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,     0,   480,     0,     0,     0,     0,     0,     0,     0,
       0,   452,   453,   454,   481,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   455,   456,   913,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,     0,   480,   452,
     453,   454,     0,     0,     0,     0,     0,     0,     0,     0,
     481,     0,     0,     0,     0,     0,     0,     0,     0,   455,
     456,   981,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,     0,   480,     0,     0,     0,
       0,     0,     0,     0,     0,   452,   453,   454,   481,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   455,   456,  1037,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,     0,   480,  1096,  1097,  1098,     0,     0,     0,     0,
       0,     0,     0,     0,   481,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1099,  1342,     0,  1100,  1101,  1102,
    1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1096,  1097,
    1098,     0,  1122,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1099,
       0,  1373,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,
    1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,  1120,  1121,     0,     0,  1096,  1097,  1098,     0,
       0,     0,     0,     0,     0,     0,     0,  1122,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1099,     0,  1271,
    1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
    1120,  1121,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1096,  1097,  1098,     0,  1122,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1099,     0,  1441,  1100,  1101,  1102,  1103,  1104,
    1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,  1118,  1119,  1120,  1121,     0,     0,  1096,
    1097,  1098,     0,     0,     0,     0,     0,     0,     0,     0,
    1122,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1099,     0,  1452,  1100,  1101,  1102,  1103,  1104,  1105,  1106,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,  1120,  1121,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1096,  1097,  1098,     0,  1122,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1099,     0,  1549,  1100,  1101,
    1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
       0,    34,    35,    36,   205,     0,   206,    40,     0,     0,
       0,     0,     0,  1122,   207,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1641,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   236,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   237,     0,     0,     0,     0,     0,     0,
       0,     0,   209,   210,   211,   212,   213,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,   214,
    1643,     0,     0,     0,   182,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   183,    97,     0,     0,     0,    99,
       0,    34,    35,    36,   205,     0,   206,    40,     0,     0,
       0,     0,     0,   104,   656,     0,     0,     0,   107,   238,
       0,     0,     0,     0,   111,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   208,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   209,   210,   211,   212,   213,     0,    81,    82,
      83,    84,    85,     0,     0,     0,     0,     0,     0,   214,
       0,     0,     0,     0,   182,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   183,    97,     0,     0,     0,    99,
       0,    34,    35,    36,   205,     0,   206,    40,     0,     0,
       0,     0,     0,   104,   207,     0,     0,     0,   107,   657,
       0,     0,     0,     0,   111,     0,    50,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   236,     0,     0,   205,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   209,   210,   211,   212,   213,    50,    81,    82,
      83,    84,    85,     0,     0,   356,   357,     0,     0,   214,
       0,     0,     0,     0,   182,    89,    90,    91,    92,     0,
      93,    94,     0,    95,   183,    97,     0,     0,     0,    99,
       0,     0,     0,   209,   210,   211,   212,   213,     0,     0,
       0,     0,     0,   104,     0,     0,     0,     0,   107,   238,
       0,     0,     0,     0,   111,   358,     0,     0,   359,     0,
       0,    93,    94,     0,    95,   183,    97,     0,     0,     0,
     452,   453,   454,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   107,
     455,   456,   978,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,     0,   480,   452,   453,
     454,     0,     0,     0,     0,     0,     0,     0,     0,   481,
       0,     0,     0,     0,     0,     0,     0,     0,   455,   456,
       0,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,     0,   480,  1096,  1097,  1098,     0,
       0,     0,     0,     0,     0,     0,     0,   481,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1099,  1457,     0,
    1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,
    1120,  1121,  1096,  1097,  1098,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1122,     0,     0,     0,     0,
       0,     0,     0,  1099,     0,     0,  1100,  1101,  1102,  1103,
    1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,  1113,
    1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1097,  1098,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1122,     0,     0,     0,     0,     0,     0,  1099,     0,
       0,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,   454,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1122,     0,     0,     0,
       0,   455,   456,     0,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,  1098,   480,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     481,     0,     0,     0,     0,     0,  1099,     0,     0,  1100,
    1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,
    1121,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   455,   456,  1122,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   479,     0,   480,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     456,   481,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,     0,   480,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   481,   457,
     458,   459,   460,   461,   462,   463,   464,   465,   466,   467,
     468,   469,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   479,     0,   480,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   481
};

static const yytype_int16 yycheck[] =
{
       5,     6,   161,     8,     9,    10,    11,    12,    13,   129,
      15,    16,    17,    18,     4,    56,    21,    22,    23,    24,
      25,    26,    27,    28,    29,   186,    31,    33,   520,   673,
      60,     4,  1173,     4,   405,     4,   752,   405,     4,    44,
      46,     4,   405,   405,   669,    51,   239,    52,   108,    54,
     670,   161,    57,    30,    59,   699,   516,   517,    88,   539,
       4,    91,   166,   954,  1334,    30,    30,   187,    57,    30,
     108,   599,   600,   649,   480,   512,   551,  1160,   943,   250,
     108,    86,   356,   357,   358,   545,   512,    57,   824,   831,
     108,   800,     9,   974,    98,     9,    44,     9,   102,   103,
    1051,     9,    14,   108,     9,    56,    32,    48,    32,   990,
     547,     9,   587,     9,     9,     9,     9,     9,    14,    14,
       9,   547,    36,     9,   128,     9,     9,     9,     9,     9,
      48,     9,    48,     9,    70,    70,     9,    70,    86,    48,
      83,    70,     9,     9,     9,     9,   184,   251,    90,     9,
      50,    51,   106,   107,   134,   135,   184,  1038,   160,    90,
      83,    48,   160,    70,    83,   115,   184,    38,   102,    70,
     102,   196,   102,     4,   102,   121,   544,   215,   166,   184,
     181,  1774,   160,   181,   130,   129,   191,   215,   161,   196,
      81,   179,    50,    51,   196,   196,   671,   215,    19,    20,
     238,   199,    38,   181,    38,   134,   135,    70,    38,    38,
     215,   199,    83,    70,    70,   157,   196,   160,    70,   390,
     238,    70,    53,    70,   160,    56,   157,  1092,   178,    70,
     164,    70,   164,   238,   164,    70,   164,    70,  1831,   130,
     884,   196,    73,   187,    70,    57,   200,    83,   253,    83,
     111,   256,   193,    83,    83,   174,   197,    69,   263,   264,
     196,   196,    54,   199,   199,    96,   199,    98,    70,   181,
     199,   102,   103,   199,   198,    70,    70,   193,   201,   197,
     197,   442,   182,   197,   993,  1468,   198,  1370,   197,   197,
    1560,   257,   199,   198,  1377,   261,  1379,   128,   199,  1250,
     198,   199,   198,   198,   198,   198,   198,   348,   160,   198,
     197,   790,   198,   160,   198,   198,   198,   198,   198,  1061,
     198,  1063,   198,  1204,   197,  1408,   356,   357,   358,   359,
     197,   197,   197,   197,    56,     0,   199,   197,   174,   377,
     174,   816,   199,   199,   174,   174,   821,   199,    83,   377,
     199,   971,   199,   881,   882,   196,   932,   196,   199,   377,
     199,   522,    70,  1228,   199,   160,   199,   397,    83,   200,
     196,   376,   377,   165,   494,    90,     8,   160,   383,   384,
     385,   386,   387,   388,   160,   166,  1569,    70,   393,   106,
     107,    38,   433,    83,   196,   790,    14,   348,   105,   106,
     107,   196,   196,   197,   199,   181,   196,   412,    83,    84,
    1593,   232,  1595,    54,    32,   420,   199,   909,   199,   196,
     196,   411,    83,   199,   235,   196,   257,   432,   376,    83,
     261,  1514,   436,    51,   265,   495,    83,   385,   386,   174,
     388,   199,  1158,   158,   159,   196,   419,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   419,   481,   196,   483,   484,
     485,    70,   433,   200,   174,   489,   490,   491,   492,   204,
     495,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   480,   196,   676,   495,   678,   513,   514,
    1246,   516,   517,   518,   519,   480,   480,   348,   523,   480,
     174,   526,    70,   960,   165,   495,   201,   987,   687,  1008,
     535,   192,   537,    14,   960,   486,   181,   196,   199,   180,
     545,   386,     4,   388,    31,   366,   196,  1172,   553,   493,
     555,   196,    83,   746,   375,    70,   377,    19,    20,    90,
     282,   382,   284,    50,   553,   595,    53,   544,  1188,   599,
     600,  1191,   943,   394,    83,   943,   181,   687,    83,    84,
     943,   943,   533,  1058,  1449,   778,   134,   135,   419,   995,
      83,   196,   558,   754,   120,    31,   181,    90,   429,   873,
     605,   875,   433,    83,   130,   436,  1134,   134,   135,    83,
      90,   196,   160,  1008,    50,    83,    90,    53,   340,   657,
     196,   102,   103,   794,   795,   486,   111,   158,   159,    70,
     801,   802,   181,    70,   119,   120,   121,   122,   123,   124,
     119,   120,   121,   122,   123,   124,   108,   196,   196,   158,
     159,   512,   657,    75,    76,   486,   487,   488,   489,   490,
     491,   492,  1378,   856,   157,   158,   159,  1532,  1169,    70,
      70,  1941,   533,   200,   205,   868,   165,   157,   158,   159,
     670,   512,   196,   544,   158,   159,   547,    32,    83,   157,
     158,   159,   697,   164,   199,    90,   201,   801,  1089,   520,
    1091,   705,   533,   134,   135,   710,   191,  1223,   430,  1225,
      83,   433,   191,   196,   687,   164,   547,    90,   529,   198,
      38,  1092,   184,   198,  1092,   484,  1206,   558,   198,  1092,
    1092,   198,  1357,   134,   135,    75,    76,  1197,   198,   744,
     198,  1220,  1221,  1222,  1223,  1224,  1225,   578,  1208,   198,
     199,  1230,  1372,   215,   513,   198,   160,  1249,   198,   518,
     164,   102,   103,   158,   159,   132,   133,  1483,   773,   198,
     232,    70,   603,   604,   198,   199,   238,   181,  1919,    50,
      51,    52,    53,    54,    55,   158,   159,   198,   199,    53,
      54,    55,  1933,    57,   199,   257,    70,   391,    69,   261,
      70,   395,    70,   808,    70,    69,   160,   638,   639,   356,
     357,    53,    54,    55,   820,   105,   106,   107,    70,   824,
     631,   632,   122,   123,   124,  1220,  1221,    69,   422,  1224,
     424,   425,   426,   427,    70,  1230,   112,   113,   114,  1805,
    1806,   814,    70,   873,   196,   875,  1338,   196,   659,  1324,
     572,   881,   882,   160,  1570,  1801,  1802,  1228,   841,   842,
    1228,   196,   164,   829,   198,  1228,  1228,  1368,    49,    69,
     814,   181,   160,   196,   705,   696,   196,   203,     9,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,   196,   160,     8,  1515,    14,   870,   119,   120,
     121,   122,   123,   124,   366,   160,   198,  1386,   913,  1388,
     915,   196,   917,   375,  1374,   377,   160,   160,   198,   198,
     382,   199,     9,   928,    59,    60,   870,   130,    14,   198,
     181,   742,   394,   130,   755,  1410,   197,   942,    14,   102,
     197,   931,   202,   197,   666,   667,   111,   197,   197,   196,
     196,  1426,     9,   675,    94,   196,   157,   419,   931,   790,
     931,   792,   931,   968,   954,   931,   197,   197,   931,     9,
     191,   197,   197,   978,   198,   796,   981,   798,   983,   952,
      14,   971,   987,   814,   196,   181,  1179,   931,     9,   196,
     994,  1386,   198,  1388,  1885,   946,   198,   828,   829,   134,
     135,   199,  1503,   198,  1505,   826,   199,   197,   952,    83,
     197,   197,   132,   198,  1905,   197,   196,     9,   995,   203,
       9,    70,   833,  1914,   203,   203,   203,   203,   839,    32,
     995,   995,  1037,   133,   995,   180,   160,   136,  1044,   870,
       9,  1501,  1521,   197,  1523,   160,  1525,    14,   879,   880,
     193,  1530,   119,   120,   121,   122,   123,   124,   520,     9,
    1535,     9,   197,   130,   131,   182,     9,   197,    14,  1544,
     132,   203,   893,   200,  1047,   906,  1049,   203,  1449,  1045,
       9,  1449,    14,  1558,   203,   946,  1449,  1449,   909,   910,
     197,  1151,   197,   203,   905,   196,   558,   958,   102,   960,
     931,   160,   197,  1047,  1134,  1049,   173,   198,   198,     9,
     136,     9,  1613,   160,   197,   946,  1617,   196,    70,   841,
     842,   952,   199,    70,   191,    70,  1521,   958,  1523,   960,
    1525,    70,    70,   196,     9,  1530,   200,    50,    51,    52,
      53,    54,    55,  1094,    57,  1624,  1151,    14,   198,   182,
       9,   199,    14,    14,   199,   197,    69,   203,   193,  1634,
     198,  1532,  1151,   994,  1532,   196,    32,  1157,   196,  1532,
    1532,    14,    32,   196,    31,  1006,  1007,  1008,  1039,  1184,
     196,  1825,    14,    52,  1157,   196,  1157,    70,  1157,    70,
      70,  1157,  1197,    70,  1157,   196,    70,   160,  1188,     9,
     197,  1191,   198,  1208,  1209,   198,   196,    14,  1039,   136,
     182,   933,   136,  1157,  1045,     9,  1047,   197,  1049,   160,
      69,  1032,   203,     9,    81,    83,   200,   949,   198,  1624,
    1051,  1052,  1238,     9,   696,   200,   196,   136,   196,  1070,
     962,  1246,   198,    14,    83,   199,   103,   198,   197,   136,
     203,  1256,   196,     9,   196,    91,   157,   197,    32,  1903,
      77,   197,   199,  1094,   199,   199,   182,  1078,   125,   991,
     198,   136,  1773,   198,  1085,  1241,  1777,    32,   197,   197,
    1253,   138,   139,   140,   141,   142,   143,   144,   203,     9,
       9,   203,  1123,   755,   203,  1939,  1771,   203,   203,   136,
    1779,  1780,     9,   197,   161,   200,  1781,   164,   165,  1253,
     167,   168,     9,   170,   171,   172,   198,    83,   197,    14,
     198,     9,   136,   196,   200,  1484,  1157,   199,   198,   197,
       9,   136,   203,   197,   796,     9,   798,  1342,   195,     6,
     203,   197,   197,  1065,  1349,   199,  1336,  1069,  1353,   196,
    1355,  1826,   814,   197,    32,   203,  1346,   203,  1363,  1479,
     198,   203,  1183,  1336,   826,   198,   197,   829,  1373,  1374,
     197,   197,   136,  1346,   198,   112,   199,   169,   198,    14,
     165,    48,  1372,    83,  1779,  1780,   117,   199,   197,  1220,
    1221,  1222,  1223,  1224,  1225,   197,  1871,   136,   197,  1230,
     136,    14,   181,   199,    14,   198,    83,  1218,   870,    14,
    1241,    83,  1913,  1234,   197,   196,   198,   197,   136,   136,
     198,    14,  1253,    14,    14,   198,     9,  1928,  1249,  1250,
    1591,   893,  1263,     9,   199,    68,    83,   200,   181,   196,
      83,     9,   115,   102,   198,   112,   199,   909,   910,  1171,
     117,  1173,   119,   120,   121,   122,   123,   124,   125,   160,
      19,    20,   102,  1938,   172,    36,  1277,   182,  1943,   931,
    1281,    30,    14,   196,   198,  1286,   197,   182,  1200,   178,
     196,  1203,  1293,    83,   182,   175,  1459,   197,     9,    83,
     952,   197,    56,  1498,   198,   197,  1501,    56,    14,  1330,
     167,   168,   195,   170,    83,   199,     9,    14,    83,    14,
      83,  1484,    14,    83,  1504,  1459,  1894,  1338,  1134,   492,
    1510,   487,  1512,   988,   191,  1515,   934,   489,  1910,  1247,
    1633,  1504,  1905,   200,  1478,  1423,   607,  1510,  1620,  1512,
    1262,  1658,  1571,  1743,  1534,  1950,  1268,  1474,  1755,    78,
      79,    80,  1926,  1616,  1090,  1386,  1224,  1388,   387,  1563,
    1470,  1534,    91,  1536,  1161,  1086,     6,  1219,  1007,  1220,
    1034,   958,  1545,  1937,  1860,  1541,   841,   383,  1142,  1071,
     433,  1123,  1393,  1045,    -1,  1047,  1397,  1049,  1462,  1051,
    1052,  1952,  1536,  1404,    -1,    -1,    -1,    -1,  1759,    -1,
      -1,  1545,    -1,  1608,    -1,    -1,    -1,    -1,    48,  1331,
    1332,    -1,    -1,    -1,    -1,  1566,   145,   146,   147,   148,
     149,    -1,    -1,    -1,    -1,    -1,    -1,   156,  1459,    -1,
      -1,    -1,  1622,   162,   163,    -1,    -1,  1468,    -1,    -1,
      -1,    -1,    -1,  1474,    -1,    -1,    -1,   176,    -1,  1622,
      -1,    -1,    -1,    -1,    -1,  1599,    -1,    -1,  1631,    -1,
      -1,   190,    -1,    -1,  1637,    -1,  1632,  1633,    -1,    -1,
      -1,  1644,   112,   232,    -1,    -1,    -1,   117,    -1,   119,
     120,   121,   122,   123,   124,   125,    -1,  1631,    -1,    -1,
    1521,    -1,  1523,  1637,  1525,  1157,    -1,    -1,    -1,  1530,
    1644,    -1,    -1,    -1,    -1,  1536,    -1,    -1,    -1,  1431,
    1541,  1433,    -1,  1754,  1545,    -1,    -1,    -1,   282,  1878,
     284,  1183,    -1,   282,    -1,   284,    -1,   167,   168,    -1,
     170,    -1,  1563,    -1,    -1,  1566,    -1,    -1,  1569,    -1,
      -1,    -1,   119,   120,   121,   122,   123,   124,  1579,    -1,
      -1,   191,    -1,   130,   131,  1586,    -1,  1479,    -1,  1764,
     200,    -1,  1593,    -1,  1595,    -1,    -1,    -1,    -1,    -1,
    1601,    -1,  1234,    -1,    -1,    -1,   340,    -1,  1898,  1241,
      -1,   340,    -1,    -1,    -1,    -1,    -1,  1249,  1250,    -1,
      -1,  1253,    -1,  1624,   171,     6,   173,    -1,    -1,    -1,
    1631,  1632,  1633,  1754,    -1,    -1,  1637,   366,    -1,   186,
      -1,   188,  1785,  1644,   191,  1819,   375,    -1,    -1,    -1,
      -1,    -1,    -1,   382,  1546,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   394,    -1,    48,    -1,    -1,
      -1,  1785,    -1,    -1,    -1,    -1,   405,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1827,    -1,    -1,    -1,    -1,    -1,
      -1,  1834,    -1,    -1,    -1,    -1,   430,    -1,    -1,   433,
      -1,   430,    -1,    -1,   433,    -1,  1338,    -1,    -1,    -1,
      -1,    -1,    -1,  1827,    -1,    -1,    -1,    -1,    -1,    -1,
    1834,    -1,    -1,    -1,    -1,  1885,  1869,    -1,    -1,    -1,
      -1,   112,    -1,    -1,    -1,  1878,   117,    -1,   119,   120,
     121,   122,   123,   124,   125,  1905,     6,    -1,  1891,    -1,
      -1,   480,    -1,  1754,  1914,  1869,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1656,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1948,  1775,    -1,    -1,  1891,  1779,  1780,
      -1,    19,    20,    -1,  1785,  1960,   167,   168,    48,   170,
      -1,   520,    -1,  1794,    -1,  1970,    -1,    -1,  1973,    -1,
    1801,  1802,    -1,    -1,  1805,  1806,    -1,    -1,  1951,    -1,
     191,    -1,    -1,  1956,  1795,  1796,    -1,    -1,  1819,   200,
      -1,    -1,    -1,    -1,    -1,    -1,  1827,  1459,    -1,    -1,
      -1,    -1,    -1,  1834,    -1,    -1,    -1,  1951,   572,    -1,
     574,    -1,  1956,   572,    -1,   574,     6,    -1,   577,    -1,
      -1,    -1,   112,    -1,    -1,    -1,    -1,   117,    -1,   119,
     120,   121,   122,   123,   124,   125,  1758,    -1,  1869,    -1,
      -1,    -1,    -1,    -1,     6,    -1,  1877,    -1,    -1,    -1,
     609,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,
    1891,    -1,    -1,    -1,    -1,    -1,  1897,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1536,    -1,    -1,   167,   168,  1541,
     170,    -1,    -1,  1545,    19,    20,    48,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,   191,   666,   667,    -1,    -1,    -1,   666,   667,    -1,
     200,   675,    -1,    69,    -1,    -1,   675,    -1,    -1,    -1,
    1951,  1843,   112,    -1,    -1,  1956,    -1,   117,    -1,   119,
     120,   121,   122,   123,   124,   125,    -1,   696,    -1,     6,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     112,    -1,    -1,    -1,    -1,   117,    -1,   119,   120,   121,
     122,   123,   124,   125,   232,    -1,    -1,    -1,    -1,  1631,
    1632,  1633,    -1,    -1,    -1,  1637,    -1,   167,   168,    -1,
     170,    48,  1644,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   755,  1919,    -1,    -1,
      -1,   191,    -1,    -1,    -1,   167,   168,    -1,   170,  1931,
     200,  1933,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,   191,
    1952,   790,  1954,    -1,    -1,    -1,    -1,   796,   200,   798,
      -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,
     117,    -1,   119,   120,   121,   122,   123,   124,   125,    -1,
      -1,    59,    60,    -1,    -1,    -1,    -1,   826,   827,    81,
     834,    83,    84,    -1,    -1,   834,    -1,   841,   842,    -1,
      -1,    -1,   841,   842,   843,   844,   845,   846,   847,    -1,
      -1,   103,    -1,    -1,    -1,    -1,   855,   232,   366,    -1,
     167,   168,    -1,   170,    -1,    -1,    -1,   375,    -1,    -1,
      -1,    -1,   871,    -1,   382,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1785,   191,    -1,   394,   139,   140,   141,
     142,   143,    -1,   200,   893,    -1,   134,   135,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   907,    -1,
     909,   910,    -1,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,    -1,    -1,    -1,  1827,    -1,    -1,    -1,   933,
      -1,    -1,  1834,    -1,   933,   934,    -1,    19,    20,    -1,
      -1,    -1,    -1,   195,   943,   949,    -1,   199,    30,   201,
     949,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   962,   197,
      -1,    -1,    -1,   962,    -1,    -1,    -1,  1869,    -1,    -1,
      -1,   970,    -1,    -1,   973,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   991,    -1,  1891,
      -1,   366,   991,    -1,    -1,    -1,   995,    -1,    -1,    -1,
     375,    -1,    -1,    -1,    -1,    -1,    -1,   382,    -1,  1008,
      -1,    -1,   520,    -1,    -1,    -1,    -1,    -1,    -1,   394,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,
     405,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,  1951,
      -1,    -1,  1051,  1052,  1956,    -1,    -1,    -1,    -1,    -1,
      -1,  1065,    -1,    -1,    -1,  1069,  1065,  1071,    -1,    -1,
    1069,    -1,  1071,    81,    -1,    -1,    59,    60,    -1,    -1,
      -1,    -1,    -1,    91,    -1,    -1,    -1,  1086,  1087,  1088,
    1089,  1090,  1091,  1092,    -1,   103,  1095,  1096,  1097,  1098,
    1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,  1122,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,  1138,
      -1,    -1,    -1,    -1,    -1,   520,    -1,    -1,    -1,    -1,
     232,   134,   135,   161,    -1,    -1,   164,    -1,    -1,   167,
     168,    -1,   170,   171,   172,    -1,   174,  1171,    -1,  1173,
      -1,    -1,  1171,    -1,  1173,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1183,    -1,    -1,   195,   696,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1200,    -1,    -1,  1203,
      -1,  1200,    -1,    -1,  1203,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   197,    -1,    -1,    81,    -1,    83,
      84,  1220,  1221,  1222,  1223,  1224,  1225,    -1,    -1,  1228,
      -1,  1230,    -1,    -1,    -1,  1234,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   755,    -1,    -1,
    1249,  1250,    -1,  1252,    -1,    -1,    -1,    -1,  1262,    -1,
      -1,    -1,    -1,  1262,  1268,    -1,    -1,    -1,    -1,  1268,
      -1,    -1,  1271,    -1,  1273,   139,   140,   141,   142,   143,
      -1,    -1,    -1,    -1,   366,    -1,    -1,    -1,   796,    -1,
     798,    -1,    -1,   375,    -1,    -1,    -1,    -1,  1297,    -1,
     382,   165,    -1,   167,   168,    -1,   170,   171,   172,    -1,
      -1,    -1,   394,    -1,    -1,    -1,    -1,    -1,   826,    -1,
      -1,   696,    -1,   405,    10,    11,    12,  1331,  1332,    -1,
      -1,   195,  1331,  1332,    -1,   199,  1335,   201,    -1,  1338,
      -1,    -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,
     755,    -1,    -1,    69,    -1,   893,    -1,  1386,    -1,  1388,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   480,    -1,
      -1,   909,   910,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   796,    -1,   798,    81,    -1,    -1,  1431,    -1,  1433,
      -1,    -1,  1431,    -1,  1433,    -1,    -1,    -1,   520,    -1,
    1439,    -1,  1441,    -1,  1443,    -1,   103,    -1,    -1,  1448,
    1449,   826,    -1,  1452,   111,  1454,    -1,    -1,  1457,    -1,
      -1,    -1,    -1,    -1,  1468,    -1,    -1,    -1,    -1,  1468,
    1469,    -1,    -1,  1472,    -1,  1479,    -1,    -1,    -1,    -1,
    1479,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   577,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,   203,   893,    -1,
      -1,    -1,  1521,    -1,  1523,    -1,  1525,   609,   185,    -1,
      -1,  1530,    -1,  1532,   909,   910,    68,    -1,   195,   196,
      -1,    -1,  1546,  1051,  1052,    -1,    -1,  1546,    -1,    81,
    1549,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1561,  1562,    -1,  1569,    -1,    -1,   943,    -1,
    1569,   103,  1571,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,  1593,
      -1,  1595,    -1,    -1,  1593,    -1,  1595,  1601,    -1,    69,
      -1,    -1,  1601,    -1,    -1,    -1,   138,   139,   140,   141,
     142,   143,   144,    -1,   696,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1624,    -1,    -1,    -1,   161,
      -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,    -1,  1641,  1642,  1643,    -1,    -1,    -1,    -1,  1648,
      -1,  1650,  1656,   185,    -1,    -1,    -1,  1656,    -1,  1658,
      10,    11,    12,   195,   196,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   755,    -1,  1183,  1051,  1052,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,   790,    -1,
      -1,    -1,    -1,    -1,   796,    -1,   798,  1092,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,  1234,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1249,  1250,    -1,   826,   827,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1758,    -1,    -1,    -1,    -1,  1758,
      31,   843,   844,   845,   846,   847,    -1,    -1,    -1,    -1,
      -1,  1775,    -1,   855,    -1,    -1,  1775,    -1,    -1,    -1,
    1779,  1780,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   871,
    1794,    -1,    -1,    -1,    -1,  1794,    -1,    -1,    19,    20,
      -1,  1800,    -1,    -1,    -1,    -1,    -1,    -1,  1183,    30,
      81,   893,  1811,    -1,    -1,    -1,    -1,    -1,  1817,    81,
      91,    -1,  1821,    -1,    -1,   907,    -1,   909,   910,    -1,
    1338,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,  1843,
      -1,   103,    -1,    -1,  1843,    -1,    -1,    -1,    -1,    -1,
     200,    -1,   934,  1228,    -1,    -1,    -1,    -1,    -1,  1234,
      -1,   943,    -1,    -1,    -1,    -1,    -1,    -1,   139,   140,
     141,   142,   143,    -1,  1249,  1250,    -1,   139,   140,   141,
     142,   143,    -1,    -1,  1883,    81,    -1,    -1,   970,    -1,
     161,   973,    -1,   164,  1893,    -1,   167,   168,    -1,   170,
     171,   172,   164,   174,    -1,   167,   168,   103,   170,   171,
     172,  1910,    -1,   995,    -1,  1919,    -1,    -1,    -1,    -1,
    1919,    -1,    -1,    -1,   195,    -1,  1008,  1931,    -1,  1933,
      -1,    -1,  1931,   195,  1933,    -1,    -1,   199,    -1,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,  1952,    -1,
    1954,    -1,    -1,  1952,    -1,  1954,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1338,    -1,    -1,    -1,    -1,    -1,  1051,
    1052,   167,   168,    -1,   170,   171,   172,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,   195,
     196,    -1,    -1,    -1,  1086,  1087,  1088,  1089,  1090,  1091,
    1092,   232,    69,  1095,  1096,  1097,  1098,  1099,  1100,  1101,
    1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,
    1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,  1138,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    30,    31,  1449,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,  1183,    59,    60,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    69,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    -1,    -1,  1220,  1221,
    1222,  1223,  1224,  1225,    -1,   366,  1228,    69,  1230,    -1,
      -1,    -1,  1234,    -1,   375,    -1,    -1,  1532,    -1,    -1,
      -1,   382,    -1,    -1,    -1,    -1,    -1,  1249,  1250,    -1,
    1252,    -1,    -1,   394,    -1,    -1,    -1,   134,   135,    -1,
      -1,    -1,    -1,    -1,   405,    -1,    -1,    -1,    -1,  1271,
      -1,  1273,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    31,  1297,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,   200,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    69,  1335,    -1,    -1,  1338,    -1,    -1,   480,
      -1,    -1,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,   520,
      -1,    -1,    69,    -1,  1386,    -1,  1388,    -1,    -1,    -1,
      -1,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,   577,  1439,    -1,  1441,
      -1,  1443,    -1,    -1,    -1,    -1,  1448,  1449,    -1,    -1,
    1452,    -1,  1454,    -1,    -1,  1457,    -1,    -1,    10,    11,
      12,    -1,    -1,   200,    -1,    -1,    -1,  1469,   609,    -1,
    1472,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
     577,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,   609,   200,    -1,    50,    51,    69,    -1,  1521,
      -1,  1523,    -1,  1525,    -1,    -1,    -1,    -1,  1530,    -1,
    1532,    -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,  1549,    -1,    -1,
      10,    11,    12,    -1,    -1,   696,    91,    -1,    -1,  1561,
    1562,   200,    -1,    -1,    -1,    -1,    -1,    -1,   103,  1571,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,    69,
      -1,    -1,    -1,    -1,   755,    -1,    -1,    -1,    -1,    -1,
      -1,   156,  1624,    -1,    -1,    -1,   161,    -1,    -1,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,  1641,
    1642,  1643,    -1,    -1,    -1,   197,  1648,    -1,  1650,    -1,
     185,    -1,    -1,    -1,    -1,   796,  1658,   798,    -1,    -1,
     195,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,   826,   827,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   843,   844,   845,   846,   847,    59,    60,    -1,
      -1,    -1,    -1,    -1,   855,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     827,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
     200,    -1,    -1,    -1,    -1,    -1,   843,   844,   845,   846,
     847,    -1,   893,    -1,    -1,    -1,   103,    -1,   855,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   909,   910,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1779,  1780,    -1,
      -1,    -1,   134,   135,    -1,    81,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,   144,  1800,    -1,
      -1,    -1,   943,    -1,    -1,    -1,    -1,   103,    -1,  1811,
      -1,    -1,    -1,    -1,   161,  1817,    -1,   164,   165,  1821,
     167,   168,    -1,   170,   171,   172,    -1,   174,    -1,   970,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,   195,   196,
      -1,    -1,    -1,    -1,   995,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,   164,    -1,
      -1,   167,   168,   970,   170,   171,   172,    -1,    -1,    -1,
      -1,  1883,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1893,    -1,    10,    11,    12,    -1,    -1,    -1,   195,
      -1,    -1,    -1,    -1,   200,    -1,    -1,    -1,  1910,    -1,
    1051,  1052,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,  1086,  1087,  1088,  1089,  1090,
    1091,  1092,    69,    -1,  1095,  1096,  1097,  1098,  1099,  1100,
    1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,
    1111,  1112,  1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,
    1121,  1122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1086,
    1087,    -1,    -1,  1090,    -1,    -1,    -1,  1138,  1095,  1096,
    1097,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,
    1107,  1108,  1109,  1110,  1111,  1112,  1113,  1114,  1115,  1116,
    1117,  1118,  1119,  1120,  1121,  1122,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1138,  1183,    -1,    -1,    -1,    30,    31,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   200,    -1,    69,    -1,  1228,    -1,    -1,
      -1,    -1,    -1,  1234,    -1,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1249,  1250,
      -1,  1252,    -1,    -1,     3,     4,    -1,     6,     7,    -1,
      -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
    1271,    -1,  1273,   139,   140,   141,   142,   143,    -1,    28,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,  1252,  1297,    -1,    -1,   165,
      -1,   167,   168,   169,   170,   171,   172,    -1,    57,    -1,
      -1,    -1,    -1,    -1,  1271,    -1,  1273,    -1,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,   195,
     196,    -1,    81,    -1,    83,    84,    -1,  1338,    -1,    -1,
    1297,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,   128,
      -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,   138,
     139,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   161,    -1,    -1,    -1,    -1,    -1,   167,   168,
      -1,   170,   171,   172,   173,    -1,   175,    -1,    -1,   178,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,  1439,    -1,
    1441,    -1,  1443,    -1,    -1,    -1,   195,  1448,  1449,    -1,
     199,  1452,   201,  1454,    30,    31,  1457,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      81,    57,  1439,    -1,  1441,    -1,  1443,    -1,    -1,    -1,
      -1,  1448,    -1,    69,    -1,  1452,    -1,  1454,    -1,    -1,
    1457,    -1,   103,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,   128,    -1,    -1,
      -1,  1532,    -1,    -1,    -1,    -1,    -1,    69,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,  1549,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,     7,    -1,    -1,    -1,   167,   168,    13,   170,
     171,   172,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1549,    38,   195,   196,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,   200,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
    1641,  1642,  1643,    -1,    -1,    -1,    91,  1648,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1657,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,   127,   128,  1641,  1642,  1643,    -1,    -1,    -1,
      -1,  1648,   137,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,   176,    81,    -1,   179,    -1,    -1,    -1,    -1,    -1,
     185,    10,    11,    12,    -1,   190,   191,   192,    -1,    -1,
     195,   196,    -1,    -1,   103,    -1,   201,   202,    -1,   204,
     205,    30,    31,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
     139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,  1800,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1811,    -1,    -1,    -1,    -1,    -1,  1817,    -1,   167,   168,
    1821,   170,   171,   172,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,  1800,  1845,    -1,   195,   196,    -1,    -1,
      -1,    -1,    -1,    -1,  1811,    -1,    27,    28,    29,    -1,
    1817,    -1,    -1,    -1,  1821,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    -1,  1883,    -1,    -1,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    87,    -1,    -1,    -1,
      91,    92,    93,    94,    -1,    96,  1883,    98,    -1,   100,
      -1,   200,   103,   104,    -1,    -1,    -1,   108,   109,   110,
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
      92,    93,    94,    -1,    96,    97,    98,    -1,   100,    -1,
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
      -1,    -1,    -1,   119,   120,   121,   122,   123,   124,    -1,
      -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   137,   138,   139,   140,   141,   142,   143,    -1,   145,
     146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,
     156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
     176,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,
      -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,
     196,    -1,   198,    -1,    -1,   201,   202,    -1,   204,   205,
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
      -1,   174,    -1,   176,    -1,    -1,   179,    -1,    -1,    -1,
      -1,    -1,   185,    -1,    -1,    -1,    -1,   190,   191,   192,
      -1,    -1,   195,   196,    -1,    -1,    -1,    -1,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,
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
     190,   191,   192,    -1,    -1,   195,   196,    -1,    -1,   199,
      -1,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
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
      -1,    -1,   119,   120,   121,   122,   123,   124,    -1,    -1,
     127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,   174,    -1,   176,
      -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,   185,    -1,
      -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,
      -1,    -1,    -1,    -1,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   103,    -1,    -1,    -1,    -1,   108,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,   120,
     121,   122,   123,   124,    -1,    -1,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    -1,    -1,   176,    -1,    -1,   179,    -1,
      -1,    -1,    -1,   200,   185,    -1,    -1,    -1,    -1,   190,
     191,   192,    -1,    -1,   195,   196,    -1,    -1,    -1,    -1,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
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
      -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,    10,
      11,    12,    -1,   201,   202,    -1,   204,   205,     3,     4,
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
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,   176,    -1,    -1,   179,    -1,    -1,   198,    -1,    -1,
     185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,
     195,   196,    -1,   198,    11,    12,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    30,    31,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,
      -1,    -1,    69,    -1,    56,    -1,    58,    59,    60,    61,
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
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   190,   191,   192,    -1,    -1,   195,   196,    10,    11,
      12,    -1,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    51,    -1,    69,    -1,    -1,
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
     176,    -1,    -1,   179,    -1,    -1,   198,    -1,    -1,   185,
      -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,   195,
     196,   197,    -1,    -1,    -1,   201,   202,    -1,   204,   205,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    32,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    50,    51,    -1,
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
      -1,    -1,    -1,    13,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    -1,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
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
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    50,    51,    -1,    -1,
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
      -1,   195,   196,    -1,    -1,    -1,    -1,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    50,
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
      -1,   179,    -1,    -1,   198,    -1,    -1,   185,    -1,    -1,
      -1,    -1,   190,   191,   192,    -1,    -1,   195,   196,    10,
      11,    12,    -1,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    27,    -1,    13,    30,
      31,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    -1,    69,    -1,
      -1,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    78,    79,    80,    81,    82,    83,    84,
      -1,   102,    -1,    -1,    -1,    -1,    91,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   137,   138,   139,   140,   141,   142,   143,    -1,
     145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,    -1,
      -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,     3,
       4,   176,     6,     7,   179,    -1,    10,    11,    12,    13,
     185,    -1,    -1,    -1,    -1,   190,   191,   192,    -1,    -1,
     195,   196,    -1,    -1,    28,    29,   201,   202,    -1,   204,
     205,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    83,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    81,   128,    -1,   130,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,     6,     7,    -1,   161,    10,    11,
      12,    13,    -1,   167,   168,    -1,   170,   171,   172,   173,
      -1,   175,    -1,    -1,   178,    -1,    28,    29,    -1,    31,
      -1,   139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,
      -1,   195,    -1,    -1,    -1,   199,    -1,   201,    -1,    -1,
      -1,    -1,    -1,   161,    -1,    57,   164,   165,    -1,   167,
     168,    -1,   170,   171,   172,    -1,    68,    -1,    -1,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,    -1,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,    -1,   128,    -1,    -1,   131,
     132,   133,    -1,    -1,    -1,   137,   138,   139,   140,   141,
     142,   143,   144,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,
      -1,    -1,   164,   165,    -1,   167,   168,    -1,   170,   171,
     172,   173,   103,   175,    -1,    -1,   178,     3,     4,    -1,
       6,     7,    -1,   185,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,   195,   196,    -1,    -1,    -1,   200,    -1,
      -1,    -1,    28,    29,    -1,    31,    -1,    -1,   139,   140,
     141,   142,   143,    -1,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    57,    -1,    -1,    -1,    -1,   167,   168,    -1,   170,
     171,   172,    68,    -1,    69,    71,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   195,    -1,    92,    93,    94,    95,
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
     130,   131,   132,   133,    -1,    -1,    -1,   137,   138,   139,
     140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    83,    -1,    85,    -1,    -1,    -1,
      -1,   161,    -1,    -1,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,   173,   103,   175,    -1,    -1,   178,     3,
       4,    -1,     6,     7,    -1,   185,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,   195,   196,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    29,    -1,    31,    -1,    -1,
     139,   140,   141,   142,   143,    -1,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    57,    -1,    -1,    -1,    -1,   167,   168,
      -1,   170,   171,   172,    68,    -1,    69,    71,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,    -1,   128,    -1,    -1,   131,   132,   133,
      -1,    -1,    -1,   137,   138,   139,   140,   141,   142,   143,
     144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   161,    -1,    -1,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,   173,
     103,   175,    -1,    -1,   178,    -1,     3,     4,    -1,     6,
       7,   185,   186,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,   195,   196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    29,    -1,    31,    -1,   139,   140,   141,   142,
     143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    -1,   165,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    68,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   195,    -1,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,    -1,    -1,   131,   132,   133,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   161,    -1,    -1,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,   173,   103,   175,    -1,
      -1,   178,     3,     4,     5,     6,     7,    -1,   185,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,   195,   196,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,   164,    -1,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     161,   162,   163,    -1,    81,    -1,   167,   168,    -1,   170,
     171,   172,   173,    -1,   175,   176,    -1,   178,    -1,    -1,
      -1,    -1,    -1,    -1,   185,   186,   103,   188,    -1,   190,
     191,    -1,     3,     4,   195,     6,     7,    -1,    -1,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,   125,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    29,    -1,
      -1,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   161,    -1,    57,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    81,   128,    -1,   130,
     131,   132,   133,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,   144,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,    -1,
     161,    10,    11,    12,    13,    -1,   167,   168,    -1,   170,
     171,   172,   173,    -1,   175,    -1,    -1,   178,    -1,    28,
      29,    -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,
      -1,    -1,    -1,    -1,   195,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,   164,
      -1,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     195,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    -1,    -1,    -1,   167,   168,    -1,   170,   171,   172,
      -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   195,    -1,    -1,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      -1,   128,    -1,    -1,   131,   132,   133,    -1,    -1,    -1,
     137,   138,   139,   140,   141,   142,   143,   144,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   161,    -1,    -1,    -1,    -1,    -1,
     167,   168,    -1,   170,   171,   172,   173,    -1,   175,    -1,
      -1,   178,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,
      -1,    -1,    30,    31,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
     198,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,   198,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,
      -1,    -1,    -1,    31,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   138,   139,   140,   141,   142,   143,   144,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    81,    -1,   197,   161,    -1,    -1,   164,
     165,    -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,
     185,    -1,    32,    -1,    -1,    -1,   191,    -1,    -1,    -1,
     195,   196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    51,    -1,   197,    -1,    -1,    56,    -1,    58,    -1,
     138,   139,   140,   141,   142,   143,   144,    -1,    -1,    -1,
      70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    79,
      80,    81,    -1,   161,    -1,    -1,   164,   165,    -1,   167,
     168,    91,   170,   171,   172,    -1,   174,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,   185,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,   196,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,   139,
     140,   141,   142,   143,   103,   145,   146,   147,   148,   149,
      -1,    -1,   111,   112,    -1,    -1,   156,    38,    -1,    -1,
      -1,   161,   162,   163,   164,   165,    -1,   167,   168,    -1,
     170,   171,   172,    -1,    -1,    -1,   176,    -1,    -1,    -1,
     139,   140,   141,   142,   143,   185,    -1,    -1,    -1,    70,
     190,    -1,    -1,    -1,    -1,   195,   196,    78,    79,    80,
      81,    -1,    83,    84,    -1,   164,    -1,    -1,   167,   168,
      91,   170,   171,   172,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,   119,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
      -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,
     161,   162,   163,   164,   165,    -1,   167,   168,    -1,   170,
     171,   172,    -1,    50,    51,   176,    -1,    -1,    -1,    56,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   190,
      -1,    -1,    -1,    70,   195,   196,    -1,    -1,    -1,    -1,
     201,    78,    79,    80,    81,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,    -1,    30,    31,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,   138,   139,   140,   141,   142,   143,    69,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    70,    -1,    -1,   176,
      -1,    -1,    -1,    -1,    78,    79,    80,    81,   185,    83,
      84,    -1,    -1,   190,    -1,    -1,    -1,    91,   195,   196,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   138,   139,   140,   141,   142,   143,
      -1,   145,   146,   147,   148,   149,    -1,    -1,    -1,    -1,
      -1,    -1,   156,    -1,    -1,    -1,    -1,   161,   162,   163,
     164,   165,    -1,   167,   168,    -1,   170,   171,   172,    70,
      -1,    -1,   176,    -1,    -1,    -1,    -1,    78,    79,    80,
      81,    -1,    83,    84,    -1,    -1,   190,    -1,    -1,    -1,
      91,   195,   196,    -1,    -1,   199,    -1,   201,    -1,    -1,
      -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   137,   138,   139,   140,
     141,   142,   143,    -1,   145,   146,   147,   148,   149,    -1,
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
      -1,    -1,    -1,    -1,    31,   136,    -1,    34,    35,    36,
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
      54,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,   136,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,   136,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,    -1,   136,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
      -1,    -1,    -1,    69,    91,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   136,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   130,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,
     136,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,
      -1,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
      -1,    -1,    -1,   190,    91,    -1,    -1,    -1,   195,   196,
      -1,    -1,    -1,    -1,   201,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,    -1,   145,   146,
     147,   148,   149,    -1,    -1,    -1,    -1,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,
      -1,    78,    79,    80,    81,    -1,    83,    84,    -1,    -1,
      -1,    -1,    -1,   190,    91,    -1,    -1,    -1,   195,   196,
      -1,    -1,    -1,    -1,   201,    -1,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,   141,   142,   143,   103,   145,   146,
     147,   148,   149,    -1,    -1,   111,   112,    -1,    -1,   156,
      -1,    -1,    -1,    -1,   161,   162,   163,   164,   165,    -1,
     167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,   176,
      -1,    -1,    -1,   139,   140,   141,   142,   143,    -1,    -1,
      -1,    -1,    -1,   190,    -1,    -1,    -1,    -1,   195,   196,
      -1,    -1,    -1,    -1,   201,   161,    -1,    -1,   164,    -1,
      -1,   167,   168,    -1,   170,   171,   172,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,
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
      -1,    -1,    30,    31,    69,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    69,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69
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
     216,   217,   218,   219,   223,    83,   201,   293,   294,    83,
     295,   121,   130,   120,   130,   196,   196,   196,   196,   213,
     265,   484,   196,   196,    70,    70,    70,    70,    70,   338,
      83,    90,   157,   158,   159,   473,   474,   164,   199,   223,
     223,   213,   266,   484,   165,   196,   484,   484,    83,   192,
     199,   359,    28,   336,   340,   348,   349,   453,   457,   228,
     199,   462,    90,   414,   473,    90,   473,   473,    32,   164,
     181,   485,   196,     9,   198,    38,   245,   165,   264,   484,
     119,   191,   246,   328,   198,   198,   198,   198,   198,   198,
     198,   198,    10,    11,    12,    30,    31,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      57,    69,   198,    70,    70,   199,   160,   131,   171,   173,
     186,   188,   267,   326,   327,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    59,    60,
     134,   135,   443,    70,   199,   448,   196,   196,    70,   199,
     196,   245,   246,    14,   348,   198,   136,    49,   213,   438,
      90,   336,   349,   160,   453,   136,   203,     9,   423,   260,
     336,   349,   453,   485,   160,   196,   415,   443,   448,   197,
     348,    32,   230,     8,   360,     9,   198,   230,   231,   338,
     339,   348,   213,   279,   234,   198,   198,   198,   138,   144,
     505,   505,   181,   504,   196,   111,   505,    14,   160,   138,
     144,   161,   213,   215,   198,   198,   198,   240,   115,   178,
     198,   216,   218,   216,   218,   164,   218,   223,   223,   199,
       9,   424,   198,   102,   164,   199,   453,     9,   198,    14,
       9,   198,   130,   130,   453,   477,   338,   336,   349,   453,
     456,   457,   197,   181,   257,   137,   453,   466,   467,   348,
     368,   369,   338,   389,   389,   368,   389,   198,    70,   443,
     157,   474,    82,   348,   453,    90,   157,   474,   223,   212,
     198,   199,   252,   262,   398,   400,    91,   196,   361,   362,
     364,   407,   411,   459,   461,   478,    14,   102,   479,   355,
     356,   357,   289,   290,   441,   442,   197,   197,   197,   197,
     197,   200,   229,   230,   247,   254,   261,   441,   348,   202,
     204,   205,   213,   486,   487,   505,    38,   174,   291,   292,
     348,   481,   196,   484,   255,   245,   348,   348,   348,   348,
      32,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   348,   348,   348,   412,   348,   348,   463,   463,
     348,   469,   470,   130,   199,   214,   215,   462,   265,   213,
     266,   484,   484,   264,   246,    38,   340,   343,   345,   348,
     348,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   165,   199,   213,   444,   445,   446,   447,   462,
     463,   348,   291,   291,   463,   348,   466,   245,   197,   348,
     196,   437,     9,   423,   197,   197,    38,   348,    38,   348,
     415,   197,   197,   197,   460,   461,   462,   291,   199,   213,
     444,   445,   462,   197,   228,   283,   199,   345,   348,   348,
      94,    32,   230,   277,   198,    27,   102,    14,     9,   197,
      32,   199,   280,   505,    31,    91,   174,   226,   498,   499,
     500,   196,     9,    50,    51,    56,    58,    70,   138,   139,
     140,   141,   142,   143,   185,   196,   224,   375,   378,   381,
     384,   387,   393,   408,   416,   417,   419,   420,   213,   503,
     228,   196,   238,   199,   198,   199,   198,   223,   198,   102,
     164,   111,   112,   219,   220,   221,   222,   219,   213,   348,
     294,   417,    83,     9,   197,   197,   197,   197,   197,   197,
     197,   198,    50,    51,   494,   496,   497,   132,   270,   196,
       9,   197,   197,   136,   203,     9,   423,     9,   423,   203,
     203,   203,   203,    83,    85,   213,   475,   213,    70,   200,
     200,   209,   211,    32,   133,   269,   180,    54,   165,   180,
     402,   349,   136,     9,   423,   197,   160,   505,   505,    14,
     360,   289,   228,   193,     9,   424,   505,   506,   443,   448,
     443,   200,     9,   423,   182,   453,   348,   197,     9,   424,
      14,   352,   248,   132,   268,   196,   484,   348,    32,   203,
     203,   136,   200,     9,   423,   348,   485,   196,   258,   253,
     263,    14,   479,   256,   245,    71,   453,   348,   485,   203,
     200,   197,   197,   203,   200,   197,    50,    51,    70,    78,
      79,    80,    91,   138,   139,   140,   141,   142,   143,   156,
     185,   213,   376,   379,   382,   385,   388,   408,   419,   426,
     428,   429,   433,   436,   213,   453,   453,   136,   268,   443,
     448,   197,   348,   284,    75,    76,   285,   228,   337,   228,
     339,   102,    38,   137,   274,   453,   417,   213,    32,   230,
     278,   198,   281,   198,   281,     9,   423,    91,   226,   136,
     160,     9,   423,   197,   174,   486,   487,   488,   486,   417,
     417,   417,   417,   417,   422,   425,   196,    70,    70,    70,
      70,    70,   196,   417,   160,   199,    10,    11,    12,    31,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    69,   160,   485,   200,   408,   199,   242,   218,
     218,   213,   219,   219,     9,   424,   200,   200,    14,   453,
     198,   182,     9,   423,   213,   271,   408,   199,   466,   137,
     453,    14,   348,   348,   203,   348,   200,   209,   505,   271,
     199,   401,    14,   197,   348,   361,   462,   198,   505,   193,
     200,    32,   492,   442,    38,    83,   174,   444,   445,   447,
     444,   445,   505,    38,   174,   348,   417,   289,   196,   408,
     269,   353,   249,   348,   348,   348,   200,   196,   291,   270,
      32,   269,   505,    14,   268,   484,   412,   200,   196,    14,
      78,    79,    80,   213,   427,   427,   429,   431,   432,    52,
     196,    70,    70,    70,    70,    70,    90,   157,   196,   160,
       9,   423,   197,   437,    38,   348,   269,   200,    75,    76,
     286,   337,   230,   200,   198,    95,   198,   274,   453,   196,
     136,   273,    14,   228,   281,   105,   106,   107,   281,   200,
     505,   182,   136,   160,   505,   213,   174,   498,     9,   197,
     423,   136,   203,     9,   423,   422,   370,   371,   417,   390,
     417,   418,   390,   370,   390,   361,   363,   365,   197,   130,
     214,   417,   471,   472,   417,   417,   417,    32,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   503,    83,   243,   200,   200,   222,   198,   417,
     497,   102,   103,   493,   495,     9,   299,   197,   196,   340,
     345,   348,   136,   203,   200,   479,   299,   166,   179,   199,
     397,   404,   166,   199,   403,   136,   198,   492,   505,   360,
     506,    83,   174,    14,    83,   485,   453,   348,   197,   289,
     199,   289,   196,   136,   196,   291,   197,   199,   505,   199,
     198,   505,   269,   250,   415,   291,   136,   203,     9,   423,
     428,   431,   372,   373,   429,   391,   429,   430,   391,   372,
     391,   157,   361,   434,   435,    81,   429,   453,   199,   337,
      32,    77,   230,   198,   339,   273,   466,   274,   197,   417,
     101,   105,   198,   348,    32,   198,   282,   200,   182,   505,
     213,   136,   174,    32,   197,   417,   417,   197,   203,     9,
     423,   136,   203,     9,   423,   203,   203,   203,   136,     9,
     423,   197,   136,   200,     9,   423,   417,    32,   197,   228,
     198,   198,   213,   505,   505,   493,   408,     6,   112,   117,
     120,   125,   167,   168,   170,   200,   300,   325,   326,   327,
     332,   333,   334,   335,   441,   466,   348,   200,   199,   200,
      54,   348,   348,   348,   360,    38,    83,   174,    14,    83,
     348,   196,   492,   197,   299,   197,   289,   348,   291,   197,
     299,   479,   299,   198,   199,   196,   197,   429,   429,   197,
     203,     9,   423,   136,   203,     9,   423,   203,   203,   203,
     136,   197,     9,   423,   299,    32,   228,   198,   197,   197,
     197,   235,   198,   198,   282,   228,   136,   505,   505,   136,
     417,   417,   417,   417,   361,   417,   417,   417,   199,   200,
     495,   132,   133,   186,   214,   482,   505,   272,   408,   112,
     335,    31,   125,   138,   144,   165,   171,   309,   310,   311,
     312,   408,   169,   317,   318,   128,   196,   213,   319,   320,
     301,   246,   505,     9,   198,     9,   198,   198,   479,   326,
     197,   296,   165,   399,   200,   200,    83,   174,    14,    83,
     348,   291,   117,   350,   492,   200,   492,   197,   197,   200,
     199,   200,   299,   289,   136,   429,   429,   429,   429,   361,
     200,   228,   233,   236,    32,   230,   276,   228,   505,   197,
     417,   136,   136,   136,   228,   408,   408,   484,    14,   214,
       9,   198,   199,   482,   479,   312,   181,   199,     9,   198,
       3,     4,     5,     6,     7,    10,    11,    12,    13,    27,
      28,    29,    57,    71,    72,    73,    74,    75,    76,    77,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   137,   138,   145,   146,   147,   148,   149,   161,   162,
     163,   173,   175,   176,   178,   185,   186,   188,   190,   191,
     213,   405,   406,     9,   198,   165,   169,   213,   320,   321,
     322,   198,    83,   331,   245,   302,   482,   482,    14,   246,
     200,   297,   298,   482,    14,    83,   348,   197,   196,   492,
     198,   199,   323,   350,   492,   296,   200,   197,   429,   136,
     136,    32,   230,   275,   276,   228,   417,   417,   417,   200,
     198,   198,   417,   408,   305,   505,   313,   314,   416,   310,
      14,    32,    51,   315,   318,     9,    36,   197,    31,    50,
      53,    14,     9,   198,   215,   483,   331,    14,   505,   245,
     198,    14,   348,    38,    83,   396,   199,   228,   492,   323,
     200,   492,   429,   429,   228,    99,   241,   200,   213,   226,
     306,   307,   308,     9,   423,     9,   423,   200,   417,   406,
     406,    68,   316,   321,   321,    31,    50,    53,   417,    83,
     181,   196,   198,   417,   484,   417,    83,     9,   424,   228,
     200,   199,   323,    97,   198,   115,   237,   160,   102,   505,
     182,   416,   172,    14,   494,   303,   196,    38,    83,   197,
     200,   228,   198,   196,   178,   244,   213,   326,   327,   182,
     417,   182,   287,   288,   442,   304,    83,   200,   408,   242,
     175,   213,   198,   197,     9,   424,   122,   123,   124,   329,
     330,   287,    83,   272,   198,   492,   442,   506,   197,   197,
     198,   195,   489,   329,    38,    83,   174,   492,   199,   490,
     491,   505,   198,   199,   324,   506,    83,   174,    14,    83,
     489,   228,     9,   424,    14,   493,   228,    38,    83,   174,
      14,    83,   348,   324,   200,   491,   505,   200,    83,   174,
      14,    83,   348,    14,    83,   348,   348
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   206,   208,   207,   209,   209,   210,   210,   210,   210,
     210,   210,   210,   210,   211,   210,   212,   210,   210,   210,
     210,   210,   210,   210,   210,   210,   213,   213,   213,   213,
     213,   213,   213,   213,   213,   213,   213,   213,   213,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   215,   215,   216,   216,   217,   217,   218,
     219,   219,   219,   219,   220,   220,   221,   222,   222,   222,
     223,   223,   224,   224,   224,   225,   226,   227,   227,   228,
     228,   229,   229,   229,   229,   230,   230,   230,   231,   230,
     232,   230,   233,   230,   234,   230,   230,   230,   230,   230,
     230,   230,   230,   230,   230,   230,   230,   230,   230,   230,
     230,   235,   230,   236,   230,   230,   237,   230,   238,   230,
     230,   230,   230,   230,   230,   230,   230,   230,   230,   230,
     230,   230,   230,   230,   240,   239,   241,   241,   243,   242,
     244,   244,   245,   245,   246,   248,   247,   249,   247,   250,
     247,   252,   251,   253,   251,   255,   254,   256,   254,   257,
     254,   258,   254,   260,   259,   262,   261,   263,   261,   264,
     264,   265,   266,   267,   267,   267,   267,   267,   268,   268,
     269,   269,   270,   270,   271,   271,   272,   272,   273,   273,
     274,   274,   274,   275,   275,   276,   276,   277,   277,   278,
     278,   279,   279,   280,   280,   280,   280,   281,   281,   281,
     282,   282,   283,   283,   284,   284,   285,   285,   286,   286,
     287,   287,   287,   287,   287,   287,   287,   287,   288,   288,
     288,   288,   288,   288,   288,   288,   289,   289,   289,   289,
     289,   289,   289,   289,   290,   290,   290,   290,   290,   290,
     290,   290,   291,   291,   292,   292,   292,   292,   292,   292,
     293,   293,   294,   294,   294,   295,   295,   295,   295,   296,
     296,   297,   298,   299,   299,   301,   300,   302,   300,   300,
     300,   300,   303,   300,   304,   300,   300,   300,   300,   300,
     300,   300,   300,   305,   305,   305,   306,   307,   307,   308,
     308,   309,   309,   310,   310,   311,   311,   312,   312,   312,
     312,   312,   312,   312,   313,   313,   314,   315,   315,   316,
     316,   317,   317,   318,   319,   319,   319,   320,   320,   320,
     320,   321,   321,   321,   321,   321,   321,   321,   322,   322,
     322,   323,   323,   324,   324,   325,   325,   326,   326,   327,
     327,   328,   328,   328,   328,   328,   328,   328,   329,   329,
     330,   330,   330,   331,   331,   331,   331,   332,   332,   333,
     333,   334,   334,   335,   336,   336,   336,   336,   336,   336,
     337,   338,   338,   339,   339,   340,   340,   340,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   348,   348,   348,
     348,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   349,   349,   349,   349,   349,   350,   350,
     352,   351,   353,   351,   355,   354,   356,   354,   357,   354,
     358,   354,   359,   354,   360,   360,   360,   361,   361,   362,
     362,   363,   363,   364,   364,   365,   365,   366,   367,   367,
     368,   368,   369,   369,   370,   370,   371,   371,   372,   372,
     373,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,   386,   387,   388,   389,   389,   390,
     390,   391,   391,   392,   393,   394,   394,   395,   395,   395,
     395,   395,   395,   395,   395,   395,   395,   395,   396,   396,
     396,   396,   397,   398,   398,   399,   399,   400,   400,   401,
     401,   402,   403,   403,   404,   404,   404,   405,   405,   405,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   407,   408,   408,   409,   409,   409,   409,   409,   410,
     410,   411,   411,   411,   411,   412,   412,   412,   413,   413,
     413,   414,   414,   414,   415,   415,   416,   416,   416,   416,
     416,   416,   416,   416,   416,   416,   416,   416,   416,   416,
     416,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   418,   418,   419,   420,
     420,   421,   421,   421,   421,   421,   421,   421,   422,   422,
     423,   423,   424,   424,   425,   425,   425,   425,   426,   426,
     426,   426,   426,   427,   427,   427,   427,   428,   428,   429,
     429,   429,   429,   429,   429,   429,   429,   429,   429,   429,
     429,   429,   429,   429,   430,   430,   431,   431,   432,   432,
     432,   432,   433,   433,   434,   434,   435,   435,   436,   436,
     437,   437,   438,   438,   440,   439,   441,   442,   442,   443,
     443,   444,   444,   444,   445,   445,   446,   446,   447,   447,
     448,   448,   449,   449,   450,   450,   451,   451,   452,   452,
     453,   453,   453,   453,   453,   453,   453,   453,   453,   453,
     453,   454,   454,   454,   454,   454,   454,   454,   454,   454,
     455,   455,   455,   455,   455,   455,   455,   455,   455,   456,
     457,   457,   458,   458,   459,   459,   459,   460,   461,   461,
     461,   462,   462,   462,   462,   463,   463,   464,   464,   464,
     464,   464,   464,   465,   465,   465,   465,   465,   466,   466,
     466,   466,   466,   466,   467,   467,   468,   468,   468,   468,
     468,   468,   468,   468,   469,   469,   470,   470,   470,   470,
     471,   471,   472,   472,   472,   472,   473,   473,   473,   473,
     474,   474,   474,   474,   474,   474,   475,   475,   475,   476,
     476,   476,   476,   476,   476,   476,   476,   476,   476,   476,
     477,   477,   478,   478,   479,   479,   480,   480,   480,   480,
     481,   481,   482,   482,   483,   483,   484,   484,   485,   485,
     486,   486,   487,   488,   488,   488,   488,   489,   489,   490,
     490,   491,   491,   492,   492,   493,   493,   494,   495,   495,
     496,   496,   496,   496,   497,   497,   497,   498,   498,   498,
     498,   499,   499,   500,   500,   500,   500,   501,   502,   503,
     503,   504,   504,   505,   505,   505,   505,   505,   505,   505,
     505,   505,   505,   505,   506,   506
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     1,     4,     3,     0,     6,     0,     5,     3,     4,
       4,     4,     6,     7,     7,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     3,     3,     1,     2,
       1,     2,     3,     4,     3,     1,     2,     1,     2,     2,
       1,     3,     1,     3,     2,     2,     2,     5,     4,     2,
       0,     1,     1,     1,     1,     3,     5,     8,     0,     4,
       0,     6,     0,    10,     0,     4,     2,     3,     2,     3,
       2,     3,     3,     3,     3,     3,     3,     5,     1,     1,
       1,     0,     9,     0,    10,     5,     0,    13,     0,     5,
       3,     3,     2,     2,     2,     2,     2,     2,     3,     2,
       2,     3,     2,     2,     0,     4,     9,     0,     0,     4,
       2,     0,     1,     0,     1,     0,     9,     0,    10,     0,
      11,     0,     9,     0,    10,     0,     8,     0,     9,     0,
       7,     0,     8,     0,     8,     0,     7,     0,     8,     1,
       1,     1,     1,     1,     2,     3,     3,     2,     2,     0,
       2,     0,     2,     0,     1,     3,     1,     3,     2,     0,
       1,     2,     4,     1,     4,     1,     4,     1,     4,     1,
       4,     3,     5,     3,     4,     4,     5,     5,     4,     0,
       1,     1,     4,     0,     5,     0,     2,     0,     3,     0,
       7,     8,     6,     2,     5,     6,     4,     0,     4,     5,
       7,     6,     6,     7,     9,     8,     6,     7,     5,     2,
       4,     5,     3,     0,     3,     4,     6,     5,     5,     6,
       8,     7,     2,     0,     1,     2,     2,     3,     4,     4,
       3,     1,     1,     2,     4,     3,     5,     1,     3,     2,
       0,     2,     3,     2,     0,     0,     4,     0,     5,     2,
       2,     2,     0,    11,     0,    12,     3,     3,     3,     4,
       4,     3,     5,     2,     2,     0,     6,     5,     4,     3,
       1,     1,     3,     4,     1,     2,     1,     1,     5,     6,
       1,     1,     4,     1,     1,     3,     2,     2,     0,     2,
       0,     1,     3,     1,     1,     1,     1,     3,     4,     4,
       4,     1,     1,     2,     2,     2,     3,     3,     1,     1,
       1,     1,     3,     1,     3,     1,     1,     1,     0,     1,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       1,     1,     1,     3,     5,     1,     3,     5,     4,     3,
       3,     3,     4,     3,     3,     3,     2,     2,     1,     1,
       3,     3,     1,     1,     0,     1,     2,     4,     3,     3,
       6,     2,     3,     2,     3,     6,     1,     1,     1,     1,
       1,     6,     3,     4,     6,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     5,     4,     3,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     2,     1,     5,     0,
       0,    12,     0,    13,     0,     4,     0,     7,     0,     5,
       0,     3,     0,     6,     2,     2,     4,     1,     1,     5,
       3,     5,     3,     2,     0,     2,     0,     4,     4,     3,
       2,     0,     5,     3,     2,     0,     5,     3,     2,     0,
       5,     3,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     2,     0,     2,
       0,     2,     0,     4,     4,     4,     4,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     3,     4,
       1,     2,     4,     2,     6,     0,     1,     4,     0,     2,
       0,     1,     1,     3,     1,     3,     1,     1,     3,     3,
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
       3,     3,     2,     4,     2,     4,     5,     5,     5,     5,
       1,     1,     1,     1,     1,     1,     3,     3,     4,     4,
       3,     1,     1,     1,     1,     3,     1,     4,     3,     3,
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
#line 751 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
#line 6833 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 754 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
#line 6841 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 761 "hphp.y" /* yacc.c:1646  */
    { _p->addTopStatement((yyvsp[0]));}
#line 6847 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 762 "hphp.y" /* yacc.c:1646  */
    { }
#line 6853 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 765 "hphp.y" /* yacc.c:1646  */
    { _p->nns((yyvsp[0]).num(), (yyvsp[0]).text()); (yyval) = (yyvsp[0]);}
#line 6859 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 766 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6865 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 767 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6871 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 768 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6877 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 769 "hphp.y" /* yacc.c:1646  */
    { _p->nns(); (yyval) = (yyvsp[0]);}
#line 6883 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 770 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 6889 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 771 "hphp.y" /* yacc.c:1646  */
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
#line 6897 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 774 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text(), true);
                                         (yyval).reset();}
#line 6904 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 776 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart((yyvsp[-1]).text());}
#line 6910 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 777 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 6916 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 778 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceStart("");}
#line 6922 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 779 "hphp.y" /* yacc.c:1646  */
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[-1]);}
#line 6928 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 780 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]),
                                           &Parser::useClassAndNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6936 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 784 "hphp.y" /* yacc.c:1646  */
    {
                                         only_in_hh_syntax(_p);
                                         _p->onUse((yyvsp[-1]), &Parser::useNamespace);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6945 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 789 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6952 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 792 "hphp.y" /* yacc.c:1646  */
    { _p->onUse((yyvsp[-1]), &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 795 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           nullptr);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6967 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 799 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useFunction);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6975 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 803 "hphp.y" /* yacc.c:1646  */
    { _p->onGroupUse((yyvsp[-4]).text(), (yyvsp[-2]),
                                           &Parser::useConst);
                                         _p->nns(T_USE); (yyval).reset();}
#line 6983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 806 "hphp.y" /* yacc.c:1646  */
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 6990 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 811 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 6996 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 812 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7002 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 813 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7008 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 814 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7014 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 815 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7020 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 816 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7026 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 817 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7032 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 818 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7038 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 819 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7044 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 820 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7050 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 821 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7056 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 822 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7062 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 823 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7068 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 902 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7074 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 904 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7080 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 909 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7086 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 910 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7093 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 916 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7099 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 920 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7105 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 921 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[0]).text(),"");}
#line 7111 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 923 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7117 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 925 "hphp.y" /* yacc.c:1646  */
    { _p->onUseDeclaration((yyval), (yyvsp[-2]).text(),(yyvsp[0]).text());}
#line 7123 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 930 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7129 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 931 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         _p->addStatement((yyval),(yyval),(yyvsp[0]));}
#line 7136 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 937 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7142 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 941 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useClassAndNamespace);}
#line 7149 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 943 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useFunction);}
#line 7156 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 945 "hphp.y" /* yacc.c:1646  */
    { _p->onMixedUseDeclaration((yyval), (yyvsp[0]),
                                           &Parser::useConst);}
#line 7163 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 950 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7169 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 952 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + (yyvsp[-1]) + (yyvsp[0]); (yyval) = (yyvsp[-2]).num() | 2;}
#line 7175 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 955 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 1;}
#line 7181 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 957 "hphp.y" /* yacc.c:1646  */
    { (yyval).set((yyvsp[0]).num() | 2, _p->nsDecl((yyvsp[0]).text()));}
#line 7187 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 958 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = (yyval).num() | 2;}
#line 7193 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 963 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),0));
                                         }
                                         (yyval) = (yyvsp[-1]);}
#line 7202 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 970 "hphp.y" /* yacc.c:1646  */
    { if ((yyvsp[-1]).num() & 1) {
                                           (yyvsp[-1]).setText(_p->resolve((yyvsp[-1]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 7211 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 978 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7218 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 981 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-2]).setText(_p->nsDecl((yyvsp[-2]).text()));
                                         _p->onConst((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 7225 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 987 "hphp.y" /* yacc.c:1646  */
    { _p->addStatement((yyval),(yyvsp[-1]),(yyvsp[0]));}
#line 7231 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 988 "hphp.y" /* yacc.c:1646  */
    { _p->onStatementListStart((yyval));}
#line 7237 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 991 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7243 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 992 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7249 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 993 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7255 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 994 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7261 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 997 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-1]));}
#line 7267 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1001 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 7273 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 1006 "hphp.y" /* yacc.c:1646  */
    { _p->onIf((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[-2]));}
#line 7279 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1007 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7286 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1009 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7294 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1013 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7301 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1016 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[-3]),(yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7309 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1020 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7316 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1022 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[-7]),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7324 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1025 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1027 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[-2]),(yyvsp[0]));
                                         _p->onCompleteLabelScope(false);}
#line 7339 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1030 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, NULL);}
#line 7345 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1031 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), true, &(yyvsp[-1]));}
#line 7351 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1032 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, NULL);}
#line 7357 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1033 "hphp.y" /* yacc.c:1646  */
    { _p->onBreakContinue((yyval), false, &(yyvsp[-1]));}
#line 7363 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1034 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), NULL);}
#line 7369 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1035 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7375 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1036 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldBreak((yyval));}
#line 7381 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1037 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobal((yyval), (yyvsp[-1]));}
#line 7387 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1038 "hphp.y" /* yacc.c:1646  */
    { _p->onStatic((yyval), (yyvsp[-1]));}
#line 7393 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1039 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7399 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1040 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[-1]), 0);}
#line 7405 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1041 "hphp.y" /* yacc.c:1646  */
    { _p->onUnset((yyval), (yyvsp[-2]));}
#line 7411 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1042 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval) = ';';}
#line 7417 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1043 "hphp.y" /* yacc.c:1646  */
    { _p->onEcho((yyval), (yyvsp[0]), 1);}
#line 7423 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1044 "hphp.y" /* yacc.c:1646  */
    { _p->onHashBang((yyval), (yyvsp[0]));
                                         (yyval) = T_HASHBANG;}
#line 7430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1048 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7437 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1050 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-6]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), false);
                                         _p->onCompleteLabelScope(false);}
#line 7445 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1055 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7452 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1057 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-3]),(yyvsp[0]), true);
                                         _p->onCompleteLabelScope(false);}
#line 7460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1061 "hphp.y" /* yacc.c:1646  */
    { _p->onDeclare((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]);
                                         (yyval) = T_DECLARE;}
#line 7468 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1070 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7474 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1071 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval),(yyvsp[-11]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 7480 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1074 "hphp.y" /* yacc.c:1646  */
    { _p->onCompleteLabelScope(false);}
#line 7486 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1075 "hphp.y" /* yacc.c:1646  */
    { _p->onTry((yyval), (yyvsp[-3]), (yyvsp[0]));}
#line 7492 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 1076 "hphp.y" /* yacc.c:1646  */
    { _p->onThrow((yyval), (yyvsp[-1]));}
#line 7498 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 1077 "hphp.y" /* yacc.c:1646  */
    { _p->onGoto((yyval), (yyvsp[-1]), true);
                                         _p->addGoto((yyvsp[-1]).text(),
                                                     _p->getRange(),
                                                     &(yyval));}
#line 7507 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 1081 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7513 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 1082 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7519 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 1083 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7525 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 1084 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7531 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 1085 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7537 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 1086 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7543 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 1087 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1]));}
#line 7549 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 1088 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7555 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 1089 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7561 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 1090 "hphp.y" /* yacc.c:1646  */
    { _p->onReturn((yyval), &(yyvsp[-1])); }
#line 7567 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 1091 "hphp.y" /* yacc.c:1646  */
    { _p->onExpStatement((yyval), (yyvsp[-1]));}
#line 7573 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 1092 "hphp.y" /* yacc.c:1646  */
    { _p->onLabel((yyval), (yyvsp[-1]));
                                         _p->addLabel((yyvsp[-1]).text(),
                                                      _p->getRange(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[-1]));}
#line 7583 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 1100 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);}
#line 7589 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 1101 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 7595 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 1110 "hphp.y" /* yacc.c:1646  */
    { _p->onCatch((yyval), (yyvsp[-8]), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-1]));}
#line 7601 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 1111 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7607 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 1115 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
#line 7614 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 1117 "hphp.y" /* yacc.c:1646  */
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[-1]));
                                         _p->onCompleteLabelScope(false);}
#line 7622 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 1123 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7628 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 1124 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7634 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 1128 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 1;}
#line 7640 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 1129 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7646 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 1133 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation(); }
#line 7652 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 1139 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7661 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 1145 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),nullptr,(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7670 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 1152 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7679 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 1158 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7688 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 1165 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsDecl((yyvsp[0]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[0]));
                                         _p->pushLabelInfo();}
#line 7697 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 1171 "hphp.y" /* yacc.c:1646  */
    { _p->onFunction((yyval),&(yyvsp[-9]),(yyvsp[-1]),(yyvsp[-7]),(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),&(yyvsp[-10]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 7706 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 1179 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 7713 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 1183 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),0); }
#line 7719 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 1187 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[0]));}
#line 7726 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 1191 "hphp.y" /* yacc.c:1646  */
    { _p->onEnum((yyval),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-9])); }
#line 7732 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 1197 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 7739 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 1200 "hphp.y" /* yacc.c:1646  */
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
#line 7757 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 1215 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart((yyvsp[-1]).num(),(yyvsp[0]));}
#line 7764 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 208:
#line 1218 "hphp.y" /* yacc.c:1646  */
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
#line 7782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 1232 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 7789 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 1235 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7797 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 1240 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[0]));}
#line 7804 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 1243 "hphp.y" /* yacc.c:1646  */
    { _p->onInterface((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-7]));
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7812 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 1249 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpressionStart(); }
#line 7818 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 1252 "hphp.y" /* yacc.c:1646  */
    { _p->onClassExpression((yyval), (yyvsp[-5]), (yyvsp[-4]), (yyvsp[-3]), (yyvsp[-1])); }
#line 7824 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 1256 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 7831 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 1259 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7842 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 1267 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).setText(_p->nsClassDecl((yyvsp[0]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[0]));}
#line 7849 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 1270 "hphp.y" /* yacc.c:1646  */
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[-5]),t_ext,(yyvsp[-3]),
                                                     (yyvsp[-1]), &(yyvsp[-7]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
#line 7860 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 1278 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7866 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 1279 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); _p->pushTypeScope();
                                            _p->pushClass(true); (yyval) = (yyvsp[0]);}
#line 7873 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 1283 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7879 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 1286 "hphp.y" /* yacc.c:1646  */
    { _p->pushClass(false); (yyval) = (yyvsp[0]);}
#line 7885 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 1289 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_CLASS;}
#line 7891 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 1290 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT; }
#line 7897 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 1291 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; }
#line 7905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 1294 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; }
#line 7911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 1295 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 7917 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 1299 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7923 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 1300 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 1303 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7935 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 1304 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7941 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 1307 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 1308 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7953 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 1311 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), NULL, (yyvsp[0]));}
#line 7959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 1313 "hphp.y" /* yacc.c:1646  */
    { _p->onInterfaceName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 7965 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 1316 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), NULL, (yyvsp[0]));}
#line 7971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 1318 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitName((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 7977 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 1322 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 7983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 1323 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 7989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 1326 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 7995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 1327 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8001 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 1328 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-1]), NULL);}
#line 8007 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 1332 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8013 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1334 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1337 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1339 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1342 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8037 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1344 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8043 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1347 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[0]));}
#line 8049 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1349 "hphp.y" /* yacc.c:1646  */
    { _p->onBlock((yyval), (yyvsp[-2]));}
#line 8055 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1353 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 8061 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1355 "hphp.y" /* yacc.c:1646  */
    {_p->onDeclareList((yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                           (yyval) = (yyvsp[-4]);}
#line 8068 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1360 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8074 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1361 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8080 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1362 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8086 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1363 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 8092 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1368 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]));}
#line 8098 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1370 "hphp.y" /* yacc.c:1646  */
    { _p->onCase((yyval),(yyvsp[-3]),NULL,(yyvsp[0]));}
#line 8104 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1371 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8110 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1374 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8116 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1375 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8122 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1380 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 8128 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1381 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8134 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1386 "hphp.y" /* yacc.c:1646  */
    { _p->onElseIf((yyval),(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 8140 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1387 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8146 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1390 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8152 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1391 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8158 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1394 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8164 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1395 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8170 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1403 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8177 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1409 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8184 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 272:
#line 1415 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval) = (yyvsp[-5]); }
#line 8192 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 273:
#line 1419 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8198 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 274:
#line 1423 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),false,
                                                            &(yyvsp[-4]),&(yyvsp[-3])); }
#line 8205 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 275:
#line 1428 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),true,
                                                            &(yyvsp[-5]),&(yyvsp[-4])); }
#line 8212 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 276:
#line 1433 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-2]));
                                        (yyval).reset(); }
#line 8220 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 277:
#line 1436 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8226 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 278:
#line 1442 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8233 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 279:
#line 1446 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8240 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 280:
#line 1451 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8247 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 281:
#line 1456 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8254 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 282:
#line 1461 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]),0,
                                                     NULL,&(yyvsp[-3]),&(yyvsp[-2]));}
#line 8261 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 283:
#line 1466 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-2]),(yyvsp[0]),1,
                                                     NULL,&(yyvsp[-4]),&(yyvsp[-3]));}
#line 8268 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 284:
#line 1472 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-8]),(yyvsp[-4]),(yyvsp[-2]),1,
                                                     &(yyvsp[0]),&(yyvsp[-6]),&(yyvsp[-5]));}
#line 8275 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 285:
#line 1478 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-3]),(yyvsp[-2]),0,
                                                     &(yyvsp[0]),&(yyvsp[-5]),&(yyvsp[-4]));}
#line 8282 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 286:
#line 1486 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),
                                        false,&(yyvsp[-3]),NULL); }
#line 8289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 287:
#line 1491 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[0]),
                                        true,&(yyvsp[-4]),NULL); }
#line 8296 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 288:
#line 1496 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval) = (yyvsp[-4]); }
#line 8304 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 289:
#line 1500 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8310 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 290:
#line 1503 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),
                                                            false,&(yyvsp[-3]),NULL); }
#line 8317 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 291:
#line 1507 "hphp.y" /* yacc.c:1646  */
    { _p->onVariadicParam((yyval),NULL,(yyvsp[-3]),(yyvsp[0]),
                                                            true,&(yyvsp[-4]),NULL); }
#line 8324 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 292:
#line 1511 "hphp.y" /* yacc.c:1646  */
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[-2]), (yyvsp[-1]), NULL);
                                        (yyval).reset(); }
#line 8332 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 293:
#line 1514 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8338 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 294:
#line 1519 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8345 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 295:
#line 1522 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8352 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 296:
#line 1526 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8359 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 297:
#line 1530 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),NULL,(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8366 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 298:
#line 1534 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-4]),(yyvsp[-1]),(yyvsp[0]),false,
                                                     NULL,&(yyvsp[-2]),NULL); }
#line 8373 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 299:
#line 1538 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-5]),(yyvsp[-2]),(yyvsp[0]),true,
                                                     NULL,&(yyvsp[-3]),NULL); }
#line 8380 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 300:
#line 1543 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-2]),true,
                                                     &(yyvsp[0]),&(yyvsp[-5]),NULL); }
#line 8387 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 301:
#line 1548 "hphp.y" /* yacc.c:1646  */
    { _p->onParam((yyval),&(yyvsp[-6]),(yyvsp[-3]),(yyvsp[-2]),false,
                                                     &(yyvsp[0]),&(yyvsp[-4]),NULL); }
#line 8394 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 302:
#line 1554 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8400 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 303:
#line 1555 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 304:
#line 1558 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,false);}
#line 8412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 305:
#line 1559 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),true,false);}
#line 8418 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 306:
#line 1560 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),NULL,(yyvsp[0]),false,true);}
#line 8424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 307:
#line 1562 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-2]),(yyvsp[0]),false, false);}
#line 8430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 308:
#line 1564 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),false,true);}
#line 8436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 309:
#line 1566 "hphp.y" /* yacc.c:1646  */
    { _p->onCallParam((yyval),&(yyvsp[-3]),(yyvsp[0]),true, false);}
#line 8442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 310:
#line 1570 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 8448 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 311:
#line 1571 "hphp.y" /* yacc.c:1646  */
    { _p->onGlobalVar((yyval), NULL, (yyvsp[0]));}
#line 8454 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 312:
#line 1574 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8460 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 313:
#line 1575 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 8466 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 314:
#line 1576 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 1;}
#line 8472 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 315:
#line 1580 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 8478 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 316:
#line 1582 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 8484 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 317:
#line 1583 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[0]),0);}
#line 8490 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 318:
#line 1584 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 8496 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 319:
#line 1589 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 8502 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 320:
#line 1590 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8508 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 321:
#line 1593 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 8515 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 322:
#line 1598 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 8521 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 323:
#line 1604 "hphp.y" /* yacc.c:1646  */
    { _p->onClassStatement((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 8527 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 324:
#line 1605 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8533 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 325:
#line 1608 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[0]));}
#line 8539 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 326:
#line 1609 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 8546 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 327:
#line 1612 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableModifer((yyvsp[-1]));}
#line 8552 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 328:
#line 1613 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-3]));}
#line 8559 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 329:
#line 1615 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL);}
#line 8566 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 330:
#line 1618 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[-1]),NULL, true);}
#line 8573 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 331:
#line 1620 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 8579 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 332:
#line 1623 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 8587 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 333:
#line 1630 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8596 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 334:
#line 1638 "hphp.y" /* yacc.c:1646  */
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[-1]), (yyvsp[-4]));
                                         _p->pushLabelInfo();}
#line 8604 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 335:
#line 1645 "hphp.y" /* yacc.c:1646  */
    { _p->onMethod((yyval),(yyvsp[-10]),(yyvsp[-2]),(yyvsp[-8]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[0]),&(yyvsp[-11]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
#line 8613 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 336:
#line 1650 "hphp.y" /* yacc.c:1646  */
    { _p->xhpSetAttributes((yyvsp[-1]));}
#line 8619 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 337:
#line 1652 "hphp.y" /* yacc.c:1646  */
    { xhp_category_stmt(_p,(yyval),(yyvsp[-1]));}
#line 8625 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 338:
#line 1654 "hphp.y" /* yacc.c:1646  */
    { xhp_children_stmt(_p,(yyval),(yyvsp[-1]));}
#line 8631 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 339:
#line 1656 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), true); }
#line 8637 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 340:
#line 1658 "hphp.y" /* yacc.c:1646  */
    { _p->onClassRequire((yyval), (yyvsp[-1]), false); }
#line 8643 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 341:
#line 1659 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[-1]),t); }
#line 8650 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 342:
#line 1662 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitUse((yyval),(yyvsp[-3]),(yyvsp[-1])); }
#line 8656 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 343:
#line 1665 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 8662 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 344:
#line 1666 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitRule((yyval),(yyvsp[-1]),(yyvsp[0])); }
#line 8668 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 345:
#line 1667 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 8674 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 346:
#line 1673 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitPrecRule((yyval),(yyvsp[-5]),(yyvsp[-3]),(yyvsp[-1]));}
#line 8680 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 347:
#line 1678 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[-4]),(yyvsp[-2]),
                                                                    (yyvsp[-1]));}
#line 8687 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 348:
#line 1681 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[-3]),(yyvsp[-1]),
                                                                    t);}
#line 8695 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 349:
#line 1688 "hphp.y" /* yacc.c:1646  */
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 8701 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 350:
#line 1689 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[0]));}
#line 8708 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 351:
#line 1694 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[0]));}
#line 8715 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 352:
#line 1697 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute_list(_p,(yyval), &(yyvsp[-2]),(yyvsp[0]));}
#line 8721 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 353:
#line 1704 "hphp.y" /* yacc.c:1646  */
    { xhp_attribute(_p,(yyval),(yyvsp[-3]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));
                                         (yyval) = 1;}
#line 8728 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 354:
#line 1706 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 8734 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 355:
#line 1710 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8740 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 357:
#line 1715 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8746 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 358:
#line 1717 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8752 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 359:
#line 1719 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 4;}
#line 8758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 360:
#line 1720 "hphp.y" /* yacc.c:1646  */
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[0]));}
#line 8769 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 361:
#line 1726 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 6;}
#line 8775 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 362:
#line 1728 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = 7;}
#line 8781 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 363:
#line 1729 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 9; }
#line 8787 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 364:
#line 1733 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,0,(yyvsp[0]),0);}
#line 8793 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 365:
#line 1735 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),0,(yyvsp[0]),0);}
#line 8799 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 366:
#line 1740 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 8805 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 367:
#line 1743 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8811 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 368:
#line 1744 "hphp.y" /* yacc.c:1646  */
    { scalar_null(_p, (yyval));}
#line 8817 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 369:
#line 1748 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "1");}
#line 8823 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 370:
#line 1749 "hphp.y" /* yacc.c:1646  */
    { scalar_num(_p, (yyval), "0");}
#line 8829 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 371:
#line 1753 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[0]),t,0);}
#line 8836 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 372:
#line 1756 "hphp.y" /* yacc.c:1646  */
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[-2]),&(yyvsp[0]),t,0);}
#line 8843 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 373:
#line 1761 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 8850 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 374:
#line 1766 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 2;}
#line 8856 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 375:
#line 1767 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1;}
#line 8863 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 376:
#line 1769 "hphp.y" /* yacc.c:1646  */
    { (yyval) = 0;}
#line 8869 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 377:
#line 1773 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-1]), 0);}
#line 8875 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 378:
#line 1774 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 1);}
#line 8881 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 379:
#line 1775 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 2);}
#line 8887 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 380:
#line 1776 "hphp.y" /* yacc.c:1646  */
    { xhp_children_paren(_p, (yyval), (yyvsp[-2]), 3);}
#line 8893 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 381:
#line 1780 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8899 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 382:
#line 1781 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[0]),0,  0);}
#line 8905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 383:
#line 1782 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),1,  0);}
#line 8911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 384:
#line 1783 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),2,  0);}
#line 8917 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 385:
#line 1784 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-1]),3,  0);}
#line 8923 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 386:
#line 1786 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),4,&(yyvsp[0]));}
#line 8929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 387:
#line 1788 "hphp.y" /* yacc.c:1646  */
    { xhp_children_decl(_p,(yyval),(yyvsp[-2]),5,&(yyvsp[0]));}
#line 8935 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 388:
#line 1792 "hphp.y" /* yacc.c:1646  */
    { (yyval) = -1;
                                         if ((yyvsp[0]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[0]).same("pcdata")) (yyval) = 2;}
#line 8943 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 389:
#line 1795 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();  (yyval) = (yyvsp[0]); (yyval) = 3;}
#line 8949 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 390:
#line 1796 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(0); (yyval) = (yyvsp[0]); (yyval) = 4;}
#line 8955 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 391:
#line 1800 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8961 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 392:
#line 1801 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 8967 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 393:
#line 1805 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8973 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 394:
#line 1806 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyval), (yyvsp[-1])); (yyval) = 1;}
#line 8979 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 395:
#line 1809 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8985 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 396:
#line 1810 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 8991 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 397:
#line 1813 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 8997 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 398:
#line 1814 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9003 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 399:
#line 1817 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),NULL,(yyvsp[0]));}
#line 9009 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 400:
#line 1819 "hphp.y" /* yacc.c:1646  */
    { _p->onMemberModifier((yyval),&(yyvsp[-1]),(yyvsp[0]));}
#line 9015 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 401:
#line 1822 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9021 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 402:
#line 1823 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9027 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 403:
#line 1824 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9033 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 404:
#line 1825 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_STATIC;}
#line 9039 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 405:
#line 1826 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ABSTRACT;}
#line 9045 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 406:
#line 1827 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_FINAL;}
#line 9051 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 407:
#line 1828 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_ASYNC;}
#line 9057 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 408:
#line 1832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9063 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 409:
#line 1833 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9069 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 410:
#line 1836 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PUBLIC;}
#line 9075 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 411:
#line 1837 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PROTECTED;}
#line 9081 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 412:
#line 1838 "hphp.y" /* yacc.c:1646  */
    { (yyval) = T_PRIVATE;}
#line 9087 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 413:
#line 1842 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 9093 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 414:
#line 1844 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),&(yyvsp[-4]),(yyvsp[-2]),&(yyvsp[0]));}
#line 9099 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 415:
#line 1845 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[0]),0);}
#line 9105 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 416:
#line 1846 "hphp.y" /* yacc.c:1646  */
    { _p->onClassVariable((yyval),0,(yyvsp[-2]),&(yyvsp[0]));}
#line 9111 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 417:
#line 1850 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),&(yyvsp[-4]),(yyvsp[-2]),(yyvsp[0]));}
#line 9117 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 418:
#line 1852 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConstant((yyval),0,(yyvsp[-2]),(yyvsp[0]));}
#line 9123 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 419:
#line 1856 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),&(yyvsp[-2]),(yyvsp[0]));}
#line 9129 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 420:
#line 1858 "hphp.y" /* yacc.c:1646  */
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[0]));}
#line 9135 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 421:
#line 1862 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[-1]), t);
                                          _p->popTypeScope(); }
#line 9143 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 422:
#line 1866 "hphp.y" /* yacc.c:1646  */
    { _p->onClassTypeConstant((yyval), (yyvsp[-3]), (yyvsp[0]));
                                          _p->popTypeScope(); }
#line 9150 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 423:
#line 1870 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9156 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 424:
#line 1874 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9162 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 425:
#line 1876 "hphp.y" /* yacc.c:1646  */
    { _p->onNewObject((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 9168 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 426:
#line 1877 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9174 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 427:
#line 1878 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_CLONE,1);}
#line 9180 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 428:
#line 1879 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9186 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 429:
#line 1880 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9192 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 430:
#line 1883 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9198 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 431:
#line 1887 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 9204 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 432:
#line 1888 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 9210 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 433:
#line 1892 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9216 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 434:
#line 1893 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9222 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 435:
#line 1897 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), NULL);}
#line 9228 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 436:
#line 1898 "hphp.y" /* yacc.c:1646  */
    { _p->onYield((yyval), &(yyvsp[0]));}
#line 9234 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 437:
#line 1899 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldPair((yyval), &(yyvsp[-2]), &(yyvsp[0]));}
#line 9240 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 438:
#line 1900 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9246 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 439:
#line 1904 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9252 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 440:
#line 1909 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9258 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 441:
#line 1913 "hphp.y" /* yacc.c:1646  */
    { _p->onYieldFrom((yyval),&(yyvsp[0]));}
#line 9264 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 442:
#line 1917 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9270 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 443:
#line 1921 "hphp.y" /* yacc.c:1646  */
    { _p->onAwait((yyval), (yyvsp[0])); }
#line 9276 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 444:
#line 1925 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0, true);}
#line 9282 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 445:
#line 1930 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]), true);}
#line 9288 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 446:
#line 1934 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9294 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 447:
#line 1935 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9300 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 448:
#line 1936 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9306 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 449:
#line 1937 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9312 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 450:
#line 1938 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9318 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 451:
#line 1943 "hphp.y" /* yacc.c:1646  */
    { _p->onListAssignment((yyval), (yyvsp[-3]), &(yyvsp[0]));}
#line 9324 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 452:
#line 1944 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 9330 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 453:
#line 1945 "hphp.y" /* yacc.c:1646  */
    { _p->onAssign((yyval), (yyvsp[-3]), (yyvsp[0]), 1);}
#line 9336 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 454:
#line 1948 "hphp.y" /* yacc.c:1646  */
    { _p->onAssignNew((yyval),(yyvsp[-5]),(yyvsp[-1]),(yyvsp[0]));}
#line 9342 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 455:
#line 1949 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PLUS_EQUAL);}
#line 9348 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 456:
#line 1950 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MINUS_EQUAL);}
#line 9354 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 457:
#line 1951 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MUL_EQUAL);}
#line 9360 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 458:
#line 1952 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_DIV_EQUAL);}
#line 9366 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 459:
#line 1953 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_CONCAT_EQUAL);}
#line 9372 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 460:
#line 1954 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_MOD_EQUAL);}
#line 9378 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 461:
#line 1955 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_AND_EQUAL);}
#line 9384 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 462:
#line 1956 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_OR_EQUAL);}
#line 9390 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 463:
#line 1957 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_XOR_EQUAL);}
#line 9396 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 464:
#line 1958 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL_EQUAL);}
#line 9402 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 465:
#line 1959 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR_EQUAL);}
#line 9408 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 466:
#line 1960 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW_EQUAL);}
#line 9414 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 467:
#line 1961 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_INC,0);}
#line 9420 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 468:
#line 1962 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INC,1);}
#line 9426 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 469:
#line 1963 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_DEC,0);}
#line 9432 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 470:
#line 1964 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DEC,1);}
#line 9438 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 471:
#line 1965 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 9444 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 472:
#line 1966 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 9450 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 473:
#line 1967 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 9456 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 474:
#line 1968 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 9462 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 475:
#line 1969 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 9468 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 476:
#line 1970 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 9474 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 477:
#line 1971 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 9480 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 478:
#line 1972 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 9486 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 479:
#line 1973 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 9492 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 480:
#line 1974 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 9498 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 481:
#line 1975 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 9504 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 482:
#line 1976 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 9510 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 483:
#line 1977 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 9516 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 484:
#line 1978 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 9522 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 485:
#line 1979 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 9528 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 486:
#line 1980 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_PIPE);}
#line 9534 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 487:
#line 1981 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 9540 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 488:
#line 1982 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 9546 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 489:
#line 1983 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 9552 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 490:
#line 1984 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 9558 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 491:
#line 1985 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 9564 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 492:
#line 1986 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 9570 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 493:
#line 1987 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 9576 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 494:
#line 1988 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 9582 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 495:
#line 1989 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 9588 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 496:
#line 1990 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 9594 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 497:
#line 1991 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 9600 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 498:
#line 1992 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 9607 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 499:
#line 1994 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 9613 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 500:
#line 1995 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 9620 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 501:
#line 1997 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 9626 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 502:
#line 1999 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_INSTANCEOF);}
#line 9632 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 503:
#line 2000 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 9638 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 504:
#line 2001 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 9644 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 505:
#line 2002 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 9650 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 506:
#line 2003 "hphp.y" /* yacc.c:1646  */
    { _p->onNullCoalesce((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 9656 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 507:
#line 2004 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9662 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 508:
#line 2005 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INT_CAST,1);}
#line 9668 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 509:
#line 2006 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_DOUBLE_CAST,1);}
#line 9674 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 510:
#line 2007 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_STRING_CAST,1);}
#line 9680 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 511:
#line 2008 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_ARRAY_CAST,1);}
#line 9686 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 512:
#line 2009 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_OBJECT_CAST,1);}
#line 9692 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 513:
#line 2010 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_BOOL_CAST,1);}
#line 9698 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 514:
#line 2011 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_UNSET_CAST,1);}
#line 9704 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 515:
#line 2012 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_EXIT,1);}
#line 9710 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 516:
#line 2013 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'@',1);}
#line 9716 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 517:
#line 2014 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9722 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 518:
#line 2015 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9728 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 519:
#line 2016 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9734 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 520:
#line 2017 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9740 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 521:
#line 2018 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9746 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 522:
#line 2019 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9752 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 523:
#line 2020 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 524:
#line 2021 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9764 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 525:
#line 2022 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'`',(yyvsp[-1]));}
#line 9770 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 526:
#line 2023 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_PRINT,1);}
#line 9776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 527:
#line 2024 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 9782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 528:
#line 2031 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 9788 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 529:
#line 2032 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 9794 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 530:
#line 2037 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 9803 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 531:
#line 2043 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, nullptr,
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9814 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 532:
#line 2051 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
#line 9823 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 533:
#line 2057 "hphp.y" /* yacc.c:1646  */
    { _p->finishStatement((yyvsp[-1]), (yyvsp[-1])); (yyvsp[-1]) = 1;
                                         (yyval) = _p->onClosure(
                                           ClosureType::Long, &(yyvsp[-12]),
                                           (yyvsp[-10]),(yyvsp[-7]),(yyvsp[-4]),(yyvsp[-1]),(yyvsp[-5]),&(yyvsp[-3]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9834 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 534:
#line 2067 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),0,
                                                     NULL,NULL,NULL);}
#line 9847 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 535:
#line 2075 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         (yyvsp[-3]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[-3]), nullptr, (yyvsp[-3]));
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[-3]),
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9861 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 536:
#line 2085 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9871 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 537:
#line 2093 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         (yyvsp[-6]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[-6]), nullptr, (yyvsp[-6]));
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[-6]),
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9885 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 538:
#line 2103 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9895 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 539:
#line 2109 "hphp.y" /* yacc.c:1646  */
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
#line 9911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 540:
#line 2120 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[0]),NULL,u,(yyvsp[0]),0,
                                                     NULL,NULL,NULL);}
#line 9924 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 541:
#line 2128 "hphp.y" /* yacc.c:1646  */
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[-2]),w,(yyvsp[0]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9936 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 542:
#line 2135 "hphp.y" /* yacc.c:1646  */
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
#line 9946 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 543:
#line 2143 "hphp.y" /* yacc.c:1646  */
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[0]), (yyvsp[0])); (yyvsp[0]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[-3]),v,(yyvsp[0]),(yyvsp[-1]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
#line 9958 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 544:
#line 2153 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 9964 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 545:
#line 2154 "hphp.y" /* yacc.c:1646  */
    { (yyval) = _p->onExprForLambda((yyvsp[0]));}
#line 9970 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 546:
#line 2156 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 9976 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 547:
#line 2160 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[0]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 9983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 548:
#line 2162 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 9989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 549:
#line 2169 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 9995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 550:
#line 2172 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10001 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 551:
#line 2179 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10007 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 552:
#line 2182 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 10013 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 553:
#line 2187 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 554:
#line 2188 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 555:
#line 2193 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 556:
#line 2194 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 10037 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 557:
#line 2198 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval), (yyvsp[-1]), T_ARRAY);}
#line 10043 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 558:
#line 2202 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10049 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 559:
#line 2203 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 10055 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 560:
#line 2208 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10061 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 561:
#line 2209 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10067 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 562:
#line 2214 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10073 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 563:
#line 2215 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10079 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 564:
#line 2220 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10085 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 565:
#line 2221 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10091 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 566:
#line 2227 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10097 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 567:
#line 2229 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10103 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 568:
#line 2234 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10109 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 569:
#line 2235 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10115 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 570:
#line 2241 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10121 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 571:
#line 2243 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10127 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 572:
#line 2247 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10133 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 573:
#line 2251 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10139 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 574:
#line 2255 "hphp.y" /* yacc.c:1646  */
    { _p->onDict((yyval), (yyvsp[-1])); }
#line 10145 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 575:
#line 2259 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10151 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 576:
#line 2263 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10157 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 577:
#line 2267 "hphp.y" /* yacc.c:1646  */
    { _p->onVec((yyval), (yyvsp[-1])); }
#line 10163 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 578:
#line 2271 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10169 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 579:
#line 2275 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10175 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 580:
#line 2279 "hphp.y" /* yacc.c:1646  */
    { _p->onKeyset((yyval), (yyvsp[-1])); }
#line 10181 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 581:
#line 2283 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10187 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 582:
#line 2287 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10193 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 583:
#line 2291 "hphp.y" /* yacc.c:1646  */
    { _p->onVArray((yyval),(yyvsp[-1])); }
#line 10199 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 584:
#line 2295 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10205 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 585:
#line 2299 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10211 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 586:
#line 2303 "hphp.y" /* yacc.c:1646  */
    { _p->onDArray((yyval),(yyvsp[-1])); }
#line 10217 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 587:
#line 2308 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10223 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 588:
#line 2309 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10229 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 589:
#line 2314 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10235 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 590:
#line 2315 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10241 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 591:
#line 2320 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10247 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 592:
#line 2321 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 593:
#line 2326 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10261 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 594:
#line 2333 "hphp.y" /* yacc.c:1646  */
    { Token t;
                                         _p->onName(t,(yyvsp[-3]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[-1]),T_COLLECTION);}
#line 10269 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 595:
#line 2340 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10275 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 596:
#line 2342 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 10281 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 597:
#line 2346 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10287 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 598:
#line 2347 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10293 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 599:
#line 2348 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10299 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 600:
#line 2349 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10305 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 601:
#line 2350 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10311 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 602:
#line 2351 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10317 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 603:
#line 2352 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10323 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 604:
#line 2353 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10329 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 605:
#line 2354 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0])); }
#line 10336 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 606:
#line 2356 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10342 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 607:
#line 2357 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10348 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 608:
#line 2361 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10354 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 609:
#line 2362 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 10360 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 610:
#line 2363 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),0);}
#line 10366 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 611:
#line 2364 "hphp.y" /* yacc.c:1646  */
    { _p->onClosureParam((yyval),  0,(yyvsp[0]),1);}
#line 10372 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 612:
#line 2371 "hphp.y" /* yacc.c:1646  */
    { xhp_tag(_p,(yyval),(yyvsp[-2]),(yyvsp[-1]));}
#line 10378 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 613:
#line 2374 "hphp.y" /* yacc.c:1646  */
    { Token t1; _p->onArray(t1,(yyvsp[-1]));
                                         Token t2; _p->onArray(t2,(yyvsp[0]));
                                         Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onCallParam((yyvsp[-1]),NULL,t1,0,0);
                                         _p->onCallParam((yyval), &(yyvsp[-1]),t2,0,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),file,0,0);
                                         _p->onCallParam((yyvsp[-1]), &(yyvsp[-1]),line,0,0);
                                         (yyval).setText("");}
#line 10392 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 614:
#line 2385 "hphp.y" /* yacc.c:1646  */
    { Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onArray((yyvsp[-2]),(yyvsp[-5]));
                                         _p->onArray((yyvsp[-1]),(yyvsp[-3]));
                                         _p->onCallParam((yyvsp[-4]),NULL,(yyvsp[-2]),0,0);
                                         _p->onCallParam((yyval), &(yyvsp[-4]),(yyvsp[-1]),0,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),file,0,0);
                                         _p->onCallParam((yyvsp[-4]), &(yyvsp[-4]),line,0,0);
                                         (yyval).setText((yyvsp[0]).text());}
#line 10406 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 615:
#line 2396 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText("");}
#line 10412 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 616:
#line 2397 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); (yyval).setText((yyvsp[0]));}
#line 10418 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 617:
#line 2402 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 10424 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 618:
#line 2403 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10430 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 619:
#line 2406 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-1]),0,(yyvsp[0]),0);}
#line 10436 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 620:
#line 2407 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 10442 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 621:
#line 2410 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10449 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 622:
#line 2414 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));}
#line 10457 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 623:
#line 2417 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 10463 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 624:
#line 2420 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();
                                         if ((yyvsp[0]).htmlTrim()) {
                                           (yyvsp[0]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[0]));
                                         }
                                       }
#line 10475 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 625:
#line 2427 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 10481 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 626:
#line 2428 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 10487 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 627:
#line 2432 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10493 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 628:
#line 2434 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + ":" + (yyvsp[0]);}
#line 10499 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 629:
#line 2436 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) + "-" + (yyvsp[0]);}
#line 10505 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 630:
#line 2440 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10511 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 631:
#line 2441 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10517 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 632:
#line 2442 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10523 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 633:
#line 2443 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10529 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 634:
#line 2444 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10535 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 635:
#line 2445 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10541 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 636:
#line 2446 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10547 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 637:
#line 2447 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10553 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 638:
#line 2448 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10559 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 639:
#line 2449 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10565 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 640:
#line 2450 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10571 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 641:
#line 2451 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10577 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 642:
#line 2452 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10583 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 643:
#line 2453 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10589 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 644:
#line 2454 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10595 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 645:
#line 2455 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10601 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 646:
#line 2456 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10607 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 647:
#line 2457 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10613 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 648:
#line 2458 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10619 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 649:
#line 2459 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10625 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 650:
#line 2460 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10631 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 651:
#line 2461 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10637 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 652:
#line 2462 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10643 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 653:
#line 2463 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10649 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 654:
#line 2464 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10655 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 655:
#line 2465 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10661 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 656:
#line 2466 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10667 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 657:
#line 2467 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10673 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 658:
#line 2468 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10679 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 659:
#line 2469 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10685 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 660:
#line 2470 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10691 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 661:
#line 2471 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10697 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 662:
#line 2472 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10703 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 663:
#line 2473 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10709 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 664:
#line 2474 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10715 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 665:
#line 2475 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10721 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 666:
#line 2476 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10727 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 667:
#line 2477 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10733 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 668:
#line 2478 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10739 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 669:
#line 2479 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10745 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 670:
#line 2480 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10751 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 671:
#line 2481 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10757 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 672:
#line 2482 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10763 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 673:
#line 2483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10769 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 674:
#line 2484 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10775 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 675:
#line 2485 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10781 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 676:
#line 2486 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10787 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 677:
#line 2487 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10793 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 678:
#line 2488 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10799 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 679:
#line 2489 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10805 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 680:
#line 2490 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10811 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 681:
#line 2491 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10817 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 682:
#line 2492 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10823 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 683:
#line 2493 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10829 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 684:
#line 2494 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10835 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 685:
#line 2495 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10841 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 686:
#line 2496 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10847 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 687:
#line 2497 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10853 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 688:
#line 2498 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10859 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 689:
#line 2499 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10865 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 690:
#line 2500 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10871 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 691:
#line 2501 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10877 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 692:
#line 2502 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10883 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 693:
#line 2503 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10889 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 694:
#line 2504 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10895 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 695:
#line 2505 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10901 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 696:
#line 2506 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10907 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 697:
#line 2507 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10913 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 698:
#line 2508 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10919 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 699:
#line 2509 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10925 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 700:
#line 2510 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10931 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 701:
#line 2511 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10937 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 702:
#line 2512 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10943 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 703:
#line 2513 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10949 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 704:
#line 2514 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10955 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 705:
#line 2515 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10961 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 706:
#line 2516 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10967 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 707:
#line 2517 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10973 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 708:
#line 2518 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10979 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 709:
#line 2519 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10985 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 710:
#line 2520 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 10991 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 711:
#line 2525 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 10997 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 712:
#line 2529 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11003 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 713:
#line 2530 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel(); (yyval) = (yyvsp[0]);}
#line 11009 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 714:
#line 2534 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11015 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 715:
#line 2535 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11021 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 716:
#line 2536 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11027 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 717:
#line 2537 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11034 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 718:
#line 2539 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[-1]),
                                         Parser::StaticClassExprName);}
#line 11041 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 719:
#line 2543 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11047 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 720:
#line 2552 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11053 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 721:
#line 2555 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 11059 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 722:
#line 2556 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11066 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 723:
#line 2558 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval), (yyvsp[0]),
                                         Parser::StaticClassExprName);}
#line 11073 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 724:
#line 2568 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 11079 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 725:
#line 2572 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StringName);}
#line 11085 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 726:
#line 2573 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::StaticName);}
#line 11091 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 727:
#line 2574 "hphp.y" /* yacc.c:1646  */
    { _p->onName((yyval),(yyvsp[0]),Parser::ExprName);}
#line 11097 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 728:
#line 2578 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11103 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 729:
#line 2579 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11109 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 730:
#line 2580 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11115 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 731:
#line 2584 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11121 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 732:
#line 2585 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), 0);}
#line 11127 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 733:
#line 2586 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11133 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 734:
#line 2590 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11139 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 735:
#line 2591 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11145 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 736:
#line 2595 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11151 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 737:
#line 2596 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11157 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 738:
#line 2597 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11163 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 739:
#line 2598 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[0]));}
#line 11170 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 740:
#line 2600 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LINE,     (yyvsp[0]));}
#line 11176 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 741:
#line 2601 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FILE,     (yyvsp[0]));}
#line 11182 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 742:
#line 2602 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DIR,      (yyvsp[0]));}
#line 11188 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 743:
#line 2603 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[0]));}
#line 11194 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 744:
#line 2604 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[0]));}
#line 11200 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 745:
#line 2605 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[0]));}
#line 11206 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 746:
#line 2606 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[0]));}
#line 11212 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 747:
#line 2607 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[0]));}
#line 11218 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 748:
#line 2608 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[0]));}
#line 11224 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 749:
#line 2611 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11230 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 750:
#line 2613 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11236 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 751:
#line 2617 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11242 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 752:
#line 2618 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11248 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 753:
#line 2620 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11254 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 754:
#line 2621 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11260 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 755:
#line 2623 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11266 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 756:
#line 2624 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11272 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 757:
#line 2625 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11278 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 758:
#line 2626 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11284 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 759:
#line 2627 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11290 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 760:
#line 2628 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11296 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 761:
#line 2629 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11302 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 762:
#line 2630 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11308 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 763:
#line 2631 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11314 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 764:
#line 2633 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_OR);}
#line 11320 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 765:
#line 2635 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_BOOLEAN_AND);}
#line 11326 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 766:
#line 2637 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_OR);}
#line 11332 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 767:
#line 2639 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_AND);}
#line 11338 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 768:
#line 2641 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_LOGICAL_XOR);}
#line 11344 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 769:
#line 2642 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'|');}
#line 11350 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 770:
#line 2643 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'&');}
#line 11356 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 771:
#line 2644 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'^');}
#line 11362 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 772:
#line 2645 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'.');}
#line 11368 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 773:
#line 2646 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'+');}
#line 11374 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 774:
#line 2647 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'-');}
#line 11380 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 775:
#line 2648 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'*');}
#line 11386 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 776:
#line 2649 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'/');}
#line 11392 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 777:
#line 2650 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'%');}
#line 11398 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 778:
#line 2651 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SL);}
#line 11404 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 779:
#line 2652 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SR);}
#line 11410 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 780:
#line 2653 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_POW);}
#line 11416 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 781:
#line 2654 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'!',1);}
#line 11422 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 782:
#line 2655 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'~',1);}
#line 11428 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 783:
#line 2656 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11434 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 784:
#line 2657 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11440 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 785:
#line 2659 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_IDENTICAL);}
#line 11446 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 786:
#line 2661 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_IDENTICAL);}
#line 11452 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 787:
#line 2663 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_EQUAL);}
#line 11458 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 788:
#line 2665 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_IS_NOT_EQUAL);}
#line 11464 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 789:
#line 2666 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'<');}
#line 11470 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 790:
#line 2668 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_SMALLER_OR_EQUAL);}
#line 11477 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 791:
#line 2670 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),'>');}
#line 11483 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 792:
#line 2673 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),
                                              T_IS_GREATER_OR_EQUAL);}
#line 11490 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 793:
#line 2677 "hphp.y" /* yacc.c:1646  */
    { BEXP((yyval),(yyvsp[-2]),(yyvsp[0]),T_SPACESHIP);}
#line 11496 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 794:
#line 2680 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-4]), &(yyvsp[-2]), (yyvsp[0]));}
#line 11502 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 795:
#line 2681 "hphp.y" /* yacc.c:1646  */
    { _p->onQOp((yyval), (yyvsp[-3]),   0, (yyvsp[0]));}
#line 11508 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 796:
#line 2685 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 11514 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 797:
#line 2686 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 11520 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 798:
#line 2692 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 11526 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 799:
#line 2698 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 1);}
#line 11532 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 800:
#line 2699 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11538 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 801:
#line 2703 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11544 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 802:
#line 2704 "hphp.y" /* yacc.c:1646  */
    { _p->onConstantValue((yyval), (yyvsp[0]));}
#line 11550 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 803:
#line 2705 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11556 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 804:
#line 2706 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11562 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 805:
#line 2707 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'"',(yyvsp[-1]));}
#line 11568 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 806:
#line 2708 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),'\'',(yyvsp[-1]));}
#line 11574 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 807:
#line 2710 "hphp.y" /* yacc.c:1646  */
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[-1]));}
#line 11581 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 808:
#line 2715 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11587 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 809:
#line 2716 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11593 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 810:
#line 2720 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11599 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 811:
#line 2721 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11605 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 812:
#line 2724 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval).reset();}
#line 11611 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 813:
#line 2725 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11617 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 814:
#line 2731 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11623 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 815:
#line 2733 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11629 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 816:
#line 2735 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11635 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 817:
#line 2736 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11641 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 818:
#line 2740 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[0]));}
#line 11647 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 819:
#line 2741 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[0]));}
#line 11653 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 820:
#line 2742 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[0]));}
#line 11659 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 821:
#line 2745 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[-1]));}
#line 11665 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 822:
#line 2747 "hphp.y" /* yacc.c:1646  */
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
#line 11671 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 823:
#line 2750 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[0]));}
#line 11677 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 824:
#line 2751 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[0]));}
#line 11683 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 825:
#line 2752 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[0]));}
#line 11689 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 826:
#line 2753 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 11695 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 827:
#line 2757 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[0]));}
#line 11702 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 828:
#line 2760 "hphp.y" /* yacc.c:1646  */
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[-2]) + (yyvsp[0]));}
#line 11710 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 830:
#line 2767 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11716 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 831:
#line 2768 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11722 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 832:
#line 2771 "hphp.y" /* yacc.c:1646  */
    { HPHP_PARSER_ERROR("User-defined "
                                        "constants are not allowed in "
                                        "user attribute expressions", _p);}
#line 11730 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 833:
#line 2774 "hphp.y" /* yacc.c:1646  */
    { constant_ae(_p,(yyval),(yyvsp[0]));}
#line 11736 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 834:
#line 2775 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'+',1);}
#line 11742 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 835:
#line 2776 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),'-',1);}
#line 11748 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 836:
#line 2778 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11754 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 837:
#line 2779 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11760 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 838:
#line 2781 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY); }
#line 11766 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 839:
#line 2782 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11772 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 840:
#line 2783 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11778 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 841:
#line 2784 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11784 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 842:
#line 2785 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11790 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 843:
#line 2786 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11796 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 844:
#line 2791 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 11802 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 845:
#line 2792 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 11808 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 846:
#line 2797 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11814 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 847:
#line 2798 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11820 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 848:
#line 2803 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11826 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 849:
#line 2805 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11832 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 850:
#line 2807 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 11838 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 851:
#line 2808 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11844 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 852:
#line 2812 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 11850 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 853:
#line 2813 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 11856 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 854:
#line 2818 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 11862 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 855:
#line 2819 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 11868 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 856:
#line 2824 "hphp.y" /* yacc.c:1646  */
    {  _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0); }
#line 11874 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 857:
#line 2827 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0); }
#line 11880 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 858:
#line 2832 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11886 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 859:
#line 2833 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11892 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 860:
#line 2836 "hphp.y" /* yacc.c:1646  */
    { _p->onArray((yyval),(yyvsp[-1]),T_ARRAY);}
#line 11898 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 861:
#line 2837 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
#line 11905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 862:
#line 2844 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),&(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0]));}
#line 11911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 863:
#line 2846 "hphp.y" /* yacc.c:1646  */
    { _p->onUserAttribute((yyval),  0,(yyvsp[-1]),(yyvsp[0]));}
#line 11917 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 864:
#line 2849 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);}
#line 11923 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 865:
#line 2851 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11929 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 866:
#line 2854 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 11935 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 867:
#line 2857 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11941 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 868:
#line 2858 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 11947 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 869:
#line 2862 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 0;}
#line 11953 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 870:
#line 2863 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1;}
#line 11959 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 871:
#line 2867 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 11965 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 872:
#line 2868 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropXhpAttr;}
#line 11971 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 873:
#line 2869 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval) = HPHP::ObjPropNormal;}
#line 11977 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 874:
#line 2873 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11983 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 875:
#line 2878 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = HPHP::ObjPropNormal;}
#line 11989 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 876:
#line 2883 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 11995 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 877:
#line 2884 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12001 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 878:
#line 2888 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12007 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 879:
#line 2893 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12013 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 880:
#line 2898 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12019 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 881:
#line 2899 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12025 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 882:
#line 2903 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12031 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 883:
#line 2905 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12037 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 884:
#line 2910 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12043 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 885:
#line 2912 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-2]), (yyvsp[0]));}
#line 12049 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 886:
#line 2918 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12063 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 887:
#line 2929 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12077 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 888:
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
#line 12091 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 889:
#line 2956 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-3]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12105 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 890:
#line 2968 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12111 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 891:
#line 2969 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12117 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 892:
#line 2970 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12123 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 893:
#line 2971 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12129 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 894:
#line 2972 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12135 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 895:
#line 2973 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12141 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 896:
#line 2975 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12155 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 897:
#line 2992 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12161 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 898:
#line 2994 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12167 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 899:
#line 2996 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12173 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 900:
#line 2997 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12179 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 901:
#line 3001 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12185 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 902:
#line 3002 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12191 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 903:
#line 3003 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12197 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 904:
#line 3004 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12203 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 905:
#line 3012 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12217 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 906:
#line 3021 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12223 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 907:
#line 3023 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12229 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 908:
#line 3024 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12235 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 909:
#line 3028 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12241 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 910:
#line 3033 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12247 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 911:
#line 3034 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12253 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 912:
#line 3035 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12259 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 913:
#line 3036 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12265 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 914:
#line 3037 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12271 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 915:
#line 3038 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12277 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 916:
#line 3039 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12283 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 917:
#line 3041 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12289 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 918:
#line 3043 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),NULL);}
#line 12295 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 919:
#line 3047 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12301 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 920:
#line 3051 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12307 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 921:
#line 3052 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12313 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 922:
#line 3058 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-6]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12319 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 923:
#line 3062 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectMethodCall((yyval),(yyvsp[-7]),(yyvsp[-5]).num(),(yyvsp[-4]),(yyvsp[-1]));}
#line 12325 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 924:
#line 3069 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),0,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-6]));}
#line 12331 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 925:
#line 3078 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-3]),(yyvsp[-1]),&(yyvsp[-5]));}
#line 12337 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 926:
#line 3082 "hphp.y" /* yacc.c:1646  */
    { _p->onCall((yyval),1,(yyvsp[-4]),(yyvsp[-1]),&(yyvsp[-7]));}
#line 12343 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 927:
#line 3086 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12349 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 928:
#line 3095 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12355 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 929:
#line 3096 "hphp.y" /* yacc.c:1646  */
    { _p->onRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12361 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 930:
#line 3097 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12367 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 931:
#line 3101 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12373 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 932:
#line 3102 "hphp.y" /* yacc.c:1646  */
    { _p->onPipeVariable((yyval));}
#line 12379 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 933:
#line 3103 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 0);}
#line 12385 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 934:
#line 3105 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]) = 1; _p->onIndirectRef((yyval), (yyvsp[-1]), (yyvsp[0]));}
#line 12391 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 935:
#line 3110 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12397 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 936:
#line 3111 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12403 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 937:
#line 3122 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12409 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 938:
#line 3123 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12415 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 939:
#line 3124 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12421 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 940:
#line 3127 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12435 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 941:
#line 3138 "hphp.y" /* yacc.c:1646  */
    { _p->onStaticMember((yyval),(yyvsp[-2]),(yyvsp[0]));}
#line 12441 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 942:
#line 3139 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12447 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 944:
#line 3143 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12453 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 945:
#line 3144 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);}
#line 12459 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 946:
#line 3147 "hphp.y" /* yacc.c:1646  */
    { _p->onObjectProperty(
                                        (yyval),
                                        (yyvsp[-2]),
                                        !(yyvsp[-1]).num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        (yyvsp[0])
                                      );
                                    }
#line 12473 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 947:
#line 3156 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12479 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 948:
#line 3160 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-1]),NULL);}
#line 12485 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 949:
#line 3161 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),&(yyvsp[-2]),&(yyvsp[0]));}
#line 12491 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 950:
#line 3163 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),&(yyvsp[-5]),(yyvsp[-1]));}
#line 12497 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 951:
#line 3164 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,NULL);}
#line 12503 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 952:
#line 3165 "hphp.y" /* yacc.c:1646  */
    { _p->onAListVar((yyval),NULL,&(yyvsp[0]));}
#line 12509 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 953:
#line 3166 "hphp.y" /* yacc.c:1646  */
    { _p->onAListSub((yyval),NULL,(yyvsp[-1]));}
#line 12515 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 954:
#line 3171 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12521 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 955:
#line 3172 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset();}
#line 12527 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 956:
#line 3176 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12533 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 957:
#line 3177 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12539 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 958:
#line 3178 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12545 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 959:
#line 3179 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12551 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 960:
#line 3182 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-5]),&(yyvsp[-3]),(yyvsp[0]),1);}
#line 12557 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 961:
#line 3184 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-3]),  0,(yyvsp[0]),1);}
#line 12563 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 962:
#line 3185 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-3]),(yyvsp[0]),1);}
#line 12569 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 963:
#line 3186 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),1);}
#line 12575 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 964:
#line 3191 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12581 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 965:
#line 3192 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 12587 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 966:
#line 3196 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12593 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 967:
#line 3197 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12599 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 968:
#line 3198 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12605 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 969:
#line 3199 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12611 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 970:
#line 3204 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12617 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 971:
#line 3205 "hphp.y" /* yacc.c:1646  */
    { _p->onEmptyCollection((yyval));}
#line 12623 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 972:
#line 3210 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-4]),&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12629 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 973:
#line 3212 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),&(yyvsp[-2]),  0,(yyvsp[0]),0);}
#line 12635 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 974:
#line 3214 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,&(yyvsp[-2]),(yyvsp[0]),0);}
#line 12641 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 975:
#line 3215 "hphp.y" /* yacc.c:1646  */
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[0]),0);}
#line 12647 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 976:
#line 3219 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), -1);}
#line 12653 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 977:
#line 3221 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), &(yyvsp[-1]), (yyvsp[0]), 0);}
#line 12659 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 978:
#line 3222 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[0]), -1);}
#line 12665 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 979:
#line 3224 "hphp.y" /* yacc.c:1646  */
    { _p->addEncap((yyval), NULL, (yyvsp[-1]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[0]), -1); }
#line 12672 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 980:
#line 3229 "hphp.y" /* yacc.c:1646  */
    { _p->onSimpleVariable((yyval), (yyvsp[0]));}
#line 12678 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 981:
#line 3231 "hphp.y" /* yacc.c:1646  */
    { _p->encapRefDim((yyval), (yyvsp[-3]), (yyvsp[-1]));}
#line 12684 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 982:
#line 3233 "hphp.y" /* yacc.c:1646  */
    { _p->encapObjProp(
                                           (yyval),
                                           (yyvsp[-2]),
                                           !(yyvsp[-1]).num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           (yyvsp[0])
                                         );
                                       }
#line 12698 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 983:
#line 3243 "hphp.y" /* yacc.c:1646  */
    { _p->onDynamicVariable((yyval), (yyvsp[-1]), 1);}
#line 12704 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 984:
#line 3245 "hphp.y" /* yacc.c:1646  */
    { _p->encapArray((yyval), (yyvsp[-4]), (yyvsp[-2]));}
#line 12710 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 985:
#line 3246 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 12716 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 986:
#line 3249 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_STRING;}
#line 12722 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 987:
#line 3250 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_NUM_STRING;}
#line 12728 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 988:
#line 3251 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = T_VARIABLE;}
#line 12734 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 989:
#line 3255 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_ISSET,1);}
#line 12740 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 990:
#line 3256 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EMPTY,1);}
#line 12746 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 991:
#line 3257 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12752 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 992:
#line 3258 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12758 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 993:
#line 3259 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12764 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 994:
#line 3260 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),'!',1);}
#line 12770 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 995:
#line 3261 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE,1);}
#line 12776 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 996:
#line 3262 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_INCLUDE_ONCE,1);}
#line 12782 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 997:
#line 3263 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[-1]),T_EVAL,1);}
#line 12788 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 998:
#line 3264 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE,1);}
#line 12794 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 999:
#line 3265 "hphp.y" /* yacc.c:1646  */
    { UEXP((yyval),(yyvsp[0]),T_REQUIRE_ONCE,1);}
#line 12800 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1000:
#line 3269 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), NULL, (yyvsp[0]));}
#line 12806 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1001:
#line 3270 "hphp.y" /* yacc.c:1646  */
    { _p->onExprListElem((yyval), &(yyvsp[-2]), (yyvsp[0]));}
#line 12812 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1002:
#line 3275 "hphp.y" /* yacc.c:1646  */
    { _p->onClassConst((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 12818 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1003:
#line 3277 "hphp.y" /* yacc.c:1646  */
    { _p->onClassClass((yyval), (yyvsp[-2]), (yyvsp[0]), 0);}
#line 12824 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1006:
#line 3291 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 12832 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1007:
#line 3296 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-3]).setText(_p->nsClassDecl((yyvsp[-3]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-3]), (yyvsp[-1]), &(yyvsp[-5]));
                                         _p->popTypeScope(); }
#line 12840 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1008:
#line 3300 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]));
                                         _p->popTypeScope(); }
#line 12848 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1009:
#line 3305 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-4]).setText(_p->nsClassDecl((yyvsp[-4]).text()));
                                         _p->onTypedef((yyval), (yyvsp[-4]), (yyvsp[-1]), &(yyvsp[-6]));
                                         _p->popTypeScope(); }
#line 12856 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1010:
#line 3311 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12862 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1011:
#line 3312 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 12868 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1012:
#line 3316 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12874 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1013:
#line 3317 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[0]); }
#line 12880 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1014:
#line 3323 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 12886 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1015:
#line 3327 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[-3]); }
#line 12892 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1016:
#line 3333 "hphp.y" /* yacc.c:1646  */
    { _p->pushTypeScope(); (yyval) = (yyvsp[0]); }
#line 12898 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1017:
#line 3337 "hphp.y" /* yacc.c:1646  */
    { Token t; _p->setTypeVars(t, (yyvsp[-3]));
                                         _p->pushTypeScope(); (yyval) = t; }
#line 12905 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1018:
#line 3344 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12911 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1019:
#line 3345 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12917 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1020:
#line 3349 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[0]), t);
                                         (yyval) = (yyvsp[0]); }
#line 12925 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1021:
#line 3352 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-2]), (yyvsp[0]));
                                         (yyval) = (yyvsp[-2]); }
#line 12932 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1022:
#line 3358 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12938 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1023:
#line 3363 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]); }
#line 12944 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1024:
#line 3364 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 12950 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1025:
#line 3365 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12956 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1026:
#line 3366 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12962 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1033:
#line 3387 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 12968 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1034:
#line 3388 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); (yyval) = 1; }
#line 12974 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1037:
#line 3397 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 12980 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1040:
#line 3408 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 12986 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1041:
#line 3410 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[0]).text()); }
#line 12992 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1042:
#line 3414 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 12998 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1043:
#line 3417 "hphp.y" /* yacc.c:1646  */
    { _p->addTypeVar((yyvsp[-1]).text()); }
#line 13004 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1044:
#line 3421 "hphp.y" /* yacc.c:1646  */
    {}
#line 13010 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1045:
#line 3422 "hphp.y" /* yacc.c:1646  */
    {}
#line 13016 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1046:
#line 3423 "hphp.y" /* yacc.c:1646  */
    {}
#line 13022 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1047:
#line 3429 "hphp.y" /* yacc.c:1646  */
    { validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0])); }
#line 13029 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1048:
#line 3434 "hphp.y" /* yacc.c:1646  */
    {
                                     validate_shape_keyname((yyvsp[-2]), _p);
                                     _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13039 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1049:
#line 3443 "hphp.y" /* yacc.c:1646  */
    { _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0])); }
#line 13045 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1050:
#line 3449 "hphp.y" /* yacc.c:1646  */
    {
                                     _p->onClsCnsShapeField((yyval), (yyvsp[-4]), (yyvsp[-2]), (yyvsp[0]));
                                     _p->onShapeFieldSpecialization((yyval), '?');
                                   }
#line 13054 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1051:
#line 3457 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyval), (yyvsp[0])); }
#line 13060 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1052:
#line 3458 "hphp.y" /* yacc.c:1646  */
    { }
#line 13066 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1053:
#line 3464 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-2]), true); }
#line 13072 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1054:
#line 3466 "hphp.y" /* yacc.c:1646  */
    { _p->onShape((yyval), (yyvsp[-1]), false); }
#line 13078 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1055:
#line 3467 "hphp.y" /* yacc.c:1646  */
    {
                                         Token t;
                                         t.reset();
                                         _p->onShape((yyval), t, true);
                                       }
#line 13088 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1056:
#line 3472 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         _p->onShape((yyval), t, false); }
#line 13095 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1057:
#line 3478 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);
                                        (yyval).setText("array"); }
#line 13102 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1058:
#line 3483 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13108 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1059:
#line 3488 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                        _p->onTypeAnnotation((yyval), (yyvsp[-2]), t);
                                        _p->onTypeList((yyval), (yyvsp[0])); }
#line 13116 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1060:
#line 3492 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13122 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1061:
#line 3497 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);}
#line 13128 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1062:
#line 3499 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeList((yyvsp[-3]), (yyvsp[-1])); (yyval) = (yyvsp[-3]);}
#line 13134 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1063:
#line 3505 "hphp.y" /* yacc.c:1646  */
    { _p->onTypeSpecialization((yyvsp[0]), '?');
                                         (yyval) = (yyvsp[0]); }
#line 13141 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1064:
#line 3507 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[0]), '@');
                                         (yyval) = (yyvsp[0]); }
#line 13149 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1065:
#line 3510 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13155 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1066:
#line 3511 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13163 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1067:
#line 3514 "hphp.y" /* yacc.c:1646  */
    { Token t; t.reset();
                                         (yyvsp[0]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t); }
#line 13171 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1068:
#line 3517 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13177 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1069:
#line 3520 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[-2]), (yyvsp[0]));
                                         _p->onTypeSpecialization((yyval), 'a'); }
#line 13185 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1070:
#line 3523 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[-1]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[-1]), (yyvsp[0])); }
#line 13192 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1071:
#line 3525 "hphp.y" /* yacc.c:1646  */
    { (yyvsp[0]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[0]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
#line 13201 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1072:
#line 3531 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-1]), (yyvsp[-4]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[-6]), (yyvsp[-1]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
#line 13210 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1073:
#line 3537 "hphp.y" /* yacc.c:1646  */
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[-4]), (yyvsp[-2]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[-4]));
                                        _p->onTypeSpecialization((yyval), 't'); }
#line 13220 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1074:
#line 3545 "hphp.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 13226 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;

  case 1075:
#line 3546 "hphp.y" /* yacc.c:1646  */
    { (yyval).reset(); }
#line 13232 "hphp.7.tab.cpp" /* yacc.c:1646  */
    break;


#line 13236 "hphp.7.tab.cpp" /* yacc.c:1646  */
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
#line 3549 "hphp.y" /* yacc.c:1906  */

/* !PHP5_ONLY*/
/* REMOVED */
/* !END */
/* !PHP7_ONLY*/
bool Parser::parseImpl7() {
/* !END */
  return yyparse(this) == 0;
}
